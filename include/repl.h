#ifndef MLC_REPL_H
#define MLC_REPL_H

#include <signal.h>
#include <termios.h>
#include <unistd.h>

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

static struct termios term, oterm;

typedef struct {
  char *buffer;
  char *currentToken;
  int offset;
  int length;
  int capacity;
} Line;

static int getch();
static int kbhit();
static int kbesc();
static int kbget();

void callback(int);
static void MLC_repl(int, VM *);
void initMLC_repl(VM *);
static int getLineOffset();
static void FPSetLastLine();
static void FPMovePrevLine();
static void FPMoveNextLine();
static int FPPeek();
static void FPSetEnd();
static void FPReadCurrentLine(char *, int);
static void setColor(const char *);
static int REPLmatchToken(char *, char *);
static void printBack(int);
static void clearLine();
static void colorIfMatch(char *);

#endif