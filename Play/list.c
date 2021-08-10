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
  char* key; // managed: we own
  lint num;
  void* data; // owned by user
} list;


// Adds to *DP list a KEY string (copied) associated with NUM storing DATA ptr.
//
// Usage:
//   list* d= NULL;
//   ladd(&d, "a", 42, "FOO");
//   ladd(&d, NULL, 42, "foo");
//   ladd(&d, "b", 0, "foo");
void ladd(list** dp, char* key, lint num, void* data) {
  *dp= malloc(sizeof(list));
  **dp= (list){*dp, key?strdup(key):key, num, data};
}

// Usage:
//   char* bar=  "BAR";
//   list* r=  lfindkey(d, "foo");
//   list* r=  lfindnum(d, 42);
//   list* r=  lfinddata(d, bar);
//   list* r=  lfindhash(d, "foo", 42);
list* lfindkey(list* d, char* key) {
  while(d &&(!d->key || strcmp(d->key, key))) d= d->next;
  return d;
}

list* lfindnum(list* d, lint num) {
  while(d && d->num!=num) d= d->next;
  return d;
}

list* lfinddata(list* d, void* data) {
  while(d && d->data!=data) d= d->next;
  return d;
}

// Find exact using both KEY and NUM.
//   KEY can be NULL but only matches NULL.
//
// Usage:
//   ladd(&d, "foo", 42, "bar");
//   ladd(&d, NULL, 17, "fum");
//
//   r= lfind(d, "foo", 42); // yes
//   r= lfind(d, NULL, 17);  // yes
//   r= lfind(d, "foo", 0);  // NULL
//   r= lfind(d, "fie", 17); // NULL
list* lfind(list* d, char* key, lint num) {
  while(d) {
    if (d->num==num) {
      if (d->key==key) break; // eq/null
      if (key && d->key && strcmp(d->key, key)) break;
    }
    d= d->next;
  }
  return d;
}

// Finds/creates key/num entry.
// Returns NULL or old data value (to free?)
// Usage:
//   char* save= lput(&d, "foo", 0, "new");
//   FREE(lput(&d, "foo", 0, "new"));
void* lput(list** dp, char* key, lint num, void* data) {
  list* r= lfindhash(key, num);
  void* dat= r ? r->data : NULL;
  if (r) ladd(dp, key, num, data);
  r->data= data;
  return dat;
}

// Return the length of the list
size_t llen(list* d) {
  size_t n= 0;
  while(d) d= d->next, n++;
  return n;
}

// Return R removed from the list.
//   NULL if not found.
// NOTE: it's up to user to free R.
//   (use FREE
//
// Usage:
//   // free key + data if on heap
//   free(r->key); free(r->data);
//   lrem(&d, r);
//   free(r); // FREE(r)
list* lrem(list** dp, list* r) {
  if (!dp || !*dp || !r) return NULL;
  list* n= *dp;
  while(n && n!=r) n= *(dp= &n->next);

  if (n) {
    // unlink
    printf("[--unlink %s--]\n", n->key);
    *dp = n->next;
    n->next= NULL;
  }
  return n;
}

void* FREE(r) { if (r) free(r); return NULL; }
#define FREEVAR(r) r= FREE(r)

// Free a list (or a lrem result)
// Returns NULL.
//
// Usage:
//   list* d= NULL;
//   ladd(&d, "foo", 17, "bar");
//   ladd(&d, "fie", 42, "fum");
//   list* r= lfindkey("42");
//   r= lrem(&d, r);
//   char* s= r->data;
//   r= lfree(r, false); // one item NOT del data
//   d= lfree(d); // all of the items del ALL data
list* lfree(list* d, bool freeData) {
  while(d) {
    list* next= d->next;
    FREE(d->key);
    if (freeData) FREE(d->data);
    FREE(d);
    d= next;
  }
  return NULL;
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
