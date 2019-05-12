#ifndef INTERPRCESS_ACQ_H
#define INTERPRCESS_ACQ_H

#include <stdint.h>

#define SHARED_AREA_KEY     0x20000000
//#define SHARED_AREA_KEY     IPC_PRIVATE

#define MAX_DATA     (200000)

#define UNUSED(x) ((void)x)

typedef enum { FALSE, TRUE } BOOL;

typedef struct
{
  char data[30];
  uint32_t timestamp;
  uint32_t validity;
} descValue;


typedef struct
{
  uint32_t indexData;
  uint32_t indexAcqOnGoing;
} basic_data;

typedef struct
{
  basic_data data_area[MAX_DATA];
  descValue desc[MAX_DATA * 2];
} shared_memory_area_struct;

#endif /* INTERPRCESS_ACQ_H */


