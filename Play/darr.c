#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// long or pointer
typedef intptr_t dint;

#define DARR_STEP 64

typedef struct darr {
  int first, next, max;
  dint data[0];
} darr;

darr* darray(darr* d, int capacity) {
  int max= d ? d->max : 0;
  if (capacity < max) return d;
  capacity+= DARR_STEP;
  int bytes= sizeof(darr)+ capacity*sizeof(dint);
  d = realloc(d, bytes);
  if (!max) memset(d, 0, bytes);
  d->max = capacity;
  return d;
}

int dlen(darr* d) {
  return !d ? 0 : d->next - d->first;
}

// from D array at index I get data
dint dget(darr* d, int i) {
  if (i<0) i=d->next+i; // from end
  if (!d || i<0 || i >= d->next) return 0; // error
  return d->data[i];
}

// in D array, put DATA at index I 
// returns old_val || 0
darr* dput(darr* d, int i, dint data) {
  if (i<0) i=d->next+i; // from end
  if (i<0) return d; // error - ignore!
  d= darray(d, i);
  d->data[i]= data;
  if (i >= d->next) d->next = i+1;
  return d;
}

// stack using: dpush, dpop
// circular with: dunshift, dshift
// WARNING: dput/dget indices are not what you expect
darr* dpush(darr* d, dint data) {
  return dput(d, d->next, data);
}

dint dpop(darr* d) {
  if (d->next == d->first) return 0;
  dint val= dget(d, d->next-1);
  dput(d, d->next-1, 0);
  d->next--;
  return val;
}

darr* dunshift(darr* d, dint data) {
  if (--d->first < 0) {
    darray(d, d->max + DARR_STEP);
    memcpy(d->data+DARR_STEP, d->data, (d->max-DARR_STEP)*sizeof(dint));
    memset(d->data, 0, DARR_STEP*sizeof(dint));
    d->first+= DARR_STEP;
    d->next+= DARR_STEP;
  }
  return dput(d, d->first, data);
}

dint dshift(darr* d) {
  if (d->first >= d->next) return 0;
  // TODO: shrink if used as circular...
 // if (d->first > 2*DARR_STEP) ...
  dint val= dget(d, d->first);
  dput(d, d->first, 0);
  d->first++;
  return val;
}

/* ENDWCOUNT */
void dprarr(darr* d) {
  printf("\n\ndaray[%d]={ first=%d next=%d max=%d\n", dlen(d), d->first, d->next, d->max);
  for(int i=0; i<d->max; i++) {
    dint p = dget(d, i);
    if (p) 
      printf("%d: %ld\n", i, p);
    else 
      putchar('.');
  }
  printf("\n");
}

int main(void) {
  dint p;

  printf("\n======== array: put get\n");
  darr* d= NULL; //darray(NULL, 0);
  d= dput(d, 0, 100);
  d= dput(d, 1, 101);
  p = dget(d, 1); printf("[1] %ld == 101?\n", p);
  d= dput(d, 2, 102);
  d= dput(d, 3, 103);
  p = dget(d, 4); printf("emty [4] %ld\n", p);
  d= dput(d, 4, 104);

  dprarr(d);

  printf("\n======== stack: push + pop\n");
  d= dpush(d, 105);
  d= dpush(d, 106);

  dprarr(d);

  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("pop %ld\n", p);
  p = dpop(d); printf("over pop %ld (poop?)\n", p);
  p = dpop(d); printf("over pop %ld (poop?)\n", p);

  dprarr(d);

  printf("\n======== expand down\n");
  d= dunshift(d, 200);
  dprarr(d);

  printf("\n======== queue: push + shift\n");
  d= dpush(d, 300);
  d= dpush(d, 301);
  d= dpush(d, 302);
  p = dshift(d); printf("shift %ld\n", p);
  p = dshift(d); printf("shift %ld\n", p);
  d = dpush(d, 303);
  dprarr(d);

  p = dshift(d); printf("shift %ld\n", p);
  p = dshift(d); printf("shift %ld\n", p);
  p = dshift(d); printf("shift %ld\n", p);
  
  return 0;
}

