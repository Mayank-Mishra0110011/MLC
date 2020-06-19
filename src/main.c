#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

int main() {
  initVM();
  Chunk chunk;
  initChunk(&chunk);
  int constantIndex = addConst(&chunk, 1.2);
  writeChunk(&chunk, OP_CONST, 1);
  writeChunk(&chunk, constantIndex, 1);
  writeChunk(&chunk, OP_RETURN, 1);
  disassembleChunk(&chunk, "Test Chunk");
  interpret(&chunk);
  deleteVM();
  deleteChunk(&chunk);
  return 0;
}

/*
  OP_RETURN
    _____opcode
   |      
   |
  [01]
    1byte

  OP_CONST
    _____opcode
   |      
   |
  [00][26]<-----const index
    2byte
*/