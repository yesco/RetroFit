#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "jio.h"

extern int screen_rows= 24, screen_cols= 80;
// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
void screen_init() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  screen_rows = w.ws_row;
  screen_cols = w.ws_col;
}

////////////////////////////////////////
// - ansi screen

void reset() { printf("\e[48;5;0m\e[38;5;7m\e[23m\e[24m\e[0m"); }

//void cls() { printf("\e[H[2J[3J"); }

void clear() { printf("\x1b[2J\x1b[H"); }
void clearend() { printf("\x1b[K"); }
void cleareos() { printf("\x1b[J"); }

void gotorc(int r, int c) { printf("\x1b[%d;%dH", r+1, c+1); }

void inverse(int on) { printf(on ? "\x1b[7m" : "\x1b[m"); }
void fgcolor(int c) { printf("\x1b[[3%dm", c); } // 0black 1red 2green 3yellow 4blue 5magnenta 6cyan 7white 9default
void bgcolor(int c) { printf("\x1b[[4%dm", c); } // 0black 1red 2green 3yellow 4blue 5magnenta 6cyan 7white 9default

void savescreen() { printf("\x1b[?47h"); }
void restorescreen() { printf("\x1b[?47l"); }
// cursor
void hide() { printf("\e[?25l"); }
void show() { printf("\e[?25h"); }
void save() { printf("\e7"); }
void restore() { printf("\e8"); }

void cursoroff() { printf("\x1b[?25l"); }
void cursoron() { printf("\x1b[?25h"); }


// can use insert characters/line from
// - http://vt100.net/docs/vt102-ug/chapter5.html
void insertmode(int on) { printf("\x1b[4%c", on ? 'h' : 'l'); }

void _color(int c) {
  if (c >= 0) {
    printf("5;%dm", c);
  } else {
    c= -c;
    int b= c&0xff, g= (c>>8)&0xff, r=(c>>16);
    printf("2;%d;%d;%dm", r, g, b);
  }
}

enum color _fg= black, _bg= white;

void fg(int c) { printf("\e[38;"); _color(c); _fg= c; }
void bg(int c) { printf("\e[48;"); _color(c); _bg= c; }

int bold(int c /* 0-7 */) { return c+8; }
int rgb(int r, int g, int b /* 0-5 */) { return 0x10+ 36*r + 6*g + b; }
int gray(int c /* 0-7 */) { return 0xe8+  c; }
int RGB(int r, int g, int b /* 0-255 */) { return -(r*256+g)*256+b; }
void underline() { printf("\e[4m"); }
void end_underline() { printf("\e[24m"); }

// adjusted colors
void C(int n) {
  if (n==blue) fg(rgb(0, 0, 1));
  fg(n + 8*(n!=0 && n<8));
}
void B(int n) { bg(n); }

////////////////////////////////////////
// - keyboard

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

  // TODO: fix ESC (by itself)
  // fix multi-char key-encodings
  if (c==ESC) c=toupper(key())+META;
  if (c==META+'[') c=tolower(key())+META;
  if (c==META+'3') c=key(), c= DEL;
  // function keys
  if (c==META+'O') c=key()-'P'+1+META;
  if (c==META+'1') c=key()-'0'+META, key(), c= c==5+META?c:c-1;
  if (c==META+'2') c=key()-'0'+9+META, key(), c= c>10+META?c-1:c;

  return c;
}

void print_key(int c) {
  if (0) ;
  else if (c==TAB) printf(" TAB");
  else if (c==RETURN) printf(" RETURN");
  else if (c<32) printf(" ^%c", c+64);
  else if (c<127) putchar(c);
  else if (c==BACKSPACE) printf(" BACKSPACE");
  else if (c==DEL) printf(" DEL");
  else if (c==S_TAB) printf(" S_TAB");
  else if (c==META+'a') printf(" UP");
  else if (c==META+'b') printf(" DOWN");
  else if (c==META+'c') printf(" RIGHT");
  else if (c==META+'d') printf(" LEFT");
  else if (c>=META+' ') printf(" M-%c", c-META);
  else if (c>=META)
    printf(" F-%d", c-META);
  fflush(stdout);
}

int flines(FILE* f) {
  fseek(f, 0, SEEK_SET);
  int n= 0, c;
  while((c= fgetc(f)) != EOF)
    if (c=='\n') n++;
  return n++;
}

// Read full line from FILE
// Return NULL, if none, or malloced line without '\n'
char* fgetline(FILE* f) {
  // save pos, figure out how long line
  long p= ftell(f), len= 2, c;
  while((c= fgetc(f)) != EOF && c!='\n')
    len++;
  // reset, allocate and read
  fseek(f, p, SEEK_SET);
  char* r= calloc(len, 1);
  if (!fgets(r, len, f)) free(r), r= NULL;
  if (r && r[strlen(r)-1]=='\n')
    r[strlen(r)-1]= 0;
  return r;
}

// In FILE Find and return numbered LINE.
// Result: see fgetline()
char* fgetlinenum(FILE* f, long line) {
  fseek(f, 0, SEEK_SET);
  int c;
  while(line > 1 && (c= fgetc(f)) != EOF)
    if (c=='\n') line--;
  if (line > 1) return NULL;
  return fgetline(f);
}

// Return true if S (!null) ends with END
// NULL is considered same as ""
int endswith(char* s, char* end) {
  if (!s || !end) return !end && s;
  int i = strlen(s)-strlen(end);
  return i<0 ? 0 : strcmp(s+i, end)==0;
}
