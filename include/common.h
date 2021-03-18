#ifndef MLC_COMMON_H
#define MLC_COMMON_H

#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define FRAMES_MAX 256
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)
#define UINT8_COUNT (UINT8_MAX + 1)

typedef enum {
  TOKEN_LEFT_PAREN,     // 0
  TOKEN_RIGHT_PAREN,    // 1
  TOKEN_LEFT_BRACE,     // 2
  TOKEN_RIGHT_BRACE,    // 3
  TOKEN_COMMA,          // 4
  TOKEN_DOT,            // 5
  TOKEN_MINUS,          // 6
  TOKEN_PLUS,           // 7
  TOKEN_INCREMENT,      // 8
  TOKEN_DECREMENT,      // 9
  TOKEN_MODULO,         // 10
  TOKEN_SEMI,           // 11
  TOKEN_STAR,           // 12
  TOKEN_TYPE_OF,        // 13
  TOKEN_INSTANCE_OF,    // 14
  TOKEN_NEW,            // 15
  TOKEN_DELETE,         // 16
  TOKEN_SLASH,          // 17
  TOKEN_BANG,           // 18
  TOKEN_BANG_EQUAL,     // 19
  TOKEN_EQUAL,          // 20
  TOKEN_EQUAL_EQUAL,    // 21
  TOKEN_GREATER,        // 22
  TOKEN_GREATER_EQUAL,  // 23
  TOKEN_LESS,           // 24
  TOKEN_LESS_EQUAL,     // 25
  TOKEN_LOGICAL_AND,    // 26
  TOKEN_LOGICAL_OR,     // 27
  TOKEN_LOGICAL_NOT,    // 28
  TOKEN_BITWISE_AND,    // 29
  TOKEN_BITWISE_OR,     // 30
  TOKEN_BITWISE_NOT,    // 31
  TOKEN_BITWISE_XOR,    // 32
  TOKEN_LEFT_SHIFT,     // 33
  TOKEN_RIGHT_SHIFT,    // 34
  TOKEN_IDENTIFIER,     // 35
  TOKEN_STRING,         // 36
  TOKEN_NUMBER,         // 37
  TOKEN_CONST,          // 38
  TOKEN_ENUM,           // 39
  TOKEN_IF,             // 40
  TOKEN_ELSE,           // 41
  TOKEN_SWITCH,         // 42
  TOKEN_CASE,           // 43
  TOKEN_DEFAULT,        // 44
  TOKEN_TRY,            // 45
  TOKEN_CATCH,          // 46
  TOKEN_FINALYY,        // 47
  TOKEN_EXP,            // 48
  TOKEN_IMP,            // 49
  TOKEN_TRUE,           // 50
  TOKEN_FALSE,          // 51
  TOKEN_FX,             // 52
  TOKEN_DO,             // 53
  TOKEN_WHILE,          // 54
  TOKEN_FROM,           // 55
  TOKEN_RETURN,         // 56
  TOKEN_GOTO,           // 57
  TOKEN_THROW,          // 58
  TOKEN_THROWS,         // 59
  TOKEN_YIELD,          // 60
  TOKEN_BRK,            // 61
  TOKEN_CONT,           // 62
  TOKEN_MIXIN,          // 63
  TOKEN_STRUCT,         // 64
  TOKEN_OBJECT,         // 65
  TOKEN_EXCEPTION,      // 66
  TOKEN_UNION,          // 67
  TOKEN_CLASS,          // 68
  TOKEN_IFACE,          // 69
  TOKEN_EXT,            // 70
  TOKEN_FINAL,          // 71
  TOKEN_VR,             // 72
  TOKEN_ABS,            // 73
  TOKEN_PUB,            // 74
  TOKEN_PRIV,           // 75
  TOKEN_PROT,           // 76
  TOKEN_SUPER,          // 77
  TOKEN_IMPL,           // 78
  TOKEN_SELF,           // 79
  TOKEN_NULL,           // 80
  TOKEN_VAR,            // 81
  TOKEN_STATIC,         // 82
  TOKEN_UNSAFE,         // 83
  TOKEN_LABEL,          // 84
  TOKEN_PRINT,          // 85
  TOKEN_EOF,            // 86
  TOKEN_ERR             // 87
} TokenType;

typedef struct {
  TokenType type;
  const char* start;
  int length;
  int line;
} Token;

typedef struct {
  const char* start;
  const char* current;
  int line;
} Scanner;

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
  PRE_NONE,
  PRE_ASSIGN,
  PRE_LOGICAL_OR,
  PRE_LOGICAL_AND,
  PRE_EQUALITY,
  PRE_COMP,
  PRE_TERM,
  PRE_FACTOR,
  PRE_UNARY,
  PRE_CALL,
  PRE_PRIMARY
} Precedence;

typedef enum {
  _BOOLEAN,
  _NULL,
  _NUMBER,
  _OBJECT
} ValueType;

typedef enum {
  UPVALUE_OBJECT,
  CLOSURE_OBJECT,
  NATIVE_OBJECT,
  STRING_OBJECT,
  FUNCTION_OBJECT
} ObjectType;

typedef struct Object Object;

struct Object {
  ObjectType type;
  bool isMarked;
  Object* next;
};

typedef struct StringObject StringObject;

struct StringObject {
  Object obj;
  int length;
  char* str;
  uint32_t hash;
};

typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Object* object;
  } as;
} Value;

typedef struct {
  int capacity;
  int count;
  Value* values;
} ValArr;

typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  int* lines;
  ValArr constants;
} Chunk;

typedef struct {
  Token cur;
  Token prev;
  bool hadErr;
  bool panic;
} Parser;

typedef enum {
  I_OK,
  I_COMPILE_ERR,
  I_RUNTIME_ERR,
} IR;

typedef struct {
  StringObject* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} HashTable;

typedef struct {
  Token name;
  int depth;
  bool isCaptured;
} Local;

typedef enum {
  TYPE_FUNCTION,
  TYPE_SCRIPT
} FunctionType;

typedef struct {
  Object obj;
  int arity;
  int upvalueCount;
  Chunk chunk;
  StringObject* name;
} FunctionObject;

typedef struct {
  uint8_t index;
  bool isLocal;
} Upvalue;

typedef struct UpvalueObject UpvalueObject;

struct UpvalueObject {
  Object obj;
  Value* loc;
  Value closed;
  UpvalueObject* next;
};

typedef struct {
  Object obj;
  FunctionObject* function;
  UpvalueObject** upvalues;
  int upvalueCount;
} ClosureObject;

typedef struct {
  ClosureObject* closure;
  uint8_t* instrPtr;
  Value* slots;
} StackFrame;

typedef struct {
  UpvalueObject* openUpvalues;
  StackFrame frames[FRAMES_MAX];
  int frameCount;
  Value stack[STACK_MAX];
  Value switchVal[STACK_MAX];
  bool caseVal[STACK_MAX];
  bool* caseValTop;
  Value* switchValTop;
  Value* stackTop;
  Object* objects;
  HashTable strings;
  HashTable globals;
  int grayCount;
  int grayCapacity;
  Object** grayStack;
  size_t bytesAllocated;
  size_t nextGC;
} VM;

typedef void (*ParseFn)(bool);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence prec;
} ParseRule;

typedef struct Compiler Compiler;

typedef Value (*NativeFx)(int argCount, Value* args);

typedef struct {
  Object obj;
  NativeFx fx;
} NativeObject;

struct Compiler {
  Compiler* enclosing;
  Local locals[UINT8_COUNT];
  Upvalue upvalues[UINT8_COUNT];
  int labels[UINT8_MAX];
  FunctionObject* function;
  FunctionType type;
  int localCount;
  int labelCount;
  int scopeDepth;
};

extern Scanner scanner;
extern Parser parser;
extern VM vm;

#endif