/*const Scanner = require("./Scanner");

if (process.argv.length > 2) {
  const fs = require("fs");
  fs.readFile(process.argv[2], "utf8", (err, data) => {
    if (err) throw err;
    const scanner = new Scanner(data);
    console.log(scanner.scanTokens());
  });
} else {
  const readline = require("readline");
  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    prompt: ">>> ",
  });

  rl.prompt();
  rl.on("line", (line) => {
    const scanner = new Scanner(line);
    console.log(scanner.scanTokens());
    rl.prompt();
  }).on("close", () => {
    console.log("\nbye");
  });
}*/

const { Binary, Unary } = require("./Expr");
let binExpr = new Binary(1, "+", 3);
let unrExpr = new Unary("-", 5);
console.log(binExpr.accept());
console.log(unrExpr.accept());
