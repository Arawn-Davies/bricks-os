#include "kernel/bricks.h"
#include "kernel/debug.h"
#include "asm/cpu.h"
#include "pthread.h"


extern int srrTest(int argc, char * argv[]);


#ifdef CONFIG_MULTITASKING
// -----------------------------------------------------------------------------
void *
thread(void * arg)
{
  printk("Threads...OK\n");
  ::halt();

  return 0;
}
#endif // CONFIG_MULTITASKING

// -----------------------------------------------------------------------------
int
bricks_main()
{
  // Welcome user :-)
  printk("Bricks-OS\n");
  printk("=========\n");

#ifdef CONFIG_MULTITASKING
  // Create thread
  pthread_t thr;
  if(pthread_create(&thr, 0, thread, 0) != 0)
    printk("ERROR: Unable to create thread!\n");
#endif // CONFIG_MULTITASKING

  // Enable interrupts
  printk("Interrupts...\n");
  local_irq_enable();
  printk("Interrupts...OK\n");

  // Execute application
  srrTest(0, 0);

  // Halt current thread
  // FIXME: Forever consuming CPU time now!
  while(true)
  {
    ::halt();
  }

  return 0;
}
