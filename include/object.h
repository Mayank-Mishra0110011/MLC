#ifndef MLC_OBJ_H
#define MLC_OBJ_H

#include <stdio.h>
#include <string.h>

#include "memory.h"

typedef enum {
  STRING_OBJECT,
  FUNCTION_OBJECT
} ObjectType;

struct ObjectStruct {
  ObjectType type;
  struct ObjectStruct *next;
};

typedef struct ObjectStruct Object;

struct ObjectStructString {
  Object obj;
  int length;
  char *str;
  uint32_t hash;
};

typedef struct ObjectStructString StringObject;

typedef struct {
  Object obj;
  int arity;
  StringObject *name;
} FunctionObject;

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define AS_STRING(value) ((StringObject *)AS_OBJECT(value))
#define AS_CSTRING(value) (((StringObject *)AS_OBJECT(value))->str)

#define ALLOCATE_OBJECT(type, objectType) (type *)allocateObject(sizeof(type), objectType)

Object *allocateObject(size_t, ObjectType);
FunctionObject *newFunction();

uint32_t hashString(const char *, int);

#endif