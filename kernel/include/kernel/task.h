#ifndef TASK_H
#define TASK_H


#include "kernel/fs.h"
#include "inttypes.h"


#define MAX_TASK_COUNT       10
#define MAX_CHANNEL_COUNT    10
#define MAX_CONNECTION_COUNT 10
#define MAX_MESSAGE_COUNT    10


// -----------------------------------------------------------------------------
enum ETaskState
{
  TS_UNINITIALIZED,
  TS_RUNNING,
  TS_READY,
  TS_SEND_BLOCKED,
  TS_RECEIVE_BLOCKED,
  TS_REPLY_BLOCKED
};

// -----------------------------------------------------------------------------
struct SMessage
{
  bool bUsed;
  const void * pSndMsg;
  int iSndSize;
  void * pRcvMsg;
  int iRcvSize;
  int iRetVal;
};

// -----------------------------------------------------------------------------
struct SChannel
{
  SMessage msg[MAX_MESSAGE_COUNT];
};

// -----------------------------------------------------------------------------
struct SConnection
{
  SChannel * channel;
};

// -----------------------------------------------------------------------------
class CTask
{
public:
  virtual ~CTask();

  // Switch tasks! Function can do two things:
  //  - Setup stack so interrupt return will couse this task to run.
  //  - Jump to task immediately.
  virtual void run() = 0;

  // Message bus management
  SChannel    * pChannel_[MAX_CHANNEL_COUNT];
  SConnection * pConnection_[MAX_CONNECTION_COUNT];

  // Task state
  ETaskState    eState_;

  // Linked list
  CTask       * prev;
  CTask       * next;

protected:
  CTask();
};

// -----------------------------------------------------------------------------
class CTaskManager
{
public:
  static void addTask(CTask * pTask);
  static void removeTask(CTask * pTask);
  static bool schedule();

  static uint32_t   iTaskCount_;
  static CTask    * pCurrentTask_;
  static CTask    * taskTable_[MAX_TASK_COUNT];

private:
  CTaskManager(){}
};


// -----------------------------------------------------------------------------
// To be implemeted in arch
extern CTask * getNewTask(void * entry, size_t stack, size_t svcstack, int argc = 0, char * argv[] = 0);


#endif
