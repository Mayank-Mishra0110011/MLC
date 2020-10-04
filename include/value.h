#ifndef MLC_VAL_H
#define MLC_VAL_H

#include <stdio.h>

#include "common.h"
#include "memory.h"

typedef enum {
  _BOOLEAN,
  _NULL,
  _NUMBER
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
  } as;
} Value;

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define TO_BOOL(value) ((Value){_BOOLEAN, {.boolean = value}})
#define TO_NUMBER(value) ((Value){_NUMBER, {.number = value}})
#define TO_NULL ((Value){_NULL, {.number = 0}})

#define IS_BOOL(value) ((value).type == _BOOLEAN)
#define IS_NUMBER(value) ((value).type == _NUMBER)
#define IS_NULL(value) ((value).type == _NULL)

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValArr;

void initVal(ValArr *);
void writeVal(ValArr *, Value);
void deleteVal(ValArr *);
void printVal(Value val);
bool isEqual(Value, Value);

#endif