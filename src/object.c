#include "object.h"

Object *allocateObject(size_t size, ObjectType type) {
  Object *object = (Object *)reallocate(NULL, 0, size);
  object->type = type;
  return object;
}

uint32_t hashString(const char *str, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= str[i];
    hash *= 16777619;
  }
  return hash;
}

FunctionObject *newFunction() {
  FunctionObject *fx = ALLOCATE_OBJECT(FunctionObject, FUNCTION_OBJECT);
  fx->arity = 0;
  fx->name = NULL;
  return fx;
}