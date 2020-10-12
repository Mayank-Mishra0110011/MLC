#ifndef MLC_HASH_TABLE_H
#define MLC_HASH_TABLE_H

#include "value.h"

#define HASH_MAX_LOAD 0.5

typedef struct {
  StringObject *key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry *entries;
} HashTable;

void hashTableInit(HashTable *);
void hashTableDelete(HashTable *);
void hashTableCopy(HashTable *, HashTable *);
StringObject *hashTableFindString(HashTable *, const char *, int, uint32_t);
StringObject *copyString(const char *, int, HashTable *);
StringObject *allocateString(char *, int, uint32_t, HashTable *);
StringObject *getAllocatedString(char *, int, HashTable *);
bool hashTableGetValue(HashTable *, StringObject *, Value *);
bool hashTableInsertValue(HashTable *, StringObject *, Value);
bool hashTableDeleteValue(HashTable *, StringObject *);
static Entry *findEntry(Entry *, int, StringObject *);
static void increaseCapacity(HashTable *, int);

#endif