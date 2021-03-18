#include "scanner.h"

void initScanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

void ignoreWhiteSpaceAndComments() {
  while (true) {
    char c = peek();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advanceScanner();
        break;
      case '\n':
        scanner.line++;
        advanceScanner();
        break;
      case '/':
        if (peekNext() == '#') {
          advanceScanner();
          advanceScanner();
          while (peek() != '#' && peekNext() != '/' && !isAtEnd()) {
            advanceScanner();
          }
        } else {
          return;
        }
        break;
      case '#':
        while (peek() != '\n' && !isAtEnd()) advanceScanner();
        break;
      default:
        return;
    }
  }
}

bool isAtEnd() {
  return *(scanner.current) == '\0';
}

bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '@';
}

bool match(char expected) {
  if (isAtEnd()) return false;
  if (*(scanner.current) != expected) return false;
  scanner.current++;
  return true;
}

Token scanToken() {
  ignoreWhiteSpaceAndComments();
  scanner.start = scanner.current;
  if (isAtEnd()) return makeToken(TOKEN_EOF);
  char c = advanceScanner();
  if (isAlpha(c)) return identifierToken();
  if (isDigit(c)) return numberToken();
  switch (c) {
    case '(':
      return makeToken(TOKEN_LEFT_PAREN);
    case ')':
      return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
      return makeToken(TOKEN_LEFT_BRACE);
    case '}':
      return makeToken(TOKEN_RIGHT_BRACE);
    case ',':
      return makeToken(TOKEN_COMMA);
    case ';':
      return makeToken(TOKEN_SEMI);
    case '.':
      return makeToken(TOKEN_DOT);
    case ':':
      return makeToken(TOKEN_LABEL);
    case '+':
      return makeToken(match('+') ? TOKEN_INCREMENT : TOKEN_PLUS);
    case '%':
      return makeToken(TOKEN_MODULO);
    case '-':
      return makeToken(match('-') ? TOKEN_DECREMENT : TOKEN_MINUS);
    case '*':
      return makeToken(TOKEN_STAR);
    case '/':
      return makeToken(TOKEN_SLASH);
    case '!':
      return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '&':
      return makeToken(match('&') ? TOKEN_LOGICAL_AND : TOKEN_BITWISE_AND);
    case '|':
      return makeToken(match('|') ? TOKEN_LOGICAL_OR : TOKEN_BITWISE_OR);
    case '~':
      return makeToken(TOKEN_BITWISE_NOT);
    case '^':
      return makeToken(TOKEN_BITWISE_XOR);
    case '=':
      return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '>':
      return makeToken(match('=') ? TOKEN_GREATER_EQUAL : match('>') ? TOKEN_RIGHT_SHIFT
                                                                     : TOKEN_GREATER);
    case '<':
      return makeToken(match('=') ? TOKEN_LESS_EQUAL : match('<') ? TOKEN_LEFT_SHIFT
                                                                  : TOKEN_LESS);
    case '"':
      return stringToken();
  }
  return errorToken("Unexpected character");
}

Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

Token errorToken(const char *message) {
  Token token;
  token.type = TOKEN_ERR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

Token stringToken() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') scanner.line++;
    advanceScanner();
  }
  if (isAtEnd()) return errorToken("Unterminated string.");
  advanceScanner();
  return makeToken(TOKEN_STRING);
}

Token numberToken() {
  while (isDigit(peek())) advanceScanner();
  if (peek() == '.' && isDigit(peekNext())) {
    advanceScanner();
    while (isDigit(peek())) advanceScanner();
  }
  return makeToken(TOKEN_NUMBER);
}

Token identifierToken() {
  while (isAlpha(peek()) || isDigit(peek())) advanceScanner();
  return makeToken(identifierType());
}

TokenType identifierType() {
  switch (scanner.start[0]) {
    case 'a':
      return checkKeyword(1, 2, "bs", TOKEN_ABS);
    case 'b':
      return checkKeyword(1, 2, "rk", TOKEN_BRK);
    case 'c':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'l':
            return checkKeyword(2, 3, "ass", TOKEN_CLASS);
          case 'a':
            switch (scanner.start[2]) {
              case 's':
                return checkKeyword(3, 1, "e", TOKEN_CASE);
              case 't':
                return checkKeyword(3, 2, "ch", TOKEN_CATCH);
            }
          case 'o':
            if (scanner.start[2] == 'n') {
              if (scanner.start[3] == 't') {
                return TOKEN_CONT;
              } else {
                return checkKeyword(3, 2, "st", TOKEN_CONST);
              }
            }
        }
      }
      break;
    case 'd':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'o':
            return TOKEN_DO;
          case 'e':
            switch (scanner.start[2]) {
              case 'l':
                return checkKeyword(3, 3, "ete", TOKEN_DELETE);
              case 'f':
                return TOKEN_DEFAULT;
            }
        }
      }
      break;
    case 'E':
      if (scanner.current - scanner.start > 1) {
        return checkKeyword(2, 8, "xception", TOKEN_EXCEPTION);
      }
      break;
    case 'e':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'l':
            return checkKeyword(2, 2, "se", TOKEN_ELSE);
          case 'n':
            return checkKeyword(2, 2, "um", TOKEN_ENUM);
        }
      }
      break;
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a':
            return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'x':
            return TOKEN_FX;
          case 'r':
            return checkKeyword(2, 2, "om", TOKEN_FROM);
          case 'i':
            if (scanner.start[2] == 'n' && scanner.start[3] == 'n' && scanner.start[4] == 'a' && scanner.start[5] == 'l') {
              if (scanner.start[6] == 'l' && scanner.start[7] == 'y') {
                return TOKEN_FINALYY;
              } else {
                return TOKEN_FINAL;
              }
            }
        }
      }
      break;
    case 'g':
      return checkKeyword(1, 3, "oto", TOKEN_GOTO);
    case 'i':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'f':
            if (scanner.start[2] == 'a') {
              return checkKeyword(3, 2, "ce", TOKEN_IFACE);
            } else {
              return checkKeyword(2, 0, "", TOKEN_IF);
            }
          case 'm':
            if (scanner.start[2] == 'p') {
              if (scanner.start[3] == 'l') {
                return TOKEN_IMPL;
              } else {
                return TOKEN_IMP;
              }
            }
          case 'n':
            return checkKeyword(2, 8, "stanceOf", TOKEN_INSTANCE_OF);
        }
      }
      break;
    case 'm':
      return checkKeyword(1, 4, "ixin", TOKEN_MIXIN);
    case 'n':
      if (scanner.current - scanner.start >= 1) {
        switch (scanner.start[1]) {
          case 'e':
            return checkKeyword(2, 1, "w", TOKEN_NEW);
          case 'u':
            return checkKeyword(2, 2, "ll", TOKEN_NULL);
        }
      }
      break;
    case 'O':
      if (scanner.current - scanner.start > 1) {
        return checkKeyword(2, 5, "bject", TOKEN_OBJECT);
      }
      break;
    case 'p':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'u':
            return checkKeyword(2, 1, "b", TOKEN_PUB);
          case 'r':
            switch (scanner.start[2]) {
              case 'i':
                if (scanner.start[3] == 'n') {
                  return checkKeyword(4, 1, "t", TOKEN_PRINT);
                }
                return checkKeyword(3, 1, "v", TOKEN_PRIV);
              case 'o':
                return checkKeyword(3, 1, "t", TOKEN_PROT);
            }
        }
      }
      break;
    case 'r':
      return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'e':
            return checkKeyword(2, 2, "lf", TOKEN_SELF);
          case 't':
            switch (scanner.start[2]) {
              case 'a':
                return checkKeyword(3, 3, "tic", TOKEN_STATIC);
              case 'r':
                return checkKeyword(3, 3, "uct", TOKEN_STRUCT);
            }
          case 'u':
            return checkKeyword(2, 3, "per", TOKEN_SUPER);
          case 'w':
            return checkKeyword(2, 4, "itch", TOKEN_SWITCH);
        }
      }
      break;
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h':
            if (scanner.start[2] == 'r' && scanner.start[3] == 'o' && scanner.start[4] == 'w') {
              if (scanner.start[5] == 's') {
                return TOKEN_THROWS;
              } else {
                return TOKEN_THROW;
              }
            }
            break;
          case 'r':
            switch (scanner.start[2]) {
              case 'y':
                return TOKEN_TRY;
              case 'u':
                return checkKeyword(3, 1, "e", TOKEN_TRUE);
            }
          case 'y':
            return checkKeyword(2, 4, "peOf", TOKEN_TYPE_OF);
        }
      }
      break;
    case 'u':
      if (scanner.current - scanner.start > 1 && scanner.start[1] == 'n') {
        switch (scanner.start[2]) {
          case 's':
            return checkKeyword(3, 3, "afe", TOKEN_UNSAFE);
          case 'i':
            return checkKeyword(3, 2, "on", TOKEN_UNION);
        }
      }
    case 'v':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a':
            return checkKeyword(2, 1, "r", TOKEN_VAR);
          case 'r':
            return TOKEN_VR;
        }
      }
      break;
    case 'w':
      return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    case 'y':
      return checkKeyword(1, 4, "ield", TOKEN_YIELD);
  }
  return TOKEN_IDENTIFIER;
}

TokenType checkKeyword(int start, int length, const char *rest, TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

char advanceScanner() {
  scanner.current++;
  return scanner.current[-1];
}

char peek() {
  return *(scanner.current);
}

char peekNext() {
  if (isAtEnd()) return '\0';
  return scanner.current[1];
}