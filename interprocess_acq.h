#ifndef INTERPRCESS_ACQ_H
#define INTERPRCESS_ACQ_H

#include <stdint.h>

#define SHARED_AREA_KEY     0x10000001  // il semble que les premiers bit de l'ID serve --> pour 1: depasse pas 4ko d'allocation
//#define SHARED_AREA_KEY     IPC_PRIVATE

#define MAX_DATA     (200000)

#define UNUSED(x) ((void)x)

typedef enum { FALSE, TRUE } BOOL;

typedef struct
{
  char data[30];
  uint32_t time;
} basic_data;

typedef struct
{
  BOOL flag[2];
  int turn;
  basic_data data_area[MAX_DATA];
} shared_memory_area_struct;




#endif /* INTERPRCESS_ACQ_H */


