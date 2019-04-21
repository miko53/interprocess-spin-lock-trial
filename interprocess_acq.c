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
  .tv_nsec = 15625
};

static void enter_critical_section(shared_memory_area_struct* pSharedMemory)
{
  pSharedMemory->flag[1] = TRUE;
  pSharedMemory->turn = 0;

  __sync_synchronize();
  while ((pSharedMemory->flag[0] == TRUE) && (pSharedMemory->turn == 0))
  {
    // active loop
  }
}


static void leave_critical_section(shared_memory_area_struct* pSharedMemory)
{
  pSharedMemory->flag[1] = FALSE;
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

