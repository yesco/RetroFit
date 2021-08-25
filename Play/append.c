#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

char buf[1024];
long three= -1;

void readall(FILE *f) {
  static long pos= 0;
  fseek(f, pos, SEEK_SET);
  printf("===readall\n");
  int n= 0;
  while(fgets(buf, sizeof(buf), f)) {
    if (++n==3) three= ftell(f);
    printf("%d e@%ld: %s", n, ftell(f), buf);
  }
  // make it read where it left off next time
  pos= ftell(f);
}

int main(void) {
  FILE *f= fopen("foo", "a+");
  printf("Tell%ld\n", ftell(f));

  fseek(f, 0, SEEK_SET);
  readall(f);

  snprintf(buf, sizeof(buf), "first time %ld\n", time(NULL));
  fputs(buf, f);
  printf("%s\n", buf);

  printf("Tell%ld\n", ftell(f));

  readall(f);

  // write
  snprintf(buf, sizeof(buf), "time %ld\n", time(NULL));
  fputs(buf, f);
  printf("wrote: %s\n", buf);
  printf("Tell%ld\n\n", ftell(f));

  // write
  snprintf(buf, sizeof(buf), "time %ld\n", time(NULL));
  fputs(buf, f);
  printf("wrote: %s\n", buf);
  printf("Tell%ld\n\n", ftell(f));

  readall(f);

  three= fseek(f, three, SEEK_SET);
  printf("seek three=%ld\n", three);
  printf("read: %s\n", fgets(buf, sizeof(buf), f));
  printf("Tell%ld\n", ftell(f));

  readall(f);

  //p= ftruncate(fileno(stdout), ftell(stdout)-5);
  return 0;
}
