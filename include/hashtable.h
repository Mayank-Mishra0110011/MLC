#ifndef MLC_HASHTABLE_H
#define MLC_HASHTABLE_H

#include "common.h"
#include "memory.h"

#define HASH_MAX_LOAD 0.5

void hashTableInit(HashTable *);
void hashTableDelete(HashTable *);
void hashTableCopy(HashTable *, HashTable *);
void increaseCapacity(HashTable *, int);
void markTable(HashTable *);
void hashTableRemoveWhite(HashTable *);

bool hashTableGetValue(HashTable *, StringObject *, Value *);
bool hashTableInsertValue(HashTable *, StringObject *, Value);
bool hashTableDeleteValue(HashTable *, StringObject *);

StringObject *hashTableFindString(HashTable *, const char *, int, uint32_t);

static Entry *findEntry(Entry *, int, StringObject *);

#endif
