#ifndef MLC_OBJ_H
#define MLC_OBJ_H

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "memory.h"

typedef enum {
  STRING_OBJECT,
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
};

typedef struct ObjectStructString StringObject;

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define AS_STRING(value) ((StringObject *)AS_OBJECT(value))
#define AS_CSTRING(value) (((StringObject *)AS_OBJECT(value))->str)

#define ALLOCATE_OBJECT(type, objectType) (type *)allocateObject(sizeof(type), objectType)

StringObject *copyString(const char *, int);
StringObject *getAllocatedString(char *, int);
static Object *allocateObject(size_t, ObjectType);
static StringObject *allocateString(char *, int);

#endif