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
  TOKEN_SEMI,         // 8
  TOKEN_STAR,         // 9
  TOKEN_TYPE_OF,      // 10
  TOKEN_INSTANCE_OF,  // 11
  TOKEN_NEW,          // 12
  TOKEN_DELETE,       // 13

  TOKEN_SLASH,          // 14
  TOKEN_BANG,           // 15
  TOKEN_BANG_EQUAL,     // 16
  TOKEN_EQUAL,          // 17
  TOKEN_EQUAL_EQUAL,    // 18
  TOKEN_GREATER,        // 19
  TOKEN_GREATER_EQUAL,  // 20
  TOKEN_LESS,           // 21
  TOKEN_LESS_EQUAL,     // 22
  TOKEN_LOGICAL_AND,    // 23
  TOKEN_LOGICAL_OR,     // 24
  TOKEN_LOGICAL_NOT,    // 25
  TOKEN_BITWISE_AND,    // 26
  TOKEN_BITWISE_OR,     // 27
  TOKEN_BITWISE_NOT,    // 28
  TOKEN_BITWISE_XOR,    // 29
  TOKEN_LEFT_SHIFT,     // 30
  TOKEN_RIGHT_SHIFT,    // 31

  TOKEN_IDENTIFIER,  // 32
  TOKEN_STRING,      // 33
  TOKEN_NUMBER,      // 34
  TOKEN_CONST,       // 35
  TOKEN_ENUM,        // 36

  TOKEN_IF,         // 37
  TOKEN_ELIF,       // 38
  TOKEN_ELSE,       // 39
  TOKEN_SWITCH,     // 40
  TOKEN_CASE,       // 41
  TOKEN_DEFAULT,    // 42
  TOKEN_TRY,        // 43
  TOKEN_CATCH,      // 44
  TOKEN_FINALYY,    // 45
  TOKEN_EXP,        // 46
  TOKEN_IMP,        // 47
  TOKEN_TRUE,       // 48
  TOKEN_FALSE,      // 49
  TOKEN_FX,         // 50
  TOKEN_DO,         // 51
  TOKEN_WHILE,      // 52
  TOKEN_FROM,       // 53
  TOKEN_TO,         // 54
  TOKEN_RETURN,     // 55
  TOKEN_GOTO,       // 56
  TOKEN_THROW,      // 57
  TOKEN_THROWS,     // 58
  TOKEN_YIELD,      // 59
  TOKEN_BRK,        // 60
  TOKEN_CONT,       // 61
  TOKEN_MIXIN,      // 62
  TOKEN_STRUCT,     // 63
  TOKEN_OBJECT,     // 64
  TOKEN_EXCEPTION,  // 65
  TOKEN_UNION,      // 66
  TOKEN_CLASS,      // 67
  TOKEN_IFACE,      // 68
  TOKEN_EXT,        // 69
  TOKEN_FINAL,      // 70
  TOKEN_VR,         // 71
  TOKEN_ABS,        // 72
  TOKEN_PUB,        // 73
  TOKEN_PRIV,       // 74
  TOKEN_PROT,       // 75
  TOKEN_SUPER,      // 76
  TOKEN_IMPL,       // 77
  TOKEN_SELF,       // 78
  TOKEN_NULL,       // 79
  TOKEN_VAR,        // 80
  TOKEN_STATIC,     // 81
  TOKEN_UNSAFE,     // 82
  TOKEN_PRINT,      // 83

  TOKEN_EOF,  // 84
  TOKEN_ERR   // 85
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