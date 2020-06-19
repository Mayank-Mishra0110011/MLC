#include "chunk.h"

void initChunk(Chunk *chunk) {
  chunk->code = NULL;
  chunk->lines = NULL;
  chunk->count = 0;
  chunk->capacity = 0;
  initVal(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int curCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(curCapacity);
    chunk->code = GROW_ARRAY(chunk->code, uint8_t, curCapacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(chunk->lines, int, curCapacity, chunk->capacity);
  }
  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

void deleteChunk(Chunk *chunk) {
  DELETE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  DELETE_ARRAY(int, chunk->lines, chunk->capacity);
  deleteVal(&chunk->constants);
  initChunk(chunk);
}

int addConst(Chunk *chunk, Value val) {
  writeVal(&chunk->constants, val);
  return chunk->constants.count - 1;
}