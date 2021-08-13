// partly from old imacs.c
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>

enum { ESC=27, CONTROL=-64, META=128, UP=META+'a', DOWN, RIGHT, LEFT, TAB=9, SHIFT_TAB=META+'z' };

int key() {
  struct termios old, tmp;

  tcgetattr(0, &old);

  // modify
  tmp= old;
  //cfmakeraw(&new_termios); // ^C & ^Z !
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
int nlines= 42, nright= 10, ntab= 8;

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

int main(void) {
  int c= 0, q=0;
  while(1) {

    // read key
    c= key();
    if (c==ESC) c=toupper(key())+META;
    if (c==META+'[') c=tolower(key())+META;

    // action
    COUNT(top, DOWN, UP, nlines);
    COUNT_WRAP(right, RIGHT, LEFT, nright);
    COUNT_WRAP(tab, TAB, SHIFT_TAB, ntab);

    printf("%d %d %d ", top, right, tab);

    // pretty print
    if (0) ; // debug
    else if (c<32)
      printf(" ^%c", c+64);
    else if (c>127)
      printf(" M-%c", c-META);
    else
      putchar(c);
    putchar('\n');
      
    // quit?
    q= c<33 ? 0 : q*2 + c;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");

  return 0;
}
