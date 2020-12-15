#ifndef MLC_VM_H
#define MLC_VM_H

#include <math.h>
#include <stdarg.h>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "hashtable.h"

#define STACK_MAX 256

typedef struct {
  Chunk *chunk;
  uint8_t *instrPtr;
  Value stack[STACK_MAX];
  Value switchVal[STACK_MAX];
  bool caseVal[STACK_MAX];
  bool *caseValTop;
  Value *switchValTop;
  Value *stackTop;
  Object *objects;
  HashTable strings;
  HashTable globals;
} VM;

typedef enum {
  I_OK,
  I_COMPILE_ERR,
  I_RUNTIME_ERR,
} IR;

void initVM(VM *);
void deleteVM(VM *);
IR interpret(VM *, const char *);
void push(VM *, Value);
Value pop(VM *);

static void initStack(VM *);
static IR run(VM *);
static void concatString(VM *);
static Value vmStackPeek(VM *, int);
static void runtimeError(VM *, const char *, ...);
static bool isFalse(Value);
static bool underflow(VM *);
static void freeObject(Object *);

#endif