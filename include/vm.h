#ifndef MLC_VM_H
#define MLC_VM_H

#include <stdio.h>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
  Chunk *chunk;
  uint8_t *instrPtr;
  Value stack[STACK_MAX];
  Value *stackTop;
} VM;

typedef enum {
  I_OK,
  I_COMPILE_ERR,
  I_RUNTIME_ERR,
} IR;

void initVM(VM *);
void deleteVM(VM *);
IR interpret(VM *, const char *source);
void push(VM *, Value);
Value pop(VM *);
static IR run(VM *);
static void initStack(VM *);

#endif