#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DSTR_STEP 64

typedef struct dstr {
  int max;
  char s[0];
} dstr;

// Usage: d= dstrncat(d, "foo", -1);
//   d, add: can be NULL (both->alloc N)
//   n: -1) strlen(add), or copy N chars
// 
// Returns: d or newly allocated dstr
dstr* dstrncat(dstr* d, char* add, int n) {
  int len= d ? strlen(d->s) : 0, max= d ? d->max : 0;
  n= (n<0 && d)? strlen(add) : n;
  if (!d || len+n+1>max) {
    max= ((max+n)/DSTR_STEP + 1)*DSTR_STEP;
    d= realloc(d, sizeof(dstr)+max);
    d->s[len]= 0; // if new allocated
  }
  d->max= max;
  if (add) strncat(d->s, add, n);
  return d;
}

int main(void) {
  dstr* d= NULL;
  // preallocate (add to max)
  // d= dstrncat(d, NULL, +1070);
  while (!d || strlen(d->s) < 1024) {
    // add whole string (strcat)
    d= dstrncat(d, "foo", -1);
    // add n chars from string (strncat)
    d= dstrncat(d, "barfie", 3);
    char spc= ' ';
    // add 1 char!
    d= dstrncat(d, &spc, 1);
    printf("%lu ", strlen(d->s));
  }
  printf("\"%s\"\n", d->s);
  free(d);
}

// poste in FB: Minimalistic Computing
// - https://m.facebook.com/groups/441341066956027/permalink/510852976671502
/*
Dynamic strings in C?

I think it's not the first time, I'v done it, but I feel this one is pretty good; I only have one function

dstr= dstrncat(dstr, char* add, int n);

It automatically allocates if dstr is NULL to start with, otherwise reallocates chunkwise. dstr->s gives access to the c-string. As the string is stored as part of the struct; free can be used without any problem.

I think it trivially can be used instead of all the longwinded dr*vel in https://locklessinc.com/articles/dynamic_cstrings

The example code shows 4 different usages.

(yeah, I need this  for my minimialistic web-browser)
*/

/* answer to question what "n" is:

Like in strncat, it indicates the maximum number of chars to copy.

For convenience, n=-1 works like strcat().

If dstr is NULL (first call), it alocates.

If add is NULL it just extends the buffer capacity with n, so it can be used to preallocate.

The last one just was just a natural side-effect, but useful.

I'm especially happy with that you can add a char c with

d=dstrncat(d, &c, 1);

Google: "man strncat" "man strcat" to see those functions.

This is like a combination of both, plus strdup, and even calloc in one!

Things like this makes C fun. LOL.
*/
