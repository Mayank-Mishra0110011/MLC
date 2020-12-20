#ifndef MLC_VM_H
#define MLC_VM_H

#include <math.h>
#include <stdarg.h>
#include <time.h>

#include "compiler.h"
#include "debug.h"
#include "hashtable.h"

#define FRAMES_MAX 256
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
  ClosureObject *closure;
  uint8_t *instrPtr;
  Value *slots;
} StackFrame;

typedef struct {
  UpvalueObject *openUpvalues;
  StackFrame frames[FRAMES_MAX];
  int frameCount;
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
static bool callValue(VM *, Value, int);
static bool vmCall(VM *, ClosureObject *, int);
static void freeObject(Object *);
static void closeUpvalues(VM *, Value *);
static void defineNative(VM *, const char *, NativeFx);
static UpvalueObject *captureUpvalue(VM *, Value *);
static Value nativeClock(int, Value *);

#endif