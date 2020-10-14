#ifndef MLC_CHUNK_H
#define MLC_CHUNK_H

#include "value.h"

typedef enum {
  OP_CONST,
  OP_ADD,
  OP_MODULO,
  OP_DEFINE_GLOBAL,
  OP_SUBTRACT,
  OP_NOT,
  OP_POP,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_RETURN,
  OP_NULL,
  OP_TRUE,
  OP_FALSE,
  OP_EQUAL,
  OP_NOT_EQUAL,
  OP_GREATER,
  OP_GREATER_EQUAL,
  OP_LESS,
  OP_LESS_EQUAL,
  OP_PRINT
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