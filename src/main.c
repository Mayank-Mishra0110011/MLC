#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

static void MLC_repl();
static void MLC_compile(const char *filePath);
static char *readFile(const char *filePath);

int main(int argc, const char *argv[]) {
  initVM();
  if (argc == 1) {
    MLC_repl();
  } else if (argc == 2) {
    MLC_compile(argv[1]);
  } else {
    fprintf(stderr, "Usage: MLC [path]\n");
    exit(64);
  }
  deleteVM();
  return 0;
}

void MLC_repl() {
  char line[1024];
  while (true) {
    printf(">>> ");
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }
    IR res = interpret(line);
  }
}

void MLC_compile(const char *filePath) {
  char *source = readFile(filePath);
  IR res = interpret(source);
  free(source);
  if (res == I_COMPILE_ERR) exit(65);
  if (res == I_RUNTIME_ERR) exit(70);
}

char *readFile(const char *filePath) {
  FILE *file = fopen(filePath, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not find file \"%s\".\n");
    exit(74);
  }
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);
  char *buffer = (char *)malloc(fileSize + 1);
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  buffer[bytesRead] = '\0';
  fclose(file);
  return buffer;
}