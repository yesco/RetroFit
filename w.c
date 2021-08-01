#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NL " br hr pre code h1 h2 h3 h4 h5 h6 h7 h8 h9 blockquote ul ol li dl dt dd table tr noscript address "
#define TB " td /td th /th "
#define TT " /td /th "
#define HL " strong em i b u s q cite a ins del caption noscript abbr acronym "
#define PR " pre code "
#define CD " pre code bdo kbd dfn samp var "
#define FM " form input textarea select option optgroup button label fieldset legend "
#define SKIP " script head "

#define W "wget -O - %s 2>/dev/null"

char* pres[]= {NL, "\n", TB, "\t", HL, "\001", CD, "\002", FM, "\n", " p ", "\n  "};
char* posts[]= {NL, "\n", TT, "\T", HL, "\007", CD, "\007", FM, "\n"};

int pre= 0, ws= 0, nl= 0, tb= 0;
void p(char c) {
  // TODO: handle hard \N \T
  int _nl= (c=='\n' || c=='\r');
  int _tb= (c=='\t');
  int _ws= (c==' ' || _tb || _nl);

  if (_ws) {
    if (!ws || pre) putchar(c);
    ws= 1; nl= _nl; tb= _tb;
  } else {
    putchar(c);
    ws= 0;
  }
}
// 0 black, 1 red, 2 green, 3 yellow
// 4 blue, 5 magnenta, 6 cyan, 7 white
#define c(n) printf("\e[3%dm", n);
#define b(n) printf("\e[4%dm", n);

int skiptill(FILE* f, char* endchars, char* s) {
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

int main(int argc, char**argv) {
  char* url= argv[1];
  printf("URL=%s\n", url);
  
  char* wget= calloc(strlen(url) + strlen(W) + 1, 0);
  sprintf(wget, W, url);
  FILE* f= popen(wget, "r");
  if (!f) return 404;
  //while (fgets(buf, 128, ptr) != NULL) {
  int c;
  while ((c= fgetc(f)) != EOF) {
    if (c=='<') { // <tag...>
      char end[32] = {0}, tag[32];
      do {
        if (*end) skiptill(f, "<", NULL);
        c= skiptill(f, " >", tag);
        printf("\n---%s\n", tag);
        if (c==' ') skiptill(f, ">", NULL);
        pre= strstr(PR, tag);
        if (strstr(NL, tag)) p('\n');
        if (strstr(SKIP, tag)) {
          end[0]= ' ';
          end[1]= '/';
          strcpy(end+2, tag+1);
          printf("\n...skip till end tag=%s\n", end);
        }
      } while (*end && !strstr(end, tag));
    } else {
      p(c);
    }
  }
  pclose(f);
  return 0;
}
