#define MAIN

#include <stdint.h>
#include "dstrncat.c"

// long or pointer
typedef uintptr_t dint;

#define DARR_STEP (DSTR_STEP)

typedef struct darr {
  int itembytes, first, last, n;
  dstr* ds;
} darr;

darr* darr(darr* d, int capacity) {
  d= d ? d : calloc(sizeof(*d), 0);
  // TODO: dstr(d, NULL, capacity*sizeof(dint))
  int cap= (d->ds ? d->ds->max : 0) / sizeof(dint);
  if (cap < capacity)
    d->ds= dstr(d->ds, capacity*sizeof(dint)-cap); // grow
  return d;
}

int dlen(darr* d) {
  return !d ? 0 : d->last - d->first;
}

dint dget(darr* d, int i) {
  if (!d || i<0 || i > d->last) return 0; // error
  dint* p= d->ds->s;
  return p[i];
}

// in D array, put DATA at index I 
// returns old_val || 0
dint dput(darr* d, int i, dint data) {
  if (i<0) return 0; // error
  darr(d, i, -1);
  dint* p= d->ds->s;
  dint old= p[i];
  if (!old) d->n++;
  p[i]= data;
  if (i>d->last) d->last= i;
  return old;
}

// stack using: dpush, dpop
// circular with: dunshift, dshift
// WARNING: dput/dget indices are not what you expect
void dpush(darr* d, dint data) {
  darrput(d, d->last+1, data);
  return d;
}

dint dpop(darr* d) {
  dint old= darrput(d, d->last, 0);
  d->last--;
  return old;
}

dunshift(darr* d, dint data) {
  if (--d->first < 0) {
    darr(d, d->last + DARR_STEP);
    memcpy(
    d->first= 0;
  }
  darrput(d, d->first, data);
}

dint dshift(darr* d, dint data) {
  dint old= darrput(d, d->first, 0);
  d->first++;
  return old;
}

int main(void) {
  return 0;
}

