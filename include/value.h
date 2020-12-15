#ifndef MLC_VAL_H
#define MLC_VAL_H

#include "object.h"

typedef enum {
  _BOOLEAN,
  _NULL,
  _NUMBER,
  _OBJECT
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Object *object;
  } as;
} Value;

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJECT(value) ((value).as.object)
#define AS_FUNCTION(value) ((FunctionObject *)AS_OBJECT(value))

#define TO_BOOL(value) ((Value){_BOOLEAN, {.boolean = value}})
#define TO_NUMBER(value) ((Value){_NUMBER, {.number = value}})
#define TO_NULL ((Value){_NULL, {.number = 0}})
#define TO_OBJECT(obj) ((Value){_OBJECT, {.object = (Object *)obj}})

#define IS_BOOL(value) ((value).type == _BOOLEAN)
#define IS_NUMBER(value) ((value).type == _NUMBER)
#define IS_NULL(value) ((value).type == _NULL)
#define IS_OBJECT(value) ((value).type == _OBJECT)
#define IS_STRING(value) isObjectType(value, STRING_OBJECT)
#define IS_FUNCTION(value) isObjectType(value, FUNCTION_OBJECT)

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
void printObject(Value);

static inline bool isObjectType(Value value, ObjectType type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif