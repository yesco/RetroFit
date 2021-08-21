#include <stdio.h>
#include <string.h>

// char *strpbrk(char *s, char *brk)|0
// char *strdep(char **s, char *delim)
// char *strstr()/strcasestr()|0
// int strspn(char *s, char* accepts)
// int strcspn(char *s, char* rejects)

char *sskip(char *s, char *w) {
  if (!s || !w) return s;
  char *r= s;
  while(*r && *w && *r++==*w++);
  return *w ? s : r;
}

char *struncp(char *s, char *p) {
  if (p) *p= 0;
  return s;
}

char *strunc(char *s, char *w) {
  return struncp(s, strstr(s, w));
}

char *strunch(char *s, char c) {
  return struncp(s, strchr(s, c));
}

char *struncafter(char *s, char *w, char c) {
  char *r= strstr(s, w);
  if (r) memset(r+strlen(w), c, strlen(r)-strlen(w));
  return s;
}

char *sdel(char *s, char *w) {
  char *r= strstr(s, w);
  if (!r) return s;
  strcpy(r, r+strlen(w));
  return s;
}

char *sdelall(char *s, char *w) {
  if (!s || !w) return s;
  char *r;
  while((r= strstr(s, w)))
    sdel(r, w);
  return s;
}

char *srepl(char *s, char *w, char c) {
  char *r= strstr(s, w);
  if (r) while(*w++ && (*r++= c));
  return s;
}

char *strrstr(char *s, char* w) {
  if (!s || !w) return s;
  char *r= s+strlen(s)-strlen(w);
  while(r>=s && !strstr(r, w)) r--;
  return r<s? NULL : r;
}

char *sreplbetween(char *s, char *first, char *last, char c, int keep) {
  char *f= strstr(s, first);
  char *l= strrstr(f, last);
  if (f && l) {
    if (!keep) {
      f+= strlen(first);
      l+= strlen(last);
    }
    if (f<l) memset(f+1, c, l-f-1);
  }
  return s;
}

char *scollapse(char *s, char c, int n) {
  char *d= s, *p= d;
  while(*p) {
    // skip till c
    while(*p && *p!=c) *d++= *p++;

    // while c if more than n remove
    int i= 0;
    while(*p && *p==c) {
      if (i++<n)
        *d++= *p++;
      else
        p++;
    }
  }
  *d= 0;
  return s;
}

int main(void) {
//  char u[]= "http://www.digit.com/index.html/foo/bar/fie/fum/cgi-bin/foo.html?fish&more&and&more";
  char u[]= "http://patrickgillespie.com/";

  char *r= u;
  printf("%s\n", r);
  r= sskip(r, "https://");
  r= sskip(r, "http://");
  r= sskip(r, "www.");
  printf("%s\n", r);
  r= struncafter(r, "&", '.');
  //r= strunc(r, "?");
  //r= strunch(r, '?');
  r= sdel(r, "/cgi-bin");
  r= sdel(r, "/cgi");
  r= srepl(r, "index", '.');
  printf("%s\n", r);
  r= sdelall(r, ".html");
  r= sdel(r, ".htm");
  printf("%s\n", r);

  r= sreplbetween(r, "/", "/", '.', 0);
  printf("%s\n", r);
  r= scollapse(r, '.', 3);
  
  printf("%s\n", r);
}
