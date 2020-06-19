#ifndef MLC_VAL_H
#define MLC_VAL_H

#include <stdio.h>

#include "common.h"
#include "memory.h"

typedef double Value;

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValArr;

void initVal(ValArr *);
void writeVal(ValArr *, Value);
void deleteVal(ValArr *);
void printVal(Value val);

#endif