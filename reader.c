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

int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  int shmid;
  shared_memory_area_struct* pSharedMemory;

  shmid = shmget(SHARED_AREA_KEY, sizeof(shared_memory_area_struct), 0644 /*| SHM_HUGETLB */ /*| IPC_CREAT*/);
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
  fprintf(stdout, "pSharedMemory = %p\n", pSharedMemory);

  while (1)
  {
    int i;
    int j;
    char toTest;
    for (i = 0; i < MAX_DATA; i++)
    {
      /* if (i ==0)
       {
         fprintf(stdout,"pSharedMemory->data_area[0].indexAcqOnGoing = %d\n", pSharedMemory->data_area[0].indexAcqOnGoing);
         fprintf(stdout,"pSharedMemory->data_area[0].indexAcqOnGoing = %d\n", pSharedMemory->data_area[0].indexData);
       }*/

      //int indexData = pSharedMemory->data_area[i].indexData;
      int indexData = __atomic_load_n(&pSharedMemory->data_area[i].indexData, __ATOMIC_SEQ_CST);
      descValue* pData = &pSharedMemory->desc[indexData];
      toTest = pData->data[0];
      strcpy(stringRead, pData->data);
      stringRead[29] = '\0';

      //check if all is coherent
      for (j = 0; j < 29; j++)
      {
        if (stringRead[j] != toTest)
        {
          fprintf(stdout, "Pb at %d (%c != %c)\n", j, stringRead[j], toTest);
          fprintf(stdout, "Incoherence ==> data[%i] = %s (%ul)\n", i, stringRead, pData->timestamp);
          exit(EXIT_FAILURE);
          break;
        }
      }

      if (i == 0)
      {
        fprintf(stdout, "data is %s ts = %d\n", stringRead, pData->timestamp);
      }

    }

    nanosleep(&waitTime, NULL);
    //fprintf(stdout, "Busy Wait : %ld s, %ld ns\n", end.tv_sec - begin.tv_sec, end.tv_nsec - begin.tv_nsec);
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


