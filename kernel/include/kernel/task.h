#ifndef TASK_H
#define TASK_H


#include "asm/arch/config.h"
#include "kernel/syscall.h"
#include "kernel/fs.h"
#include "kernel/queue.h"
#include "kernel/pthread_k.h"
#include "kernel/srrChannel.h"
#include "kernel/srrConnection.h"
#ifdef CONFIG_MMU
#include "asm/aspace.h"
#endif
#include "asm/task.h"
#include "inttypes.h"


#define MAX_CHANNEL_COUNT         10
#define MAX_OUT_CONNECTION_COUNT  10
#define RCVID_TO_COIDX(rcvid) (rcvid)
#define COIDX_TO_RCVID(coidx) (coidx)


// -----------------------------------------------------------------------------
enum EThreadState
{
  TS_UNINITIALIZED,
  TS_READY,
  TS_RUNNING,
  TS_WAITING,
  TS_DEAD,
  TS_DESTROY,
  TS_SEND_BLOCKED,
  TS_RECEIVE_BLOCKED,
  TS_REPLY_BLOCKED
};

class CTask;
class CThread;
TAILQ_HEAD(STaskQueue, CTask);
TAILQ_HEAD(SThreadQueue, CThread);
// -----------------------------------------------------------------------------
class CThread
{
public:
  CThread(CTask * task);
  ~CThread();

  void state(EThreadState state);

  CThread * createChild(void * entry, int argc = 0, char * argv[] = 0);

  inline CThreadImpl & impl(){return impl_;}

public:
  uint32_t iTimeout_;                                // Timeout in us
  void * pWaitObj_;                                  // Object we're waiting on
  int iWaitReturn_;                                  // Return value when waiting

  TAILQ_ENTRY(CThread) thread_qe;                    // All threads queue entry
  TAILQ_ENTRY(CThread) state_qe;                     // Thread state queue entry
  TAILQ_ENTRY(CThread) wait_qe;                      // Thread wait queue entry

  // Tree structure of threads
  CTask * pTask_;
  CThread * pParent_;
  SThreadQueue children_queue;                       // Threads children queue
  TAILQ_ENTRY(CThread) children_qe;                  // Threads children queue entry

private:
  CThreadImpl impl_;

  EThreadState eState_;
};

// -----------------------------------------------------------------------------
class CTask
{
public:
  CTask();
  ~CTask();

  int msgSend(int iConnectionID, const void * pSndMsg, int iSndSize, void * pRcvMsg, int iRcvSize);
  int msgReceive(int iChannelID, void * pRcvMsg, int iRcvSize);
  int msgReply(int iReceiveID, int iStatus, const void * pReplyMsg, int iReplySize);

  int addInConnection(IConnection * connection);
  int addOutConnection(IConnection * connection);

  int channelCreate(unsigned iFlags);
  int channelDestroy(int iChannelID);
  int channelConnectAttach(uint32_t iNodeID, pid_t iProcessID, int iChannelID, int iFlags);
  int channelConnectDetach(int iConnectionID);

#ifdef CONFIG_MMU
  // Address Space
  inline CAddressSpace & aspace(){return cASpace_;}
#endif

  // Main thread of the task
  inline CThread & thread(){return thr_;}

  pid_t iPID_;

  TAILQ_ENTRY(CTask) task_qe;                          // All tasks queue

private:
  int channelConnectAttach(pid_t iProcessID, int iChannelID, int iFlags);

private:
  // Tasks channels (CTask has ownership)
  // NOTE: We are using an array here becouse it is FAST, and we need the
  //       msgReceive function to locate channels FAST.
  IChannel * pChannel_[MAX_CHANNEL_COUNT];

  // Tasks connections (to other tasks channel) (CTask has ownership)
  // NOTE: We are using an array here becouse it is FAST, and we need the
  //       msgSend function to locate connections FAST.
  IConnection * pConnectionsOut_[MAX_OUT_CONNECTION_COUNT];

  // Tasks connections (to this tasks channels) (CTask::pConnectionsOut_ has ownership)
  // NOTE: We are using an array here becouse it is FAST, and we need the
  //       msgSend function to locate connections FAST.
  IConnection * pConnectionsIn_[MAX_IN_CONNECTION_COUNT];

#ifdef CONFIG_MMU
  // Addess space
  CAddressSpace cASpace_;
#endif
  CThread thr_;
};

// -----------------------------------------------------------------------------
class CTaskManager
{
public:
  static void updateSleepers();
  static void removeDestroyed();
  static bool schedule();
  static CTask * getTaskFromPID(pid_t pid);

  static CTask * pCurrentTask_;
  static CThread * pCurrentThread_;
  static CThread * pIdleThread_;
  static STaskQueue task_queue;        // All tasks
  static SThreadQueue thread_queue;    // All threads
  static SThreadQueue ready_queue;     // Ready to run threads
  static SThreadQueue timer_queue;     // Sleeping on timer threads
  static SThreadQueue wait_queue;      // Sleeping on objects
  static SThreadQueue dead_queue;      // Ready to be destroyed
  static uint32_t iPIDCount_;
  static useconds_t iCurrentTime_;

private:
  CTaskManager(){}
};


#endif
