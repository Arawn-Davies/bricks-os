#include "kernel/debug.h"
#include "kernel/srr_k.h"
#include "kernel/srrChannel_k.h"
#include "kernel/task.h"
#include "kernel/pthread_k.h"
#include "kernel/syscall.h"
#include "asm/cpu.h"
#include "asm/arch/config.h"
#include "string.h"


//---------------------------------------------------------------------------
int
k_msgSend(int iConnectionID, const void * pSndMsg, int iSndSize, void * pRcvMsg, int iRcvSize)
{
  int iRetVal(-1);

#ifndef CONFIG_DIRECT_ACCESS_KERNEL_FUNC
  if(iConnectionID <= 0)
  {
    // Invalid ID
    printk("k_msgSend: Invalid connection id: %d\n", iConnectionID);
  }
  else if(iConnectionID == scKERNEL)
  {
    // Kernel ID
    SKernelMessageHeader * pHeader = (SKernelMessageHeader *)pSndMsg;

    if((pHeader->iHeaderSize == sizeof(SKernelMessageHeader)) &&
       (pHeader->iVersion == INTERFACE_VERSION(1, 0)))
    {
      switch(pHeader->iFunctionID)
      {
        unwrapfunc1r(channelCreate,          pSndMsg, unsigned, iFlags);
        unwrapfunc1r(channelDestroy,         pSndMsg, int, iChannelID);
        unwrapfunc4r(channelConnectAttach,   pSndMsg, uint32_t, iNodeID, pid_t, iProcessID, int, iChannelID, int, iFlags);
        unwrapfunc1r(channelConnectDetach,   pSndMsg, int, iConnectionID);

        unwrapfunc4r(pthread_create,         pSndMsg, pthread_t *, thread, const pthread_attr_t *, attr, pthread_func_entry, start_routine, void *, arg);
        unwrapfunc1 (pthread_exit,           pSndMsg, void *, status);
        unwrapfunc2r(pthread_cond_init,      pSndMsg, pthread_cond_t *, cond, const pthread_condattr_t *, attr);
        unwrapfunc1r(pthread_cond_destroy,   pSndMsg, pthread_cond_t *, cond);
        unwrapfunc2r(pthread_cond_wait,      pSndMsg, pthread_cond_t *, cond, pthread_mutex_t *, mutex);
        unwrapfunc3r(pthread_cond_timedwait, pSndMsg, pthread_cond_t *, cond, pthread_mutex_t *, mutex, const struct timespec *, ts);
        unwrapfunc1r(pthread_cond_signal,    pSndMsg, pthread_cond_t *, cond);
        unwrapfunc1r(pthread_cond_broadcast, pSndMsg, pthread_cond_t *, cond);
        unwrapfunc2r(pthread_mutex_init,     pSndMsg, pthread_mutex_t *, mutex, const pthread_mutexattr_t *, attr);
        unwrapfunc1r(pthread_mutex_destroy,  pSndMsg, pthread_mutex_t *, mutex);
        unwrapfunc1r(pthread_mutex_lock,     pSndMsg, pthread_mutex_t *, mutex);
        unwrapfunc1r(pthread_mutex_trylock,  pSndMsg, pthread_mutex_t *, mutex);
        unwrapfunc1r(pthread_mutex_unlock,   pSndMsg, pthread_mutex_t *, mutex);

        //unwrapfunc1r(brk,                    pSndMsg, void *, addr);
        //unwrapfunc1r(close,                  pSndMsg, int, iFD);
        //unwrapfunc1 (_exit,                  pSndMsg, int, iStatus);
        unwrapfunc0r(getpid);
        //unwrapfunc3r(read,                   pSndMsg, int, iFD, void *, pBuf, size_t, size);
        //unwrapfunc1r(sbrk,                   pSndMsg, intptr_t, increment);
        unwrapfunc1r(sleep,                  pSndMsg, unsigned int, iSeconds);
        unwrapfunc1r(usleep,                 pSndMsg, useconds_t, useconds);
        //unwrapfunc3r(write,                  pSndMsg, int, iFD, const void *, pBuf, size_t, size);

        default:
          printk("k_msgSend: Invalid function id: %d\n", pHeader->iFunctionID);
      };
    }
    else
    {
      printk("k_msgSend: Invalid kernel header(0x%x, %d, %d, 0x%x)\n", pHeader, pHeader->iHeaderSize, iSndSize, pHeader->iVersion);
    }
  }
  else if(iConnectionID < CONNECTION_ID_BASE)
  {
    // Kernel Drivers ID
    printk("k_msgSend: Invalid connection id: %d\n", iConnectionID);
  }
  else
#endif // CONFIG_DIRECT_ACCESS_KERNEL_FUNC
  {
    // User ID
    int iConnectionIDX(CONNECTION_ID_TO_IDX(iConnectionID));

    if((iConnectionIDX >= 0) &&
       (iConnectionIDX < MAX_CONNECTION_COUNT) &&
       (CTaskManager::pCurrentTask_->pConnection_[iConnectionIDX] != NULL))
    {
      SChannel * pChannel = CTaskManager::pCurrentTask_->pConnection_[iConnectionIDX];
      // Wait for channel to become free
      k_pthread_mutex_lock(&pChannel->mutex);
      while(pChannel->iState != CS_FREE)
        k_pthread_cond_wait(&pChannel->stateCond, &pChannel->mutex);
      // Fill channel
      pChannel->pSndMsg  = pSndMsg;
      pChannel->iSndSize = iSndSize;
      pChannel->pRcvMsg  = pRcvMsg;
      pChannel->iRcvSize = iRcvSize;
      pChannel->iState   = CS_MSG_SEND;
      // Notify others of changed channel state
      k_pthread_cond_broadcast(&pChannel->stateCond);
      // Wait for message to be replied
      do
        k_pthread_cond_wait(&pChannel->stateCond, &pChannel->mutex);
      while(pChannel->iState != CS_MSG_REPLIED);
      iRetVal = pChannel->iRetVal;
      pChannel->iState   = CS_FREE;
      // Notify others of changed channel state
      k_pthread_cond_broadcast(&pChannel->stateCond);
      // Unlock channel
      k_pthread_mutex_unlock(&pChannel->mutex);
    }
    else
    {
      printk("k_msgSend: Invalid connection id: %d(%d)\n", iConnectionID, iConnectionIDX);
    }
  }

  return iRetVal;
}

//---------------------------------------------------------------------------
int
k_msgReceive(int iChannelID, void * pRcvMsg, int iRcvSize)
{
  int iRetVal(-1);
  int iChannelIDX(CHANNEL_ID_TO_IDX(iChannelID));

  if((iChannelIDX >= 0) &&
     (iChannelIDX < MAX_CHANNEL_COUNT) &&
     (CTaskManager::pCurrentTask_->pChannel_[iChannelIDX] != NULL))
  {
    SChannel * pChannel = CTaskManager::pCurrentTask_->pChannel_[iChannelIDX];
    // Wait for message to be sent
    k_pthread_mutex_lock(&pChannel->mutex);
    while(pChannel->iState != CS_MSG_SEND)
      k_pthread_cond_wait(&pChannel->stateCond, &pChannel->mutex);
    if(pChannel->iSndSize < iRcvSize)
      iRcvSize = pChannel->iSndSize;
    memcpy(pRcvMsg, pChannel->pSndMsg, iRcvSize);
    pChannel->iState = CS_MSG_RECEIVED;
    // Notify others of changed channel state
    k_pthread_cond_broadcast(&pChannel->stateCond);
    // Unlock channel
    k_pthread_mutex_unlock(&pChannel->mutex);

    iRetVal = iChannelID;
  }
  else
  {
    printk("k_msgReceive: Invalid channel id: %d(%d)\n", iChannelID, iChannelIDX);
  }

  return iRetVal;
}

//---------------------------------------------------------------------------
int
k_msgReply(int iReceiveID, int iStatus, const void * pReplyMsg, int iReplySize)
{
  int iRetVal(-1);
  int iReceiveIDX(CHANNEL_ID_TO_IDX(iReceiveID));

  if((iReceiveIDX >= 0) &&
     (iReceiveIDX < MAX_CHANNEL_COUNT) &&
     (CTaskManager::pCurrentTask_->pChannel_[iReceiveIDX] != NULL))
  {
    SChannel * pChannel = CTaskManager::pCurrentTask_->pChannel_[iReceiveIDX];

    // We can only reply a received message
    k_pthread_mutex_lock(&pChannel->mutex);
    if(pChannel->iState == CS_MSG_RECEIVED)
    {
      // Copy reply data
      if(iReplySize < pChannel->iRcvSize)
        iReplySize = pChannel->iRcvSize;
      memcpy(pChannel->pRcvMsg, pReplyMsg, iReplySize);
      pChannel->iRetVal = iStatus;
      pChannel->iState = CS_MSG_REPLIED;
      // Notify others of changed channel state
      k_pthread_cond_broadcast(&pChannel->stateCond);
      // Unlock channel
      k_pthread_mutex_unlock(&pChannel->mutex);
      iRetVal = 0;
    }
    else
    {
      k_pthread_mutex_unlock(&pChannel->mutex);
      printk("k_msgReply: No message received, can't reply\n");
    }
  }
  else
  {
    printk("k_msgReply: Invalid receive id: %d(%d)\n", iReceiveID, iReceiveIDX);
  }

  return iRetVal;
}
