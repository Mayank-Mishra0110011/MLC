#include "scanner.h"

void initScanner(Scanner *scanner, const char *source) {
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
}

Token scanToken(Scanner *scanner) {
  ignoreWhiteSpaceAndComments(scanner);
  scanner->start = scanner->current;
  if (isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);
  char c = advanceScanner(scanner);
  if (isAlpha(c)) return identifierToken(scanner);
  if (isDigit(c)) return numberToken(scanner);
  switch (c) {
    case '(':
      return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')':
      return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{':
      return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}':
      return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case ',':
      return makeToken(scanner, TOKEN_COMMA);
    case ';':
      return makeToken(scanner, TOKEN_SEMI);
    case '.':
      return makeToken(scanner, TOKEN_DOT);
    case '+':
      return makeToken(scanner, TOKEN_PLUS);
    case '%':
      return makeToken(scanner, TOKEN_MODULO);
    case '-':
      return makeToken(scanner, TOKEN_MINUS);
    case '*':
      return makeToken(scanner, TOKEN_STAR);
    case '/':
      return makeToken(scanner, TOKEN_SLASH);
    case '!':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '&':
      return makeToken(scanner, match(scanner, '&') ? TOKEN_LOGICAL_AND : TOKEN_BITWISE_AND);
    case '|':
      return makeToken(scanner, match(scanner, '|') ? TOKEN_LOGICAL_OR : TOKEN_BITWISE_OR);
    case '~':
      return makeToken(scanner, TOKEN_BITWISE_NOT);
    case '^':
      return makeToken(scanner, TOKEN_BITWISE_XOR);
    case '=':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '>':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : match(scanner, '>') ? TOKEN_RIGHT_SHIFT : TOKEN_GREATER);
    case '<':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : match(scanner, '<') ? TOKEN_LEFT_SHIFT : TOKEN_LESS);
    case '"':
      return stringToken(scanner);
  }
  return errorToken(scanner, "Unexpected character");
}

bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '@';
}

Token identifierToken(Scanner *scanner) {
  while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) advanceScanner(scanner);
  return makeToken(scanner, identifierType(scanner));
}

TokenType checkKeyword(Scanner *scanner, int start, int length, const char *rest, TokenType type) {
  if (scanner->current - scanner->start == start + length &&
      memcmp(scanner->start + start, rest, length) == 0) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

TokenType identifierType(Scanner *scanner) {
  switch (scanner->start[0]) {
    case 'a':
      return checkKeyword(scanner, 1, 2, "bs", TOKEN_ABS);
    case 'b':
      return checkKeyword(scanner, 1, 2, "rk", TOKEN_BRK);
    case 'c':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'l':
            return checkKeyword(scanner, 2, 3, "ass", TOKEN_CLASS);
          case 'a':
            switch (scanner->start[2]) {
              case 's':
                return checkKeyword(scanner, 3, 1, "e", TOKEN_CASE);
              case 't':
                return checkKeyword(scanner, 3, 2, "ch", TOKEN_CATCH);
            }
          case 'o':
            if (scanner->start[2] == 'n') {
              if (scanner->start[3] == 't') {
                return TOKEN_CONT;
              } else {
                return checkKeyword(scanner, 3, 2, "st", TOKEN_CONST);
              }
            }
        }
      }
      break;
    case 'd':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'o':
            return TOKEN_DO;
          case 'e':
            switch (scanner->start[2]) {
              case 'l':
                return checkKeyword(scanner, 3, 3, "ete", TOKEN_DELETE);
              case 'f':
                return checkKeyword(scanner, 3, 4, "ault", TOKEN_DEFAULT);
            }
        }
      }
      break;
    case 'E':
      if (scanner->current - scanner->start > 1) {
        return checkKeyword(scanner, 2, 8, "xception", TOKEN_EXCEPTION);
      }
      break;
    case 'e':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'l':
            switch (scanner->start[2]) {
              case 's':
                return checkKeyword(scanner, 3, 1, "e", TOKEN_ELSE);
              case 'i':
                return checkKeyword(scanner, 3, 1, "f", TOKEN_ELIF);
            }
          case 'n':
            return checkKeyword(scanner, 2, 2, "um", TOKEN_ENUM);
        }
      }
      break;
    case 'f':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'a':
            return checkKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
          case 'x':
            return TOKEN_FX;
          case 'r':
            return checkKeyword(scanner, 2, 2, "om", TOKEN_FROM);
          case 'i':
            if (scanner->start[2] == 'n' && scanner->start[3] == 'n' && scanner->start[4] == 'a' && scanner->start[5] == 'l') {
              if (scanner->start[6] == 'l' && scanner->start[7] == 'y') {
                return TOKEN_FINALYY;
              } else {
                return TOKEN_FINAL;
              }
            }
        }
      }
      break;
    case 'g':
      return checkKeyword(scanner, 1, 3, "oto", TOKEN_GOTO);
    case 'i':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'f':
            return checkKeyword(scanner, 2, 3, "ace", TOKEN_IFACE);
          case 'm':
            if (scanner->start[2] == 'p') {
              if (scanner->start[3] == 'l') {
                return TOKEN_IMPL;
              } else {
                return TOKEN_IMP;
              }
            }
          case 'n':
            return checkKeyword(scanner, 2, 8, "stanceOf", TOKEN_INSTANCE_OF);
        }
      }
      break;
    case 'm':
      return checkKeyword(scanner, 1, 4, "ixin", TOKEN_MIXIN);
    case 'n':
      if (scanner->current - scanner->start >= 1) {
        switch (scanner->start[1]) {
          case 'e':
            return checkKeyword(scanner, 2, 1, "w", TOKEN_NEW);
          case 'u':
            return checkKeyword(scanner, 2, 2, "ll", TOKEN_NULL);
        }
      }
      break;
    case 'O':
      if (scanner->current - scanner->start > 1) {
        return checkKeyword(scanner, 2, 5, "bject", TOKEN_OBJECT);
      }
      break;
    case 'p':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'u':
            return checkKeyword(scanner, 2, 1, "b", TOKEN_PUB);
          case 'r':
            switch (scanner->start[2]) {
              case 'i':
                if (scanner->start[3] == 'n') {
                  return checkKeyword(scanner, 4, 1, "t", TOKEN_PRINT);
                }
                return checkKeyword(scanner, 3, 1, "v", TOKEN_PRIV);
              case 'o':
                return checkKeyword(scanner, 3, 1, "t", TOKEN_PROT);
            }
        }
      }
      break;
    case 'r':
      return checkKeyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
    case 's':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'e':
            return checkKeyword(scanner, 2, 2, "lf", TOKEN_SELF);
          case 't':
            switch (scanner->start[2]) {
              case 'a':
                return checkKeyword(scanner, 3, 3, "tic", TOKEN_STATIC);
              case 'r':
                return checkKeyword(scanner, 3, 3, "uct", TOKEN_STRUCT);
            }
          case 'u':
            return checkKeyword(scanner, 2, 3, "per", TOKEN_SUPER);
          case 'w':
            return checkKeyword(scanner, 2, 4, "itch", TOKEN_SWITCH);
        }
      }
      break;
    case 't':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'h':
            if (scanner->start[2] == 'r' && scanner->start[3] == 'o' && scanner->start[4] == 'w') {
              if (scanner->start[5] == 's') {
                return TOKEN_THROWS;
              } else {
                return TOKEN_THROW;
              }
            }
            break;
          case 'o':
            return TOKEN_TO;
          case 'r':
            switch (scanner->start[2]) {
              case 'y':
                return TOKEN_TRY;
              case 'u':
                return checkKeyword(scanner, 3, 1, "e", TOKEN_TRUE);
            }
          case 'y':
            return checkKeyword(scanner, 2, 4, "peOf", TOKEN_TYPE_OF);
        }
      }
      break;
    case 'u':
      if (scanner->current - scanner->start > 1 && scanner->start[1] == 'n') {
        switch (scanner->start[2]) {
          case 's':
            return checkKeyword(scanner, 3, 3, "afe", TOKEN_UNSAFE);
          case 'i':
            return checkKeyword(scanner, 3, 2, "on", TOKEN_UNION);
        }
      }
    case 'v':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'a':
            return checkKeyword(scanner, 2, 1, "r", TOKEN_VAR);
          case 'r':
            return TOKEN_VR;
        }
      }
      break;
    case 'w':
      return checkKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
    case 'y':
      return checkKeyword(scanner, 1, 4, "ield", TOKEN_YIELD);
  }
  return TOKEN_IDENTIFIER;
}

Token stringToken(Scanner *scanner) {
  while (peek(scanner) != '"' && !isAtEnd(scanner)) {
    if (peek(scanner) == '\n') scanner->line++;
    advanceScanner(scanner);
  }
  if (isAtEnd(scanner)) return errorToken(scanner, "Unterminated string.");
  advanceScanner(scanner);
  return makeToken(scanner, TOKEN_STRING);
}

bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

Token numberToken(Scanner *scanner) {
  while (isDigit(peek(scanner))) advanceScanner(scanner);
  if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
    advanceScanner(scanner);
    while (isDigit(peek(scanner))) advanceScanner(scanner);
  }
  return makeToken(scanner, TOKEN_NUMBER);
}

void ignoreWhiteSpaceAndComments(Scanner *scanner) {
  while (true) {
    char c = peek(scanner);
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advanceScanner(scanner);
        break;
      case '\n':
        scanner->line++;
        advanceScanner(scanner);
        break;
      case '/':
        if (peekNext(scanner) == '#') {
          advanceScanner(scanner);
          advanceScanner(scanner);
          while (peek(scanner) != '#' && peekNext(scanner) != '/' && !isAtEnd(scanner)) {
            advanceScanner(scanner);
          }
        } else {
          return;
        }
        break;
      case '#':
        while (peek(scanner) != '\n' && !isAtEnd(scanner)) advanceScanner(scanner);
        break;
      default:
        return;
    }
  }
}

char peekNext(Scanner *scanner) {
  if (isAtEnd(scanner)) return '\0';
  return scanner->current[1];
}

char peek(Scanner *scanner) {
  return *(scanner->current);
}

char advanceScanner(Scanner *scanner) {
  scanner->current++;
  return scanner->current[-1];
}

bool match(Scanner *scanner, char expected) {
  if (isAtEnd(scanner)) return false;
  if (*(scanner->current) != expected) return false;
  scanner->current++;
  return true;
}

bool isAtEnd(Scanner *scanner) {
  return *(scanner->current) == '\0';
}

Token makeToken(Scanner *scanner, TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.length = (int)(scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}

Token errorToken(Scanner *scanner, const char *message) {
  Token token;
  token.type = TOKEN_ERR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner->line;
  return token;
}