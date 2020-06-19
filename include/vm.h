#ifndef MLC_VM_H
#define MLC_VM_H

#include <stdio.h>

#include "chunk.h"
#include "debug.h"

typedef struct {
  Chunk *chunk;
  uint8_t *instrPtr;
} VM;

typedef enum {
  I_OK,
  I_COMPILE_ERR,
  I_RUNTIME_ERR,
} IR;

VM vm;

void initVM();
void deleteVM();
IR interpret(Chunk *);
static IR run();

#endif