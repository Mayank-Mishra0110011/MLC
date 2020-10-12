#ifndef MLC_COMPILER_H
#define MLC_COMPILER_H

#include <stdlib.h>

#include "hashtable.h"
#include "object.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#else
#include "chunk.h"
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
  PRE_OR,
  PRE_AND,
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

static Chunk *compilingChunk;

bool compile(const char *, Chunk *, HashTable *);
static void expression(Parser *, Scanner *, HashTable *);
static void parsePrecedence(Parser *, Scanner *, HashTable *, Precedence);

static void grouping(Parser *, Scanner *, HashTable *, bool);
static void string(Parser *, Scanner *, HashTable *, bool);
static void literal(Parser *, Scanner *, HashTable *, bool);
static void binary(Parser *, Scanner *, HashTable *, bool);
static void unary(Parser *, Scanner *, HashTable *, bool);
static void number(Parser *, Scanner *, HashTable *, bool);
static void variable(Parser *, Scanner *, HashTable *, bool);
static void namedVar(Parser *, Scanner *, HashTable *, bool);

static ParseRule *getRule(TokenType);
static uint8_t makeConst(Value);
static void emitConst(Parser *, Value);
static Chunk *currentChunk();
static void emitBytes(uint8_t, uint8_t, Parser *);
static void emitByte(uint8_t, Parser *);
static void consume(Parser *, Scanner *, TokenType, const char *);
static void error(Parser *, const char *);
static void errorAt(Parser *, const char *);
static void errorAtCurrent(Parser *, const char *);
static void advance(Parser *, Scanner *);
static void emitReturn(Parser *);
static void endCompilation(Parser *);
static bool matchToken(Parser *, Scanner *, TokenType);
static void statement(Parser *, Scanner *, HashTable *);
static void declaration(Parser *, Scanner *, HashTable *);
static bool check(Parser *, TokenType);
static void printStatement(Parser *, Scanner *, HashTable *);
static void expressionStatement(Parser *, Scanner *, HashTable *);
static void synchronize(Parser *, Scanner *);
static void varDeclaration(Parser *, Scanner *, HashTable *);
static uint8_t parseVariable(Parser *, Scanner *, HashTable *, const char *);
static uint8_t indentifierConst(Token *, HashTable *);
static void defineVariable(Parser *, uint8_t);

#endif