#ifndef MLC_COMPILER_H
#define MLC_COMPILER_H

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "scanner.h"
#include "vm.h"

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

typedef void (*ParseFn)();

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence prec;
} ParseRule;

static Chunk *compilingChunk;

bool compile(const char *, Chunk *);
static void advance(Parser *, Scanner *);
static void consume(Parser *, Scanner *, TokenType, const char *);
static Chunk *currentChunk();
static void emitByte(uint8_t, Parser *);
static void emitBytes(uint8_t, uint8_t, Parser *);
static void errorAtCurrent(Parser *, const char *);
static void errorAt(Parser *, const char *);
static void error(Parser *, const char *);
static void endCompilation(Parser *);
static void expression(Parser *, Scanner *);
static void emitReturn(Parser *);
static void number(Parser *, Scanner *);
static void emitConst(Parser *, Value);
static uint8_t makeConst(Value);
static void grouping(Parser *, Scanner *);
static void unary(Parser *, Scanner *);
static void parsePrecedence(Parser *, Scanner *, Precedence);
static void binary(Parser *, Scanner *);
static ParseRule *getRule(TokenType);

#endif