class Expr {
  accept() {
    console.log(this);
    return this;
  }
}

class Binary extends Expr {
  constructor(left, operator, right) {
    super();
    this.left = left;
    this.operator = operator;
    this.right = right;
  }
  accept() {
    super.accept(this);
  }
}

class Unary extends Expr {
  constructor(operator, right) {
    super();
    this.operator = operator;
    this.right = right;
  }
  accept() {
    super.accept(this);
  }
}

module.exports = {
  Binary,
  Unary,
};
