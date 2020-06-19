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
  printf("%g", val);
}