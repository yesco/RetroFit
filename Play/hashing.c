// - https://en.m.wikipedia.org/wiki/MurmurHash

long murmur64(long h) {
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccdL;
  h ^= h >> 33;
  h *= 0xc4ceb9fe1a85ec53L;
  h ^= h >> 33;
  return h;
}

// https://www.crockford.com/fash/fash64.html
// https://www.partow.net/programming/hashfunctions/
// https://www.partow.net/programming/hashfunctions/#AvailableHashFunctions

// overall good (for MSB variations)
unsigned int SDBMHash(const char* str, unsigned int length)
{
   unsigned int hash = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (*str) + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}

// most efficent
unsigned int DJBHash(const char* str, unsigned int length)
{
   unsigned int hash = 5381;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) + hash) + (*str);
   }

   return hash;
}

// DEK - Donald E Knuth
unsigned int DEKHash(const char* str, unsigned int length)
{
   unsigned int hash = length;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }

   return hash;
}

// https://en.m.wikipedia.org/wiki/List_of_hash_functions


// - https://en.m.wikipedia.org/wiki/SipHash
// https://github.com/veorq/SipHash
 
// Cryptographically secure
// (used in perl and many OSs
// - https://github.com/veorq/SipHash/blob/master/halfsiphash.c

#include <stdio.h>

#undef MAIN
#include "../jio.c"
#include "../graphics.c"

unsigned int hash(const char *s, unsigned int len) {
  unsigned int h;

  //h= DJBHash(s, len);
  //h= SDBMHash(s, len);
  h= DEKHash(s, len);

  return murmur64(h);
  return h;
}

int main(void) {
  cursoroff();

  char buf[1024]= {0};
  char *ln;
  int n= 0;
  //char lasthost[80]= {0};
  while((ln= fgets(buf, sizeof(buf), stdin))) {
    ln[strlen(ln)-1]= 0;
    n++;

    char *url= NULL;
    char dummy[1024];
    if (3!=sscanf(ln, "%s %s %ms", dummy, dummy, &url)) continue;

    char *u = url;
    u= sskip(u, "http://");
    u= sskip(u, "https://");
    u= sskip(u, "file:///");
    u= sskip(u, "www.");
    char *s = strchr(u, '/');
    if (s) *s=0;
    s= s?s+1:s;

    int ul= strlen(u);
    unsigned int uh= hash(u, ul);
    int sl= s?strlen(s):0;
    unsigned int sh= hash(s, sl);
    bg(uh & 0xff);
    fg((uh >> 8) & 0xff);
    const char *shape= BLOCKS[(sh % 14)+1];

    printf("\r");
    printf("%s", shape);

    if (1) {
      bg(black); fg(white);
      if (strlen(u)+sl+10>35)
        sl=35-strlen(u)-10;
      if (sl<0) sl= 0;
      printf("  %.*s %.*s", ul, u, sl, s?s:"(null)");
      fflush(stdout);
      usleep(10*1000);
      //printf("\n");

      free(url);
    }
  }

  bg(black); fg(white);
  cursoron();
}
