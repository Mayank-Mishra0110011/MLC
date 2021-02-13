#ifndef MLC_CHUNK_H
#define MLC_CHUNK_H

#include <float.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "memory.h"

#define HASH_MAX_LOAD 0.5

typedef enum {
  OP_SWITCH_START,   // 0
  OP_SWITCH_END,     // 1
  OP_CASE,           // 2
  OP_BRK,            // 3
  OP_CONT,           // 4
  OP_CONST,          // 5
  OP_ADD,            // 6
  OP_MODULO,         // 7
  OP_DEFINE_GLOBAL,  // 8
  OP_SUBTRACT,       // 9
  OP_NOT,            // 10
  OP_POP,            // 11
  OP_GET_GLOBAL,     // 12
  OP_SET_GLOBAL,     // 13
  OP_GET_LOCAL,      // 14
  OP_SET_LOCAL,      // 15
  OP_JMP_IF_FALSE,   // 16
  OP_LOOP,           // 17
  OP_JMP,            // 18
  OP_MULTIPLY,       // 19
  OP_DIVIDE,         // 20
  OP_NEGATE,         // 21
  OP_RETURN,         // 22
  OP_NULL,           // 23
  OP_TRUE,           // 24
  OP_FALSE,          // 25
  OP_EQUAL,          // 26
  OP_NOT_EQUAL,      // 27
  OP_GREATER,        // 28
  OP_GREATER_EQUAL,  // 29
  OP_LESS,           // 30
  OP_LESS_EQUAL,     // 31
  OP_PRINT,          // 32
  OP_PRINT_LN,       // 33
  OP_CALL,           // 34
  OP_CLOSURE,        // 35
  OP_GET_UPVALUE,    // 36
  OP_SET_UPVALUE,    // 37
  OP_CLOSE_UPVALUE,  // 38
} OpCode;

typedef enum {
  UPVALUE_OBJECT,
  CLOSURE_OBJECT,
  NATIVE_OBJECT,
  STRING_OBJECT,
  FUNCTION_OBJECT
} ObjectType;

struct ObjectStruct {
  ObjectType type;
  struct ObjectStruct *next;
};

typedef struct ObjectStruct Object;

struct ObjectStructString {
  Object obj;
  int length;
  char *str;
  uint32_t hash;
};

typedef struct ObjectStructString StringObject;

typedef enum {
  _BOOLEAN,
  _NULL,
  _NUMBER,
  _OBJECT
} ValueType;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Object *object;
  } as;
} Value;

typedef struct {
  int capacity;
  int count;
  Value *values;
} ValArr;

typedef struct {
  int count;
  int capacity;
  uint8_t *code;
  int *lines;
  ValArr constants;
} Chunk;

typedef struct {
  Object obj;
  int arity;
  int upvalueCount;
  Chunk chunk;
  StringObject *name;
} FunctionObject;

typedef struct {
  Object obj;
  Value *loc;
  Value closed;
  struct UpvalueObject *next;
} UpvalueObject;

typedef struct {
  Object obj;
  FunctionObject *function;
  UpvalueObject **upvalues;
  int upvalueCount;
} ClosureObject;

typedef struct {
  StringObject *key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry *entries;
} HashTable;

typedef Value (*NativeFx)(int argCount, Value *args);

typedef struct {
  Object obj;
  NativeFx fx;
} NativeObject;

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define AS_STRING(value) ((StringObject *)AS_OBJECT(value))
#define AS_CSTRING(value) (((StringObject *)AS_OBJECT(value))->str)

#define ALLOCATE_OBJECT(type, objectType) (type *)allocateObject(sizeof(type), objectType)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJECT(value) ((value).as.object)
#define AS_FUNCTION(value) ((FunctionObject *)AS_OBJECT(value))
#define AS_NATIVE(value) (((NativeObject *)AS_OBJECT(value))->fx)
#define AS_CLOSURE(value) ((ClosureObject *)AS_OBJECT(value))

#define TO_BOOL(value) ((Value){_BOOLEAN, {.boolean = value}})
#define TO_NUMBER(value) ((Value){_NUMBER, {.number = value}})
#define TO_NULL ((Value){_NULL, {.number = 0}})
#define TO_OBJECT(obj) ((Value){_OBJECT, {.object = (Object *)obj}})

#define IS_BOOL(value) ((value).type == _BOOLEAN)
#define IS_NUMBER(value) ((value).type == _NUMBER)
#define IS_NULL(value) ((value).type == _NULL)
#define IS_OBJECT(value) ((value).type == _OBJECT)
#define IS_CLOSURE(value) isObjectType(value, CLOSURE_OBJECT);
#define IS_NATIVE(value) isObjectType(value, NATIVE_OBJECT);
#define IS_STRING(value) isObjectType(value, STRING_OBJECT)
#define IS_FUNCTION(value) isObjectType(value, FUNCTION_OBJECT)

NativeObject *newNative(NativeFx);
int addConst(Chunk *, Value);
void initChunk(Chunk *);
void writeChunk(Chunk *, uint8_t, int);
void deleteChunk(Chunk *);
void initVal(ValArr *);
void writeVal(ValArr *, Value);
void deleteVal(ValArr *);
void printVal(Value val);
void printObject(Value);
void printFunction(FunctionObject *);
void hashTableInit(HashTable *);
void hashTableDelete(HashTable *);
void hashTableCopy(HashTable *, HashTable *);
bool isEqual(Value, Value);
bool hashTableGetValue(HashTable *, StringObject *, Value *);
bool hashTableInsertValue(HashTable *, StringObject *, Value);
bool hashTableDeleteValue(HashTable *, StringObject *);
static void increaseCapacity(HashTable *, int);
static Entry *findEntry(Entry *, int, StringObject *);
uint32_t hashString(const char *, int);
StringObject *hashTableFindString(HashTable *, const char *, int, uint32_t);
StringObject *copyString(const char *, int, HashTable *);
StringObject *allocateString(char *, int, uint32_t, HashTable *);
StringObject *getAllocatedString(char *, int, HashTable *);
FunctionObject *newFunction();
ClosureObject *newClosure(FunctionObject *);
UpvalueObject *newUpvalue(Value *);
Object *allocateObject(size_t, ObjectType);

static inline bool isObjectType(Value value, ObjectType type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif