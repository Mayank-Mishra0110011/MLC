class Token {
  constructor(type, lexme, literal, line) {
    this.type = type;
    this.lexme = lexme;
    this.literal = literal;
    this.line = line;
  }
  toStr() {
    return this.type + " " + this.lexme + " " + this.literal;
  }
}

module.exports = Token;
