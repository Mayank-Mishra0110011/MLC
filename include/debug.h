#ifndef MLC_DEBUG_H
#define MLC_DEBUG_H

#include "common.h"
#include "value.h"

// #define DEBUG_PRINT_CODE
// #define DEBUG_TRACE_EXECUTION
#define DEBUG_GC

void disassembleChunk(Chunk *, const char *);

int disassembleInstruction(Chunk *, int);

static int simpleInstruction(const char *, int);
static int constantInstruction(const char *, Chunk *, int);
static int byteInstruction(const char *, Chunk *, int);
static int jumpInstruction(const char *, int, Chunk *, int);

#endif
