//
// list.c - LIST for C
//
// (>) CC-BY 2021 Jonas S Karlsson
//         jsk@yesco.org

#include <stdio.h>
#include <stdlib.h>

typedef intptr_t lint;

typedef struct list {
  struct list* next;
  char* key;
  lint num;
  void* data;
} list;

// Usage:
//   list* d= NULL;
//   d = ladd(d, item);
list* ladd(list* d, char* key, lint num, void* data) {
  return memcpy(malloc(sizeof(*d)), &(list){ d, key, num, data}, sizeof(*d));
}

// Usage:
//   char* bar=  "BAR";
//   list* d=  ladd(NULL, "foo", 42, bar);

//   list* f=  lfindkey(d, "foo");
//   list* f=  lfindnum(d, 42);
//   list* f=  lfindata(d, bar);
list* lfindkey(list* d, char* key) {
  for(; d &&(!d->key || strcmp(d->key, key)); d= d->next) ;
  return d;
}

list* lfindnum(list* d, lint num) {
  for(; d && d->num!=num; d= d->next) ;
  return d;
}

list* lfinddata(list* d, void* data) {
  for(; d && d->data!=data; d= d->next) ;
  return d;
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

char* lstring(list* d) { return d?d->data:"(NULL)"; }
void* ldata(list* d, void* v) { return d?d->data:v; }

// ENDWCOUNT

void lpr(list* p) {
  printf("\n");
  for(; p; p= p->next) {
    printf("%lu %s: %s\n", p->num, p->key ? p->key : "(NULL)", lstring(p));
  }
  printf("\n");
}

int main(void) { 
  list* d= NULL;
  char* mystring="MINE";

  d= ladd(d, "a", 42, "foo");

  d= ladd(d, "b", 42, "FOO");
  d= ladd(d, "c", 37, "bar");
  d= ladd(d, "q", 0, mystring);
  
  lpr(d);

  list* r;

  // find 42 ("FOO", last added 42)
  r= lfindnum(d, 42);
  printf("1. %s - 42: FOO\n", lstring(r));

  // find "a" ("foo", unique)
  r= lfindkey(d, "a");
  printf("2. %s - a: foo\n", lstring(r));

  // find "none" (none)
  r= lfindkey(d, "none");
  printf("3. %s - not have\n", lstring(r));

  // remove it
  d= lrem(d, lfindnum(d, 42));

  // safe to try to remove non-existing
  d= lrem(d, lfindnum(d, 4711));

  // find 42 ("foo")
  r= lfindnum(d, 42);
  printf("4. %s - 42: foo\n", lstring(r));
  d= lrem(d, r);

  // find nothing
  r= lfindnum(d, 42);
  printf("5. %s - nothing\n", lstring(r));

  // find mystring ("mystring")
  r= lfinddata(d, mystring);
  printf("6. %s - 42: mystring\n", (char*)ldata(r, "not"));
  d= lrem(d, r);

  printf("\n--- only 37: bar - remains\n");
  lpr(d);

  return 0;
}
