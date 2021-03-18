#include "hashtable.h"

void hashTableInit(HashTable *hashTable) {
  hashTable->count = 0;
  hashTable->capacity = 0;
  hashTable->entries = NULL;
}

void hashTableDelete(HashTable *hashTable) {
  DELETE_ARRAY(Entry, hashTable->entries, hashTable->capacity);
  hashTableInit(hashTable);
}

void hashTableCopy(HashTable *from, HashTable *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL) {
      hashTableInsertValue(to, entry->key, entry->value);
    }
  }
}

void increaseCapacity(HashTable *hashTable, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = TO_NULL;
  }
  hashTable->count = 0;
  for (int i = 0; i < hashTable->capacity; i++) {
    Entry *entry = &hashTable->entries[i];
    if (entry->key == NULL) continue;
    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    hashTable->count++;
  }
  DELETE_ARRAY(Entry, hashTable->entries, hashTable->capacity);
  hashTable->entries = entries;
  hashTable->capacity = capacity;
}

void markTable(HashTable *hashTable) {
  for (int i = 0; i < hashTable->capacity; i++) {
    Entry *entry = &hashTable->entries[i];
    markObject((Object *)entry->key);
    markValue(entry->value);
  }
}

void hashTableRemoveWhite(HashTable *hashTable) {
  for (int i = 0; i < hashTable->capacity; i++) {
    Entry *entry = &hashTable->entries[i];
    if (entry->key != NULL && !entry->key->obj.isMarked) {
      hashTableDeleteValue(hashTable, entry->key);
    }
  }
}

bool hashTableGetValue(HashTable *hashTable, StringObject *key, Value *value) {
  if (hashTable->count == 0) return false;
  Entry *entry = findEntry(hashTable->entries, hashTable->capacity, key);
  if (entry->key == NULL) return false;
  *value = entry->value;
  return true;
}

bool hashTableInsertValue(HashTable *hashTable, StringObject *key, Value value) {
  if (hashTable->count + 1 > hashTable->capacity * HASH_MAX_LOAD) {
    int cap = GROW_CAPACITY(hashTable->capacity);
    increaseCapacity(hashTable, cap);
  }
  Entry *entry = findEntry(hashTable->entries, hashTable->capacity, key);
  bool newKey = entry->key == NULL;
  if (newKey && IS_NULL(entry->value)) hashTable->count++;
  entry->key = key;
  entry->value = value;
  return newKey;
}

bool hashTableDeleteValue(HashTable *hashTable, StringObject *key) {
  if (hashTable->count == 0) return false;
  Entry *entry = findEntry(hashTable->entries, hashTable->capacity, key);
  if (entry->key == NULL) return false;
  entry->key = NULL;
  entry->value = TO_BOOL(true);
  return true;
}

StringObject *hashTableFindString(HashTable *hashTable, const char *str, int length, uint32_t hash) {
  if (hashTable->count == 0) return NULL;
  uint32_t index = hash % hashTable->capacity;
  while (true) {
    Entry *entry = &hashTable->entries[index];
    if (entry->key == NULL) {
      if (IS_NULL(entry->value)) return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->str, str, length) == 0) {
      return entry->key;
    }
    index = (index + 1) % hashTable->capacity;
  }
}

Entry *findEntry(Entry *entries, int capacity, StringObject *key) {
  uint32_t index = key->hash % capacity;
  Entry *tombstone = NULL;
  while (true) {
    Entry *entry = &entries[index];
    if (entry->key == NULL) {
      if (IS_NULL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) {
      return entry;
    }
    index = (index + 1) % capacity;
  }
}
