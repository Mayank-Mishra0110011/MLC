#include "chunk.h"

void initChunk(Chunk *chunk) {
  chunk->code = NULL;
  chunk->lines = NULL;
  chunk->count = 0;
  chunk->capacity = 0;
  initVal(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int curCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(curCapacity);
    chunk->code = GROW_ARRAY(chunk->code, uint8_t, curCapacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(chunk->lines, int, curCapacity, chunk->capacity);
  }
  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

void deleteChunk(Chunk *chunk) {
  DELETE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  DELETE_ARRAY(int, chunk->lines, chunk->capacity);
  deleteVal(&chunk->constants);
  initChunk(chunk);
}

int addConst(Chunk *chunk, Value val) {
  writeVal(&chunk->constants, val);
  return chunk->constants.count - 1;
}

void initVal(ValArr *arr) {
  arr->values = NULL;
  arr->capacity = 0;
  arr->count = 0;
}

void writeVal(ValArr *arr, Value val) {
  if (arr->capacity < arr->count + 1) {
    int curCapacity = arr->capacity;
    arr->capacity = GROW_CAPACITY(curCapacity);
    arr->values = GROW_ARRAY(arr->values, Value, curCapacity, arr->capacity);
  }
  arr->values[arr->count] = val;
  arr->count++;
}

void deleteVal(ValArr *arr) {
  DELETE_ARRAY(Value, arr->values, arr->capacity);
  initVal(arr);
}

void printVal(Value val) {
  switch (val.type) {
    case _BOOLEAN:
      printf(AS_BOOL(val) ? "true" : "false");
      break;
    case _NULL:
      printf("null");
      break;
    case _NUMBER:
      printf("%lg", AS_NUMBER(val));
      break;
    case _OBJECT:
      printObject(val);
      break;
  }
}

bool isEqual(Value a, Value b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case _BOOLEAN:
      return AS_BOOL(a) == AS_BOOL(b);
    case _NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);
    case _NULL:
      return true;
    case _OBJECT: {
      return AS_OBJECT(a) == AS_OBJECT(b);
    }
    default:
      return false;
  }
}

void printObject(Value val) {
  switch (OBJECT_TYPE(val)) {
    case UPVALUE_OBJECT:
      printf("upvalue");
      break;
    case CLOSURE_OBJECT:
      printFunction(AS_CLOSURE(val)->function);
      break;
    case NATIVE_OBJECT:
      printf("<native fx>");
      break;
    case FUNCTION_OBJECT:
      printFunction(AS_FUNCTION(val));
      break;
    case STRING_OBJECT:
      printf("%s", AS_CSTRING(val));
      break;
  }
}

void printFunction(FunctionObject *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fx %s>", function->name->str);
}

Object *allocateObject(size_t size, ObjectType type) {
  Object *object = (Object *)reallocate(NULL, 0, size);
  object->type = type;
  return object;
}

uint32_t hashString(const char *str, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= str[i];
    hash *= 16777619;
  }
  return hash;
}

FunctionObject *newFunction() {
  FunctionObject *fx = ALLOCATE_OBJECT(FunctionObject, FUNCTION_OBJECT);
  fx->arity = 0;
  fx->upvalueCount = 0;
  fx->name = NULL;
  initChunk(&fx->chunk);
  return fx;
}

UpvalueObject *newUpvalue(Value *slot) {
  UpvalueObject *upvalue = ALLOCATE_OBJECT(UpvalueObject, UPVALUE_OBJECT);
  upvalue->loc = slot;
  upvalue->closed = TO_NULL;
  upvalue->next = NULL;
  return upvalue;
}

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

NativeObject *newNative(NativeFx fx) {
  NativeObject *obj = ALLOCATE_OBJECT(NativeObject, NATIVE_OBJECT);
  obj->fx = fx;
  return obj;
}

ClosureObject *newClosure(FunctionObject *fx) {
  UpvalueObject **upvalues = ALLOCATE_OBJECT(UpvalueObject *, fx->upvalueCount);
  for (int i = 0; i < fx->upvalueCount; i++) {
    upvalues[i] = NULL;
  }
  ClosureObject *closure = ALLOCATE_OBJECT(ClosureObject, CLOSURE_OBJECT);
  closure->function = fx;
  closure->upvalues = upvalues;
  closure->upvalueCount = fx->upvalueCount;
  return closure;
}