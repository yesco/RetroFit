// partly from old imacs.c
#include <stdlib.h>
#include <string.h>
#include <termios.h>

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

int main(void) {
  int c= 0, q=0;
  while(1) {
    c= key();
    if (c < 32)
      fprintf(stderr, "^%c", c+64);
    else
      fputc(c, stderr);
      
    // quit?
    q= c<33 ? 0 : q*2 + c;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");

  return 0;
}
