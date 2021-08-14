#include <stdio.h>
#include <termios.h>

static int screen_rows= 24, screen_cols= 80;

// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
void screen_init() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  screen_rows = w.ws_row;
  screen_cols = w.ws_col;
}

////////////////////////////////////////
// - keyboard

static void reset() { printf("\e[48;5;0m\e[38;5;7m\e[23m\e[24m"); }

//static void cls() { printf("\e[H[2J[3J"); }

static void clear() { printf("\x1b[2J\x1b[H"); }
static void clearend() { printf("\x1b[K"); }
static void cleareos() { printf("\x1b[J"); }

static void gotorc(int r, int c) { printf("\x1b[%d;%dH", r+1, c+1); }
static void cursoroff() { printf("\x1b[?25l"); }
static void cursoron() { printf("\x1b[?25h"); }
static void inverse(int on) { printf(on ? "\x1b[7m" : "\x1b[m"); }
static void fgcolor(int c) { printf("\x1b[[3%dm", c); } // 0black 1red 2green 3yellow 4blue 5magnenta 6cyan 7white 9default
static void bgcolor(int c) { printf("\x1b[[4%dm", c); } // 0black 1red 2green 3yellow 4blue 5magnenta 6cyan 7white 9default
static void savescreen() { printf("\x1b[?47h"); }
static void restorescreen() { printf("\x1b[?47l"); }
// can use insert characters/line from
// - http://vt100.net/docs/vt102-ug/chapter5.html
static void insertmode(int on) { printf("\x1b[4%c", on ? 'h' : 'l'); }

// - higher level colors
enum color{black, red, green, yellow, blue, magnenta, cyan, white, none};
enum color _fg= black, _bg= white;

void _color(c) {
  if (c >= 0) {
    printf("5;%dm", c);
  } else {
    c= -c;
    int b= c&0xff, g= (c>>8)&0xff, r=(c>>16);
    printf("2;%d;%d;%dm", r, g, b);
  }
}

void fg(int c) { printf("\e[38;"); _color(c); _fg= c; }
void bg(int c) { printf("\e[48;"); _color(c); _bg= c; }

int bold(int c /* 0-7 */) { return c+8; }
int rgb(int r, int g, int b /* 0-5 */) { return 0x10+ 36*r + 6*g + b; }
int gray(int c /* 0-7 */) { return 0xe8+  c; }
int RGB(int r, int g, int b /* 0-255 */) { return -(r*256+g)*256+b; }
void underline() { printf("\e[4m"); }
void end_underline() { printf("\e[24m"); }

// adjusted colors
void C(int n) { fg(n + 8*(n!=0 && n<8)); }
void B(int n) { bg(n); }

////////////////////////////////////////
// - keyboard

enum { ESC=27, DEL=127, CTRL=-64, META=128, UP=META+'a', DOWN, RIGHT, LEFT, TAB=9, SHIFT_TAB=META+'z', DELETE=META+'3', };

int key() {
  struct termios old, tmp;

  tcgetattr(0, &old);

  // modify
  tmp= old;
  cfmakeraw(&tmp); // ^C & ^Z !
  tmp.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(0, TCSANOW, &tmp);

  int c= getchar();

  // restore
  tcsetattr(0, TCSANOW, &old);
  return c;
}
