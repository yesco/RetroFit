//
// dlist.c - Dynmamic LIST for C
//
// (>) CC-BY 2021 Jonas S Karlsson
//         jsk@yesco.org

#include <stdio.h>
#include <stdlib.h>

typedef unsigned long ulong;

typedef struct list {
  struct list* next;
  ulong hash;
  char* key; // TODO:
  void* data;
} list;

// Usage:
//   dlist* d= NULL;
//   d = dlistadd(d, item);
list* ladd(list* d, ulong hash, void* data) {
  return memcpy(malloc(sizeof(*d)), &(dlist){ d, hash, data}, sizeof(*d));;
}

list* lfind(list* d, ulong hash) {
  while (d && d->hash!=hash) d= d->next;
  return d ? d : NULL;
}

void* ldata(list* d, void* safe) {
  return d ? d->data : safe;
}

// Usage:
//   d = lrem(d, item);
//   d = lrem(d, lfind(item)); // LOL
// That is: it'll delete node POINTING to item,
// or a NODE passed in. NULLs are ignored.
list* lrem(list* d, void* p) {
  if (!d || !p) return d;
  list* n= d->next;
  if (d==p || d->data==p) {
    free(d);
    return n;
  }
  // TODO: get rid of recursion?
  d->next= lrem(d->next, p);
  return d;
}

int main(void) {
  list* d= NULL;
  d= ladd(d, 42, "foo");
  d= ladd(d, 42, "FOO");
  d= ladd(d, 37, "bar");
  d= lrem(d, lfind(d, 42));
  list *f= lfind(d, 42);
  printf("%s\n", (char*)ldata(f, "(NULL)"));
  return 0;
}
