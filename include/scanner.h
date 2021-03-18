#ifndef MLC_SCANNER_H
#define MLC_SCANNER_H

#include "common.h"

void initScanner(const char *);

static void ignoreWhiteSpaceAndComments();

static bool isAtEnd();
static bool isDigit(char);
static bool isAlpha(char);
static bool match(char);

Token scanToken();

static Token makeToken(TokenType);
static Token errorToken(const char *);
static Token stringToken();
static Token numberToken();
static Token identifierToken();

static TokenType identifierType();
static TokenType checkKeyword(int, int, const char *, TokenType);

static char advanceScanner();
static char peek();
static char peekNext();

#endif