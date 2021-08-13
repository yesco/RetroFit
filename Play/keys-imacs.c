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

  char buf[32] = {0};
  // it blocks, but when it returns it can return "all", so for arrows that emit several keys, it
  int n= read(0, &buf[0], sizeof(buf)-1);
  printf("[%d:", n);
  char* p= &buf[0];
  while (*p) { printf(*p<32?"^%c":"%c", *p<32?*p+64:*p); p++; }
  printf("]\n");

  int c= buf[0];
  //int c= getchar();

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
