module.exports = function error(line, where, msg) {
  console.log("[line " + line + "] Error " + where + ": " + msg);
  hadErr = true;
};
