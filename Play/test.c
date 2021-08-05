#include <stdio.h>
#include <stdlib.h>

int main() {
  long l = -42;
  sscanf("&#xff21;42", "&#x%lx;", &l);
  sscanf("&#4711;42", "&#%li;", &l);
  printf("l=%li\n", l);
  exit(0);
}
