#include "object.h"

void printObject(Value val) {
  switch (OBJECT_TYPE(val)) {
    case CLASS_OBJECT:
      printf("%s", AS_CLASS(val)->name->str);
      break;
    case UPVALUE_OBJECT:
      printf("upvalue");
      break;
    case CLOSURE_OBJECT:
      printFunction(AS_CLOSURE(val)->function);
      break;
    case NATIVE_OBJECT:
      printf("<native fx>");
      break;
    case FUNCTION_OBJECT:
      printFunction(AS_FUNCTION(val));
      break;
    case STRING_OBJECT:
      printf("%s", AS_CSTRING(val));
      break;
  }
}

void printFunction(FunctionObject *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fx %s>", function->name->str);
}

StringObject *copyString(const char *str, int length) {
  uint32_t hash = hashString(str, length);
  StringObject *interned = hashTableFindString(&vm.strings, str, length, hash);
  if (interned != NULL) return interned;
  char *allocStr = ALLOCATE(char, length + 1);
  memcpy(allocStr, str, length);
  allocStr[length] = '\0';
  return allocateString(allocStr, length, hash);
}

StringObject *getString(char *str, int length) {
  uint32_t hash = hashString(str, length);
  StringObject *interned = hashTableFindString(&vm.strings, str, length, hash);
  if (interned != NULL) {
    DELETE_ARRAY(char, str, length + 1);
    return interned;
  }
  return allocateString(str, length, hash);
}

StringObject *allocateString(char *str, int length, uint32_t hash) {
  StringObject *stringObject = ALLOCATE_OBJECT(StringObject, STRING_OBJECT);
  stringObject->length = length;
  stringObject->str = str;
  stringObject->hash = hash;
  push(TO_OBJECT(stringObject));
  hashTableInsertValue(&vm.strings, stringObject, TO_NULL);
  pop();
  return stringObject;
}

ClassObject *newClass(StringObject *name) {
  ClassObject *__class__ = ALLOCATE_OBJECT(ClassObject, CLASS_OBJECT);
  __class__->name = name;
  return __class__;
}

FunctionObject *newFunction() {
  FunctionObject *fx = ALLOCATE_OBJECT(FunctionObject, FUNCTION_OBJECT);
  fx->arity = 0;
  fx->upvalueCount = 0;
  fx->name = NULL;
  initChunk(&fx->chunk);
  return fx;
}

NativeObject *newNative(NativeFx fx) {
  NativeObject *obj = ALLOCATE_OBJECT(NativeObject, NATIVE_OBJECT);
  obj->fx = fx;
  return obj;
}

UpvalueObject *newUpvalue(Value *slot) {
  UpvalueObject *upvalue = ALLOCATE_OBJECT(UpvalueObject, UPVALUE_OBJECT);
  upvalue->loc = slot;
  upvalue->closed = TO_NULL;
  upvalue->next = NULL;
  return upvalue;
}

ClosureObject *newClosure(FunctionObject *fx) {
  UpvalueObject **upvalues = ALLOCATE_OBJECT(UpvalueObject *, fx->upvalueCount);
  for (int i = 0; i < fx->upvalueCount; i++) {
    upvalues[i] = NULL;
  }
  ClosureObject *closure = ALLOCATE_OBJECT(ClosureObject, CLOSURE_OBJECT);
  closure->function = fx;
  closure->upvalues = upvalues;
  closure->upvalueCount = fx->upvalueCount;
  for (int i = 0; i < closure->upvalueCount; i++) {
    closure->upvalues[i] = NULL;
  }
  return closure;
}

Object *allocateObject(size_t size, ObjectType type) {
  Object *object = (Object *)reallocate(NULL, 0, size);
  object->type = type;
  object->isMarked = false;
  object->next = vm.objects;
  vm.objects = object;
#ifdef DEBUG_GC
  printf("      %p allocated %zu bytes for ObjectType %d\n", (void *)object, size, type);
#endif
  return object;
}

uint32_t hashString(const char *str, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)str[i];
    hash *= 16777619;
  }
  return hash;
}