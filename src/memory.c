#include "memory.h"

void *reallocate(void *prev, size_t curSize, size_t newSize) {
  if (newSize == 0) {
    free(prev);
    return NULL;
  }
  return realloc(prev, newSize);
}