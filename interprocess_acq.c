#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include "interprocess_acq.h"

struct timespec waitTime =
{
  .tv_sec = 0,
  .tv_nsec = 1562500
};

struct timespec begin;
struct timespec end;


static void enter_critical_section(shared_memory_area_struct* pSharedMemory)
{
#ifdef PETERSON_ALGO
  pSharedMemory->flag[1] = TRUE;
  pSharedMemory->turn = 0;

  clock_gettime(CLOCK_REALTIME, &begin);

  __sync_synchronize();
  while ((pSharedMemory->flag[0] == TRUE) && (pSharedMemory->turn == 0))
  {
    // active loop
  }
  clock_gettime(CLOCK_REALTIME, &end);
#endif /* PETERSON_ALGO */
#ifdef PETERSON_ALGO_N_PROCESS
  clock_gettime(CLOCK_REALTIME, &begin);
  for (int i = 0 ; i < NB_MAX_PROCESS; i++)
  {
    pSharedMemory->level[ACQ_PROCESS_ID] = i;
    pSharedMemory->last_to_enter[i] = ACQ_PROCESS_ID;

    for (int j = 0; j < NB_MAX_PROCESS; j++)
    {
      if (j != ACQ_PROCESS_ID)
      {
        __sync_synchronize();
        while ((pSharedMemory->last_to_enter[i] == ACQ_PROCESS_ID) &&
               (pSharedMemory->level[j] >= pSharedMemory->level[ACQ_PROCESS_ID]))
          ;
      }
    }
  }
  clock_gettime(CLOCK_REALTIME, &end);
#endif /* PETERSON_ALGO_N_PROCESS*/
#ifdef WITH_SPIN_LOCK
  clock_gettime(CLOCK_REALTIME, &begin);
  while (__sync_bool_compare_and_swap(&pSharedMemory->lock, FALSE, TRUE) == FALSE)
  {
    //active loop
    ;
  }
  clock_gettime(CLOCK_REALTIME, &end);
#endif /* WITH_SPIN_LOCK */
#ifdef WITH_SPIN_LOCK_WITH_PROCESS_ID
  clock_gettime(CLOCK_REALTIME, &begin);
  while (__sync_val_compare_and_swap(&pSharedMemory->lock, SPIN_LOCK_NO_LOCKED, ACQ_PROCESS_ID) != SPIN_LOCK_NO_LOCKED)
  {
    //active loop
    ;
  }
  clock_gettime(CLOCK_REALTIME, &end);
#endif /* WITH_SPIN_LOCK_WITH_PROCESS_ID */
}


static void leave_critical_section(shared_memory_area_struct* pSharedMemory)
{
#ifdef PETERSON_ALGO
  pSharedMemory->flag[1] = FALSE;
#endif /* PETERSON_ALGO */
#ifdef PETERSON_ALGO_N_PROCESS
  pSharedMemory->level[ACQ_PROCESS_ID] = -1;
#endif /* PETERSON_ALGO_N_PROCESS */
#ifdef WITH_SPIN_LOCK
  __sync_synchronize();
  pSharedMemory->lock = 0;
#endif /* WITH_SPIN_LOCK */
#ifdef WITH_SPIN_LOCK_WITH_PROCESS_ID
  __sync_synchronize();
  pSharedMemory->lock = SPIN_LOCK_NO_LOCKED;
#endif /* WITH_SPIN_LOCK_WITH_PROCESS_ID */
}


int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  int shmid;
  shared_memory_area_struct* pSharedMemory;

  shmid = shmget(SHARED_AREA_KEY, sizeof(shared_memory_area_struct), 0644 | IPC_CREAT);

  if (shmid == -1)
  {
    fprintf(stdout, "errno = %d\n", errno);
    perror("Shared memory");
    return EXIT_FAILURE;
  }

  // Attach to the memory
  pSharedMemory = shmat(shmid, NULL, 0);
  if (pSharedMemory == (void*) - 1)
  {
    fprintf(stdout, "errno = %d\n", errno);
    perror("Shared memory attach");
    return EXIT_FAILURE;
  }

#ifdef PETERSON_ALGO_N_PROCESS
  //initialisation
  for (int i = 0; i < NB_MAX_PROCESS; i++)
  {
    pSharedMemory->level[i] = -1;
  }
#endif /* PETERSON_ALGO_N_PROCESS */
#ifdef WITH_SPIN_LOCK
  //initialize the spinlock
  __sync_synchronize();
  pSharedMemory->lock = 0;
#endif /* WITH_SPIN_LOCK */
#ifdef WITH_SPIN_LOCK_WITH_PROCESS_ID
  __sync_synchronize();
  pSharedMemory->lock = SPIN_LOCK_NO_LOCKED;
#endif /* WITH_SPIN_LOCK_WITH_PROCESS_ID */

  int time = 0;
  char charToFill = 0;

  while (1)
  {
    ///simulate an acquisition
    for (int i = 0 ; i < MAX_DATA; i++)
    {
      //BEGIN critical section
      enter_critical_section(pSharedMemory);
      pSharedMemory->data_area[i].time = time++;
      charToFill++;
      charToFill %= 26;
      memset(pSharedMemory->data_area[i].data, 'A' + charToFill, 29);
      pSharedMemory->data_area[i].data[29] = '\0';
      //END critical section
      leave_critical_section(pSharedMemory);
    }

    nanosleep(&waitTime, NULL);

    if (time % 2048 == 0)
    {
      fprintf(stdout, "time = %d\n", time);
      fprintf(stdout, "Busy Wait : %ld s, %ld ns\n", end.tv_sec - begin.tv_sec, end.tv_nsec - begin.tv_nsec);
#ifdef WITH_SPIN_LOCK_WITH_PROCESS_ID
      fprintf(stdout, "current process ID = %d\n", pSharedMemory->lock);
#endif /* WITH_SPIN_LOCK_WITH_PROCESS_ID */
    }

  }


  if (shmdt(pSharedMemory) == -1)
  {
    fprintf(stdout, "errno = %d\n", errno);
    perror("shmdt");
    return 1;
  }

  fprintf(stdout, "Done\n");

  return EXIT_SUCCESS;
}

