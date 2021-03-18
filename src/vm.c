#include "vm.h"

void initVM() {
  initStack();
  vm.objects = NULL;
  vm.openUpvalues = NULL;
  vm.grayStack = NULL;
  vm.grayCount = 0;
  vm.grayCapacity = 0;
  vm.bytesAllocated = 0;
  vm.nextGC = 1024 * 1024;
  hashTableInit(&vm.strings);
  hashTableInit(&vm.globals);
  defineNative("clock", nativeClock);
}

void deleteVM() {
  hashTableDelete(&vm.strings);
  hashTableDelete(&vm.globals);
#ifdef GC_ON
  freeObjects();
  free(vm.grayStack);
#endif
}

void push(Value value) {
  *(vm.stackTop) = value;
  vm.stackTop++;
}

void initStack() {
  vm.stackTop = vm.stack;
  vm.switchValTop = vm.switchVal;
  vm.caseValTop = vm.caseVal;
  vm.frameCount = 0;
}

void runtimeError(const char* format, ...) {
  StackFrame* frame = &vm.frames[vm.frameCount - 1];
  size_t instr = frame->instrPtr - frame->closure->function->chunk.code - 1;
  int line = frame->closure->function->chunk.lines[instr];
  fprintf(stderr, "\n\x1b[31;1mError on [line %d] in script:\n\x1b[32;1m  => ", line);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n\n\x1b[0m", stderr);
  int lim = 0;
  if (vm.frameCount == FRAMES_MAX) {
    lim = FRAMES_MAX - 5;
  }
  for (int i = vm.frameCount - 1; i >= lim; i--) {
    StackFrame* frame = &vm.frames[i];
    FunctionObject* function = frame->closure->function;
    size_t instr = frame->instrPtr - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instr]);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s\n", function->name->str);
    }
  }
  initStack();
}

void concatString() {
  StringObject* b = AS_STRING(vmStackPeek(0));
  StringObject* a = AS_STRING(vmStackPeek(1));
  int length = a->length + b->length;
  char* str = ALLOCATE(char, length + 1);
  memcpy(str, a->str, a->length);
  memcpy(str + a->length, b->str, b->length);
  str[length] = '\0';
  StringObject* concatedStr = getString(str, length);
  pop();
  pop();
  push(TO_OBJECT(concatedStr));
}

void defineNative(const char* name, NativeFx fx) {
  push(TO_OBJECT(copyString(name, (int)strlen(name))));
  push(TO_OBJECT(newNative(fx)));
  hashTableInsertValue(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

void closeUpvalues(Value* last) {
  while (vm.openUpvalues != NULL && vm.openUpvalues->loc >= last) {
    UpvalueObject* upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->loc;
    upvalue->loc = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

bool isFalse(Value val) {
  return IS_NULL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}

bool callValue(Value callee, int argCount) {
  if (IS_OBJECT(callee)) {
    switch (OBJECT_TYPE(callee)) {
      case NATIVE_OBJECT: {
        NativeFx fx = AS_NATIVE(callee);
        Value val = fx(argCount, vm.stackTop - argCount);
        vm.stackTop -= argCount + 1;
        push(val);
        return true;
      }
      case CLOSURE_OBJECT: {
        return vmCall(AS_CLOSURE(callee), argCount);
      }
      default:
        break;
    }
  }
  runtimeError("Type Error: Not a function");
  return false;
}

bool vmCall(ClosureObject* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError(argCount < closure->function->arity ? "Too few arguments to fx" : "Too many arguments to fx");
    return false;
  }
  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Recursion Error: Maximum recursion depth exceeded\n                      %d stack frames were dropped", vm.frameCount);
    return false;
  }
  StackFrame* frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->instrPtr = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

Value pop() {
  vm.stackTop--;
  return *(vm.stackTop);
}

Value nativeClock(int argCount, Value* args) {
  return TO_NUMBER((double)clock() / CLOCKS_PER_SEC);
}

Value vmStackPeek(int far) {
  return vm.stackTop[-1 - far];
}

IR interpret(const char* source) {
  FunctionObject* function = compile(source);
  if (function == NULL) return I_COMPILE_ERR;
  push(TO_OBJECT(function));
  ClosureObject* closure = newClosure(function);
  pop();
  push(TO_OBJECT(closure));
  callValue(TO_OBJECT(closure), 0);
  return run();
}

IR run() {
  StackFrame* frame = &vm.frames[vm.frameCount - 1];
#define READ_BYTE() (*frame->instrPtr++)
#define READ_CONST() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_SHORT() (frame->instrPtr += 2, (uint16_t)((frame->instrPtr[-2] << 8) | frame->instrPtr[-1]))
#define READ_STRING() AS_STRING(READ_CONST())
#define BINARY_OP(valType, op)                                      \
  do {                                                              \
    if (!IS_NUMBER(vmStackPeek(0)) || !IS_NUMBER(vmStackPeek(0))) { \
      runtimeError("Operand must be a \"Number\" type");            \
      return I_RUNTIME_ERR;                                         \
    }                                                               \
    double b = AS_NUMBER(pop());                                    \
    double a = AS_NUMBER(pop());                                    \
    push(valType(a op b));                                          \
  } while (false)
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("\nSTACK after evaluating last instruction :");
    if (vm.stack >= vm.stackTop) {
      printf(" []");
    }
    for (Value* val = vm.stack; val < vm.stackTop; val++) {
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
      case OP_BRK:
        *(vm.caseValTop - 1) = false;
        break;
      case OP_SWITCH_START:
        *(vm.switchValTop) = pop();
        vm.switchValTop++;
        *(vm.caseValTop) = false;
        vm.caseValTop++;
        break;
      case OP_SWITCH_END:
        vm.switchValTop--;
        vm.caseValTop--;
        if (vm.switchValTop == &vm.switchVal[0]) pop();
        break;
      case OP_CASE: {
        bool eq = *(vm.caseValTop - 1) || isEqual(*(vm.switchValTop - 1), pop());
        *(vm.caseValTop - 1) = eq;
        push(TO_BOOL(eq));
        break;
      }
      case OP_EQUAL:
        a = pop();
        b = pop();
        push(TO_BOOL(isEqual(a, b)));
        break;
      case OP_NOT_EQUAL:
        a = pop();
        b = pop();
        push(TO_BOOL(!isEqual(a, b)));
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
        push(TO_BOOL(isFalse(pop())));
        break;
      case OP_NULL:
        push(TO_NULL);
        break;
      case OP_TRUE:
        push(TO_BOOL(true));
        break;
      case OP_FALSE:
        push(TO_BOOL(false));
        break;
      case OP_CONST:
        constant = READ_CONST();
        push(constant);
        break;
      case OP_ADD:
        if (IS_STRING(vmStackPeek(0)) && IS_STRING(vmStackPeek(1))) {
          concatString();
        } else if (IS_NUMBER(vmStackPeek(0)) && IS_NUMBER(vmStackPeek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(TO_NUMBER(a + b));
        } else {
          runtimeError("Invalid Operation! Operand must be \"Number\" or \"String\" type");
        }
        break;
      case OP_SUBTRACT:
        BINARY_OP(TO_NUMBER, -);
        break;
      case OP_MODULO: {
        double b = AS_NUMBER(pop());
        double a = AS_NUMBER(pop());
        push(TO_NUMBER(fmod(a, b)));
        break;
      }
      case OP_MULTIPLY:
        BINARY_OP(TO_NUMBER, *);
        break;
      case OP_DIVIDE:
        BINARY_OP(TO_NUMBER, /);
        break;
      case OP_NEGATE:
        if (!IS_NUMBER(vmStackPeek(0))) {
          runtimeError("Operand must be a \"Number\" type");
          return I_RUNTIME_ERR;
        }
        push(TO_NUMBER(-AS_NUMBER(pop())));
        break;
      case OP_CALL: {
        int argCount = READ_BYTE();
        if (!callValue(vmStackPeek(argCount), argCount)) return I_RUNTIME_ERR;
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }
      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->loc);
        break;
      }
      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->loc = vmStackPeek(0);
        break;
      }
      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }
      case OP_JMP: {
        uint16_t offset = READ_SHORT();
        frame->instrPtr += offset;
        break;
      }
      case OP_JMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (isFalse(vmStackPeek(0))) frame->instrPtr += offset;
        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->instrPtr -= offset;
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = vmStackPeek(0);
        break;
      }
      case OP_DEFINE_GLOBAL: {
        StringObject* name = READ_STRING();
        hashTableInsertValue(&vm.globals, name, vmStackPeek(0));
        pop();
        break;
      }
      case OP_GET_GLOBAL: {
        StringObject* name = READ_STRING();
        Value val;
        if (!hashTableGetValue(&vm.globals, name, &val)) {
          runtimeError("Undefined variable '%s'.", name->str);
          return I_RUNTIME_ERR;
        }
        push(val);
        break;
      }
      case OP_SET_GLOBAL: {
        StringObject* name = READ_STRING();
        if (hashTableInsertValue(&vm.globals, name, vmStackPeek(0))) {
          hashTableDeleteValue(&vm.globals, name);
          runtimeError("Undefined variable '%s'.", name->str);
          return I_RUNTIME_ERR;
        }
        break;
      }
      case OP_POP:
        pop();
        break;
      case OP_PRINT:
        printVal(pop());
        break;
      case OP_PRINT_LN:
        printf("\n");
        break;
      case OP_CLOSURE: {
        FunctionObject* fx = AS_FUNCTION(READ_CONST());
        ClosureObject* closure = newClosure(fx);
        push(TO_OBJECT(closure));
        for (int i = 0; i < closure->upvalueCount; i++) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }
      case OP_CLOSE_UPVALUE: {
        closeUpvalues(vm.stackTop - 1);
        pop();
        break;
      }
      case OP_RETURN: {
        Value res = pop();
        closeUpvalues(frame->slots);
        vm.frameCount--;
        if (vm.frameCount == 0) {
          pop();
          return I_OK;
        }
        vm.stackTop = frame->slots;
        push(res);
        frame = &vm.frames[vm.frameCount - 1];
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

UpvalueObject* captureUpvalue(Value* local) {
  UpvalueObject* prevUpvalue = NULL;
  UpvalueObject* upvalue = vm.openUpvalues;
  while (upvalue != NULL && upvalue->loc > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }
  if (upvalue != NULL && upvalue->loc == local) return upvalue;
  UpvalueObject* createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;
  if (prevUpvalue == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }
  return createdUpvalue;
}