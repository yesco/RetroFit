//
// dmap.c - Dynmamic MAP for C
//
// (>) CC-BY 2021 Jonas S Karlsson
//         jsk@yesco.org

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

void* dlistfind(dlist* d, ulong hash) {
  while (d && d->hash!=hash) d= d->next;
  return d ? d : NULL;
}

void* ddata(dlist* d, void* safe) {
  return d ? d->data : safe;
}

// Usage:
//   d = dlistrem(d, item);
//   d = dlistrem(d, dlistfind(item)); // LOL
// That is: it'll delete node POINTING to item,
// or a NODE passed in. NULLs are ignored.
dlist* dlistrem(dlist* d, void* p) {
  if (!d || !p) return d;
  dlist* n= d->next;
  if (d==p || d->data==p) {
    free(d);
    return n;
  }
  // TODO: get rid of recursion?
  d->next= dlistrem(d->next, p);
  return d;
}

int main(void) {
  dlist* d= NULL;
  d= dlistadd(d, 42, "foo");
  d= dlistadd(d, 42, "FOO");
  d= dlistadd(d, 37, "bar");
  d= dlistrem(d, dlistfind(d, 42));
  dlist *f= dlistfind(d, 42);
  printf("%s\n", (char*)ddata(f, "(NULL)"));
  return 0;
}
