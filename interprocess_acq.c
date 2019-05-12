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

int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  int shmid;
  shared_memory_area_struct* pSharedMemory;

  shmid = shmget(SHARED_AREA_KEY, sizeof(shared_memory_area_struct), 0644 | IPC_CREAT /*| SHM_HUGETLB*/);
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

  //initialisation allocation of data from pool
  for (int i = 0; i < MAX_DATA; i++)
  {
    pSharedMemory->data_area[i].indexAcqOnGoing = i * 2;
    pSharedMemory->data_area[i].indexData = i * 2 + 1;
  }

  int time = 0;
  char charToFill = 0;
  fprintf(stdout, "Start...\n");
  fprintf(stdout, "pSharedMemory = %p\n", pSharedMemory);
  fprintf(stdout, "for 0\n");
  fprintf(stdout, "pSharedMemory->data_area[0].indexAcqOnGoing = %d\n", pSharedMemory->data_area[0].indexAcqOnGoing);
  fprintf(stdout, "pSharedMemory->data_area[0].indexAcqOnGoing = %d\n", pSharedMemory->data_area[0].indexData);

  while (1)
  {
    ///simulate an acquisition
    for (int i = 0 ; i < MAX_DATA; i++)
    {
      charToFill++;
      charToFill %= 26;
      int indexAcqOnGoing;
      indexAcqOnGoing = pSharedMemory->data_area[i].indexAcqOnGoing;

      pSharedMemory->desc[indexAcqOnGoing].timestamp = time;
      memset(pSharedMemory->desc[indexAcqOnGoing].data, 'A' + charToFill, 29);

      //move from old to new
      int temp = pSharedMemory->data_area[i].indexData;
      __atomic_store_n(&pSharedMemory->data_area[i].indexData, pSharedMemory->data_area[i].indexAcqOnGoing, __ATOMIC_SEQ_CST);
      //pSharedMemory->data_area[i].indexData= pSharedMemory->data_area[i].indexAcqOnGoing;
      pSharedMemory->data_area[i].indexAcqOnGoing = temp;
    }

    nanosleep(&waitTime, NULL);
    time++;
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

