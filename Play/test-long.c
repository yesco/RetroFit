#include <stdio.h>

void fun() {
  printf("FUN");
}

long long string = "string";
long long l = "foo";
long long f = fun;

void fun2() {
  printf("FUN");
}

long long f2 = fun2;

typedef (*F)();

// this function must be defined "last"
// (It relies on function < "foo" !)
int isString(long long l) {
  return (l > isString);
}

int is(long long l) {
  printf("%d ", isString(l));
}

void main() {
  long l = -42;
  sscanf("&#4711;", "&#%li;", &l);
  printf("l=%li\n", l);
  exit(0);

#ifdef FOO
  long long c = calloc(35, 0);
  strcpy(c, "heap");

  is(f); printf("fun   : %ld ", f);
  ((F)f)(); printf("\n");
  is(f2); printf("fun2  : %ld ", f2);
  ((F)f2)(); printf("\n");
  
  is(string); printf("string: %ld %s\n", string, string);
  is(l); printf("string: %ld %s\n", l, l);
  is(c); printf("string: %ld %s\n", c, c);
#endif
}
