#ifndef MLC_VM_H
#define MLC_VM_H

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "hashtable.h"
#include "memory.h"
#include "object.h"
#include "value.h"

void initVM();
void deleteVM();
void push(Value);

static void initStack();
static void runtimeError(const char *, ...);
static void concatString();
static void defineNative(const char *, NativeFx);
static void closeUpvalues(Value *);

static bool isFalse(Value);
static bool callValue(Value, int);
static bool vmCall(ClosureObject *, int);

Value pop();

static Value nativeClock(int, Value *);
static Value vmStackPeek(int);

IR interpret(const char *);
static IR run();

static UpvalueObject *captureUpvalue(Value *);

#endif