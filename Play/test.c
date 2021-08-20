#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main() {
  char* s="Ａ";
  printf("isalnum=%s %02x %d\n",
         s, s[0], isalnum(s[0]));
  printf("c=%c\n", 25+'a');
  printf("truncate:%.*s:end\n", 0, "FOOBAR");
  exit(0);
}
