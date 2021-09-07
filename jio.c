// Requires sizeof(int) >= 4

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
//#include <sys/ioctl.h>

#include "jio.h"


int screen_rows= 24, screen_cols= 80;
// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c

void screen_init(int sig) {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  screen_rows = w.ws_row;
  screen_cols = w.ws_col;
}

struct termios _orig_termios, _jio_termios;

void _jio_exit() {
  tcsetattr(0, TCSANOW, &_orig_termios);

  // deinit mouse/jio
  fprintf(stderr, "\e[?1000;1003;1006;1015l");

  // sanity
  cursoron();
}

void jio() {
  // get screen size
  screen_init(0);
  signal(SIGWINCH, screen_init);

  // xterm mouse init
  fprintf(stderr, "\e[?1000;1003;1006;1015h");

  tcgetattr(0, &_orig_termios);
  atexit(_jio_exit);
  _jio_termios= _orig_termios;

  // raw terminal in; catch ^C & ^Z
  cfmakeraw(&_jio_termios);
  // enable terminal out
  _jio_termios.c_oflag |= OPOST;

  tcsetattr(0, TCSANOW, &_jio_termios);

  // catch window resize interrupts
}

////////////////////////////////////////
// - ansi screen

// good ref
// - https://www.ibm.com/docs/en/aix/7.1?topic=x-xterm-command#xterm__mouse

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

color fg(int c) { color r=_fg; printf("\e[38;"); _color(c); _fg= c; return r; }
color bg(int c) { color r=_bg; printf("\e[48;"); _color(c); _bg= c; return r; }

// most colors are per default bold!
//int bold(int c /* 0-7 */) { return c+8; }
int bold(int c /* 0-7 */) { return c+8; }
int rgb(int r, int g, int b /* 0-5 */) { return 0x10+ 36*r + 6*g + b; }
int gray(int c /* 0-7 */) { return 0xe8+  c; }
int RGB(int r, int g, int b /* 0-255 */) { return -(r*256+g)*256+b; }
void underline() { printf("\e[4m"); }
void end_underline() { printf("\e[24m"); }

// Reverse video!
//   Ok, this is going to be "funny". Since I've been using "true video" (white on black) the bold wasn't really bolding black letters, instead I typeset it in red color. (ugh!)
//   So, turns out the solution is to use reverse video. Then bold of black works! (but not of white...).
//   Also, to pull this off, since reverse video makes fg be background and bg be foreground color, we switch those if we're in reverse mode!
//  Second, there is no way to turn off bold, except turn everything off, so we need to recolor it (and for every line of our .ANSI-file. Use recolor();

// TODO:   modify the state anmake it saveable... as a struct.

int _reverse=1; // 1 and bold works for black!

// TODO: is reverse mode with colors reverse a decent mode for "dark/night" theme?

void reverse() { printf("\e[7m"); }

void recolor() {
  if (_reverse) reverse();
  bg(_bg); fg(_fg);
  // TODO: underline, bold, italics, 
  // means need to keep track of state...
}

// adjusted colors
color _C(int n) {
  if (_reverse) reverse();
  // dark blue
  return fg(n);

}

color _B(int n) {
  if (_reverse) reverse();
  return bg(n);
}

// adjusted colors
color C(int n) {
  // TODO: generalize/move to w.c?
  // link color
  if (n==27) 
    if (_reverse && _bg==black)
      n=rgb(0,0,5);
    else
      n=rgb(0,3,5);
  return (_reverse? _B : _C)(n==blue? rgb(0,0,1) : n);
}

color B(int n) {
  return (_reverse? _C : _B)(n==blue? rgb(0,0,1) : n==white ? n+8 : n);
}

color readablefg() {
  // TODO: generalize for rgb and RGB, use hue?
  // TODO: bad name!
  int dark= _bg==black||_bg==blue||_bg==red||_bg==magenta;
  return C(dark? white : black);
}

////////////////////////////////////////
// - keyboard

// bytes buffered
static int _key_b= 0; 

int haskey() {
  if (_key_b>0) return 1;

  struct timeval tv = { 0L, 0L };
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}

// Wait and read a key stroke.
// Returns:
//   - ASCII
//   - UTF-8 bytes
//   - CTRL+'A' .. CTRL+'Z'
//   or if >= 256:
//     - META+'A'
//     - FUNC+1
//     - (CTRL/SHIFT/META)+UP/DOWN/LEFT/RIGHT
//   - TAB S_TAB RETURN DEL BACKSPACE
// Note: NOT thread-safe
keycode key() {
  // TODO: whatis good size?
  // TODO: test how much we can use?
  // estimate: rows*10=???
  static char buf[2048]= {0};

  // get next key
  if (_key_b>0) {
    memcpy(buf, &buf[1], _key_b--);
  } else {
    // read as many as available, as we're blocking=>at least one char
    // terminals return several character for one key (^[A == M-A  arrows etc)
    bzero(buf, sizeof(buf));
    _key_b= read(0, &buf[0], sizeof(buf)) - 1;
  }
  int k= buf[0];

  // TODO: how come I get ^J on RETURN?
  // but not in Play/keys.c ???

  //fprintf(stderr, " {%d} ", k);

  buf[_key_b+1]= 0;

//TDOO: BACKSPACE seems broken!
// or at least in Play/testkeys.c...

//printf("\n [k=%d] \n", k);

  // simple character, or error
  if (_key_b<=0) return _key_b<0? _key_b+1 : k;

  // fixing multi-char key-encodings
  // (these are triggered in seq!)
  if (k==ESC) k=toupper(key())+META;
  if (k==META+'[' && _key_b) k=key()+TERM;
  if (k==TERM+'3' && _key_b) key(), k= DEL;
  if (k==TERM+'<' && _key_b) k= MOUSE;
  if (!_key_b) return k;

  // mouse!
  // - https://stackoverflow.com/questions/5966903/how-to-get-mousemove-and-mouseclick-in-bash/58390575#58390575
  assert(sizeof(k)>=4);
  if (k==MOUSE) {
    int but, r, c, len= 0;
    char m;
    // this is only correct if everything is in the buffer... :-(
    int n= sscanf(&buf[1], "%d;%d;%d%n%c", &but, &c, &r, &len, &m);
    if (n>0) {
      if (n==3) c='m'; // We get ^@0 byte?
      if (n==4) len++; //ok

      //fprintf(stderr, "\n\n[n=%d ==>%d TOUCH.%s %d , %d \"%s\" ]", n, len, m=='M'?"down":"up", r, c, &buf[1]);
      // TODO: be limited to 0-256...?
      k= (m=='M'?MOUSE_DOWN:MOUSE_UP)
        + (but<<16) + (r<<8) + c;

      if (but==64) k+= SCROLL_DOWN;
      if (but==65) k+= SCROLL_UP;
    } else len= 0;
    // eat up the parsed strokes
    while(len-->0) key();
    //printf(" {%08x} ", k);
    return k;
  }

  // CTRL/SHIFT/ALT arrow keys
  if (k==TERM+'1' && _key_b==3 && buf[1]==';') {
    key();
    char mod= key();
    k= UP+ key()-'A';
    switch(mod) {
    case '2': k+= SHIFT; break;
    case '5': k+= CTRL; break;
    case '3': k+= META; break;
    }
  }

  // function keys (special encoding)
  if (k==META+'O') k=key()-'P'+1+FUNC;
  if (k==TERM+'1'&& _key_b==2) k=key()-'0'+FUNC, key(), k= k==5+FUNC?k:k-1;
  // (this only handlines FUNC
  // TODO: how about BRACKETED PASTE?
  // ......^[[200~~/GIT/RetroFit^[[201~
  if (k==TERM+'2'&& _key_b==2) k=key()-'0'+9+FUNC, key(), k= k>10+FUNC?k-1:k;

  return k;
}

// waits for a key to be pressed MAX milliseonds
// Returns passed ms.
int keywait(int ms) {
  long startms= mstime();
  long passed;
  while(!haskey() && (passed= mstime()-startms)<=ms);
  return passed;
}

keycode waitScrollEnd(keycode k) {
  keycode nk;
  while((nk= key()) && (nk & 0xff00ffff)==(k & 0xff00ffff));
  return nk;
}

// Returns a static string describing KEY
// Note: next call may change previous returned value, NOT thread-safe
char* keystring(int k) {
  static char s[32];
  memset(s, 0, sizeof(s));

  if (0) ;
  else if (k==TAB) return "TAB";
  else if (k==RETURN) return  "RETURN";
  else if (k==ESC) return "ESC";
  else if (k<32) sprintf(s, "^%c", k+64);
  else if (k==BACKSPACE) return "BACKSPACE";
  else if (k==DEL) return "DEL";
  else if (k<127) s[0]= k;
  // 127? == delete key?
  else if (k==S_TAB) return "S_TAB";

  else if (k==UP) return "UP";
  else if (k==DOWN) return "DOWN";
  else if (k==RIGHT) return "RIGHT";
  else if (k==LEFT) return "LEFT";

  // TODO: simplify
  else if (k==SHIFT+UP) return "S_UP";
  else if (k==SHIFT+DOWN) return "S_DOWN";
  else if (k==SHIFT+RIGHT) return "S_RIGHT";
  else if (k==SHIFT+LEFT) return "S_LEFT";

  else if (k==CTRL+UP) return "^UP";
  else if (k==CTRL+DOWN) return "^DOWN";
  else if (k==CTRL+RIGHT) return "^RIGHT";
  else if (k==CTRL+LEFT) return "^LEFT";

  else if (k==META+UP) return "M-UP";
  else if (k==META+DOWN) return "M-DOWN";
  else if (k==META+RIGHT) return "M-RIGHT";
  else if (k==META+LEFT) return "M-LEFT";
  // END:TODO:

  else if (k & SCROLL_UP) return "SCROLL_UP";
  else if (k & SCROLL_DOWN) return "SCROLL_DOWN";

  else if (k & MOUSE || k & SCROLL) {
    int b= (k>>16) & 0xff, r= (k>>8) & 0xff, c= k & 0xff;
    sprintf(s, "%s_%s-B%d-R%d-C%d", k&SCROLL?"SCROLL":"MOUSE",
      k&SCROLL?(k&SCROLL_UP?"UP":"DOWN"): k&MOUSE_UP?"UP":"DOWN", b, r, c);
  }
  else if (k>=FUNC && k<=FUNC+12) sprintf(s, "F-%d", k-FUNC);
  else if (k>=META+' ') sprintf(s, "M-%c", k-META);
  else sprintf(s, "\\u%06x", k);
  return s;
}

void testkeys() {
  fprintf(stderr, "\nCTRL-C ends\n");
  for(int k= 0; k!=CTRL+'C'; k= key()) {
    fprintf(stderr, "\n------%s\t", keystring(k));
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
  assert(!"jio.c: input disabled, not working, use edit!\n"); exit(1);

  return NULL;

  clearend();
  cursoron();
  char buf[256]= {0};
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

// Edits a string VALUE of max-SIZE until a non-ALLOWED or a NOT allowed key is typed.
//
// Returns key not allowed or EOF
//
// Edit keys:
//   For now, only BACKSPACE   
//
// Usage:
//   Normal usage is to envelope the envelope the function with a while loop
//   until RETURN si returned.
//    This allows the user program to still handle short-cut keys!
//    
// See Play/edith.c for example

// TODO: generalize with pass in function
//
// validate(k, ds)
//   =>  +2    accept
//   =>  +1    accept & break
//   =>   0    defer to default
//   =>  -1 reject
//   =>  -2 break;

keycode edit(dstr **dsp, int width, char *allowed, char *not, char *breaks) {
  // make sure have a pointer
  *dsp= dstrncat(*dsp, NULL, 1);

  printf("%s", (*dsp)->s); fflush(stdout);

  int k=0;
  while ((k= key())!=EOF) {
    int len= strlen((*dsp)->s);
    if (breaks && strchr(breaks, k)) break;
    if (k>=32 && k<127) {
      if (allowed && !strchr(allowed, k)) break;
      if (not && strchr(not, k)) break;
    } else if (isutf8(k))
      ; // ok
    else if (k==BACKSPACE || k==DEL) {
      if (len>0) {
        bs(); spc(); bs(); fflush(stdout);
        (*dsp)->s[len-1]= 0;
      }
      k= ' ';
      continue;
    } else if (k<32 || k>=256)
      break; // CTRL or META or TERM

    // ignore when full
    if (len>=width && width>=0) continue;
    // accept and output
    char c= k;
    putchar(c); fflush(stdout);
    *dsp= dstrncat(*dsp, &c, 1);
  }
  return k;
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

// Scan FILE until read TILL
// Returns 1: if found
//         0: at eof
int fscan(FILE *f, char *till) {
  if (!f || !till) return 0;
  int n= strlen(till), c;
  char suffix[n+1];
  memset(suffix, 0, sizeof(suffix));
  while((c= fgetc(f))!=EOF) {
    memcpy(suffix, &suffix[1], n);
    suffix[n-1]= c; // add at end
    if (!strcmp(suffix, till)) return 1;
  }
  return 0;
}

// LOL: just for fun...
// // parse till "</script"
// long script= 0;
// do {
//   if (!STEP) return;
//   script<<=8; script+=tolower(c);
// } while (script!=0x3c2f736372697074);

// Quoted Print a string
// >>>|foo %0abar%ffend|<<<#12<"
void qprintstr(char *s) {
  if (!s) {
    printf("(null)");
    fflush(stdout);
    return;
  }
  printf(">>>|");
  while(*s) {
    if (*s>31) putchar(*s);
    else printf("%%%02x", *s);
    s++;
  }
  printf("|<<<#%ld<", strlen(s));
  fflush(stdout);
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
  int n= vsnprintf(dummy, 1024, fmt, argp);
  if (!d || strlen(d->s)+n+1 > d->max)
    d= dstrncat(d, NULL, n+1);
  vsnprintf(d->s+strlen(d->s), n+1, fmt, argp);
  va_end(argp);
  return d;
}

// encode for URI
// we quote any char required plus
// any that may cause trouble in bash
//
// WARNING: has to be in sync with
//    ./wquote
dstr *dstrncaturi(dstr *d, char *s, int size) {
  d= dstrncat(d, NULL, ((size<0 && s)? strlen(s) : size)*11/10);
  
  while (s && *s && (size<0 || size-->0)) {
    if (*s<' ' || *s>127 || strchr(" !#$%&'()*-+:\\/;<>?@[]^{}`~", *s)) {
      char hex[8]; sprintf(hex, "%%%02x", (unsigned char)*s);
      d= dstrncat(d, hex, -1);
    } else {
      d= dstrncat(d, s, 1);
    }
    s++;
  }
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

long utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + 1000000*tv.tv_sec;
}

long mstime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec/1000 + 1000*tv.tv_sec;
}

///////////////////////////////////
// string goodies

void spc() { putchar(' '); }

void bs() { putchar('\b'); }

void repeat(char c, int n) {
  while(n-->0) putchar(c);
}

void spaces(int n) {
  repeat(' ', n);
}

///////////////////////////////////
// string goodies

