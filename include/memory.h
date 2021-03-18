#ifndef MLC_MEMORY_H
#define MLC_MEMORY_H

#include "chunk.h"

#define GC_ON
#define GC_HEAP_GROW_FACTOR 2

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap)*2)
#define GROW_ARRAY(prev, type, curCount, count) (type *)reallocate(prev, sizeof(type) * (curCount), sizeof(type) * (count))
#define DELETE_ARRAY(type, ptr, curCount) reallocate(ptr, sizeof(type) * (curCount), 0)
#define ALLOCATE(type, count) (type *)reallocate(NULL, 0, sizeof(type) * (count))
#define FREE(type, ptr) reallocate(ptr, sizeof(type), 0)

void *reallocate(void *, size_t, size_t);

void freeObjects();
void markValue(Value);
void markObject(Object *);

static void freeObject(Object *);
static void garbageCollect();
static void markRoots();
static void traceRefs();
static void blackenObject(Object *);
static void markArray(ValArr *);
static void sweep();

#endif
