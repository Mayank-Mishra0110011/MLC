#ifndef MLC_CHUNK_H
#define MLC_CHUNK_H

#include "common.h"
#include "value.h"

void initChunk(Chunk*);
void writeChunk(Chunk*, uint8_t, int);
void deleteChunk(Chunk*);

int addConst(Chunk*, Value);

#endif