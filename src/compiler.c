#include "compiler.h"

bool compile(const char *source, Chunk *chunk) {
  Scanner scanner;
  Parser parser;
  initScanner(&scanner, source);
  compilingChunk = chunk;
  parser.hadErr = false;
  parser.panic = false;
  advance(&parser, &scanner);
  expression(&parser, &scanner);
  consume(&parser, &scanner, TOKEN_EOF, "Expect end of expression");
  endCompilation(&parser);
  return !parser.hadErr;
}

void expression(Parser *parser, Scanner *scanner) {
  parsePrecedence(parser, scanner, PRE_ASSIGN);
}

void endCompilation(Parser *parser) {
  emitReturn(parser);
#ifdef DEBUG_PRINT_CODE
  if (!parser->hadErr) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

void emitReturn(Parser *parser) {
  emitByte(OP_RETURN, parser);
}

void advance(Parser *parser, Scanner *scanner) {
  parser->prev = parser->cur;
  while (true) {
    parser->cur = scanToken(scanner);
    if (parser->cur.type != TOKEN_ERR) break;
    errorAtCurrent(parser, parser->cur.start);
  }
}

void errorAtCurrent(Parser *parser, const char *message) {
  errorAt(parser, message);
}

void errorAt(Parser *parser, const char *message) {
  if (parser->panic) return;  //this is where you'd throw an exception but not yet
  Token token = parser->prev;
  parser->panic = true;
  fprintf(stderr, "[line %d] Error", token.line);
  if (token.type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token.type == TOKEN_ERR) {
    //nothing for now
  } else {
    fprintf(stderr, " at '%.*s'", token.length, token.start);
  }
  fprintf(stderr, " : %s\n", message);
  parser->hadErr = true;
}

void error(Parser *parser, const char *message) {
  errorAt(parser, message);
}

void consume(Parser *parser, Scanner *scanner, TokenType type, const char *message) {
  if (parser->cur.type == type) {
    advance(parser, scanner);
    return;
  }
  errorAtCurrent(parser, message);
}

void emitByte(uint8_t byte, Parser *parser) {
  writeChunk(currentChunk(), byte, parser->prev.line);
}

void emitBytes(uint8_t b1, uint8_t b2, Parser *parser) {
  emitByte(b1, parser);
  emitByte(b2, parser);
}

Chunk *currentChunk() {
  return compilingChunk;
}

void number(Parser *parser, Scanner *scanner) {
  double value = strtod(parser->prev.start, NULL);
  emitConst(parser, value);
}

void unary(Parser *parser, Scanner *scanner) {
  parsePrecedence(parser, scanner, PRE_UNARY);
  TokenType operator= parser->prev.type;
  expression(parser, scanner);
  switch (operator) {
    case TOKEN_MINUS:
      emitByte(OP_NEGATE, parser);
      break;
  }
}

void binary(Parser *parser, Scanner *scanner) {
  TokenType operator= parser->prev.type;
  ParseRule *rule = getRule(operator);
  parsePrecedence(parser, scanner, (Precedence)(rule->prec + 1));
  switch (operator) {
    case TOKEN_PLUS:
      emitByte(OP_ADD, parser);
      break;
    case TOKEN_MINUS:
      emitByte(OP_SUBTRACT, parser);
      break;
    case TOKEN_STAR:
      emitByte(OP_MULTIPLY, parser);
      break;
    case TOKEN_SLASH:
      emitByte(OP_DIVIDE, parser);
      break;
  }
}

void emitConst(Parser *parser, Value value) {
  emitBytes(OP_CONST, makeConst(value), parser);
}

uint8_t makeConst(Value value) {
  int constant = addConst(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error(NULL, "Too many constants in one chunk");
    return 0;
  }
  return (uint8_t)constant;
}

void grouping(Parser *parser, Scanner *scanner) {
  expression(parser, scanner);
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

void parsePrecedence(Parser *parser, Scanner *scanner, Precedence precedence) {
  advance(parser, scanner);
  ParseFn prefixRule = getRule(parser->prev.type)->prefix;
  if (prefixRule == NULL) {
    error(parser, "Expected expression");
    return;
  }
  prefixRule(parser, scanner);
  while (precedence <= getRule(parser->cur.type)->prec) {
    advance(parser, scanner);
    ParseFn infixRule = getRule(parser->cur.type)->infix;
    infixRule(parser, scanner);
  }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PRE_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PRE_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PRE_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PRE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRE_NONE},
    [TOKEN_DOT] = {NULL, NULL, PRE_NONE},
    [TOKEN_MINUS] = {unary, binary, PRE_NONE},
    [TOKEN_PLUS] = {NULL, binary, PRE_NONE},
    [TOKEN_SEMI] = {NULL, NULL, PRE_NONE},
    [TOKEN_STAR] = {NULL, binary, PRE_FACTOR},
    [TOKEN_SLASH] = {NULL, binary, PRE_FACTOR},
    [TOKEN_BANG] = {NULL, NULL, PRE_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_EQUAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_GREATER] = {NULL, NULL, PRE_NONE},
    [TOKEN_GREATER_EQUAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_LESS] = {NULL, NULL, PRE_NONE},
    [TOKEN_LESS_EQUAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_LOGICAL_AND] = {NULL, NULL, PRE_NONE},
    [TOKEN_LOGICAL_OR] = {NULL, NULL, PRE_NONE},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PRE_NONE},
    [TOKEN_STRING] = {NULL, NULL, PRE_NONE},
    [TOKEN_NUMBER] = {number, NULL, PRE_NONE},

    [TOKEN_CONST] = {NULL, NULL, PRE_NONE},
    [TOKEN_ENUM] = {NULL, NULL, PRE_NONE},
    [TOKEN_VAR] = {NULL, NULL, PRE_NONE},
    [TOKEN_NULL] = {NULL, NULL, PRE_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PRE_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PRE_NONE},
    [TOKEN_SELF] = {NULL, NULL, PRE_NONE},
    [TOKEN_FX] = {NULL, NULL, PRE_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PRE_NONE},
    [TOKEN_TRUE] = {NULL, NULL, PRE_NONE},
    [TOKEN_FALSE] = {NULL, NULL, PRE_NONE},
    [TOKEN_FROM] = {NULL, NULL, PRE_NONE},
    [TOKEN_TO] = {NULL, NULL, PRE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRE_NONE},
    [TOKEN_DO] = {NULL, NULL, PRE_NONE},
    [TOKEN_IF] = {NULL, NULL, PRE_NONE},
    [TOKEN_ELIF] = {NULL, NULL, PRE_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PRE_NONE},
    [TOKEN_SWITCH] = {NULL, NULL, PRE_NONE},
    [TOKEN_CASE] = {NULL, NULL, PRE_NONE},
    [TOKEN_DEFAULT] = {NULL, NULL, PRE_NONE},
    [TOKEN_TRY] = {NULL, NULL, PRE_NONE},
    [TOKEN_CATCH] = {NULL, NULL, PRE_NONE},
    [TOKEN_FINALYY] = {NULL, NULL, PRE_NONE},
    [TOKEN_EXP] = {NULL, NULL, PRE_NONE},
    [TOKEN_IMP] = {NULL, NULL, PRE_NONE},
    [TOKEN_THROW] = {NULL, NULL, PRE_NONE},
    [TOKEN_THROWS] = {NULL, NULL, PRE_NONE},
    [TOKEN_YIELD] = {NULL, NULL, PRE_NONE},
    [TOKEN_BRK] = {NULL, NULL, PRE_NONE},
    [TOKEN_CONT] = {NULL, NULL, PRE_NONE},
    [TOKEN_MIXIN] = {NULL, NULL, PRE_NONE},
    [TOKEN_STRUCT] = {NULL, NULL, PRE_NONE},
    [TOKEN_OBJECT] = {NULL, NULL, PRE_NONE},
    [TOKEN_EXCEPTION] = {NULL, NULL, PRE_NONE},
    [TOKEN_UNION] = {NULL, NULL, PRE_NONE},
    [TOKEN_IFACE] = {NULL, NULL, PRE_NONE},
    [TOKEN_EXT] = {NULL, NULL, PRE_NONE},
    [TOKEN_FINAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_VR] = {NULL, NULL, PRE_NONE},
    [TOKEN_ABS] = {NULL, NULL, PRE_NONE},
    [TOKEN_PUB] = {NULL, NULL, PRE_NONE},
    [TOKEN_PRIV] = {NULL, NULL, PRE_NONE},
    [TOKEN_PROT] = {NULL, NULL, PRE_NONE},
    [TOKEN_IMPL] = {NULL, NULL, PRE_NONE},
    [TOKEN_STATIC] = {NULL, NULL, PRE_NONE},
    [TOKEN_UNSAFE] = {NULL, NULL, PRE_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PRE_NONE},
    [TOKEN_GOTO] = {NULL, NULL, PRE_NONE},
    [TOKEN_TYPE_OF] = {NULL, NULL, PRE_NONE},
    [TOKEN_INSTANCE_OF] = {NULL, NULL, PRE_NONE},
    [TOKEN_NEW] = {NULL, NULL, PRE_NONE},
    [TOKEN_DELETE] = {NULL, NULL, PRE_NONE},
    [TOKEN_LOGICAL_NOT] = {NULL, NULL, PRE_NONE},
    [TOKEN_BITWISE_AND] = {NULL, NULL, PRE_NONE},
    [TOKEN_BITWISE_OR] = {NULL, NULL, PRE_NONE},
    [TOKEN_BITWISE_NOT] = {NULL, NULL, PRE_NONE},
    [TOKEN_BITWISE_XOR] = {NULL, NULL, PRE_NONE},
    [TOKEN_LEFT_SHIFT] = {NULL, NULL, PRE_NONE},
    [TOKEN_RIGHT_SHIFT] = {NULL, NULL, PRE_NONE},
    [TOKEN_ERR] = {NULL, NULL, PRE_NONE},
    [TOKEN_EOF] = {NULL, NULL, PRE_NONE},
};

ParseRule *getRule(TokenType type) {
  return &rules[type];
}
