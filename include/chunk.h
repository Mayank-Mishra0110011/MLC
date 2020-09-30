#ifndef MLC_CHUNK_H
#define MLC_CHUNK_H

#include <stdio.h>

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONST,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_RETURN
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