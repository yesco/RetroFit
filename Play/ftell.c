#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(void) {
  long p= ftell(stdin);
  printf("ftell(stdin)=%ld\n", p);
  printf("ftell(stdout)=%ld\n", p);
  p = ftell(stdout);
  printf("This might be erased!\n");
  printf("=================================================------------------------------------------------==\n");
  if (p>0) {
    p= fseek(stdout, p, SEEK_SET);
    printf("OVEWRITEWITH:fseek(stdout)=%ld\n", p);
    p= ftruncate(fileno(stdout), ftell(stdout)-5);
    printf("TRUNCATEDWITH:ftruncate(stdout,ftell(p)-5)=%ld (errono=%d)\n", p, errno);
    
  } else {
    printf("Can't ftell!\n");
  }
  return 0;
}
