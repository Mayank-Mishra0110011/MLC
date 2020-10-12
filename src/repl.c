#include "repl.h"

FILE *fp;
Line line = {NULL, NULL, 0, 0, 1024};
char *tBuffer = NULL;

int getLineOffset() {
  int i = 0, c, lastPos = ftell(fp);
  while ((c = fgetc(fp)) != EOF) {
    i++;
  }
  fseek(fp, lastPos, SEEK_SET);
  return i;
}

void FPSetLastLine() {
  rewind(fp);
  int i = 0, c, newLine = 0, lastNewLine = 0;
  while ((c = fgetc(fp)) != EOF) {
    if (c == '\n') {
      newLine = i;
    } else if (lastNewLine != newLine) {
      lastNewLine = newLine;
    }
    i++;
  }
  fseek(fp, ++lastNewLine, SEEK_SET);
}

void FPMovePrevLine() {
  int off = ftell(fp);
  int i = 0, c, newLine = 0, lastNewLine = 0;
  if (off == 0) return;
  rewind(fp);
  while (ftell(fp) != off) {
    c = getc(fp);
    if (c == '\n') {
      newLine = i;
    } else if (lastNewLine != newLine) {
      lastNewLine = newLine;
    }
    i++;
  }
  if (lastNewLine != 0) ++lastNewLine;
  fseek(fp, lastNewLine, SEEK_SET);
}

void FPMoveNextLine() {
  if (getc(fp) == EOF) return;
  int i = 0, c, newLine = 0, curPos = ftell(fp);
  while ((c = getc(fp)) != EOF) {
    i++;
    if (c == '\n') {
      newLine = curPos + i;
      break;
    }
  }
  if (newLine == 0) newLine = curPos;
  fseek(fp, newLine, SEEK_SET);
}

int FPPeek() {
  int off = ftell(fp);
  int c = getc(fp);
  fseek(fp, off, SEEK_SET);
  return c;
}

void FPSetEnd() {
  while (1)
    if (getc(fp) == EOF) break;
}

void FPReadCurrentLine(char *buffer, int size) {
  int off = ftell(fp);
  fgets(buffer, size, fp);
  fseek(fp, off, SEEK_SET);
}

void setColor(const char *scheme) {
  fputs(scheme, stdout);
}

int REPLmatchToken(char *tkn, char *rest) {
  return strcmp(tkn, rest);
}

int getch() {
  int c = 0;
  tcgetattr(0, &oterm);
  memcpy(&term, &oterm, sizeof(term));
  term.c_lflag &= ~(ICANON | ECHO);
  term.c_cc[VMIN] = 1;
  term.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &term);
  c = getchar();
  tcsetattr(0, TCSANOW, &oterm);
  return c;
}

int kbhit() {
  int c = 0;
  tcgetattr(0, &oterm);
  memcpy(&term, &oterm, sizeof(term));
  term.c_lflag &= ~(ICANON | ECHO);
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 1;
  tcsetattr(0, TCSANOW, &term);
  c = getchar();
  tcsetattr(0, TCSANOW, &oterm);
  if (c != -1) ungetc(c, stdin);
  return ((c != -1) ? 1 : 0);
}

int kbesc() {
  int c;
  if (!kbhit()) return KEY_ESCAPE;
  c = getch();
  if (c == '[') {
    switch (getch()) {
      case 'A':
        c = KEY_UP;
        break;
      case 'B':
        c = KEY_DOWN;
        break;
      case 'C':
        c = KEY_LEFT;
        break;
      case 'D':
        c = KEY_RIGHT;
        break;
      default:
        c = 0;
        break;
    }
  } else {
    c = 0;
  }
  if (c == 0)
    while (kbhit()) getch();
  return c;
}

int kbget() {
  int c;
  c = getch();
  return (c == KEY_ESCAPE) ? kbesc() : c;
}

void printBack(int times) {
  for (int i = 0; i < times; i++) {
    printf("\b");
  }
}

void clearLine() {
  for (int i = 0; i < 2048; i++) {
    printf("\b \b");
  }
}

void colorIfMatch(char *token) {
  switch (token[0]) {
    case 'a':
      if (REPLmatchToken(token + 1, "bs") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "sync") == 0 || REPLmatchToken(token + 1, "wait") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      }
      break;
    case 'b':
      if (REPLmatchToken(token + 1, "rk") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "ool") == 0 || REPLmatchToken(token + 1, "yte") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'c':
      if (REPLmatchToken(token + 1, "lass") == 0) {
        printBack(strlen(token));
        setColor("\x1b[36;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "ase") == 0 || REPLmatchToken(token + 1, "atch") == 0 || REPLmatchToken(token + 1, "ont") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "onst") == 0 || REPLmatchToken(token + 1, "har") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'd':
      if (REPLmatchToken(token + 1, "o") == 0 || REPLmatchToken(token + 1, "ef") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "elete") == 0 || REPLmatchToken(token + 1, "ouble") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'E':
      if (REPLmatchToken(token + 1, "xception") == 0) {
        printBack(strlen(token));
        setColor("\x1b[36;1m");
        printf("%s", token);
      }
      break;
    case 'e':
      if (REPLmatchToken(token + 1, "lse") == 0 || REPLmatchToken(token + 1, "lif") == 0 || REPLmatchToken(token + 1, "xp") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "num") == 0 || REPLmatchToken(token + 1, "xt") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'f':
      if (REPLmatchToken(token + 1, "alse") == 0 || REPLmatchToken(token + 1, "inal") == 0 || REPLmatchToken(token + 1, "32") == 0 || REPLmatchToken(token + 1, "64") == 0 || REPLmatchToken(token + 1, "loat") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "rom") == 0 || REPLmatchToken(token + 1, "inally") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "x") == 0) {
        printBack(strlen(token));
        setColor("\x1b[33;1m");
        printf("%s", token);
      }
      break;
    case 'g':
      if (REPLmatchToken(token + 1, "oto") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      }
      break;
    case 'i':
      if (REPLmatchToken(token + 1, "f") == 0 || REPLmatchToken(token + 1, "mp") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "face") == 0 || REPLmatchToken(token + 1, "mlp") == 0 || REPLmatchToken(token + 1, "nstanceOf") == 0 || REPLmatchToken(token + 1, "nt") == 0 || REPLmatchToken(token + 1, "8") == 0 || REPLmatchToken(token + 1, "16") == 0 || REPLmatchToken(token + 1, "32") == 0 || REPLmatchToken(token + 1, "64") == 0 || REPLmatchToken(token + 1, "128") == 0 || REPLmatchToken(token + 1, "size") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'm':
      if (REPLmatchToken(token + 1, "ixin") == 0) {
        printBack(strlen(token));
        setColor("\x1b[36;1m");
        printf("%s", token);
      }
      break;
    case 'n':
      if (REPLmatchToken(token + 1, "ew") == 0 || REPLmatchToken(token + 1, "ull") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'O':
      if (REPLmatchToken(token + 1, "bject") == 0) {
        printBack(strlen(token));
        setColor("\x1b[36;1m");
        printf("%s", token);
      }
      break;
    case 'p':
      if (REPLmatchToken(token + 1, "rint") == 0) {
        printBack(strlen(token));
        setColor("\x1b[33;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "ub") == 0 || REPLmatchToken(token + 1, "riv") == 0 || REPLmatchToken(token + 1, "rot") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'r':
      if (REPLmatchToken(token + 1, "eturn") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      }
      break;
    case 's':
      if (REPLmatchToken(token + 1, "tring") == 0 || REPLmatchToken(token + 1, "uper") == 0 || REPLmatchToken(token + 1, "tatic") == 0 || REPLmatchToken(token + 1, "izeOf") == 0 || REPLmatchToken(token + 1, "elf") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "witch") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "truct") == 0) {
        printBack(strlen(token));
        setColor("\x1b[36;1m");
        printf("%s", token);
      }
      break;
    case 't':
      if (REPLmatchToken(token + 1, "hrow") == 0 || REPLmatchToken(token + 1, "hrows") == 0 || REPLmatchToken(token + 1, "o") == 0 || REPLmatchToken(token + 1, "ry") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "ypeOf") == 0 || REPLmatchToken(token + 1, "rue") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'u':
      if (REPLmatchToken(token + 1, "nsafe") == 0 || REPLmatchToken(token + 1, "8") == 0 || REPLmatchToken(token + 1, "16") == 0 || REPLmatchToken(token + 1, "32") == 0 || REPLmatchToken(token + 1, "64") == 0 || REPLmatchToken(token + 1, "128") == 0 || REPLmatchToken(token + 1, "size") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      } else if (REPLmatchToken(token + 1, "nion") == 0) {
        printBack(strlen(token));
        setColor("\x1b[36;1m");
        printf("%s", token);
      }
      break;
    case 'v':
      if (REPLmatchToken(token + 1, "ar") == 0 || REPLmatchToken(token + 1, "r") == 0 || REPLmatchToken(token + 1, "oid") == 0) {
        printBack(strlen(token));
        setColor("\x1b[34;1m");
        printf("%s", token);
      }
      break;
    case 'w':
      if (REPLmatchToken(token + 1, "hile") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      }
      break;
    case 'y':
      if (REPLmatchToken(token + 1, "ield") == 0) {
        printBack(strlen(token));
        setColor("\x1b[35;1m");
        printf("%s", token);
      }
      break;
  }
}

void MLC_repl(int mode, VM *vm) {
  int c, lo;
  line.buffer = GROW_ARRAY(line.buffer, char, line.offset, line.capacity);
  line.currentToken = line.buffer;
  printf("\e[1;1H\e[2J");
  if (mode == 1) FPSetEnd();
  while (1) {
    printf("mlc(main): >>> ");
    do {
      c = kbget();
      if (c == KEY_ESCAPE) {
        continue;
      } else if (c == KEY_UP && mode == 1 && ftell(fp) != 0) {
        FPMovePrevLine();
        lo = getLineOffset();
        tBuffer = (char *)realloc(tBuffer, sizeof(char) * lo);
        FPReadCurrentLine(tBuffer, sizeof(char) * lo);
        strcpy(line.buffer, tBuffer);
        *(line.buffer + strlen(tBuffer) - 1) = '\0';
        line.offset = strlen(tBuffer) - 1;
        line.length = line.offset - 1;
        clearLine();
        printf("mlc(main): >>> ");
        printf("%s", line.buffer);
      } else if (c == KEY_DOWN && mode == 1 && FPPeek() != EOF) {
        FPMoveNextLine();
        if (FPPeek() == EOF) continue;
        lo = getLineOffset();
        tBuffer = (char *)realloc(tBuffer, sizeof(char) * lo);
        FPReadCurrentLine(tBuffer, sizeof(char) * lo);
        strcpy(line.buffer, tBuffer);
        *(line.buffer + strlen(tBuffer) - 1) = '\0';
        line.offset = strlen(tBuffer) - 1;
        line.length = line.offset - 1;
        clearLine();
        printf("mlc(main): >>> ");
        printf("%s", line.buffer);
      } else if (c == KEY_RIGHT) {
        cursorbackward(1);
        line.offset--;
      } else if (c == KEY_LEFT) {
        cursorforward(1);
        line.offset++;
      } else if (c == KEY_SPACE) {
        colorIfMatch(line.currentToken);
        setColor("\x1b[0m");
        printf(" ");
        *(line.buffer + line.offset) = c;
        line.offset++;
        line.length++;
        line.currentToken = line.buffer + line.offset;
      } else if (c == KEY_BACKSPACE) {
        if (*(line.buffer + line.offset) != '\0') {
          printf("\b%s ", (line.buffer + line.offset));
          printBack(line.length - line.offset + 1);
          line.offset--;
          strcpy(line.buffer + line.offset, line.buffer + line.offset + 1);
          *(line.buffer + line.length) = '\0';
          line.length--;
          continue;
        }
        line.offset--;
        line.length--;
        *(line.buffer + line.offset) = '\0';
        printf("\b%s ", (line.buffer + line.offset));
        printBack(line.length - line.offset + 1);
      } else if (c == KEY_ENTER) {
        break;
      } else {
        *(line.buffer + line.offset) = c;
        line.offset++;
        line.length++;
        *(line.buffer + line.offset) = '\0';
        putchar(c);
      }
    } while (c != KEY_ENTER);
    IR res = interpret(vm, line.buffer);
    if (mode == 1) {
      FPSetEnd();
      *(line.buffer + line.offset) = '\n';
      *(line.buffer + line.offset + 1) = '\0';
      fputs(line.buffer, fp);
      FPSetLastLine();
    }
    if (line.capacity != 1024) {
      line.buffer = GROW_ARRAY(line.buffer, char, line.offset, 1024);
    }
    line.currentToken = line.buffer;
    line.length = 0;
    line.offset = 0;
    line.length = 0;
  }
}

void callback(int signum) {
  if (fp) {
    fclose(fp);
  }
  reallocate(line.buffer, line.capacity, 0);
  free(tBuffer);
  tcsetattr(0, TCSAFLUSH, &term);
  tcsetattr(0, TCSAFLUSH, &oterm);
  printf("\nBye!\n");
  exit(0);
}

void initMLC_repl(VM *vm) {
  char filePath[64];
  char username[32];
  char fileName[14];
  signal(SIGINT, callback);
  strcpy(username, getlogin());
  strcpy(fileName, "/.mlc_history");
  strcpy(filePath, "/home/");
  strcat(filePath, username);
  strcat(filePath, fileName);
  fp = fopen(filePath, "a+");
  if (!fp) {
    printf("Unable to open .mlc_history file! Your home directory may not have read/write access.\n");
    printf("Press Enter to open mlc repl without history support or any other key to exit\n");
    if (kbget() == KEY_ENTER) {
      MLC_repl(0, vm);
    }
  } else {
    MLC_repl(1, vm);
    fclose(fp);
  }
}