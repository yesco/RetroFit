// BAD: This is blocking waiting for line to  complete first
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(void) {
  // setlinebuf(stdin);
  int r= setvbuf(stdin, NULL, _IONBF, 0);
  printf("setvbuf=%d\n", r);

  // _IONBF unbuffered
  // _IOLBF line buffered
  // _IOFBF fully buffered

  int c;
  while((c= fgetc(stdin)) >=0) {
    if (c<32)
      printf("^%c", c+64);
    else
      putchar(c);
    putchar('\n');
  }
  return 0;
}
