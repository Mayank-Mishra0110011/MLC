#ifndef MLC_DEBUG_H
#define MLC_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk *, const char *);
int disassembleInstruction(Chunk *, int);
static int simpleInstruction(const char *, int);
static int constantInstruction(const char *, Chunk *, int);

#endif