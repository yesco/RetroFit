#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <assert.h>

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

// Note: NOT thread-safe
int key() {
  static int b= 0;
  static char buf[32]= {0};

  struct termios old, tmp;
  tcgetattr(0, &old);

  // modify
  tmp= old;
  cfmakeraw(&tmp); // ^C & ^Z !
  tmp.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(0, TCSANOW, &tmp);

  // get next c
  // hacky code, set c by shifting bytes
  if (b>0) {
    memcpy(&buf[0], &buf[1], b--);
  } else {
    // read as many as available, as we're blocking=>at least one char
    // terminals return several character for one key (^[A == M-A  arrows etc)
    b= read(0, &buf[0], sizeof(buf)) - 1;
  }
  int k= buf[0];

  // restore
  tcsetattr(0, TCSANOW, &old);

  // simple character, or error
  if (b<=0) return b<0? b+1 : k;

  // fixing multi-char key-encodings
  // (these are triggered in seq!)
  if (k==ESC) k=toupper(key())+META;
  if (k==META+'[' && b) k=key()+TERM;
  if (k==TERM+'3' && b) key(), k= DEL;
  if (!b) return k;


  // function keys (special encoding)
  if (k==TERM+'O') k=key()-'P'+1+TERM;
  if (k==TERM+'1') k=key()-'0'+TERM, key(), k= k==5+TERM?k:k-1;
  if (k==TERM+'2') k=key()-'0'+9+TERM, key(), k= k>10+TERM?k-1:k;

  return k;
}

// Returns a static string describing KEY
// Note: next call may change previous returned value, NOT thread-safe
char* keystring(int c) {
  static char s[10];
  memset(&s[0], 0, sizeof(s));

  if (0) ;
  else if (c==TAB) return "TAB";
  else if (c==RETURN) return  "RETURN";
  else if (c==ESC) return "ESC";
  else if (c<32) sprintf(s, "^%c", c+64);
  else if (c<127) s[0]= c;
  else if (c==BACKSPACE) return "BACKSPACE";
  else if (c==DEL) return "DEL";
  // 127? == delete key?
  else if (c==S_TAB) return "S_TAB";
  else if (c==UP) return "UP";
  else if (c==DOWN) return "DOWN";
  else if (c==RIGHT) return "RIGHT";
  else if (c==LEFT) return "LEFT";
  else if (c>=TERM) sprintf(s, "F-%d", c-TERM);
  else if (c>=META+' ') sprintf(s, "M-%c", c-META);
  return &s[0];
}

void testkeys() {
  fprintf(stderr, "\nCTRL-C ends\n");
  for(int k= 0; k!=CTRL+'C'; k= key())
    fprintf(stderr, "%s ", keystring(k));
}

////////////////////////////////////////
// file IO

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

int isinsideutf8(int c) {
  return (c & 0xc0) == 0x80;
}

int isstartutf8(int c) {
  return (c & 0xc0) == 0xc0;
}

int isutf8(int c) {
  // oversimplification but...
  return c & 0x80;
}

int isfullwidth(int c) {
  // TODO: ... chinse, dingbats, sym etc..
  return c>=0xff01 && c<0xff01+96;
}

int iszerowidth(int c) {
  // TODO: ... combinding chars etc...
  // TOD0: zero-width space
  return c<32;
}

////////////////////////////////////////
// Dynamic STRings (see Play/dstrncat.c)

// Usage: d= dstrncat(d, "foo", -1);
//   d, add: can be NULL (both->alloc N)
//   n: -1) strlen(add), or copy N chars
// 
// Returns: d or newly allocated dstr
dstr* dstrncat(dstr* d, char* add, int n) {
  int len= d ? strlen(d->s) : 0, max= d ? d->max : 0;
  n= (n<0 && add)? strlen(add) : n;
  if (!d || len+n+1>max) {
    max= ((max+n)/DSTR_STEP + 1)*DSTR_STEP*13/10;
    d= realloc(d, sizeof(dstr)+max);
    d->s[len]= 0; // if new allocated
  }
  d->max= max;
  if (add) strncat(d->s, add, n);
  return d;
}

// Dynamic string D printf using FORMAT with ARGUMENTS (...)
//
// Usage: d= dstrprintf(d, format, ...)
//     d= dstrprintf(d, "c=%c s=%s i=%d f=%f\n", 65, "foobar", 4711, 3.1415);
// Returns: d or newly allocated dstr
dstr* dstrprintf(dstr* d, char* fmt, ...) {
  va_list argp;
  char dummy[1024];
  va_start(argp, fmt);
  // TODO:crashes her
  int n= vsnprintf(&dummy, 1024, fmt, argp);
  if (!d || strlen(d->s)+n+1 > d->max)
    d= dstrncat(d, NULL, n+1);
  vsnprintf(d->s+strlen(d->s), n+1, fmt, argp);
  va_end(argp);
  return d;
}

