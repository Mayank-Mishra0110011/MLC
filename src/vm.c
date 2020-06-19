#include "vm.h"

void initVM() {
}

void deleteVM() {
}

IR interpret(Chunk *chunk) {
  vm.chunk = chunk;
  vm.instrPtr = vm.chunk->code;
  return run();
}

IR run() {
#define READ_BYTE() (*vm.instrPtr++)
#define READ_CONST() (vm.chunk->constants.values[READ_BYTE()])
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    disassembleInstruction(vm.chunk, (int)(vm.instrPtr - vm.chunk->code));
#endif
    uint8_t instr;
    Value constant;
    switch (instr = READ_BYTE()) {
      case OP_CONST:
        constant = READ_CONST();
        printVal(constant);
        printf("\n");
        break;
      case OP_RETURN:
        return I_OK;
    }
  }
#undef READ_BYTE
#undef READ_CONST
}
