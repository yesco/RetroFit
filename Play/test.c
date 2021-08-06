#include <stdio.h>
#include <stdlib.h>

typedef unsigned long ulong;

typedef struct dlist {
  struct dlist* next;
  ulong hash;
  void* data;
} dlist;

// Usage:
//   dlist* d= NULL;
//   d = dlistadd(d, item);
dlist* dlistadd(dlist* d, ulong hash, void* data) {
  return memcpy(malloc(sizeof(*d)), &(dlist){ d, hash, data}, sizeof(*d));;
}

int main() {
  char* foo="foo";
  char* bar="bar";

  dlist* d= NULL;
  d= dlistadd(d, 42, "foo");
  d= dlistadd(d, 37, "bar");
  //char *f666= dlistfind(d, 666);
  printf("%s\n", d->data);
  printf("%s\n", d->next->data);
  printf("%s\n", d->next->next);
  exit(0);
}
