#include <stdio.h>
#include <string.h>

// char *strpbrk(char *s, char *brk)|0
// char *strdep(char **s, char *delim)
// char *strstr()/strcasestr()|0
// int strspn(char *s, char* accepts)
// int strcspn(char *s, char* rejects)

int pchar(char **s, char c) {
  if (!s || !*s || !**s || !c) return 0;
  return **s==c? !!(*s)++ : 0;
}

int pchars(char **s, char *chars) {
  if (!s || !*s || !**s || !chars || !*chars) return 0;
  while(*chars)
    if (pchar(s, chars++)) return 1;
  return 0;
}
  
int pstr(char **s, char *str) {
  if (!s || !*s || !**s || !str || !*str) return 0;
  char *r= *s;
  while(*r && *str && *r++==*str++);
  if (!*str) *s= r;
  return *s!=r;
}

int pspc(char **s) {
  return pchars(s, " \t\n");
}

int pdigit(char **s) {
  return pchars(s, "0123456789");
}

int palpha(char **s) {
  return s && *s && **s &&
    (pchar(s, "abcdefghijklmopqrstuvwxyz")
     || pchar(s, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
}

int palphanum(char **s, 

int lprefix(char *a, char *b) {
  if (!a || !b) return 0;
  int i= 0;
  while (s[i] && match[i] && s[i]==match[i]) i++;
  return i;
}

char *pstr(char *s, char* match) {
  if (!s || !match) return NULL;
  int i= lprefix(s, match);
  if (i==strlen(match)) return
}

char *sskip(char *s, char *w) {
  char *p= 
}

        char r= u;
        r= sskip(r, "https://");
        r= sskip(r, "http://");
        r= sskip(r, "www.");
        r= strunc(r, "?", ">");
        r= sdel(r, ".html");
        r= sdel(r, ".htm");
        printf("%.16s ", r);
