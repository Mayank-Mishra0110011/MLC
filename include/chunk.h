#ifndef MLC_CHUNK_H
#define MLC_CHUNK_H

#include "value.h"

typedef enum {
  OP_SWITCH_START,   // 0
  OP_SWITCH_END,     // 1
  OP_CASE,           // 2
  OP_BRK,            // 3
  OP_CONT,           // 4
  OP_CONST,          // 5
  OP_ADD,            // 6
  OP_MODULO,         // 7
  OP_DEFINE_GLOBAL,  // 8
  OP_SUBTRACT,       // 9
  OP_NOT,            // 10
  OP_POP,            // 11
  OP_GET_GLOBAL,     // 12
  OP_SET_GLOBAL,     // 13
  OP_GET_LOCAL,      // 14
  OP_SET_LOCAL,      // 15
  OP_JMP_IF_FALSE,   // 16
  OP_LOOP,           // 17
  OP_JMP,            // 18
  OP_MULTIPLY,       // 19
  OP_DIVIDE,         // 20
  OP_NEGATE,         // 21
  OP_RETURN,         // 22
  OP_NULL,           // 23
  OP_TRUE,           // 24
  OP_FALSE,          // 25
  OP_EQUAL,          // 26
  OP_NOT_EQUAL,      // 27
  OP_GREATER,        // 28
  OP_GREATER_EQUAL,  // 29
  OP_LESS,           // 30
  OP_LESS_EQUAL,     // 31
  OP_PRINT           // 32
} OpCode;

typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  int* lines;
  ValArr constants;
} Chunk;

void initChunk(Chunk*);
void writeChunk(Chunk*, uint8_t, int);
void deleteChunk(Chunk*);
int addConst(Chunk*, Value);

#endif