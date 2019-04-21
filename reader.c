#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "interprocess_acq.h"
#include <string.h>
#include <assert.h>

struct timespec waitTime =
{
  .tv_sec = 0,
  .tv_nsec = 750000000
};

struct timespec begin;
struct timespec end;

int process_id;


static void enter_critical_section(shared_memory_area_struct* pSharedMemory)
{
#ifdef PETERSON_ALGO
  pSharedMemory->flag[0] = TRUE;
  pSharedMemory->turn = 1;

  clock_gettime(CLOCK_REALTIME, &begin);

  __sync_synchronize();
  while ((pSharedMemory->flag[1] == TRUE) && (pSharedMemory->turn == 1))
  {
    // active loop
  }
  clock_gettime(CLOCK_REALTIME, &end);
#endif /* PETERSON_ALGO */
#ifdef PETERSON_ALGO_N_PROCESS
  clock_gettime(CLOCK_REALTIME, &begin);
  for (int i = 0 ; i < NB_MAX_PROCESS; i++)
  {
    pSharedMemory->level[process_id] = i;
    pSharedMemory->last_to_enter[i] = process_id;

    for (int j = 0; j < NB_MAX_PROCESS; j++)
    {
      if (j != process_id)
      {
        __sync_synchronize();
        while ((pSharedMemory->last_to_enter[i] == process_id) &&
               (pSharedMemory->level[j] >= pSharedMemory->level[process_id]))
          ;
      }
    }
  }
  clock_gettime(CLOCK_REALTIME, &end);
#endif /* PETERSON_ALGO_N_PROCESS */
}


static void leave_critical_section(shared_memory_area_struct* pSharedMemory)
{
#ifdef PETERSON_ALGO
  pSharedMemory->flag[0] = FALSE;
#endif /* PETERSON_ALGO */
#ifdef PETERSON_ALGO_N_PROCESS
  pSharedMemory->level[process_id] = -1;
#endif /* PETERSON_ALGO_N_PROCESS */
}



int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  int shmid;
  shared_memory_area_struct* pSharedMemory;

#ifdef PETERSON_ALGO_N_PROCESS
  if (argc != 2)
  {
    fprintf(stdout, "Error need argument\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    process_id = atoi(argv[1]);
    if ((process_id <= 0) || (process_id > NB_MAX_PROCESS))
    {
      fprintf(stdout, "wrong process ID\n");
      exit(EXIT_FAILURE);
    }
  }
#endif /* PETERSON_ALGO_N_PROCESS */

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

  char stringRead[30];
  stringRead[29] = '\0';

  while (1)
  {
    int i;
    int j;
    char toTest;
    for (i = 0; i < MAX_DATA; i++)
    {
      //BEGIN critical section
      enter_critical_section(pSharedMemory);
      toTest = pSharedMemory->data_area[i].data[0];
      strcpy(stringRead, pSharedMemory->data_area[i].data);
      stringRead[29] = '\0';
      //END critical section
      leave_critical_section(pSharedMemory);

      //check if all is coherent
      for (j = 0; j < 29; j++)
      {
        if (stringRead[j] != toTest)
        {
          fprintf(stdout, "Pb at %d (%c != %c)\n", j, stringRead[j], toTest);
          fprintf(stdout, "Incoherence ==> data[%i] = %s (%ul)\n", i, stringRead, pSharedMemory->data_area[i].time);
          exit(EXIT_FAILURE);
          break;
        }
      }

    }

    nanosleep(&waitTime, NULL);
    fprintf(stdout, "Busy Wait : %ld s, %ld ns\n", end.tv_sec - begin.tv_sec, end.tv_nsec - begin.tv_nsec);
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


