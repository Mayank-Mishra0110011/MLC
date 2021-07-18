#ifndef MLC_COMPILER_H
#define MLC_COMPILER_H

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

static Compiler *current = NULL;

FunctionObject *compile(const char *);

static FunctionObject *endCompilation();

void markCompilerRoots();

static void initCompiler(Compiler *, FunctionType);
static void synchronize();
static void varDeclaration();
static void defineVariable(uint8_t);
static void variable(bool);
static void namedVar(bool);
static void block();
static void beginScope();
static void declareLocalVar();
static void endScope();
static void markInitialized();
static void backpatchJump(int);
static void ifStatement();
static void logicalAnd(bool);
static void logicalOr(bool);
static void emitLoop(int);
static void whileStatement();
static void fromStatement();
static void switchStatement();
static void function(FunctionType);
static void functionDeclaration();
static void classDeclaration();
static void returnStatement();
static void call(bool);
static void incOrDec(bool);
static void cont(bool);
static void _brk(bool);
static void expressionStatement();
static void printStatement();
static void statement();
static void declaration();
static void expression();
static void emitReturn();
static void advance();
static void errorAtCurrent(const char *);
static void errorAt(const char *);
static void error(const char *);
static void consume(TokenType, const char *);
static void emitByte(uint8_t);
static void emitBytes(uint8_t, uint8_t);
static void number(bool);
static void unary(bool);
static void binary(bool);
static void literal(bool);
static void emitConst(Value);
static void grouping(bool);
static void parsePrecedence(Precedence);
static void string(bool);

static uint8_t argList();
static uint8_t parseVariable(const char *);
static uint8_t makeConst(Value);
static uint8_t identifierConst(Token *);

static int emitJump(uint8_t);
static int addUpvalue(Compiler *, uint8_t, bool);
static int resolveUpvalue(Compiler *);
static int resolveLocal(Compiler *);

static bool identifiersEqual(Token *, Token *);
static bool matchToken(TokenType);
static bool check(TokenType);

static Chunk *currentChunk();

static ParseRule *getRule(TokenType);

#endif