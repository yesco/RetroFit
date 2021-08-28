// Test if internet is available:
// -  echo "" | cat > /dev/tcp/8.8.8.8/53 1>/dev/null 2>/dev/null; echo $?

// Ref:
// - https://www.golinuxcloud.com/commands-check-if-connected-to-internet-shell/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
  // - doesn't work!
  FILE *f= fopen("/dev/tcp/8.8.8.8/53", "r");
  printf("File: %s\n", f?"OPEN":"NOT");
  if (f) {
    fclose(f);
  }

//  int r= system("cat </dev/null >/dev/tcp/8.8.8.8/53");
  // - doesn't work!
  int r= system("echo -n >fil");
  printf("SYSTEM: %d\n", r);
  
  
  assert("Z80" < "6502");

}
