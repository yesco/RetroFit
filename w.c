#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NL " br hr pre code h1 h2 h3 h4 h5 h6 h7 h8 h9 blockquote ul ol li dl dt dd table tr noscript address "
#define TB " td /td th /th "
#define TT " /td /th "
#define HL " strong u s q cite ins del caption noscript abbr acronym "
#define PR " pre code "
#define CD " pre code bdo kbd dfn samp var "
#define FM " form input textarea select option optgroup button label fieldset legend "
#define SKIP " script head "

#define W "wget -O - %s 2>/dev/null"

enum color{black, red, green, yellow, blue, magnenta, cyan, white, none};
enum color _fg= black, _bg= white;
#define c(n) printf("\e[3%dm", n);
#define b(n) printf("\e[4%dm", n);

int _pre= 0, _ws= 0, _nl= 0, _tb= 0, _skip= 0;

void p(char c) {
  if (_skip) return;
  // TODO: handle hard \N \T
  int nl= (c=='\n' || c=='\r');
  int tb= (c=='\t');
  int ws= (c==' ' || tb || nl);

  if (ws) {
    if (!_ws || _pre) putchar(c);
    _ws= 1; _nl= nl; _tb= tb;
  } else {
    putchar(c);
    _ws= 0;
  }
}

int parse(FILE* f, char* endchars, char* s) {
  int c;
  if (s) *s++ = ' ';
  while (((c= fgetc(f)) != EOF)
    && (!strchr(endchars, c))) {
    if (s) *s++= tolower(c);
  }
  if (s) *s++ = ' ';
  if (s) *s= 0;
  return c<0? 0: c;
}

typedef char TAG[32];

void process(TAG *end);

int level= 0;

void hi(TAG *tag, char* tags, enum color fg, enum color bg) {
  if (tag && !strstr(tags, *tag)) return;

  level++;
  printf("--->%d %s\n", level, tag?*tag:NULL);

  // save colors
  enum color sfg= _fg, sbg= _bg, spre= _pre, sskip= _skip; {
    if (_fg != none) _fg= fg;
    if (_bg != none) _bg= bg;
    if (strstr(PR, tag)) _pre= 1;
    if (strstr(SKIP, tag)) _skip= 1;
//    c(_fg); b(_fg);
    
    TAG end = {0};
    if (tag && *tag) {
      end[0]= ' ';
      end[1]= '/';
      strcpy(end+2, *tag+1);
    }
    process(end);

  } _fg= sfg; _bg= sbg; _pre= spre; _skip= sskip;

  level--;
  printf("<--%d %s\n", level, tag?*tag:NULL);
}

#define HI(tags, fg, bg) hi(tag, tags, fg, bg)

FILE* f;

void process(TAG *end) {
  int c;
  while ((c= fgetc(f)) != EOF) {
    if (c=='<') { // <tag...>
      TAG tag= {0};

      c= parse(f, " >", tag);
      printf("\n---%s\n", tag);
      if (c==' ') parse(f, ">", NULL);

      if (end && *end && strstr(end, tag)) return;

      if (strstr(NL, tag)) p('\n');
      if (strstr(" p ", tag)) {p(' ');p(' ');}

      HI(" h1 ", black, white);
      HI(" h2 ", black, green);
      HI(" h3 ", black, yellow);
      HI(" h4 ", black, cyan);
      HI(" h5 h6 ", black, magnenta);
      HI(" b strong ", red, none);
      HI(" i em ", yellow, none);
      HI(HL, magnenta, none);

      HI(FM, yellow, black);
      HI(CD, green, black);

      HI(" a ", blue, none);
      HI(SKIP, none, none);

    } else {
      p(c);
    }
  }
}

int main(int argc, char**argv) {
  char* url= argv[1];
  printf("URL=%s\n", url);
  
  char* wget= calloc(strlen(url) + strlen(W) + 1, 0);
  sprintf(wget, W, url);
  f= popen(wget, "r");
  if (!f) return 404;

  //c(_fg); b(_bg);

  process(NULL);

  pclose(f);
  return 0;
}
