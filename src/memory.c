#include "memory.h"

void *reallocate(void *prev, size_t curSize, size_t newSize) {
  vm.bytesAllocated += newSize - curSize;
  if (newSize > curSize) {
#ifdef GC_ON
    garbageCollect();
#endif
    if (vm.bytesAllocated > vm.nextGC) {
      garbageCollect();
    }
  }
  if (newSize == 0) {
    free(prev);
    return NULL;
  }

  return realloc(prev, newSize);
}

void freeObjects() {
  size_t before = vm.bytesAllocated;
  Object *obj = vm.objects;
  while (obj != NULL) {
    Object *next = obj->next;
    freeObject(obj);
    obj = next;
  }
#ifdef DEBUG_GC
  printf("      Collected %zu bytes (from %zu to %zu) next at %zu\n", before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

void markValue(Value value) {
  if (!IS_OBJECT(value)) return;
  markObject(AS_OBJECT(value));
}

void markObject(Object *obj) {
  if (obj == NULL) return;
  if (obj->isMarked) return;
#ifdef DEBUG_GC
  printf("      %p mark ", (void *)obj);
  printVal(TO_OBJECT(obj));
  printf("\n");
#endif
  obj->isMarked = true;
  if (vm.grayCapacity < vm.grayCount + 1) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Object **)realloc(vm.grayStack, sizeof(Object *) * vm.grayCapacity);
    if (vm.grayStack == NULL) exit(1);
  }
  vm.grayStack[vm.grayCount++] = obj;
}

void freeObject(Object *obj) {
#ifdef DEBUG_GC
  printf("      %p free ObjectType %d\n", (void *)obj, obj->type);
#endif
  switch (obj->type) {
    case UPVALUE_OBJECT: {
      FREE(UpvalueObject, obj);
      break;
    }
    case CLOSURE_OBJECT: {
      ClosureObject *closure = (ClosureObject *)obj;
      if (closure->upvalueCount > 0) {
        DELETE_ARRAY(UpvalueObject *, closure->upvalues, closure->upvalueCount);
      }
      FREE(ClosureObject, obj);
      break;
    }
    case FUNCTION_OBJECT: {
      FunctionObject *fx = (FunctionObject *)obj;
      deleteChunk(&fx->chunk);
      FREE(FunctionObject, obj);
      break;
    }
    case NATIVE_OBJECT: {
      FREE(NativeObject, obj);
      break;
    }
    case STRING_OBJECT: {
      StringObject *string = (StringObject *)obj;
      DELETE_ARRAY(char, string->str, string->length + 1);
      FREE(StringObject, obj);
      break;
    }
  }
}

void garbageCollect() {
#ifdef DEBUG_GC
  printf("\n                -- gc begin\n");
  size_t before = vm.bytesAllocated;
#endif
  markRoots();
  traceRefs();
  hashTableRemoveWhite(&vm.strings);
  sweep();
  vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef DEBUG_GC
  printf("                -- gc end\n\n");
  printf("      Collected %zu bytes (from %zu to %zu) next at %zu\n", before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

void markRoots() {
  for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
    markValue(*slot);
  }
  markTable(&vm.globals);
  markCompilerRoots();
  for (int i = 0; i < vm.frameCount; i++) {
    markObject((Object *)vm.frames[i].closure);
  }
  for (UpvalueObject *upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
    markObject((Object *)upvalue);
  }
}

void traceRefs() {
  while (vm.grayCount > 0) {
    Object *obj = vm.grayStack[--vm.grayCount];
    blackenObject(obj);
  }
}

void blackenObject(Object *obj) {
#ifdef DEBUG_GC
  printf("      %p blacken ", (void *)obj);
  printVal(TO_OBJECT(obj));
  printf("\n");
#endif
  switch (obj->type) {
    case UPVALUE_OBJECT:
      markValue(((UpvalueObject *)obj)->closed);
      break;
    case CLOSURE_OBJECT: {
      ClosureObject *closure = (ClosureObject *)obj;
      markObject((Object *)closure->function);
      for (int i = 0; i < closure->upvalueCount; i++) {
        markObject((Object *)closure->upvalues[i]);
      }
      break;
    }
    case FUNCTION_OBJECT: {
      FunctionObject *fx = (FunctionObject *)obj;
      markObject((Object *)fx->name);
      markArray(&fx->chunk.constants);
      break;
    }
    case NATIVE_OBJECT:
    case STRING_OBJECT:
      break;
  }
}

void markArray(ValArr *arr) {
  for (int i = 0; i < arr->count; i++) {
    markValue(arr->values[i]);
  }
}

void sweep() {
  Object *prev = NULL;
  Object *obj = vm.objects;
  while (obj != NULL) {
    if (obj->isMarked) {
      obj->isMarked = false;
      prev = obj;
      obj = obj->next;
    } else {
      Object *unreachable = obj;
      obj = obj->next;
      if (prev != NULL) {
        prev->next = obj;
      } else {
        vm.objects = obj;
      }
      freeObject(unreachable);
    }
  }
}
