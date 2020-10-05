#include "object.h"

StringObject *copyString(const char *str, int length) {
  char *allocStr = ALLOCATE(char, length + 1);
  memcpy(allocStr, str, length);
  allocStr[length] = '\0';
  return allocateString(allocStr, length);
}

StringObject *allocateString(char *str, int length) {
  StringObject *stringObject = ALLOCATE_OBJECT(StringObject, STRING_OBJECT);
  stringObject->length = length;
  stringObject->str = str;
  return stringObject;
}

Object *allocateObject(size_t size, ObjectType type) {
  Object *object = (Object *)reallocate(NULL, 0, size);
  object->type = type;
  return object;
}

/*
void printObject(Value val) {
  switch (OBJECT_TYPE(val)) {
    case STRING_OBJECT:
      printf("%s", AS_CSTRING(val));
      break;
  }
}
*/

StringObject *getAllocatedString(char *str, int length) {
  return allocateString(str, length);
}