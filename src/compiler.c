#include "compiler.h"

bool compile(const char *source, Chunk *chunk, HashTable *hash) {
  Scanner scanner;
  Parser parser;
  Compiler compiler;
  initScanner(&scanner, source);
  initCompiler(&compiler);
  compilingChunk = chunk;
  parser.hadErr = false;
  parser.panic = false;
  advance(&parser, &scanner);
  while (!matchToken(&parser, &scanner, TOKEN_EOF)) {
    declaration(&parser, &scanner, hash);
  }
  endCompilation(&parser);
  return !parser.hadErr;
}

void initCompiler(Compiler *compiler) {
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  compiler->labelCount = 0;
  current = compiler;
}

bool check(Parser *parser, TokenType type) {
  return parser->cur.type == type;
}

void statement(Parser *parser, Scanner *scanner, HashTable *hash) {
  if (matchToken(parser, scanner, TOKEN_PRINT)) {
    printStatement(parser, scanner, hash);
  } else if (matchToken(parser, scanner, TOKEN_IF)) {
    ifStatement(parser, scanner, hash);
  } else if (matchToken(parser, scanner, TOKEN_WHILE)) {
    whileStatement(parser, scanner, hash);
  } else if (matchToken(parser, scanner, TOKEN_FROM)) {
    fromStatement(parser, scanner, hash);
  } else if (matchToken(parser, scanner, TOKEN_SWITCH)) {
    switchStatement(parser, scanner, hash);
  } else if (matchToken(parser, scanner, TOKEN_LEFT_BRACE)) {
    beginScope();
    block(parser, scanner, hash);
    endScope(parser);
  } else {
    expressionStatement(parser, scanner, hash);
  }
}

void beginScope() {
  current->scopeDepth++;
}

void block(Parser *parser, Scanner *scanner, HashTable *hash) {
  while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
    declaration(parser, scanner, hash);
  }
  consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expected '}' after block");
}

void endScope(Parser *parser) {
  current->scopeDepth--;
  while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
    emitByte(OP_POP, parser);
    current->localCount--;
  }
}

void expressionStatement(Parser *parser, Scanner *scanner, HashTable *hash) {
  expression(parser, scanner, hash);
  consume(parser, scanner, TOKEN_SEMI, "Expected ';' after value");
  emitByte(OP_POP, parser);
}

void synchronize(Parser *parser, Scanner *scanner) {
  parser->panic = false;
  while (parser->cur.type) {
    if (parser->prev.type == TOKEN_SEMI) return;
    switch (parser->cur.type) {
      case TOKEN_CLASS:
      case TOKEN_FX:
      case TOKEN_VAR:
      case TOKEN_FROM:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
      default:
        return;
    }
    printf("END %d, %d\n", parser->cur.type, parser->prev.type);
    advance(parser, scanner);
  }
}

void declaration(Parser *parser, Scanner *scanner, HashTable *hash) {
  if (matchToken(parser, scanner, TOKEN_VAR)) {
    varDeclaration(parser, scanner, hash);
  } else {
    statement(parser, scanner, hash);
  }
  if (parser->panic) synchronize(parser, scanner);
}

uint8_t parseVariable(Parser *parser, Scanner *scanner, HashTable *hash, const char *err) {
  consume(parser, scanner, TOKEN_IDENTIFIER, err);
  declareLocalVar(parser);
  if (current->scopeDepth > 0) return 0;
  return indentifierConst(&parser->prev, hash);
}

bool indentifierEqual(Token *tkn1, Token *tkn2) {
  if (tkn1->length != tkn2->length) return false;
  return memcmp(tkn1->start, tkn2->start, tkn1->length) == 0;
}

void declareLocalVar(Parser *parser) {
  if (current->scopeDepth == 0) return;
  Local *local;
  for (int i = current->localCount - 1; i >= 0; i--) {
    local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break;
    }
    if (indentifierEqual(&parser->prev, &local->name)) {
      error(parser, "Identifier has already been declared.");
    }
  }
  if (current->localCount == UINT8_COUNT) {
    error(parser, "Stack Overflow.");
    return;
  }
  local = &current->locals[current->localCount++];
  local->name = parser->prev;
  local->depth = -1;
}

uint8_t indentifierConst(Token *name, HashTable *hash) {
  return makeConst(TO_OBJECT(copyString(name->start, name->length, hash)));
}

void markInitialized() {
  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

void defineVariable(Parser *parser, uint8_t global) {
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_DEFINE_GLOBAL, global, parser);
}

void variable(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  namedVar(parser, scanner, hash, canAssign);
}

void namedVar(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(parser, current, &parser->prev);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = indentifierConst(&parser->prev, hash);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }
  if (canAssign && matchToken(parser, scanner, TOKEN_EQUAL)) {
    expression(parser, scanner, hash);
    emitBytes(setOp, (uint8_t)arg, parser);
  } else {
    emitBytes(getOp, (uint8_t)arg, parser);
    if (parser->cur.type == TOKEN_INCREMENT || parser->cur.type == TOKEN_DECREMENT) {
      emitConst(parser, TO_NUMBER(1));
      emitByte(parser->cur.type == TOKEN_INCREMENT ? OP_ADD : OP_SUBTRACT, parser);
      emitBytes(setOp, (uint8_t)arg, parser);
      advance(parser, scanner);
    }
  }
}

void backpatchJump(Parser *parser, int offset) {
  int jmp = currentChunk()->count - offset - 2;
  if (jmp > UINT16_MAX) error(parser, "Stack Overflow.");
  currentChunk()->code[offset] = (jmp >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jmp & 0xff;
}

void emitLoop(int loopStart, Parser *parser) {
  emitByte(OP_LOOP, parser);
  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX) error(parser, "Stack Overflow.");
  emitBytes((offset >> 8) & 0xff, offset & 0xff, parser);
}

/*
  if none of the cases match it should jump into the def case
  however, for now if no cases match AND a def case exists, the 
  program seg faults.
  since def case can be anywhere in the switch stmt we should
  remember it's jmp offset.
  fix it later.
*/
void switchStatement(Parser *parser, Scanner *scanner, HashTable *hash) {
  beginScope();
  expression(parser, scanner, hash);
  emitByte(OP_SWITCH_START, parser);
  consume(parser, scanner, TOKEN_LEFT_BRACE, "Expected '{' after switch expression");
  int exitJmp = -1;
  while (matchToken(parser, scanner, TOKEN_CASE) || check(parser, TOKEN_DEFAULT)) {
    if (!check(parser, TOKEN_DEFAULT)) {
      expression(parser, scanner, hash);
      consume(parser, scanner, TOKEN_LABEL, "Expected ':' after case expression");
    } else {
      matchToken(parser, scanner, TOKEN_DEFAULT);
      consume(parser, scanner, TOKEN_LABEL, "Expected ':' after def");
    }
    emitByte(OP_CASE, parser);
    exitJmp = emitJump(OP_JMP_IF_FALSE, parser);
    emitByte(OP_POP, parser);
    while (!check(parser, TOKEN_CASE) && !check(parser, TOKEN_DEFAULT) && !check(parser, TOKEN_RIGHT_BRACE))
      declaration(parser, scanner, hash);
    if (!check(parser, TOKEN_DEFAULT)) backpatchJump(parser, exitJmp);
    if (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_DEFAULT)) emitByte(OP_POP, parser);
  }
  consume(parser, scanner, TOKEN_RIGHT_BRACE, "Expected '}' at the end of switch statement");
  emitByte(OP_SWITCH_END, parser);
  endScope(parser);
}

void fromStatement(Parser *parser, Scanner *scanner, HashTable *hash) {
  beginScope();
  if (matchToken(parser, scanner, TOKEN_SEMI)) {
  } else if (matchToken(parser, scanner, TOKEN_VAR)) {
    varDeclaration(parser, scanner, hash);
  } else {
    expressionStatement(parser, scanner, hash);
  }
  int curLabelCount = current->labelCount;
  int loopStart = currentChunk()->count;
  int exitJmp = -1;
  if (!matchToken(parser, scanner, TOKEN_SEMI)) {
    expression(parser, scanner, hash);
    consume(parser, scanner, TOKEN_SEMI, "Expected ';' after loop condition");
    exitJmp = emitJump(OP_JMP_IF_FALSE, parser);
    current->labels[current->labelCount++] = exitJmp;
    emitByte(OP_POP, parser);
  } else {
    exitJmp = emitJump(OP_JMP_IF_FALSE, parser);
    current->labels[current->labelCount++] = exitJmp;
    current->labels[current->labelCount++] = exitJmp;
  }
  if (!check(parser, TOKEN_LEFT_BRACE)) {
    int bodyJmp = emitJump(OP_JMP, parser);
    int incrementStart = currentChunk()->count;
    if (!matchToken(parser, scanner, TOKEN_LEFT_BRACE)) {
      expression(parser, scanner, hash);
    }
    emitByte(OP_POP, parser);
    emitLoop(loopStart, parser);
    loopStart = incrementStart;
    current->labels[current->labelCount++] = loopStart;
    backpatchJump(parser, bodyJmp);
  }
  statement(parser, scanner, hash);
  emitLoop(loopStart, parser);
  if (exitJmp != -1) {
    backpatchJump(parser, exitJmp);
    emitByte(OP_POP, parser);
  }
  endScope(parser);
  if (curLabelCount != current->labelCount) current->labelCount = curLabelCount;
}

void whileStatement(Parser *parser, Scanner *scanner, HashTable *hash) {
  int loopStart = currentChunk()->count;
  int curLabelCount = current->labelCount;
  expression(parser, scanner, hash);
  int exitJmpOffset = emitJump(OP_JMP_IF_FALSE, parser);
  current->labels[current->labelCount++] = exitJmpOffset;
  current->labels[current->labelCount++] = loopStart;
  emitByte(OP_POP, parser);
  statement(parser, scanner, hash);
  emitLoop(loopStart, parser);
  backpatchJump(parser, exitJmpOffset);
  emitByte(OP_POP, parser);
  if (curLabelCount != current->labelCount) current->labelCount = curLabelCount;
}

void ifStatement(Parser *parser, Scanner *scanner, HashTable *hash) {
  expression(parser, scanner, hash);
  int ifJumpOffset = emitJump(OP_JMP_IF_FALSE, parser);
  emitByte(OP_POP, parser);
  statement(parser, scanner, hash);
  int elseJumpOffset = emitJump(OP_JMP, parser);
  backpatchJump(parser, ifJumpOffset);
  emitByte(OP_POP, parser);
  if (matchToken(parser, scanner, TOKEN_ELSE)) statement(parser, scanner, hash);
  backpatchJump(parser, elseJumpOffset);
}

int emitJump(uint8_t instr, Parser *parser) {
  emitByte(instr, parser);
  emitByte(0xff, parser);
  emitByte(0xff, parser);
  return currentChunk()->count - 2;
}

int resolveLocal(Parser *parser, Compiler *compiler, Token *name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local *local = &compiler->locals[i];
    if (indentifierEqual(name, &local->name)) {
      if (local->depth == -1) {
        error(parser, "Cannot access local variable before initialization");
      }
      return i;
    }
  }
  return -1;
}

void varDeclaration(Parser *parser, Scanner *scanner, HashTable *hash) {
  uint8_t globalVar = parseVariable(parser, scanner, hash, "Expected a variable name");
  if (matchToken(parser, scanner, TOKEN_EQUAL)) {
    expression(parser, scanner, hash);
  } else {
    emitByte(OP_NULL, parser);
  }
  consume(parser, scanner, TOKEN_SEMI, "Expected ';' after value");
  defineVariable(parser, globalVar);
}

bool matchToken(Parser *parser, Scanner *scanner, TokenType type) {
  if (!check(parser, type)) return false;
  advance(parser, scanner);
  return true;
}

void printStatement(Parser *parser, Scanner *scanner, HashTable *hash) {
  expression(parser, scanner, hash);
  consume(parser, scanner, TOKEN_SEMI, "Expected ';' after value");
  emitByte(OP_PRINT, parser);
}

void expression(Parser *parser, Scanner *scanner, HashTable *hash) {
  parsePrecedence(parser, scanner, hash, PRE_ASSIGN);
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
  fprintf(stderr, "\n\x1b[31;1mError on [line %d]", token.line);
  if (token.type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token.type == TOKEN_ERR) {
    //nothing for now
  } else {
    fprintf(stderr, " at '%.*s'", token.length, token.start);
  }
  fprintf(stderr, " : \n\x1b[32;1m  => %s\n", message);
  fputs("\n\x1b[0m", stderr);
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

void number(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  double value = strtod(parser->prev.start, NULL);
  emitConst(parser, TO_NUMBER(value));
}

void unary(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  TokenType operatorType = parser->prev.type;
  parsePrecedence(parser, scanner, hash, PRE_UNARY);
  switch (operatorType) {
    case TOKEN_MINUS:
      emitByte(OP_NEGATE, parser);
      break;
    case TOKEN_BANG:
      emitByte(OP_NOT, parser);
      break;
  }
}

void binary(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  TokenType operator= parser->prev.type;
  ParseRule *rule = getRule(operator);
  parsePrecedence(parser, scanner, hash, (Precedence)(rule->prec + 1));
  switch (operator) {
    case TOKEN_PLUS:
      emitByte(OP_ADD, parser);
      break;
    case TOKEN_MODULO:
      emitByte(OP_MODULO, parser);
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
    case TOKEN_BANG_EQUAL:
      emitBytes(OP_EQUAL, OP_NOT, parser);
      break;
    case TOKEN_EQUAL_EQUAL:
      emitByte(OP_EQUAL, parser);
      break;
    case TOKEN_GREATER:
      emitByte(OP_GREATER, parser);
      break;
    case TOKEN_GREATER_EQUAL:
      emitByte(OP_GREATER_EQUAL, parser);
      break;
    case TOKEN_LESS:
      emitByte(OP_LESS, parser);
      break;
    case TOKEN_LESS_EQUAL:
      emitByte(OP_LESS_EQUAL, parser);
      break;
  }
}

void literal(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  switch (parser->prev.type) {
    case TOKEN_FALSE:
      emitByte(OP_FALSE, parser);
      break;
    case TOKEN_TRUE:
      emitByte(OP_TRUE, parser);
      break;
    case TOKEN_NULL:
      emitByte(OP_NULL, parser);
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

void grouping(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  expression(parser, scanner, hash);
  consume(parser, scanner, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
}

void logicalAnd(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  int jmpOffset = emitJump(OP_JMP_IF_FALSE, parser);
  emitByte(OP_POP, parser);
  parsePrecedence(parser, scanner, hash, PRE_LOGICAL_AND);
  backpatchJump(parser, jmpOffset);
}

void logicalOr(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  int elseJmpOffset = emitJump(OP_JMP_IF_FALSE, parser);
  int jmpOffset = emitJump(OP_JMP, parser);
  backpatchJump(parser, elseJmpOffset);
  emitByte(OP_POP, parser);
  parsePrecedence(parser, scanner, hash, PRE_LOGICAL_OR);
  backpatchJump(parser, jmpOffset);
}

void parsePrecedence(Parser *parser, Scanner *scanner, HashTable *hash, Precedence precedence) {
  advance(parser, scanner);
  ParseFn prefixRule = getRule(parser->prev.type)->prefix;
  if (prefixRule == NULL) {
    error(parser, "Expected expression");
    return;
  }
  bool canAssign = precedence <= PRE_ASSIGN;
  prefixRule(parser, scanner, hash, canAssign);
  while (precedence <= getRule(parser->cur.type)->prec) {
    advance(parser, scanner);
    ParseFn infixRule = getRule(parser->prev.type)->infix;
    infixRule(parser, scanner, hash, canAssign);
  }
  if (canAssign && matchToken(parser, scanner, TOKEN_EQUAL)) {
    error(parser, "Invalid assignment.");
  }
}

void incOrDec(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  // Refactor
  // This behaviour is incomplete
  // post inc/dec should happen after evaluating the current expression
  Token t = parser->prev;
  parser->prev = parser->cur;
  parser->cur = t;
  variable(parser, scanner, hash, canAssign);
}

void cont(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  emitLoop(current->labels[current->labelCount - 1], parser);
  current->labelCount -= 2;
}

static void brk(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  if (current->labelCount > 0) {
    emitConst(parser, TO_BOOL(false));
    current->labelCount--;
    emitLoop(current->labels[current->labelCount - 1] - 1, parser);
    current->labelCount--;
  } else {
    emitByte(OP_BRK, parser);
  }
}

void string(Parser *parser, Scanner *scanner, HashTable *hash, bool canAssign) {
  emitConst(parser, TO_OBJECT(copyString(parser->prev.start + 1, parser->prev.length - 2, hash)));
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PRE_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PRE_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PRE_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PRE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRE_NONE},
    [TOKEN_DOT] = {NULL, NULL, PRE_NONE},
    [TOKEN_MINUS] = {unary, binary, PRE_TERM},
    [TOKEN_PLUS] = {NULL, binary, PRE_TERM},
    [TOKEN_INCREMENT] = {incOrDec, NULL, PRE_NONE},
    [TOKEN_DECREMENT] = {incOrDec, NULL, PRE_NONE},
    [TOKEN_MODULO] = {NULL, binary, PRE_TERM},
    [TOKEN_SEMI] = {NULL, NULL, PRE_NONE},
    [TOKEN_STAR] = {NULL, binary, PRE_FACTOR},
    [TOKEN_SLASH] = {NULL, binary, PRE_FACTOR},
    [TOKEN_BANG] = {unary, NULL, PRE_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PRE_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PRE_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PRE_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PRE_COMP},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PRE_COMP},
    [TOKEN_LESS] = {NULL, binary, PRE_COMP},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PRE_COMP},
    [TOKEN_LOGICAL_AND] = {NULL, logicalAnd, PRE_LOGICAL_AND},
    [TOKEN_LOGICAL_OR] = {NULL, logicalOr, PRE_LOGICAL_OR},
    [TOKEN_IDENTIFIER] = {variable, NULL, PRE_NONE},
    [TOKEN_STRING] = {string, NULL, PRE_NONE},
    [TOKEN_NUMBER] = {number, NULL, PRE_NONE},

    [TOKEN_CONST] = {NULL, NULL, PRE_NONE},
    [TOKEN_ENUM] = {NULL, NULL, PRE_NONE},
    [TOKEN_VAR] = {NULL, NULL, PRE_NONE},
    [TOKEN_NULL] = {literal, NULL, PRE_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PRE_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PRE_NONE},
    [TOKEN_SELF] = {NULL, NULL, PRE_NONE},
    [TOKEN_FX] = {NULL, NULL, PRE_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PRE_NONE},
    [TOKEN_TRUE] = {literal, NULL, PRE_NONE},
    [TOKEN_FALSE] = {literal, NULL, PRE_NONE},
    [TOKEN_FROM] = {NULL, NULL, PRE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRE_NONE},
    [TOKEN_DO] = {NULL, NULL, PRE_NONE},
    [TOKEN_IF] = {NULL, NULL, PRE_NONE},
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
    [TOKEN_BRK] = {brk, NULL, PRE_NONE},
    [TOKEN_CONT] = {cont, NULL, PRE_NONE},
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
