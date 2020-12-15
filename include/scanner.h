#ifndef MLC_SCANNER_H
#define MLC_SCANNER_H

#include <stdio.h>
#include <string.h>

#include "common.h"

typedef enum {
  TOKEN_LEFT_PAREN,   // 0
  TOKEN_RIGHT_PAREN,  // 1
  TOKEN_LEFT_BRACE,   // 2
  TOKEN_RIGHT_BRACE,  // 3
  TOKEN_COMMA,        // 4
  TOKEN_DOT,          // 5
  TOKEN_MINUS,        // 6
  TOKEN_PLUS,         // 7
  TOKEN_INCREMENT,    // 8
  TOKEN_DECREMENT,    // 9

  TOKEN_MODULO,       // 10
  TOKEN_SEMI,         // 11
  TOKEN_STAR,         // 12
  TOKEN_TYPE_OF,      // 13
  TOKEN_INSTANCE_OF,  // 14
  TOKEN_NEW,          // 15
  TOKEN_DELETE,       // 16

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

  TOKEN_IDENTIFIER,  // 35
  TOKEN_STRING,      // 36
  TOKEN_NUMBER,      // 37
  TOKEN_CONST,       // 38
  TOKEN_ENUM,        // 39

  TOKEN_IF,         // 40
  TOKEN_ELSE,       // 41
  TOKEN_SWITCH,     // 42
  TOKEN_CASE,       // 43
  TOKEN_DEFAULT,    // 44
  TOKEN_TRY,        // 45
  TOKEN_CATCH,      // 46
  TOKEN_FINALYY,    // 47
  TOKEN_EXP,        // 48
  TOKEN_IMP,        // 49
  TOKEN_TRUE,       // 50
  TOKEN_FALSE,      // 51
  TOKEN_FX,         // 52
  TOKEN_DO,         // 53
  TOKEN_WHILE,      // 54
  TOKEN_FROM,       // 55
  TOKEN_RETURN,     // 56
  TOKEN_GOTO,       // 57
  TOKEN_THROW,      // 58
  TOKEN_THROWS,     // 59
  TOKEN_YIELD,      // 60
  TOKEN_BRK,        // 61
  TOKEN_CONT,       // 62
  TOKEN_MIXIN,      // 63
  TOKEN_STRUCT,     // 64
  TOKEN_OBJECT,     // 65
  TOKEN_EXCEPTION,  // 66
  TOKEN_UNION,      // 67
  TOKEN_CLASS,      // 68
  TOKEN_IFACE,      // 69
  TOKEN_EXT,        // 70
  TOKEN_FINAL,      // 71
  TOKEN_VR,         // 72
  TOKEN_ABS,        // 73
  TOKEN_PUB,        // 74
  TOKEN_PRIV,       // 75
  TOKEN_PROT,       // 76
  TOKEN_SUPER,      // 77
  TOKEN_IMPL,       // 78
  TOKEN_SELF,       // 79
  TOKEN_NULL,       // 80
  TOKEN_VAR,        // 81
  TOKEN_STATIC,     // 82
  TOKEN_UNSAFE,     // 83
  TOKEN_LABEL,      // 84
  TOKEN_PRINT,      // 85

  TOKEN_EOF,  // 86
  TOKEN_ERR   // 87
} TokenType;

typedef struct {
  TokenType type;
  const char *start;
  int length;
  int line;
} Token;

typedef struct {
  const char *start;
  const char *current;
  int line;
} Scanner;

void initScanner(Scanner *, const char *);
Token scanToken(Scanner *);

static bool isAlpha(char);
static Token identifierToken(Scanner *);
static TokenType checkKeyword(Scanner *, int, int, const char *, TokenType);
static TokenType identifierType(Scanner *);
static Token stringToken(Scanner *);
static bool isDigit(char c);
static Token numberToken(Scanner *);
static void ignoreWhiteSpaceAndComments(Scanner *);
static char peekNext(Scanner *);
static char peek(Scanner *);
static char advanceScanner(Scanner *);
static bool match(Scanner *, char);
static bool isAtEnd(Scanner *);
static Token makeToken(Scanner *, TokenType);
static Token errorToken(Scanner *, const char *);

#endif