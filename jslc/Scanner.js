const TokenType = require("./TokenType");
const Token = require("./Token");
const error = require("./Error");

class Scanner {
  constructor(source) {
    this.source = source;
    this.tokens = [];
    this.start = 0;
    this.current = 0;
    this.line = 1;
  }
  isAtEnd() {
    return this.current >= this.source.length;
  }
  scanTokens() {
    while (!this.isAtEnd()) {
      this.start = this.current;
      this.scanToken();
    }
    this.tokens.push(new Token(TokenType.eof, "", null, this.line));
    return this.tokens;
  }
  scanToken() {
    let c = this.advance();
    switch (c) {
      case "(":
        this.addToken(TokenType.leftParen);
        break;
      case ")":
        this.addToken(TokenType.rightParen);
        break;
      case "{":
        this.addToken(TokenType.leftBrace);
        break;
      case "}":
        this.addToken(TokenType.rigtBrace);
        break;
      case ",":
        this.addToken(TokenType.comma);
        break;
      case ".":
        this.addToken(TokenType.dot);
        break;
      case "-":
        this.addToken(TokenType.minus);
        break;
      case "+":
        this.addToken(TokenType.plus);
        break;
      case ";":
        this.addToken(TokenType.semi);
        break;
      case "*":
        if (this.match("/")) {
          error(this.line, "Unexpected character", c);
          return;
        }
        this.addToken(TokenType.star);
        break;
      case "!":
        this.addToken(this.match("=") ? TokenType.bangEqual : TokenType.bang);
        break;
      case "=":
        this.addToken(this.match("=") ? TokenType.equalEqual : TokenType.equal);
        break;
      case "<":
        this.addToken(this.match("=") ? TokenType.lessEqual : TokenType.less);
        break;
      case ">":
        this.addToken(
          this.match("=") ? TokenType.greaterEqual : TokenType.greater
        );
        break;
      case "/":
        if (this.match("/")) {
          while (this.peek() != "\n" && !this.isAtEnd()) this.advance();
        } else if (this.match("*")) {
          while (true) {
            if (this.isAtEnd() || (this.match("*") && this.match("/"))) break;
            if (this.peek() == "\n") this.line++;
            this.advance();
          }
        } else {
          this.addToken(TokenType.slash);
        }
        break;
      case " ":
      case "\r":
      case "\t":
        break;
      case "\n":
        this.line++;
        break;
      case "'":
        this.string();
        break;
      case "|":
        if (this.peek() == "|") {
          this.addToken(TokenType.or);
        }
        break;
      default:
        if (this.isDigit(c)) {
          this.number();
        } else if (this.isAlpha(c)) {
          this.identifier();
        } else {
          error(this.line, "Unexpected character", c);
        }
    }
  }
  isAlpha(c) {
    return (c >= "a" && c <= "z") || (c >= "A" && c <= "Z") || c == "_";
  }
  isAlphaNumeric(c) {
    return this.isAlpha(c) || this.isDigit(c);
  }
  identifier() {
    while (this.isAlphaNumeric(this.peek())) this.advance();
    let text = this.source.substring(this.start, this.current);
    let type = TokenType[text];
    if (type == undefined) type = TokenType.identifier;
    this.addToken(type);
  }
  number() {
    while (this.isDigit(this.peek())) this.advance();
    if (this.peek() == "." && this.isDigit(this.peekNext())) {
      this.advance();
      while (this.isDigit(this.peek())) this.advance();
    }
    this.addToken(
      TokenType.number,
      parseFloat(this.source.substring(this.start, this.current))
    );
  }
  peekNext() {
    if (this.current + 1 >= this.source.length) return "\0";
    return this.source[this.current + 1];
  }
  isDigit(c) {
    return c >= "0" && c <= "9";
  }
  string() {
    while (this.peek() != "'" && !this.isAtEnd()) {
      if (this.peek() == "\n") this.line++;
      this.advance();
    }
    if (this.isAtEnd()) {
      error(this.line, "Unterminated string.");
      return;
    }
    this.advance();
    let value = this.source.substring(this.start + 1, this.current - 1);
    this.addToken(TokenType.string, value);
  }
  peek() {
    if (this.isAtEnd()) return "\0";
    return this.source[this.current];
  }
  match(expected) {
    if (this.isAtEnd() || this.source[this.current] != expected) return false;
    this.current++;
    return true;
  }
  advance() {
    this.current++;
    return this.source[this.current - 1];
  }
  _addToken(type, literal) {
    let text = this.source.substring(this.start, this.current);
    this.tokens.push(new Token(type, text, literal, this.line));
  }
  addToken(type) {
    this._addToken(type, null);
  }
}

module.exports = Scanner;
