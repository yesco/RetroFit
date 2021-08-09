
// w3c colors
// - https://www.w3.org/wiki/CSS/Properties/color/keywords
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "../colors.h"

typedef char TAG[32];

// #112233 aabbcc #abc abc rgb(170, 33, 33)
int parse_color(TAG fnd, int dflt) {
  // parse numeric formats in fnd
  int32_t r=0, g=0, b=0, c=0;
  assert(sizeof(int)==4);
  if (sscanf(fnd, " rgb ( %i , %i , %i )", &r, &g, &b)) return -((256*r+g)*256+b);
  if (sscanf(fnd, "%x", &c) || sscanf(fnd, "#%x", &c)) {
    if (strlen(fnd)<6 && c<16*16*16) {
      b= c & 0xf; g= (c>>=4) & 0xf, r= (c>>4);
      return -((256*(16*r+r) + (16*g+g))*256 + (16*b+b));
    } else {
      return -c;
    }
  }
  return dflt;
}

// pink gray lightgreen cyan ...
// RGB (256^3) or rgb (0-255) color for C()/B()
int decode_color(char* name, int dflt) {
  int32_t c= parse_color(name, -1);
  if (c!=-1) return c;
  
  // search for ' name#'
  TAG fnd;
  snprintf(fnd, sizeof(fnd), " %s#", name);

  // if answer put in fnd
  char* m= strcasestr(COLORS, fnd);
  if (!m) return dflt;

  // skip ' name#'
  return parse_color(m+strlen(name)+2, dflt);
}

int main(int argc, char* argv[]) {

  char* name= NULL;

  name= "112233";
  printf("color '%s' has value: %x\n\n", name, -decode_color(name, -1));

  name= "#321";
  printf("color '%s' has value: %x\n\n", name, -decode_color(name, -1));

  name= "rgb(128, 64 , 32)";
  printf("color '%s' has value: %x\n\n", name, -decode_color(name, -1));

  name= "FCC";
  name= "#FCC";
  name= "FFC0CB";
  name= "FFC0CB plum#abc";
  name= "FFC0CB plum#DDA0DD powderblue#B0";
  printf("color '%s' has value: %x\n\n", name, -decode_color(name, -1));

  name= "pink";
  printf("color '%s' has value: %x\n\n", name, -decode_color(name, -1));

  return 0;
}
