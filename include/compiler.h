#ifndef MLC_COMPILER_H
#define MLC_COMPILER_H

#include "chunk.h"
#include "hashtable.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
  Token cur;
  Token prev;
  bool hadErr;
  bool panic;
} Parser;

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

typedef void (*ParseFn)(Parser *, Scanner *, HashTable *, bool);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence prec;
} ParseRule;

typedef struct {
  Token name;
  int depth;
  bool isCaptured;
} Local;

typedef struct {
  uint8_t index;
  bool isLocal;
} Upvalue;

typedef enum {
  TYPE_FUNCTION,
  TYPE_SCRIPT
} FunctionType;

typedef struct {
  struct Compiler *enclosing;
  Local locals[UINT8_COUNT];
  Upvalue upvalues[UINT8_COUNT];
  int labels[UINT8_MAX];
  FunctionObject *function;
  FunctionType type;
  int localCount;
  int labelCount;
  int scopeDepth;
} Compiler;

static Compiler *current = NULL;

FunctionObject *compile(const char *, HashTable *);
static ParseRule *getRule(TokenType);
static FunctionObject *endCompilation(Parser *);
static Chunk *currentChunk();
static uint8_t makeConst(Value);
static uint8_t parseVariable(Parser *, Scanner *, HashTable *, const char *);
static uint8_t indentifierConst(Token *, HashTable *);
static uint8_t argList(Parser *, Scanner *, HashTable *);
static int resolveLocal(Parser *, Compiler *, Token *);
static int emitJump(uint8_t, Parser *);
static int resolveUpvalue(Parser *, Compiler *, Token *);
static int addUpvalue(Parser *, Compiler *, uint8_t, bool);
static bool matchToken(Parser *, Scanner *, TokenType);
static bool indentifierEqual(Token *, Token *);
static bool check(Parser *, TokenType);
static void expression(Parser *, Scanner *, HashTable *);
static void statement(Parser *, Scanner *, HashTable *);
static void expressionStatement(Parser *, Scanner *, HashTable *);
static void parsePrecedence(Parser *, Scanner *, HashTable *, Precedence);
static void grouping(Parser *, Scanner *, HashTable *, bool);
static void call(Parser *, Scanner *, HashTable *, bool);
static void string(Parser *, Scanner *, HashTable *, bool);
static void literal(Parser *, Scanner *, HashTable *, bool);
static void binary(Parser *, Scanner *, HashTable *, bool);
static void unary(Parser *, Scanner *, HashTable *, bool);
static void number(Parser *, Scanner *, HashTable *, bool);
static void variable(Parser *, Scanner *, HashTable *, bool);
static void namedVar(Parser *, Scanner *, HashTable *, bool);
static void logicalAnd(Parser *, Scanner *, HashTable *, bool);
static void logicalOr(Parser *, Scanner *, HashTable *, bool);
static void emitConst(Parser *, Value);
static void emitBytes(uint8_t, uint8_t, Parser *);
static void emitByte(uint8_t, Parser *);
static void consume(Parser *, Scanner *, TokenType, const char *);
static void error(Parser *, const char *);
static void errorAt(Parser *, const char *);
static void errorAtCurrent(Parser *, const char *);
static void advance(Parser *, Scanner *);
static void emitReturn(Parser *);
static void declaration(Parser *, Scanner *, HashTable *);
static void printStatement(Parser *, Scanner *, HashTable *);
static void synchronize(Parser *, Scanner *);
static void varDeclaration(Parser *, Scanner *, HashTable *);
static void defineVariable(Parser *, uint8_t);
static void initCompiler(Compiler *, Parser *, HashTable *, FunctionType);
static void beginScope();
static void block(Parser *, Scanner *, HashTable *);
static void endScope(Parser *);
static void declareLocalVar(Parser *);
static void markInitialized();
static void ifStatement(Parser *, Scanner *, HashTable *);
static void whileStatement(Parser *, Scanner *, HashTable *);
static void returnStatement(Parser *, Scanner *, HashTable *);
static void switchStatement(Parser *, Scanner *, HashTable *);
static void evalCase(Parser *, Scanner *, HashTable *);
static void fromStatement(Parser *, Scanner *, HashTable *);
static void cont(Parser *, Scanner *, HashTable *, bool);
static void emitLoop(int, Parser *);
static void backpatchJump(Parser *, int);
static void functionDeclaration(Parser *, Scanner *, HashTable *);
static void function(Parser *, Scanner *, HashTable *, FunctionType);

#endif