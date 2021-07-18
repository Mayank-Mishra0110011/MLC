#ifndef MLC_OBJECT_H
#define MLC_OBJECT_H

#include "chunk.h"
#include "common.h"
#include "hashtable.h"
#include "memory.h"
#include "vm.h"

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_CLOSURE(value) ((ClosureObject *)AS_OBJECT(value))
#define AS_NATIVE(value) (((NativeObject *)AS_OBJECT(value))->fx)
#define AS_FUNCTION(value) ((FunctionObject *)AS_OBJECT(value))
#define AS_OBJECT(value) ((value).as.object)
#define AS_STRING(value) ((StringObject *)AS_OBJECT(value))
#define AS_CSTRING(value) (((StringObject *)AS_OBJECT(value))->str)
#define AS_CLASS(value) (((ClassObject *)AS_OBJECT(value)))

#define TO_BOOL(value) ((Value){_BOOLEAN, {.boolean = value}})
#define TO_NUMBER(value) ((Value){_NUMBER, {.number = value}})
#define TO_NULL ((Value){_NULL, {.number = 0}})
#define TO_OBJECT(obj) ((Value){_OBJECT, {.object = (Object *)obj}})

#define IS_BOOL(value) ((value).type == _BOOLEAN)
#define IS_NUMBER(value) ((value).type == _NUMBER)
#define IS_CLOSURE(value) isObjectType(value, CLOSURE_OBJECT);
#define IS_NATIVE(value) isObjectType(value, NATIVE_OBJECT)
#define IS_FUNCTION(value) isObjectType(value, FUNCTION_OBJECT)
#define IS_OBJECT(value) ((value).type == _OBJECT)
#define IS_STRING(value) isObjectType(value, STRING_OBJECT)
#define IS_STRING(value) isObjectType(value, STRING_OBJECT)
#define IS_CLASS(value) isObjectType(value, CLASS_OBJECT)
#define IS_NULL(value) ((value).type == _NULL)

#define ALLOCATE_OBJECT(type, objectType) (type *)allocateObject(sizeof(type), objectType)

void printObject(Value);

static void printFunction(FunctionObject *);

StringObject *copyString(const char *, int);
StringObject *getString(char *, int);

static StringObject *allocateString(char *, int, uint32_t);

FunctionObject *newFunction();
ClassObject *newClass(StringObject *);

NativeObject *newNative(NativeFx);

UpvalueObject *newUpvalue(Value *);

ClosureObject *newClosure(FunctionObject *);

static Object *allocateObject(size_t, ObjectType);

static uint32_t hashString(const char *, int);

static inline bool isObjectType(Value value, ObjectType type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif
