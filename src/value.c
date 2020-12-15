#include "value.h"

void initVal(ValArr *arr) {
  arr->values = NULL;
  arr->capacity = 0;
  arr->count = 0;
}

void writeVal(ValArr *arr, Value val) {
  if (arr->capacity < arr->count + 1) {
    int curCapacity = arr->capacity;
    arr->capacity = GROW_CAPACITY(curCapacity);
    arr->values = GROW_ARRAY(arr->values, Value, curCapacity, arr->capacity);
  }
  arr->values[arr->count] = val;
  arr->count++;
}

void deleteVal(ValArr *arr) {
  DELETE_ARRAY(Value, arr->values, arr->capacity);
  initVal(arr);
}

void printVal(Value val) {
  switch (val.type) {
    case _BOOLEAN:
      printf(AS_BOOL(val) ? "true" : "false");
      break;
    case _NULL:
      printf("null");
      break;
    case _NUMBER:
      printf("%g", AS_NUMBER(val));
      break;
    case _OBJECT:
      printObject(val);
      break;
  }
}

bool isEqual(Value a, Value b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case _BOOLEAN:
      return AS_BOOL(a) == AS_BOOL(b);
    case _NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);
    case _NULL:
      return true;
    case _OBJECT: {
      return AS_OBJECT(a) == AS_OBJECT(b);
    }
    default:
      return false;
  }
}

void printObject(Value val) {
  switch (OBJECT_TYPE(val)) {
    case FUNCTION_OBJECT:
      printf("<fx %s>", AS_FUNCTION(val)->name->str);
    case STRING_OBJECT:
      printf("%s", AS_CSTRING(val));
      break;
  }
}