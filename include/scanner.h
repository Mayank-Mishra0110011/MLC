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
  TOKEN_MODULO,       // 8
  TOKEN_SEMI,         // 9
  TOKEN_STAR,         // 10
  TOKEN_TYPE_OF,      // 11
  TOKEN_INSTANCE_OF,  // 12
  TOKEN_NEW,          // 13
  TOKEN_DELETE,       // 14

  TOKEN_SLASH,          // 15
  TOKEN_BANG,           // 16
  TOKEN_BANG_EQUAL,     // 17
  TOKEN_EQUAL,          // 18
  TOKEN_EQUAL_EQUAL,    // 19
  TOKEN_GREATER,        // 20
  TOKEN_GREATER_EQUAL,  // 21
  TOKEN_LESS,           // 22
  TOKEN_LESS_EQUAL,     // 23
  TOKEN_LOGICAL_AND,    // 24
  TOKEN_LOGICAL_OR,     // 25
  TOKEN_LOGICAL_NOT,    // 26
  TOKEN_BITWISE_AND,    // 27
  TOKEN_BITWISE_OR,     // 28
  TOKEN_BITWISE_NOT,    // 29
  TOKEN_BITWISE_XOR,    // 30
  TOKEN_LEFT_SHIFT,     // 31
  TOKEN_RIGHT_SHIFT,    // 32

  TOKEN_IDENTIFIER,  // 33
  TOKEN_STRING,      // 34
  TOKEN_NUMBER,      // 35
  TOKEN_CONST,       // 36
  TOKEN_ENUM,        // 37

  TOKEN_IF,         // 38
  TOKEN_ELIF,       // 39
  TOKEN_ELSE,       // 40
  TOKEN_SWITCH,     // 41
  TOKEN_CASE,       // 42
  TOKEN_DEFAULT,    // 43
  TOKEN_TRY,        // 44
  TOKEN_CATCH,      // 45
  TOKEN_FINALYY,    // 46
  TOKEN_EXP,        // 47
  TOKEN_IMP,        // 48
  TOKEN_TRUE,       // 49
  TOKEN_FALSE,      // 50
  TOKEN_FX,         // 51
  TOKEN_DO,         // 52
  TOKEN_WHILE,      // 53
  TOKEN_FROM,       // 54
  TOKEN_TO,         // 55
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
  TOKEN_PRINT,      // 84

  TOKEN_EOF,  // 85
  TOKEN_ERR   // 86
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