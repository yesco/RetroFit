// partly from old imacs.c
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

// normally don't include .c...
#include "jio.c"

// - limits
int nlines= 12000, nright= 10, ntab= 8;

int rows;

// - state
int top, right, tab;

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

void display(char* file, int line, int c) {
  // home
  // (no cleear screen - no flicker)
  cursoroff();
  gotorc(0, 0);
  reset();

  // pretty print state & key
  // TODO: remove
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
  const char command[]= "./wdisplay %s %d %d";
  char buf[sizeof(command)+1 + 1024]= {0};
  int len= snprintf(buf, sizeof(buf), command, file?file:".stdout", top+1, rows);
  assert(len+5 < sizeof(buf));

  clearend(); putchar('\n');
  fgcolor(0); bgcolor(7); clearend();

  system(buf);

  reset();
  cleareos();
  cursoron();
}

void help() {
  system("./w wless.html");
}

int main(void) {
  screen_init();
  rows = screen_rows-5;
  clear();

  char* file= ".stdout";

  int c= 0, q=0;
  while(1) {

    display(file, top, c);

    // read key
    c= key();
    if (c==ESC) c=toupper(key())+META;
    if (c==META+'[') c=tolower(key())+META;

    // action
    //if (c==CTRL+'H') help();
    if (c==CTRL+'C' ||c=='q') break;
    if (c==CTRL+'Z') kill(getpid(), SIGSTOP);
    // navigation
    if (c=='<') top= 0; // top
    if (c=='>') top= nlines-1; // bottom
    if (c==META+'V' || c==DELETE || c==DEL) if ((top-= rows) < 0) top= 0;
    if (c==CTRL+'V' || c==' ') if ((top+= rows) > nlines) top= nlines-1;

    // tab mgt
    if (c>='a' && c<='z') click(c);
    if (c>='A' && c<='Z') click(c);

    if (c==CTRL+'W' || c==CTRL+'K') deltab();
    if (c==CTRL+'T') newtab();
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
