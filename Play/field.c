#include <stdio.h>
#include <stdlib.h>

//#include "../jio.h"
#include "../jio.c"

char *hasfield(char *data, char *name) {
  char nm[strlen(name)+3];
  sprintf(nm, " %s:", name);
  char *f= strstr(data, nm);
  return f && f[strlen(nm)-1]==':'? f+strlen(nm) : NULL;
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

#define FIELD_CHARS ":\n\\"

dstr *dstrcatfield(dstr *d, char *name, char *val) {
  //  make sure it is prepared
  if (!name || !val)
    return dstrncat(d, NULL, 0);
  
  // - calculate length
  // >> 234:"789\\BC"<<< ==> D=14
  int len= strlen(name)+2+strlen(val)+2;
  // count number of '\\'
  char *r= val;
  while(*r)
    if (strchr(FIELD_CHARS, *r++))
      len++;
  char *quote= strchr(val, '\'');
  if (!quote) quote= strchr(val, '"');
  char q= 0;
  if (!quote && strchr(val, ' ')) quote="'";
  if (quote) q= '\''+'"'-quote[0];
  // - allocate
  d= dstrncat(d, NULL, len);

  // - cat and quote
  r= d->s+strlen(d->s);

  // name
  if (r!=d->s || *(r-1)!=' ') *r++ = ' ';
  *r = 0;
  r= stpcpy(r, name);
  *r++ = ':';
  
  // value (quoted as need)
  if (quote) *r++ = q;
  char c;
  while (c= *val) {
    if (strchr(FIELD_CHARS, c)) {
      if (c=='\n') c= 'n';
      *r++ = '\\';
    }
    *r++ = c;
    val++;
  }
  if (quote) *r++ = q;
  *r++ = 0;
  return d;
}

int main(void) { // ENDWCOUNT
  char *data= " foobar:data bar:BAR int:32 hex:0xff spaces:\"foo bar\" spcs:'foo bar' quoted:\"an inline \\\"string\\\", there\" colon:\"must be\\: quoted\" simply:\\:foo\\:nofield\\: shouldnotseethis newlines:foo\\nbar";
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

  // make sure gives "BAR"!
  printf("\nbar:    \t%s<<<\n", sfield(data, "bar", "-"));

  dstr *d= NULL;
  d= dstrcatfield(d, "foobar", "data");
  d= dstrcatfield(d, "bar", "BAR");
  //d= dstrcatifield(d, "int", 32);
  //d= dstrcatifield(d, "hex", 0xff);
  d= dstrcatfield(d, "spaces", "foo bar");
  d= dstrcatfield(d, "spcs", "an inline \"string\" there");
  d= dstrcatfield(d, "colon", "must be: quoted");
  d= dstrcatfield(d, "simply", ":foo:nofield:");
  d= dstrcatfield(d, "newlines", "foo\nbar");

  printf("\nDSTR=%s<<<\n", d->s);
}

