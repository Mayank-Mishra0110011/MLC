#ifndef MLC_REPL_H
#define MLC_REPL_H

#include "vm.h"

#define cursorforward(x) printf("\033[%dC", (x))
#define cursorbackward(x) printf("\033[%dD", (x))

#define KEY_ESCAPE 0x001b
#define KEY_DEL 0x000
#define KEY_TAB 0x009
#define KEY_BACKSPACE 0x007F
#define KEY_SPACE 0x0020
#define KEY_ENTER 0x000a
#define KEY_UP 0x0105
#define KEY_DOWN 0x0106
#define KEY_LEFT 0x0107
#define KEY_RIGHT 0x0108

typedef struct {
  char *buffer;
  char *currentToken;
  int offset;
  int length;
  int capacity;
} Line;

FILE *fp;
Line line = {NULL, NULL, 0, 0, 1024};
char *tBuffer = NULL;

static struct termios term, oterm;

static int getch();
static int kbhit();
static int kbesc();
static int kbget();

static int getLineOffset();
static int FPPeek();
static int REPLmatchToken(char *, char *);

void callback(int);
void initMLC_repl();

static void MLC_repl(int);
static void FPSetLastLine();
static void FPMovePrevLine();
static void FPMoveNextLine();
static void FPSetEnd();
static void FPReadCurrentLine(char *, int);
static void setColor(const char *);
static void printBack(int);
static void clearLine();
static void colorIfMatch(char *);

#endif