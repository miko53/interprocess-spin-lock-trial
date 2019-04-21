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

struct timespec waitTime = {
  .tv_sec = 0,
  .tv_nsec = 750000000
};

static void enter_critical_section(shared_memory_area_struct* pSharedMemory)
{
  pSharedMemory->flag[0] = TRUE;
  pSharedMemory->turn = 1;
  
  __sync_synchronize();
  while ((pSharedMemory->flag[1] == TRUE) && (pSharedMemory->turn == 1))
  {
    // active loop
  }
}


static void leave_critical_section(shared_memory_area_struct* pSharedMemory)
{
  pSharedMemory->flag[0] = FALSE;
}



int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);
  
  int shmid;
  shared_memory_area_struct* pSharedMemory;
  
  shmid = shmget(SHARED_AREA_KEY, sizeof(shared_memory_area_struct), 0644|IPC_CREAT);

  if (shmid == -1) 
  {
     fprintf(stdout, "errno = %d\n", errno);
      perror("Shared memory");
      return EXIT_FAILURE;
   }
  
   // Attach to the memory
   pSharedMemory = shmat(shmid, NULL, 0);
   if (pSharedMemory == (void *) -1) 
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
     for(i= 0; i < MAX_DATA; i++)
     {
       //BEGIN critical section 
       enter_critical_section(pSharedMemory);
       toTest = pSharedMemory->data_area[i].data[0];
       strcpy(stringRead, pSharedMemory->data_area[i].data);
       stringRead[29] = '\0';
       //END critical section
       leave_critical_section(pSharedMemory);
       
       //check if all is coherent
       for(j = 0; j < 29; j++)
       {
         if (stringRead[j] != toTest)
         {
            fprintf(stdout, "Pb at %d (%c != %c)\n", j, stringRead[j], toTest);
            fprintf(stdout, "Incoherence ==> data[%i] = %s (%ul)\n", i, stringRead, pSharedMemory->data_area[i].time);
            break;
         }
       }
       
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
 
 
