#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main() {
  char* s="Ａ";
  printf("isalnum=%s %02x %d\n",
         s, s[0], isalnum(s[0]));
  exit(0);
}
