#include "vm.h"

void initVM(VM* vm) {
  initStack(vm);
  vm->objects = NULL;
  vm->openUpvalues = NULL;
  hashTableInit(&vm->strings);
  hashTableInit(&vm->globals);
  defineNative(vm, "clock", nativeClock);
}

void deleteVM(VM* vm) {
  hashTableDelete(&vm->strings);
  hashTableDelete(&vm->globals);
  Object* obj = vm->objects;
  while (obj != NULL) {
    Object* next = obj->next;
    freeObject(obj);
    obj = next;
  }
}

void freeObject(Object* obj) {
  switch (obj->type) {
    case UPVALUE_OBJECT: {
      FREE(UpvalueObject, obj);
      break;
    }
    case CLOSURE_OBJECT: {
      ClosureObject* closure = (ClosureObject*)obj;
      DELETE_ARRAY(UpvalueObject*, closure->upvalues, closure->upvalueCount);
      FREE(ClosureObject, obj);
      break;
    }
    case FUNCTION_OBJECT: {
      FunctionObject* fx = (FunctionObject*)obj;
      deleteChunk(&fx->chunk);
      FREE(FunctionObject, obj);
      break;
    }
    case NATIVE_OBJECT: {
      FREE(NativeObject, obj);
      break;
    }
    case STRING_OBJECT: {
      StringObject* string = (StringObject*)obj;
      DELETE_ARRAY(char, string->str, string->length + 1);
      FREE(StringObject, obj);
      break;
    }
  }
}

void initStack(VM* vm) {
  vm->stackTop = vm->stack;
  vm->switchValTop = vm->switchVal;
  vm->caseValTop = vm->caseVal;
  vm->frameCount = 0;
}

IR interpret(VM* vm, const char* source) {
  FunctionObject* function = compile(source, &vm->strings);
  if (function == NULL) return I_COMPILE_ERR;
  push(vm, TO_OBJECT(function));
  ClosureObject* closure = newClosure(function);
  pop(vm);
  push(vm, TO_OBJECT(closure));
  callValue(vm, TO_OBJECT(closure), 0);
  return run(vm);
}

IR run(VM* vm) {
  StackFrame* frame = &vm->frames[vm->frameCount - 1];
#define READ_BYTE() (*frame->instrPtr++)
#define READ_CONST() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_SHORT() (frame->instrPtr += 2, (uint16_t)((frame->instrPtr[-2] << 8) | frame->instrPtr[-1]))
#define READ_STRING() AS_STRING(READ_CONST())
#define BINARY_OP(valType, op)                                              \
  do {                                                                      \
    if (!IS_NUMBER(vmStackPeek(vm, 0)) || !IS_NUMBER(vmStackPeek(vm, 0))) { \
      runtimeError(vm, "Operand must be a \"Number\" type");                \
      return I_RUNTIME_ERR;                                                 \
    }                                                                       \
    double b = AS_NUMBER(pop(vm));                                          \
    double a = AS_NUMBER(pop(vm));                                          \
    push(vm, valType(a op b));                                              \
  } while (false)
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("\nSTACK after evaluating last instruction :");
    if (vm->stack >= vm->stackTop) {
      printf(" []");
    }
    for (Value* val = vm->stack; val < vm->stackTop; val++) {
      printf(" [");
      printVal(*val);
      printf("]");
    }
    printf("\n\n");
    disassembleInstruction(&frame->closure->function->chunk, (int)(frame->instrPtr - frame->closure->function->chunk.code));
#endif
    uint8_t instr = READ_BYTE();
    Value constant, a, b;
    switch (instr) {
      case OP_CALL: {
        int argCount = READ_BYTE();
        if (!callValue(vm, vmStackPeek(vm, argCount), argCount)) return I_RUNTIME_ERR;
        frame = &vm->frames[vm->frameCount - 1];
        break;
      }
      case OP_BRK:
        *(vm->caseValTop - 1) = false;
        break;
      case OP_SWITCH_START:
        *(vm->switchValTop) = pop(vm);
        vm->switchValTop++;
        *(vm->caseValTop) = false;
        vm->caseValTop++;
        break;
      case OP_SWITCH_END:
        vm->switchValTop--;
        vm->caseValTop--;
        if (vm->switchValTop == &vm->switchVal[0]) pop(vm);
        break;
      case OP_CASE: {
        bool eq = *(vm->caseValTop - 1) || isEqual(*(vm->switchValTop - 1), pop(vm));
        *(vm->caseValTop - 1) = eq;
        push(vm, TO_BOOL(eq));
        break;
      }
      case OP_EQUAL:
        a = pop(vm);
        b = pop(vm);
        push(vm, TO_BOOL(isEqual(a, b)));
        break;
      case OP_NOT_EQUAL:
        a = pop(vm);
        b = pop(vm);
        push(vm, TO_BOOL(!isEqual(a, b)));
        break;
      case OP_GREATER:
        BINARY_OP(TO_BOOL, >);
        break;
      case OP_LESS:
        BINARY_OP(TO_BOOL, <);
        break;
      case OP_GREATER_EQUAL:
        BINARY_OP(TO_BOOL, >=);
        break;
      case OP_LESS_EQUAL:
        BINARY_OP(TO_BOOL, <=);
        break;
      case OP_NOT:
        push(vm, TO_BOOL(isFalse(pop(vm))));
        break;
      case OP_POP:
        pop(vm);
        break;
      case OP_NULL:
        push(vm, TO_NULL);
        break;
      case OP_TRUE:
        push(vm, TO_BOOL(true));
        break;
      case OP_FALSE:
        push(vm, TO_BOOL(false));
        break;
      case OP_CONST: {
        constant = READ_CONST();
        push(vm, constant);
        break;
      }
      case OP_ADD:
        if (IS_STRING(vmStackPeek(vm, 0)) && IS_STRING(vmStackPeek(vm, 1))) {
          concatString(vm);
        } else if (IS_NUMBER(vmStackPeek(vm, 0)) && IS_NUMBER(vmStackPeek(vm, 1))) {
          double b = AS_NUMBER(pop(vm));
          double a = AS_NUMBER(pop(vm));
          push(vm, TO_NUMBER(a + b));
        } else {
          runtimeError(vm, "Invalid Operation! Operand must be \"Number\" or \"String\" type");
        }
        break;
      case OP_DEFINE_GLOBAL: {
        StringObject* name = READ_STRING();
        hashTableInsertValue(&vm->globals, name, vmStackPeek(vm, 0));
        pop(vm);
        break;
      }
      case OP_MODULO: {
        double b = AS_NUMBER(pop(vm));
        double a = AS_NUMBER(pop(vm));
        push(vm, TO_NUMBER(fmod(a, b)));
        break;
      }
      case OP_SUBTRACT:
        BINARY_OP(TO_NUMBER, -);
        break;
      case OP_MULTIPLY:
        BINARY_OP(TO_NUMBER, *);
        break;
      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->instrPtr -= offset;
        break;
      }
      case OP_JMP: {
        uint16_t offset = READ_SHORT();
        frame->instrPtr += offset;
        break;
      }
      case OP_JMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (isFalse(vmStackPeek(vm, 0))) frame->instrPtr += offset;
        break;
      }
      case OP_GET_GLOBAL: {
        StringObject* name = READ_STRING();
        Value val;
        if (!hashTableGetValue(&vm->globals, name, &val)) {
          runtimeError(vm, "Undefined variable '%s'.", name->str);
          return I_RUNTIME_ERR;
        }
        push(vm, val);
        break;
      }
      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(vm, frame->slots[slot]);
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = vmStackPeek(vm, 0);
        break;
      }
      case OP_SET_GLOBAL: {
        StringObject* name = READ_STRING();
        if (hashTableInsertValue(&vm->globals, name, vmStackPeek(vm, 0))) {
          hashTableDeleteValue(&vm->globals, name);
          runtimeError(vm, "Undefined variable '%s'.", name->str);
          return I_RUNTIME_ERR;
        }
        break;
      }
      case OP_DIVIDE:
        BINARY_OP(TO_NUMBER, /);
        break;
      case OP_NEGATE:
        if (!IS_NUMBER(vmStackPeek(vm, 0))) {
          runtimeError(vm, "Operand must be a \"Number\" type");
          return I_RUNTIME_ERR;
        }
        push(vm, TO_NUMBER(-AS_NUMBER(pop(vm))));
        break;
      case OP_PRINT: {
        printVal(pop(vm));
        printf("\n");
        break;
      }
      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(vm, *frame->closure->upvalues[slot]->loc);
        break;
      }
      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        printf("SEG FAULT %d\n", *frame->closure->upvalues[slot]);
        *frame->closure->upvalues[slot]->loc = vmStackPeek(vm, 0);
        break;
      }
      case OP_CLOSURE: {
        FunctionObject* fx = AS_FUNCTION(READ_CONST());
        ClosureObject* closure = newClosure(fx);
        push(vm, TO_OBJECT(closure));
        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] = captureUpvalue(vm, frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }
      case OP_CLOSE_UPVALUE: {
        closeUpvalues(vm, vm->stackTop - 1);
        pop(vm);
        break;
      }
      case OP_RETURN: {
        Value res = pop(vm);
        closeUpvalues(vm, frame->slots);
        vm->frameCount--;
        if (vm->frameCount == 0) {
          pop(vm);
          return I_OK;
        }
        vm->stackTop = frame->slots;
        push(vm, res);
        frame = &vm->frames[vm->frameCount - 1];
        break;
      }
    }
  }
#undef READ_BYTE
#undef READ_CONST
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

void closeUpvalues(VM* vm, Value* last) {
  while (vm->openUpvalues != NULL && vm->openUpvalues->loc >= last) {
    UpvalueObject* upvalue = vm->openUpvalues;
    upvalue->closed = *upvalue->loc;
    upvalue->loc = &upvalue->closed;
    vm->openUpvalues = (UpvalueObject*)upvalue->next;
  }
}

UpvalueObject* captureUpvalue(VM* vm, Value* local) {
  UpvalueObject* prevUpvalue = NULL;
  UpvalueObject* upvalue = vm->openUpvalues;
  while (upvalue != NULL && upvalue->loc > local) {
    prevUpvalue = upvalue;
    upvalue = (UpvalueObject*)upvalue->next;
  }
  if (upvalue != NULL && upvalue->loc == local) return upvalue;
  UpvalueObject* createdUpvalue = newUpvalue(local);
  createdUpvalue->next = (struct UpvalueObject*)upvalue;
  if (prevUpvalue == NULL) {
    vm->openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = (struct UpvalueObject*)createdUpvalue;
  }
  return createdUpvalue;
}

void concatString(VM* vm) {
  StringObject* b = AS_STRING(pop(vm));
  StringObject* a = AS_STRING(pop(vm));
  int length = a->length + b->length;
  char* str = ALLOCATE(char, length + 1);
  memcpy(str, a->str, a->length);
  memcpy(str + a->length, b->str, b->length);
  str[length] = '\0';
  StringObject* concatedStr = getAllocatedString(str, length, &vm->strings);
  concatedStr->obj.next = vm->objects;
  vm->objects = &(concatedStr->obj);
  hashTableInsertValue(&vm->strings, concatedStr, TO_NULL);
  push(vm, TO_OBJECT(concatedStr));
}

bool vmCall(VM* vm, ClosureObject* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError(vm, argCount < closure->function->arity ? "Too few arguments to fx" : "Too many arguments to fx");
    return false;
  }
  if (vm->frameCount == FRAMES_MAX) {
    runtimeError(vm, "Recursion Error: Maximum recursion depth exceeded\n                      %d stack frames were dropped", vm->frameCount);
    return false;
  }
  StackFrame* frame = &vm->frames[vm->frameCount++];
  frame->closure = closure;
  frame->instrPtr = closure->function->chunk.code;
  frame->slots = vm->stackTop - argCount - 1;
  return true;
}

bool callValue(VM* vm, Value callee, int argCount) {
  if (IS_OBJECT(callee)) {
    switch (OBJECT_TYPE(callee)) {
      case NATIVE_OBJECT: {
        NativeFx fx = AS_NATIVE(callee);
        Value val = fx(argCount, vm->stackTop - argCount);
        vm->stackTop -= argCount + 1;
        push(vm, val);
        return true;
      }
      case CLOSURE_OBJECT:
        return vmCall(vm, AS_CLOSURE(callee), argCount);
      default:
        break;
    }
  }
  runtimeError(vm, "Type Error: Not a function");
  return false;
}

void push(VM* vm, Value value) {
  *(vm->stackTop) = value;
  vm->stackTop++;
}

Value pop(VM* vm) {
  if (!underflow(vm)) vm->stackTop--;
  return *(vm->stackTop);
}

void defineNative(VM* vm, const char* name, NativeFx fx) {
  push(vm, TO_OBJECT(copyString(name, (int)strlen(name), &vm->strings)));
  push(vm, TO_OBJECT(newNative(fx)));
  hashTableInsertValue(&vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);
  pop(vm);
  pop(vm);
}

Value nativeClock(int argCount, Value* args) {
  return TO_NUMBER((double)clock() / CLOCKS_PER_SEC);
}

Value vmStackPeek(VM* vm, int far) {
  return vm->stackTop[-1 - far];
}

void runtimeError(VM* vm, const char* format, ...) {
  StackFrame* frame = &vm->frames[vm->frameCount - 1];
  size_t instr = frame->instrPtr - frame->closure->function->chunk.code - 1;
  int line = frame->closure->function->chunk.lines[instr];
  fprintf(stderr, "\n\x1b[31;1mError on [line %d] in script:\n\x1b[32;1m  => ", line);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n\n\x1b[0m", stderr);
  int lim = 0;
  if (vm->frameCount == FRAMES_MAX) {
    lim = FRAMES_MAX - 5;
  }
  for (int i = vm->frameCount - 1; i >= lim; i--) {
    StackFrame* frame = &vm->frames[i];
    FunctionObject* function = frame->closure->function;
    size_t instr = frame->instrPtr - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instr]);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s\n", function->name->str);
    }
  }
  initStack(vm);
}

bool underflow(VM* vm) {
  return vm->stackTop == &vm->stack[0];
}

bool isFalse(Value val) {
  return IS_NULL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}