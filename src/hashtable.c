#include "hashtable.h"

void hashTableInit(HashTable *hash) {
  hash->count = 0;
  hash->capacity = 0;
  hash->entries = NULL;
}

void hashTableDelete(HashTable *hash) {
  DELETE_ARRAY(Entry, hash->entries, hash->capacity);
  hashTableInit(hash);
}

bool hashTableInsertValue(HashTable *hash, StringObject *key, Value value) {
  if (hash->count + 1 > hash->capacity * HASH_MAX_LOAD) {
    int cap = GROW_CAPACITY(hash->capacity);
    increaseCapacity(hash, cap);
  }
  Entry *entry = findEntry(hash->entries, hash->capacity, key);
  bool newKey = entry->key == NULL;
  if (newKey && IS_NULL(entry->value)) hash->count++;
  entry->key = key;
  entry->value = value;
  return newKey;
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

void increaseCapacity(HashTable *hash, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = TO_NULL;
  }
  hash->count = 0;
  for (int i = 0; i < hash->capacity; i++) {
    Entry *entry = &hash->entries[i];
    if (entry->key == NULL) continue;
    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    hash->count++;
  }
  DELETE_ARRAY(Entry, hash->entries, hash->capacity);
  hash->entries = entries;
  hash->capacity = capacity;
}

void hashTableCopy(HashTable *from, HashTable *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL) {
      hashTableInsertValue(to, entry->key, entry->value);
    }
  }
}

bool hashTableGetValue(HashTable *hash, StringObject *key, Value *value) {
  if (hash->count == 0) return false;
  Entry *entry = findEntry(hash->entries, hash->capacity, key);
  if (entry->key == NULL) return false;
  *value = entry->value;
  return true;
}

bool hashTableDeleteValue(HashTable *hash, StringObject *key) {
  if (hash->count == 0) return false;
  Entry *entry = findEntry(hash->entries, hash->capacity, key);
  if (entry->key == NULL) return false;
  entry->key = NULL;
  entry->value = TO_BOOL(true);
  return true;
}

StringObject *hashTableFindString(HashTable *hash, const char *str, int length, uint32_t _hash) {
  if (hash->count == 0) return NULL;
  uint32_t index = _hash % hash->capacity;
  while (true) {
    Entry *entry = &hash->entries[index];
    if (entry->key == NULL) {
      if (IS_NULL(entry->value)) return NULL;
    } else if (entry->key->length == length && entry->key->hash == _hash && memcmp(entry->key->str, str, length) == 0) {
      return entry->key;
    }
    index = (index + 1) % hash->capacity;
  }
}

StringObject *copyString(const char *str, int length, HashTable *vmStrings) {
  uint32_t hash = hashString(str, length);
  StringObject *interned = hashTableFindString(vmStrings, str, length, hash);
  if (interned != NULL) return interned;
  char *allocStr = ALLOCATE(char, length + 1);
  memcpy(allocStr, str, length);
  allocStr[length] = '\0';
  return allocateString(allocStr, length, hash, vmStrings);
}

StringObject *getAllocatedString(char *str, int length, HashTable *vmStrings) {
  uint32_t hash = hashString(str, length);
  StringObject *allocStringObj, *interned = hashTableFindString(vmStrings, str, length, hash);
  if (interned != NULL) {
    DELETE_ARRAY(char, str, length + 1);
    return interned;
  }
  return allocateString(str, length, hash, vmStrings);
}

StringObject *allocateString(char *str, int length, uint32_t hash, HashTable *vmStrings) {
  StringObject *stringObject = ALLOCATE_OBJECT(StringObject, STRING_OBJECT);
  stringObject->length = length;
  stringObject->str = str;
  stringObject->hash = hash;
  hashTableInsertValue(vmStrings, stringObject, TO_NULL);
  return stringObject;
}