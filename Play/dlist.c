//
// list.c - LIST for C
//
// (>) CC-BY 2021 Jonas S Karlsson
//         jsk@yesco.org

// This is a linked-list, with a twist.
// It basically, allows you to store a
//
//   map: key->value
// or
//   map: num->value
//
// If you only have a numeric key value
// you get an additional char* pointer to use.
//
// If you only have a string key value
// you get an additonal integer to use.
//
// However, if you store hashed key in
// a linked list, you can cache the pseudo-hash
// together with the key. For searching this
// works like a faster discriminator.

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
//   ladd(&d, "a", 42, "FOO");
//   ladd(&d, NULL, 42, "foo");
//   ladd(&d, "b", 0, "foo");
void ladd(list** dp, char* key, lint num, void* data) {
  *dp= memcpy(malloc(sizeof(list)), &(list){ *dp, key, num, data}, sizeof(list));
}

// Usage:
//   char* bar=  "BAR";
//   list* r=  lfindkey(d, "foo");
//   list* r=  lfindnum(d, 42);
//   list* r=  lfindata(d, bar);
//   list* r=  lfindhash(d, "foo", 42);
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

list* lfindhash(list* d, char* key, lint num) {
  for(; d && d->num!=num && strcmp(d->key, key); d= d->next) ;
  return d;
}

// A found result can be removed
// Usage:
//   // free key + data if on heap
//   free(r->key); free(r->data);
//   lrem(&d, r);
//   free(r);
list* lrem(list** dp, list* r) {
  if (!dp || !*dp || !r) return NULL;
  list* n= *dp;
  while (n && n!=r)
    n= *(dp=&n->next);

  if (n) {
    // unlink
    printf("[--unlink %s--]\n", n->key);
    *dp = n->next;
    n->next= NULL;
  }
  return n;
}

void* FREE(r) { if (r) free(r); return NULL; }
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

  ladd(&d, "a", 42, "foo");
  
  ladd(&d, "b", 42, "FOO");
  ladd(&d, "c", 37, "bar");
  ladd(&d, "q", 0, mystring);
  
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
  lrem(&d, lfindnum(d, 42));

  // safe to try to remove non-existing
  lrem(&d, lfindnum(d, 4711));

  // find 42 ("foo")
  r= lfindnum(d, 42);
  printf("4. %s - 42: foo\n", lstring(r));
  lrem(&d, r);

  // find nothing
  r= lfindnum(d, 42);
  printf("5. %s - nothing\n", lstring(r));

  // find mystring ("mystring")
  r= lfinddata(d, mystring);
  printf("6. %s - 42: mystring\n", (char*)ldata(r, "not"));
  lrem(&d, r);

  //lrem(&d, lfindkey(d, "c"));

  printf("\n--- only 37: bar - remains\n");
  lpr(d);

  return 0;
}
