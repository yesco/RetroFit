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
//
// Examples:
//   d= dstrndup(NULL, NULL, prealloclen);
//   d= dstrndup(d, NULL, addalloclen);
//   d= dstrndup(d, "foo", -1); 
//   d= dstrndup(d, "foobar", 3);
//   d= dstrndup(d, &c, 1);
dstr* dstrncat(dstr* d, char* add, int n) {
  int len= d ? strlen(d->s) : 0, max= d ? d->max : 0;
  n= (n<0 && add)? strlen(add) : n;
  if (!d || len+n+1>max) {
    max= ((max+n+1)/DSTR_STEP + 1)*DSTR_STEP*13/10;
    d= realloc(d, sizeof(dstr)+max);
    putchar('.');
    d->s[len]= 0; // if new allocated
  }
  d->max= max;
  if (add) strncat(d->s, add, n);
  return d;
}

dstr* dstrprintf(dstr* d, char* fmt, ...) {
  va_list argp;
  char dummy[1024];
  va_start(argp, fmt);
  printf("111111111\n");
  // TODO:crashes her
  int n= vsnprintf(&dummy, 1024, fmt, argp);
  if (!d || strlen(d->s)+n+1 > d->max)
    d= dstrncat(d, NULL, n+1);
  vsnprintf(d->s+strlen(d->s), n, fmt, argp);
  va_end(argp);
  return d;
}

#ifndef MAIN
#define MAIN __FILE__
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
  free(d); d= NULL;

  printf("\nlongstring add one char * many times\n");
  for(int i=1024; i-->0;) {
    char c= 'x';
    d= dstrncat(d, &c, 1);
  }
  printf("\nlen=%lu max=%d\n", strlen(d->s), d->max);

  printf("---------------\n");
  char buf[1024];
  int n= printf("char=%c str=%s int=%d float=%f\n", 65, "foobar", 4711, 3.141592654);
  printf("n=%d\n", n);

  printf("---------------\n");
  // dstrnprintf()
  dstr* s= dstrncat(NULL, "prefix:", -1);
  printf("\nlen=%lu max=%d str=%s\n", strlen(s->s), s->max, s->s);

  s= dstrncat(s, "foo", -1);
  printf("\nlen=%lu max=%d str=%s\n", strlen(s->s), s->max, s->s);

  s= dstrprintf(s, "char=%c str=%s int=%d float=%f\n", 65, "foobar", 4711, 3.141592654);
  printf("\nlen=%lu max=%d str=%s\n", strlen(s->s), s->max, s->s);

  s= dstrprintf(s, "char=%c str=%s int=%d float=%f\n", 65, "foobar", 4711, 3.141592654);
  printf("\nlen=%lu max=%d str=%s\n", strlen(s->s), s->max, s->s);
}
#endif

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

/*

Tack för reviewn! Det var en bugg på en rad:

  n= (n<0 && add)? strlen(add) : n;

(guarden var d inte add som den ska vara)

funktionen har 5 "olika" anropssätt: 
d= dstrndup(NULL, NULL, prealloclen);
d= dstrndup(d, NULL, addalloclen);
d= dstrndup(d, "foo", -1); 
d= dstrndup(d, "foobar", 3);
d= dstrndup(d, &c, 1);

(FB verkar ha givit mig nya menyer där edit inte fungerar...så fortsätter här)

Den enda felsituation som kan uppstå, så vitt jag ser, är att realloc (malloc) returnerar NULL. Det trappar jag hellre på annat ställe/sätt för hela systemet, med en ersatt malloc.

Jag har designat funktionen så att vid NULL pekare så hanteras det som ingen data.

Förutsatt att realloc funkade så  är d alltid satt när det derefereras, add används bara (med fixen) när den är satt, samma med strncpy.

Detta är per design.

Man kan ha flera specialfall och  lista alla kombinationer, eller så sätter man värdena rätt så att det funkar och man har färre special-fall. 

Jag började exv med två grenar med malloc och realloc,  men det finns ingent anledning egentligen då realloc(NULL, n) är malloc(n). Det kritiska är att max sätts, och att strängen null+termineras. Det funkade I det här fallet.

I företagsproduktionskod skulle jag bygga mer komplicerad och generell kod. Men då blir livet lite tråkigare också...

*/
