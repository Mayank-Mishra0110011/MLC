#include "vm.h"

void initVM(VM* vm) {
  initStack(vm);
  vm->objects = NULL;
}

void deleteVM(VM* vm) {
  Object* obj = vm->objects;
  while (obj != NULL) {
    Object* next = obj->next;
    freeObject(obj);
    obj = next;
  }
}

void freeObject(Object* obj) {
  switch (obj->type) {
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
}

IR interpret(VM* vm, const char* source) {
  Chunk chunk;
  initChunk(&chunk);
  if (!compile(source, &chunk)) {
    deleteChunk(&chunk);
    return I_COMPILE_ERR;
  }
  vm->chunk = &chunk;
  vm->instrPtr = vm->chunk->code;
  IR res = run(vm);
  // vm and chunk cleanup start
  deleteChunk(&chunk);
  vm->chunk = NULL;
  vm->instrPtr = NULL;
  initStack(vm);
  return res;
}

IR run(VM* vm) {
#define READ_BYTE() (*(vm->instrPtr)++)
#define READ_CONST() (vm->chunk->constants.values[READ_BYTE()])
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
    printf("STACK : ");
    for (Value* val = vm->stack; val < vm->stackTop; val++) {
      printf("[");
      printVal(*val);
      printf("]");
    }
    printf("\n");
    disassembleInstruction(vm->chunk, (int)(vm->instrPtr - vm->chunk->code));
#endif
    uint8_t instr;
    Value constant, a, b;
    switch (instr = READ_BYTE()) {
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
      case OP_NULL:
        push(vm, TO_NULL);
        break;
      case OP_TRUE:
        push(vm, TO_BOOL(true));
        break;
      case OP_FALSE:
        push(vm, TO_BOOL(false));
        break;
      case OP_CONST:
        constant = READ_CONST();
        push(vm, constant);
        break;
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
      case OP_SUBTRACT:
        BINARY_OP(TO_NUMBER, -);
        break;
      case OP_MULTIPLY:
        BINARY_OP(TO_NUMBER, *);
        break;
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
      case OP_RETURN:
        printVal(pop(vm));
        printf("\n");
        return I_OK;
    }
  }
#undef READ_BYTE
#undef READ_CONST
#undef BINARY_OP
}

void concatString(VM* vm) {
  StringObject* b = AS_STRING(pop(vm));
  StringObject* a = AS_STRING(pop(vm));
  int length = a->length + b->length;
  char* str = ALLOCATE(char, length + 1);
  memcpy(str, a->str, a->length);
  memcpy(str + a->length, b->str, b->length);
  str[length] = '\0';
  StringObject* concatedStr = getAllocatedString(str, length);
  concatedStr->obj.next = vm->objects;
  vm->objects = &(concatedStr->obj);
  push(vm, TO_OBJECT(concatedStr));
}

void push(VM* vm, Value value) {
  *(vm->stackTop) = value;
  vm->stackTop++;
}

Value pop(VM* vm) {
  vm->stackTop--;
  return *(vm->stackTop);
}

Value vmStackPeek(VM* vm, int far) {
  return vm->stackTop[-1 - far];
}

void runtimeError(VM* vm, const char* format, ...) {
  size_t instr = vm->instrPtr - vm->chunk->code - 1;
  int line = vm->chunk->lines[instr];
  fprintf(stderr, "\n\x1b[31;1mError at [line %d] in script:\n\x1b[32;1m  => ", line);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n\n\x1b[0m", stderr);
  initStack(vm);
}

bool isFalse(Value val) {
  return IS_NULL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}