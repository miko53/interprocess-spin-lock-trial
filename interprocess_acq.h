#ifndef INTERPRCESS_ACQ_H
#define INTERPRCESS_ACQ_H

#include <stdint.h>

//#define PETERSON_ALGO
#define PETERSON_ALGO_N_PROCESS
#define NB_MAX_PROCESS            (4)

#define SHARED_AREA_KEY     0x10000000
//#define SHARED_AREA_KEY     IPC_PRIVATE

#define MAX_DATA     (200000)

#define ACQ_PROCESS_ID      (0)

#define UNUSED(x) ((void)x)

typedef enum { FALSE, TRUE } BOOL;

typedef struct
{
  char data[30];
  uint32_t time;
} basic_data;

typedef struct
{
#ifdef PETERSON_ALGO
  BOOL flag[2];
  int turn;
#endif /* PETERSON_ALGO */
#ifdef PETERSON_ALGO_N_PROCESS
  int level[NB_MAX_PROCESS];
  int last_to_enter[NB_MAX_PROCESS];
#endif /* PETERSON_ALGO_N_PROCESS*/
  basic_data data_area[MAX_DATA];
} shared_memory_area_struct;




#endif /* INTERPRCESS_ACQ_H */


