#include "object.h"

StringObject *allocateString(char *str, int length, uint32_t hash) {
  StringObject *stringObject = ALLOCATE_OBJECT(StringObject, STRING_OBJECT);
  stringObject->length = length;
  stringObject->str = str;
  stringObject->hash = hash;
  return stringObject;
}

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