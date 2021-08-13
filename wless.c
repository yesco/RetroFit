// partly from old imacs.c
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

// TOOD: move from w.c to ansi.c
static void reset() { printf("\e[48;5;0m\e[38;5;7m\e[23m\e[24m"); }

static void clear() { printf("\x1b[2J\x1b[H"); }
static void clearend() { printf("\x1b[K"); }
static void cleareos() { printf("\x1b[J"); } // TODO: add redraw rest of screen (from pointer)
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

enum { ESC=27, BS=8, DEL=127, CTRL=-64, META=128, UP=META+'a', DOWN, RIGHT, LEFT, TAB=9, SHIFT_TAB=META+'z' };

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

int top, right, tab;

#define FIX(a, lo, hi) a=a<lo?lo: a>hi?hi:a

// TODO:
int nlines= 200, nright= 10, ntab= 8;

int lines=20; // screen size

int incdec(int v, int c, int ikey, int dkey, int min, int max, int min2val, int max2val) {
  if (c==ikey) v++;
  if (c==dkey) v--;
  // fix
  if (v<min) v= min2val;
  if (v>max) v= max2val;
  return v;
}

#define COUNT(var, ikey, dkey, limit) var= incdec(var, c, ikey, dkey, 0, limit-1, 0, limit-1)
#define COUNT_WRAP(var, ikey, dkey, limit) var= incdec(var, c, ikey, dkey, 0, limit-1, limit-1, 0)

// --- ACTIONS

void click(int c) {
}

void newtab() {
}

void deltab() {
}

// --- Display

void display(int c) {
  // home
  // (no cleear screen - no flicker)
  cursoroff();
  gotorc(0, 0);
  reset();

  // pretty print state & key
  if (1) { // debug
    printf("%3d %3d %3d ", top, right, tab);
    clearend();
    if (c<32)
      printf(" ^%c", c+64);
    else if (c>127)
      printf(" M-%c", c-META);
    else
      putchar(c);
    putchar('\n');
  }
      
  // build command
  const char command[]= "cat .stdout | perl -0777 -pe 's/(\\n#.*?)+\\n//g' | tail +%d | head -%d";
  char buf[sizeof(command)+1 + 3*15]= {0};
  int len= snprintf(buf, sizeof(buf), command, top+1, lines-2);
  assert(len+5 < sizeof(buf));

  clearend(); putchar('\n');
  fgcolor(0); bgcolor(7); clearend();

  system(buf);

  reset();
  cleareos();
  cursoron();
}


int main(void) {
  clear();

  int c= 0, q=0;
  while(1) {

    display(c);

    // read key
    c= key();
    if (c==ESC) c=toupper(key())+META;
    if (c==META+'[') c=tolower(key())+META;

    // action
    if (c==CTRL+'C') break;
    if (c==CTRL+'Z') kill(getpid(), SIGSTOP);
    // navigation
    if (c=='<') top= 0; // top
    if (c=='>') top= nlines-1; // bottom
    if (c==META+'V' || c==BS || c==DEL) if ((top-= lines-2) < 0) top= 0;
    if (c==CTRL+'V' || c==' ') if ((top+= lines+2) > nlines) top= nlines-1;

    // tab mgt
    if (c>='a' && c<='z') click(c);
    if (c>='A' && c<='Z') click(c);

    if (c==CTRL+'W' || c==CTRL+'K') deltab();
    if (c==CTRL+'N' || c==CTRL+'T') newtab();
    //if (c==CTRL+'T') // new tab
    //if (c==CTRL+'N') // new window
    //if (c==

    COUNT(top, DOWN, UP, nlines);
    COUNT(top, CTRL+'N', CTRL+'P', nlines);

    COUNT_WRAP(right, RIGHT, LEFT, nright);
    COUNT(top, CTRL+'F', CTRL+'B', nlines);

    COUNT_WRAP(tab, TAB, SHIFT_TAB, ntab);

    // quit?
    q= c<33 ? 0 : q*2 + c;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");

  gotorc(9999,9999);
  printf("\nExiting...\n");
  return 0;
}
