#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <locale.h>
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
void fgcolor(int c) { printf("\x1b[[3%dm", c); }
void bgcolor(int c) { printf("\x1b[[4%dm", c); }

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
  if (n==blue) fg(rgb(0, 0, 1)); // darkest blue
  fg(n + 8*(n!=0 && n<8));
}
void B(int n) { bg(n); }

////////////////////////////////////////
// - keyboard

int haskey() {
  struct termios old, tmp;
  tcgetattr(0, &old);
  tmp= old;
  // modify
  cfmakeraw(&tmp); // ^C & ^Z !
  tmp.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(0, TCSANOW, &tmp);

  struct timeval tv = { 0L, 0L };
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  int r= select(1, &fds, NULL, NULL, &tv);

  // restore
  tcsetattr(0, TCSANOW, &old);

  return r;
}

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
  for(int k= 0; k!=CTRL+'C'; k= key()) {
    fprintf(stderr, "%s ", keystring(k));
    while(!haskey()) {
      putchar('.'); fflush(stdout);
      usleep(300*1000);
    }
  }
}

// Print prompt and input from terminal.
// The string can be max 256 chars.
//
// Return string, len>1
//   NULL on error, or empyt line.
char* input(char* prompt) {
  char buf[256]= {0};
  clearend();
  cursoron();
  if (prompt) printf("%s", prompt);
  fflush(stdout);
  // TODO: use for full editing
  //char *s= readline((char*)prompt);
  char *s= fgets(buf, sizeof(buf), stdin);
  // remove ending newline
  if (s && s[strlen(s)-1]=='\n')
    s[strlen(s)-1]=0;
  cursoroff();
  if (s && !*s) s= NULL;
  return s? strdup(s) : NULL;
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

char *strrstr(char *s, char* w) {
  if (!s || !w) return s;
  char *r= s+strlen(s)-strlen(w);
  while(r>=s && !strstr(r, w)) r--;
  return r<s? NULL : r;
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

// char *strpbrk(char *s, char *brk)|0
// char *strdep(char **s, char *delim)
// char *strstr()/strcasestr()|0
// int strspn(char *s, char* accepts)
// int strcspn(char *s, char* rejects)

///////////////////////////////////
// string modifiers
// (safe!)

int lprefix(char *a, char *b) {
  if (!a || !b) return 0;
  int i= 0;
  while (a[i] && b[i] && a[i]==b[i]) i++;
  return i;
}

char *sskip(char *s, char *w) {
  if (!s || !w) return s;
  char *r= s;
  while(*r && *w && *r++==*w++);
  return *w ? s : r;
}

char *struncp(char *s, char *p) {
  if (p) *p= 0;
  return s;
}

char *strunc(char *s, char *w) {
  return struncp(s, strstr(s, w));
}

char *strunch(char *s, char c) {
  return struncp(s, strchr(s, c));
}

char *struncafter(char *s, char *w, char c) {
  char *r= strstr(s, w);
  if (r) memset(r+strlen(w), c, strlen(r)-strlen(w));
  return s;
}

char *sdel(char *s, char *w) {
  char *r= strstr(s, w);
  if (!r) return s;
  strcpy(r, r+strlen(w));
  return s;
}

char *sdelall(char *s, char *w) {
  if (!s || !w) return s;
  char *r;
  while((r= strstr(s, w)))
    sdel(r, w);
  return s;
}

char *srepl(char *s, char *w, char c) {
  char *r= strstr(s, w);
  if (r) while(*w++ && (*r++= c));
  return s;
}

char *sreplbetween(char *s, char *first, char *last, char c, int keep) {
  char *f= strstr(s, first);
  char *l= strrstr(f, last);
  if (f && l) {
    if (!keep) {
      f+= strlen(first);
      l+= strlen(last);
    }
    if (f<l) memset(f+1, c, l-f-1);
  }
  return s;
}

char *scollapse(char *s, char c, int n) {
  char *d= s, *p= d;
  while(*p) {
    // skip till c
    while(*p && *p!=c) *d++= *p++;

    // while c if more than n remove
    int i= 0;
    while(*p && *p==c) {
      if (i++<n)
        *d++= *p++;
      else
        p++;
    }
  }
  *d= 0;
  return s;
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


// Returns current time iso-formatted to seconds w timezone
// Note: The returned pointer is static, so you may need to copy, DON'T free!
char* isotime() {
  static char ret[32];
  memset(ret, 0, sizeof(ret));

  time_t t= time(NULL);
  struct tm *ptm= localtime(&t);
  if (ptm)
    strftime((char*)ret, sizeof(ret), "%FT%T%z", ptm);
  return (char*)ret;
}

// From a unix time epoch tell how long ago that was (or future)
// Returns a static string with "5 seconds ago", "7 months ago", "in 2 years"
char *timeago(time_t epoch) {
  static char r[50];
  time_t now= time(NULL);

  long ss= now-epoch;
  long s= ss<0? -ss: ss;
  long m= (s+30)/60;
  long h= (m+30)/60;
  int d= (h+12)/24;
  int w= (2*d+7)/14;
  int M= (d+15)/30;
  int Y= (d+365/2)/365;
  int D= (Y+5)/10;
  int H= (Y+50)/100;

  char* p= ss<0? strcpy(r, "in ")+3 : r;

  if (0) ;
  else if (s <= 90) sprintf(p, "%ld seconds", s);
  else if (m <= 90) sprintf(p, "%ld minutes", m);
  else if (h <= 72) sprintf(p, "%ld hours", h);
  else if (d <= 30) sprintf(p, "%d day", d);
  else if (w <= 11) sprintf(p, "%d weeks", w);
  else if (M <= 18) sprintf(p, "%d months", M);
  else if (Y <= 64) sprintf(p, "%d years", Y);
  else if (D <= 9) sprintf(p, "%d decades", D);
  else sprintf(p, "%d hundred years", H);
  if (ss>0) strcat(r, " ago");
  return r;
}

// Parses a date (pref ISO-timestamp)
// But can handle partials, YYYY, HH:MM or just date.
// Returns a pointer to static string from timeago
//   If it fails, just return iso string.
char *isoago(char *iso) {
  struct tm tm = {0};

  // fill in current time so we can parse partials (hh:mm)!
  time_t now= time(NULL);
  struct tm *p = localtime(&now);
  if (p) tm= *p;

  // it parses as much as it can
  if (strptime(iso, "%H:%M", &tm))
    return timeago(mktime(&tm));
  else if (strptime(iso, "%Y-%m-%dT%H:%M:%S", &tm) || 
      strptime(iso, "%Y-%m-%d %H:%M:%S", &tm) ||
      strptime(iso, "%Y", &tm) )
    return timeago(mktime(&tm));
  else
    return iso;
}

