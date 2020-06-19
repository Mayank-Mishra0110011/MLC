#ifndef MLC_MEM_H
#define MLC_MEM_H

#include <stdlib.h>

#include "common.h"

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap)*2)
#define GROW_ARRAY(prev, type, curCount, count) (type *)reallocate(prev, sizeof(type) * (curCount), sizeof(type) * (count))
#define DELETE_ARRAY(type, ptr, curCount) reallocate(ptr, sizeof(type) * (curCount), 0)

void *reallocate(void *, size_t, size_t);

#endif
