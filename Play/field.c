#include <stdio.h>
#include <stdlib.h>

char *hasfield(char *data, char *name) {
  char *f= strstr(data, name);
  return f && f[strlen(name)]==':'? f+strlen(name)+1 : NULL;
}

long *lfieldp(long *l, char *data, char *name) {
  char *v= hasfield(data, name);
  return v && 1==sscanf(v, "%li", l)? l : NULL;
}

long lifield(char *data, char *name, int dflt) {
  long l;
  return lfieldp(&l, data, name)? l : dflt;
}
//  char *v= hasfield(data, name);
//  long l= dflt;
//  return v && 1==sscanf(v, "%li", &l)? l : dflt;

int ifield(char *data, char *name, int dflt) {
  long l= lifield(data, name, dflt);
  if (l!= (int)l) l= dflt;
  return (int)(l!=(int)l? dflt: l);
}

int snfield(char *ret, int size, char *data, char *name) {
  char *v= hasfield(data, name);
  if (!v) return -1;

  // unquote
  char q= *v, *r= ret;
  if (q=='"' || q=='\'') v++; else q=' ';
  char c;
  while((c= *v) && *v!=q) {
    if (c=='\\' && (c= *++v)=='n') c='\n';
    if (ret && --size>0) *r++= c; else r++;
    v++;
  }
  if (ret) *r++= 0; else r++;
  return r-ret;
}

char *sfield(char *data, char *name, char* dflt) {
  static char ret[1024];
  ret[0]= 0;
  int n= snfield(ret, sizeof(ret), data, name);
  return n<0? dflt : ret;
}

char *msfield(char *data, char *name) {
  int n= snfield(NULL, 0, data, name);
  if (n<0) return NULL;
  char *r= malloc(n);
  snfield(r, n, data, name);
  return r;
}

int main(void) { // ENDWCOUNT
  char *data= "foobar:data int:32 hex:0xff spaces:\"foo bar\" spcs:'foo bar' quoted:\"an inline \\\"string\\\", there\" colon:\"must be\\: quoted\" simply:\\:foo\\:nofield\\: shouldnotseethis newlines:foo\\nbar";
  printf("IN: %s\n", data);

  printf("int:     \t%d  \n", ifield(data, "int", -1));
  printf("hex:     \t0x%x\n", ifield(data, "hex", -1));

  printf("foobar:  \t%s<<<\n", sfield(data, "foobar", "-"));
  printf("spaces:  \t%s<<<\n", sfield(data, "spaces", "-"));
  printf("spcs:    \t%s<<<\n", sfield(data, "spcs", "-"));
  printf("quoted:  \t%s<<<\n", sfield(data, "quoted", "-"));
  printf("colon:   \t%s<<<\n", sfield(data, "colon", "-"));
  printf("simply:  \t%s<<<\n", sfield(data, "simply", "-"));
  printf("nofield: \t%s<<<\n", sfield(data, "nofield", "-"));
  printf("newlines:\t%s<<<\n", sfield(data, "newlines", "-"));
}
