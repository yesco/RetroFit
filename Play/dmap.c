#include "dlist.c"
#include "darr.c"

#define LARSONS_SALT 0

static ulong larsons_hash(const char* s, int len) {
  len = len<0 ? strlen(key) : len;
  unsigned long h = LARSONS_SALT;
  while (len-- > 0 && *s)
    h = h * 101 + *(unsigned char*)s++;
  return h;
}


typdef struct map {
  int n, capacity;
  ulong mask;
  darr* slots;
} map;


ulong linear_hash(map* m, ulong h) {
  ulong i= h & m->mask;
  if (i > m->n) i &= (m->mask>>1);
  return i;
}

// TODO: could return struct to record pos?
list* mfind(map* m, char* key, int len) {
  ulong h= larsons_hash(key, len);
  ulong i= linear_hash(h, m->mask);
  list* l= dget(slots, i);

  // find it
  while (l=lfind(l, h)  && !strncmp(l->key, key, len))
    l= l->next;

  return l;
}

// over write - no duplicate keys
map* mset(map* m, char* key, int len, void* data) {
  ulong h= larsons_hash(key, len);
  ulong i= linear_hash(h, m->mask);
  list* l= dget(slots, i);

  // find it
  while (l=lfind(l, h)  && !strncmp(l->key, key, len))
    l= l->next;

  if (l) {
    // there -> modify
    free(l->data);
    l->data= data;
  } else {
    // not there -> add
    l= ladd(l, h, key, data);
    m->slots= dput(m->slots, i, l);
  }
  return m;
}

// add - duplicate keys ok
map* madd(map* m, char* key, int len, void* data) {
  m= map(m);
  ulong h= larsons_hash(key, len);
  ulong i= linear_hash(h, m->mask);
  list* l= dget(slots, i);

  l= ladd(l, h, key, data);
  m->slots= dput(m->slots, i, l);
  return m;
}

void mrem(map* m, list* node) {
  ulong h= l->hash;
  ulong i= linear_hash(h, m->mask);
  list* l= dget(slots, i);

  l= lrem(l, h, node);
  m->slots= dput(m->slots, i, l);
  
  //free(node); // ??
}


list* mremoved(map* m, char* key, int len) {
  list* l= mfind(m, key, len);
  mrem(m, l);
  return l;
}



int main(void) {
  map* m= NULL;
  m= madd(m, "foo", -1, "data");
  m= madd(m, "bar", -1, "more");

  list* n= mfind(m, "foo", -1);
  m= hrem(m, n);

  char* r= mremoved(m, "bar", -1);

  return 0;
}
