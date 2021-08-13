#include <stdio.h>
#include <stdint.h>

#ifdef FOO
char* u32tou8_x(uint32_t cp) {
  static char result[5];
  memset(result, 0, sizeof(result));
  if (cp<0x80) {
    result[0]= cp;
  } else if (cp<0x800) {
    result[0]= 0300 | ((cp >> 6*1) & 077);
    result[1]= 0200 | ((cp >> 6*0) & 077);
  } else if (cp<0x10000) {
    result[0]= 0300 | ((cp >> 6*2) & 077);
    result[1]= 0340 | ((cp >> 6*1) & 077);
    result[2]= 0200 | ((cp >> 6*0) & 077);
  } else {
    result[0]= 0300 | ((cp >> 6*3) & 077);
    result[1]= 0350 | ((cp >> 6*2) & 077);
    result[2]= 0200 | ((cp >> 6*1) & 077);
    result[3]= 0200 | ((cp >> 6*0) & 077);
  }
  return &result[0];
}

char* u32tou8_y(uint32_t cp) {
  static char result[5];
  memset(result, 0, sizeof(result));
  int i;
  result[ (i=3) ] = cp;
  if (cp>=0x80)    result[ (i=3) ] = 0200 | ((cp >> 6*0) & 077);
  if (cp>=0x80)    result[ (i=2) ] = 0200 | ((cp >> 6*1) & 077);
  if (cp>=0x800)   result[ (i=1) ] = 0200 | ((cp >> 6*2) & 077);
  if (cp>=0x10000) result[ (i=0) ] = 0300 | ((cp >> 6*3) & 077);
  if (cp>=0x80) result[i] |= 0xff00 >> (5-i);
  return &result[i];
}
#endif
         
// Convert an Unicode code point to utf-8
// Result is a pointer to static string
char* u32tou8(uint32_t cp) {
  // https://en.m.wikipedia.org/wiki/UTF-8
  printf("=%21o\n", cp);
  static char result[5];
  // Build utf-8 backwards
  result[4] = 0; // zero-terminate
  char* p = &result[3]; // one from last
  if (cp<0x80) {
    *--p = cp;
  } else {
    int m = 0400;
    // the storage bits of first byte must not overlap the 11+0 prefix (m)
    while(cp || (*p & m/2 & ~0200)) {
      // copy six bits
      printf("%03o ", cp & 077);
      *--p = (cp & 077) | 0200;
      cp >>= 6;
      m |= m >> 1; // 10->110 ...
      printf("%03o 0x%2x (%7o) m=%o\n", *p, *p, cp, m);
    }
    *p |= m;
  }
  return p;
}

int main(void) {
  struct {
    char* s;
    uint32_t cp;
  } tests[] = {
    {"$", 0x0024}, // 1 byte
    {"¢", 0x00a2}, // 2 bytes
    {"ह", 0x0939}, {"€", 0x20ac}, {"한", 0xd55c}, // 3 bytes
    {"𐍈", 0x10348}, // 4 bytes
    {NULL, 0}};
    
  for(int i= 0; tests[i].s; i++) {
    char* r= u32tou8(tests[i].cp);
    printf("U+%04x %s => %s\n",
      tests[i].cp, tests[i].s, r);
    do printf(" %03o", *r); while(*(++r));
    putchar('\n');


    r= tests[i].s;
    do printf(" %03o", *r); while(*(++r));
    putchar('\n');
    putchar('\n');
  }
  return 0;
}
