#include "vm.h"

void initVM(VM* vm) {
  initStack(vm);
}

void deleteVM(VM* vm) {
  free(vm);
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
  deleteChunk(&chunk);
  return res;
}

IR run(VM* vm) {
#define READ_BYTE() (*(vm->instrPtr)++)
#define READ_CONST() (vm->chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)   \
  do {                  \
    double b = pop(vm); \
    double a = pop(vm); \
    push(vm, a op b);   \
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
    Value constant;
    switch (instr = READ_BYTE()) {
      case OP_CONST:
        constant = READ_CONST();
        push(vm, constant);
        break;
      case OP_ADD:
        BINARY_OP(+);
        break;
      case OP_SUBTRACT:
        BINARY_OP(-);
        break;
      case OP_MULTIPLY:
        BINARY_OP(*);
        break;
      case OP_DIVIDE:
        BINARY_OP(/);
        break;
      case OP_NEGATE:
        push(vm, -pop(vm));
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

void push(VM* vm, Value value) {
  *(vm->stackTop) = value;
  vm->stackTop++;
}

Value pop(VM* vm) {
  vm->stackTop--;
  return *(vm->stackTop);
}
