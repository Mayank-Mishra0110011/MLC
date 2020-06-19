#include "debug.h"

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("============== %s ==============\n", name);
  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int disassembleInstruction(Chunk *chunk, int offset) {
  printf("%04d ", offset);
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   \" ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }
  uint8_t instr = chunk->code[offset];
  switch (instr) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OP_CONST:
      return constantInstruction("OP_CONST", chunk, offset);
    default:
      printf("Unknown opcode %d\n", instr);
      return offset + 1;
  }
}

int simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int constantInstruction(const char *name, Chunk *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printVal(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2;
}