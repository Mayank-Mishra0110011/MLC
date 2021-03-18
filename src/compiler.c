#include "compiler.h"

FunctionObject *compile(const char *source) {
  Compiler compiler;
  initScanner(source);
  initCompiler(&compiler, TYPE_SCRIPT);
  parser.hadErr = false;
  parser.panic = false;
  advance();
  while (!matchToken(TOKEN_EOF)) {
    declaration();
  }
  FunctionObject *function = endCompilation();
  return parser.hadErr ? NULL : function;
}

FunctionObject *endCompilation() {
  emitReturn(parser);
  FunctionObject *function = current->function;
#ifdef DEBUG_PRINT_CODE
  if (!parser.hadErr) {
    disassembleChunk(currentChunk(), function->name != NULL ? function->name->str : "<script>");
  }
#endif
  current = current->enclosing;
  return function;
}

void markCompilerRoots() {
  Compiler *compiler = current;
  while (compiler != NULL) {
    markObject((Object *)compiler->function);
    compiler = compiler->enclosing;
  }
}

void initCompiler(Compiler *compiler, FunctionType type) {
  compiler->enclosing = current;
  compiler->function = NULL;
  compiler->type = type;
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  compiler->labelCount = 0;
  compiler->function = newFunction();
  current = compiler;
  if (type != TYPE_SCRIPT) {
    current->function->name = copyString(parser.prev.start, parser.prev.length);
  }
  Local *local = &current->locals[current->localCount++];
  local->depth = 0;
  local->name.start = "";
  local->name.length = 0;
  local->isCaptured = false;
}

void synchronize() {
  parser.panic = false;
  while (parser.cur.type) {
    if (parser.prev.type == TOKEN_SEMI) return;
    switch (parser.cur.type) {
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
    printf("END %d, %d\n", parser.cur.type, parser.prev.type);
    advance();
  }
}

void varDeclaration() {
  uint8_t globalVar = parseVariable("Expected a variable name");
  if (matchToken(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NULL);
  }
  consume(TOKEN_SEMI, "Expected ';' after value");
  defineVariable(globalVar);
}

void defineVariable(uint8_t global) {
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_DEFINE_GLOBAL, global);
}

void variable(bool canAssign) {
  namedVar(canAssign);
}

void namedVar(bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(current);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else if ((arg = resolveUpvalue(current)) != -1) {
    getOp = OP_GET_UPVALUE;
    setOp = OP_SET_UPVALUE;
  } else {
    arg = identifierConst(&parser.prev);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }
  if (canAssign && matchToken(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
    if (parser.cur.type == TOKEN_INCREMENT || parser.cur.type == TOKEN_DECREMENT) {
      emitConst(TO_NUMBER(1));
      emitByte(parser.cur.type == TOKEN_INCREMENT ? OP_ADD : OP_SUBTRACT);
      emitBytes(setOp, (uint8_t)arg);
      advance();
    }
  }
}

void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }
  consume(TOKEN_RIGHT_BRACE, "Expected '}' after block");
}

void beginScope() {
  current->scopeDepth++;
}

void declareLocalVar() {
  if (current->scopeDepth == 0) return;
  Local *local;
  for (int i = current->localCount - 1; i >= 0; i--) {
    local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break;
    }
    if (identifiersEqual(&parser.prev, &local->name)) {
      error("Identifier has already been declared.");
    }
  }
  if (current->localCount == UINT8_COUNT) {
    error("Stack Overflow.");
    return;
  }
  local = &current->locals[current->localCount++];
  local->name = parser.prev;
  local->depth = -1;
  local->isCaptured = false;
}

void endScope() {
  current->scopeDepth--;
  while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
    if (current->locals[current->localCount - 1].isCaptured) {
      emitByte(OP_CLOSE_UPVALUE);
    } else {
      emitByte(OP_POP);
    }
    current->localCount--;
  }
}

void markInitialized() {
  if (current->scopeDepth == 0) return;
  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

void backpatchJump(int offset) {
  int jmp = currentChunk()->count - offset - 2;
  if (jmp > UINT16_MAX) error("Stack Overflow.");
  currentChunk()->code[offset] = (jmp >> 8) & 0xff;
  currentChunk()->code[offset + 1] = jmp & 0xff;
}

void ifStatement() {
  expression();
  int ifJumpOffset = emitJump(OP_JMP_IF_FALSE);
  emitByte(OP_POP);
  statement();
  int elseJumpOffset = emitJump(OP_JMP);
  backpatchJump(ifJumpOffset);
  emitByte(OP_POP);
  if (matchToken(TOKEN_ELSE)) statement();
  backpatchJump(elseJumpOffset);
}

void logicalAnd(bool canAssign) {
  int jmpOffset = emitJump(OP_JMP_IF_FALSE);
  emitByte(OP_POP);
  parsePrecedence(PRE_LOGICAL_AND);
  backpatchJump(jmpOffset);
}

void logicalOr(bool canAssign) {
  int elseJmpOffset = emitJump(OP_JMP_IF_FALSE);
  int jmpOffset = emitJump(OP_JMP);
  backpatchJump(elseJmpOffset);
  emitByte(OP_POP);
  parsePrecedence(PRE_LOGICAL_OR);
  backpatchJump(jmpOffset);
}

void emitLoop(int loopStart) {
  emitByte(OP_LOOP);
  int offset = currentChunk()->count - loopStart + 2;
  if (offset > UINT16_MAX) error("Stack Overflow.");
  emitBytes((offset >> 8) & 0xff, offset & 0xff);
}

void whileStatement() {
  int loopStart = currentChunk()->count;
  int curLabelCount = current->labelCount;
  expression();
  int exitJmpOffset = emitJump(OP_JMP_IF_FALSE);
  current->labels[current->labelCount++] = exitJmpOffset;
  current->labels[current->labelCount++] = loopStart;
  emitByte(OP_POP);
  statement();
  emitLoop(loopStart);
  backpatchJump(exitJmpOffset);
  emitByte(OP_POP);
  if (curLabelCount != current->labelCount) current->labelCount = curLabelCount;
}

void fromStatement() {
  beginScope();
  if (matchToken(TOKEN_SEMI)) {
  } else if (matchToken(TOKEN_VAR)) {
    varDeclaration();
  } else {
    expressionStatement();
  }
  int curLabelCount = current->labelCount;
  int loopStart = currentChunk()->count;
  int exitJmp = -1;
  if (!matchToken(TOKEN_SEMI)) {
    expression();
    consume(TOKEN_SEMI, "Expected ';' after loop condition");
    exitJmp = emitJump(OP_JMP_IF_FALSE);
    current->labels[current->labelCount++] = exitJmp;
    emitByte(OP_POP);
  } else {
    exitJmp = emitJump(OP_JMP_IF_FALSE);
    current->labels[current->labelCount++] = exitJmp;
    current->labels[current->labelCount++] = exitJmp;
  }
  if (!check(TOKEN_LEFT_BRACE)) {
    int bodyJmp = emitJump(OP_JMP);
    int incrementStart = currentChunk()->count;
    if (!matchToken(TOKEN_LEFT_BRACE)) {
      expression();
    }
    emitByte(OP_POP);
    emitLoop(loopStart);
    loopStart = incrementStart;
    current->labels[current->labelCount++] = loopStart;
    backpatchJump(bodyJmp);
  }
  statement();
  emitLoop(loopStart);
  if (exitJmp != -1) {
    backpatchJump(exitJmp);
    emitByte(OP_POP);
  }
  endScope(parser);
  if (curLabelCount != current->labelCount) current->labelCount = curLabelCount;
}

void switchStatement() {
  beginScope();
  expression();
  emitByte(OP_SWITCH_START);
  consume(TOKEN_LEFT_BRACE, "Expected '{' after switch expression");
  int exitJmp = -1;
  while (matchToken(TOKEN_CASE) || check(TOKEN_DEFAULT)) {
    if (!check(TOKEN_DEFAULT)) {
      expression();
      consume(TOKEN_LABEL, "Expected ':' after case expression");
    } else {
      matchToken(TOKEN_DEFAULT);
      consume(TOKEN_LABEL, "Expected ':' after def");
    }
    emitByte(OP_CASE);
    exitJmp = emitJump(OP_JMP_IF_FALSE);
    emitByte(OP_POP);
    while (!check(TOKEN_CASE) && !check(TOKEN_DEFAULT) && !check(TOKEN_RIGHT_BRACE))
      declaration();
    if (!check(TOKEN_DEFAULT)) backpatchJump(exitJmp);
    if (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_DEFAULT)) emitByte(OP_POP);
  }
  consume(TOKEN_RIGHT_BRACE, "Expected '}' at the end of switch statement");
  emitByte(OP_SWITCH_END);
  endScope(parser);
}

void function(FunctionType t) {
  Compiler compiler;
  initCompiler(&compiler, TYPE_FUNCTION);
  beginScope();
  consume(TOKEN_LEFT_PAREN, "Expected '(' after fx name");
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      current->function->arity++;
      if (current->function->arity > 255) {
        errorAtCurrent("Too many params");
      }
      uint8_t paramConst = parseVariable("Expected param name");
      defineVariable(paramConst);
    } while (matchToken(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expected ')' after fx params");
  consume(TOKEN_LEFT_BRACE, "Expected '{' before fx body");
  block();
  FunctionObject *function = endCompilation(parser);
  emitBytes(OP_CLOSURE, makeConst(TO_OBJECT(function)));
  for (int i = 0; i < function->upvalueCount; i++) {
    emitBytes(compiler.upvalues[i].isLocal ? 1 : 0, compiler.upvalues[i].index);
  }
}

void functionDeclaration() {
  uint8_t global = parseVariable("Expected fx name");
  markInitialized();
  function(TYPE_FUNCTION);
  defineVariable(global);
}

void returnStatement() {
  if (current->type == TYPE_SCRIPT) error("Syntax Error: 'return' outside function");
  if (matchToken(TOKEN_SEMI)) {
    emitReturn(parser);
  } else {
    expression();
    consume(TOKEN_SEMI, "Expected ';' after value");
    emitByte(OP_RETURN);
  }
}

void call(bool canAssign) {
  uint8_t argCount = argList();
  emitBytes(OP_CALL, argCount);
}

void incOrDec(bool canAssign) {
  // Refactor
  // This behaviour is incomplete
  // post inc/dec should happen after evaluating the current expression
  Token t = parser.prev;
  parser.prev = parser.cur;
  parser.cur = t;
  variable(canAssign);
}

void cont(bool canAssign) {
  emitLoop(current->labels[current->labelCount - 1]);
  current->labelCount -= 2;
}

void _brk(bool canAssign) {
  if (current->labelCount > 0) {
    emitConst(TO_BOOL(false));
    current->labelCount--;
    emitLoop(current->labels[current->labelCount - 1] - 1);
    current->labelCount--;
  } else {
    emitByte(OP_BRK);
  }
}

void expressionStatement() {
  expression();
  consume(TOKEN_SEMI, "Expected ';' after value");
  emitByte(OP_POP);
}

void printStatement() {
  do {
    expression();
    emitByte(OP_PRINT);
  } while (matchToken(TOKEN_COMMA));
  consume(TOKEN_SEMI, "Expected ';' after value");
  emitByte(OP_PRINT_LN);
}

void statement() {
  if (matchToken(TOKEN_PRINT)) {
    printStatement();
  } else if (matchToken(TOKEN_IF)) {
    ifStatement();
  } else if (matchToken(TOKEN_WHILE)) {
    whileStatement();
  } else if (matchToken(TOKEN_FROM)) {
    fromStatement();
  } else if (matchToken(TOKEN_SWITCH)) {
    switchStatement();
  } else if (matchToken(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope(parser);
  } else if (matchToken(TOKEN_RETURN)) {
    returnStatement();
  } else {
    expressionStatement();
  }
}

void declaration() {
  if (matchToken(TOKEN_FX)) {
    functionDeclaration();
  } else if (matchToken(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }
  if (parser.panic) synchronize();
}

void expression() {
  parsePrecedence(PRE_ASSIGN);
}

void emitReturn() {
  emitBytes(OP_NULL, OP_RETURN);
}

void advance() {
  parser.prev = parser.cur;
  while (true) {
    parser.cur = scanToken();
    if (parser.cur.type != TOKEN_ERR) break;
    errorAtCurrent(parser.cur.start);
  }
}

void errorAtCurrent(const char *message) {
  errorAt(message);
}

void errorAt(const char *message) {
  if (parser.panic) return;  //this is where you'd throw an exception but not yet
  Token token = parser.prev;
  parser.panic = true;
  fprintf(stderr, "[line %d] Error", token.line);
  if (token.type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token.type == TOKEN_ERR) {
    //nothing for now
  } else {
    fprintf(stderr, " at '%.*s'", token.length, token.start);
  }
  fprintf(stderr, " : %s\n", message);
  parser.hadErr = true;
}

void error(const char *message) {
  errorAt(message);
}

void consume(TokenType type, const char *message) {
  if (parser.cur.type == type) {
    advance();
    return;
  }
  errorAtCurrent(message);
}

void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.prev.line);
}

void emitBytes(uint8_t b1, uint8_t b2) {
  emitByte(b1);
  emitByte(b2);
}

void number(bool canAssign) {
  double value = strtod(parser.prev.start, NULL);
  emitConst(TO_NUMBER(value));
}

void unary(bool canAssign) {
  TokenType operatorType = parser.prev.type;
  parsePrecedence(PRE_UNARY);
  switch (operatorType) {
    case TOKEN_MINUS:
      emitByte(OP_NEGATE);
      break;
    case TOKEN_BANG:
      emitByte(OP_NOT);
      break;
  }
}

void binary(bool canAssign) {
  TokenType operator= parser.prev.type;
  ParseRule *rule = getRule(operator);
  parsePrecedence((Precedence)(rule->prec + 1));
  switch (operator) {
    case TOKEN_PLUS:
      emitByte(OP_ADD);
      break;
    case TOKEN_MODULO:
      emitByte(OP_MODULO);
      break;
    case TOKEN_MINUS:
      emitByte(OP_SUBTRACT);
      break;
    case TOKEN_STAR:
      emitByte(OP_MULTIPLY);
      break;
    case TOKEN_SLASH:
      emitByte(OP_DIVIDE);
      break;
    case TOKEN_BANG_EQUAL:
      emitBytes(OP_EQUAL, OP_NOT);
      break;
    case TOKEN_EQUAL_EQUAL:
      emitByte(OP_EQUAL);
      break;
    case TOKEN_GREATER:
      emitByte(OP_GREATER);
      break;
    case TOKEN_GREATER_EQUAL:
      emitByte(OP_GREATER_EQUAL);
      break;
    case TOKEN_LESS:
      emitByte(OP_LESS);
      break;
    case TOKEN_LESS_EQUAL:
      emitByte(OP_LESS_EQUAL);
      break;
  }
}

void literal(bool canAssign) {
  switch (parser.prev.type) {
    case TOKEN_FALSE:
      emitByte(OP_FALSE);
      break;
    case TOKEN_TRUE:
      emitByte(OP_TRUE);
      break;
    case TOKEN_NULL:
      emitByte(OP_NULL);
      break;
  }
}

void emitConst(Value value) {
  emitBytes(OP_CONST, makeConst(value));
}

void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

void parsePrecedence(Precedence precedence) {
  advance();
  ParseFn prefixRule = getRule(parser.prev.type)->prefix;
  if (prefixRule == NULL) {
    error("Expected expression");
    return;
  }
  bool canAssign = precedence <= PRE_ASSIGN;
  prefixRule(canAssign);
  while (precedence <= getRule(parser.cur.type)->prec) {
    advance();
    ParseFn infixRule = getRule(parser.prev.type)->infix;
    infixRule(canAssign);
  }
  if (canAssign && matchToken(TOKEN_EQUAL)) {
    error("Invalid assignment.");
  }
}

void string(bool canAssign) {
  emitConst(TO_OBJECT(copyString(parser.prev.start + 1, parser.prev.length - 2)));
}

uint8_t argList() {
  uint8_t argCount = 0;
  if (!check(TOKEN_RIGHT_PAREN)) {
    do {
      expression();
      if (argCount == 255) {
        error("Too many args");
      }
      argCount++;
    } while (matchToken(TOKEN_COMMA));
  }
  consume(TOKEN_RIGHT_PAREN, "Expected ')' after fx args");
  return argCount;
}

uint8_t parseVariable(const char *err) {
  consume(TOKEN_IDENTIFIER, err);
  declareLocalVar(parser);
  if (current->scopeDepth > 0) return 0;
  return identifierConst(&parser.prev);
}

uint8_t makeConst(Value value) {
  int constant = addConst(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk");
    return 0;
  }
  return (uint8_t)constant;
}

uint8_t identifierConst(Token *name) {
  return makeConst(TO_OBJECT(copyString(name->start, name->length)));
}

int emitJump(uint8_t instr) {
  emitByte(instr);
  emitByte(0xff);
  emitByte(0xff);
  return currentChunk()->count - 2;
}

int addUpvalue(Compiler *compiler, uint8_t index, bool isLocal) {
  int upvalueCount = compiler->function->upvalueCount;
  for (int i = 0; i < upvalueCount; i++) {
    Upvalue *upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) return i;
  }
  if (upvalueCount == UINT8_COUNT) {
    error("Too many closure variable");
    return 0;
  }
  compiler->upvalues[upvalueCount].isLocal = isLocal;
  compiler->upvalues[upvalueCount].index = index;
  return compiler->function->upvalueCount++;
}

int resolveUpvalue(Compiler *compiler) {
  if (compiler->enclosing == NULL) return -1;
  int local = resolveLocal(compiler->enclosing);
  if (local != -1) {
    compiler->enclosing->locals[local].isCaptured = true;
    return addUpvalue(compiler, (uint8_t)local, true);
  }
  int upvalue = resolveUpvalue(compiler->enclosing);
  if (upvalue != -1) {
    return addUpvalue(compiler, (uint8_t)upvalue, false);
  }
  return -1;
}

int resolveLocal(Compiler *compiler) {
  Token *name = &parser.prev;
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local *local = &compiler->locals[i];
    if (identifiersEqual(name, &local->name)) {
      if (local->depth == -1) {
        error("Cannot access local variable before initialization");
      }
      return i;
    }
  }
  return -1;
}

bool identifiersEqual(Token *tkn1, Token *tkn2) {
  if (tkn1->length != tkn2->length) return false;
  return memcmp(tkn1->start, tkn2->start, tkn1->length) == 0;
}

bool matchToken(TokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
}

bool check(TokenType type) {
  return parser.cur.type == type;
}

Chunk *currentChunk() {
  return &current->function->chunk;
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, call, PRE_CALL},
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
    [TOKEN_BRK] = {_brk, NULL, PRE_NONE},
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
