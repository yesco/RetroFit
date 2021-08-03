
//
// ./w - a minimal web-browser in C
//
//    (>) 2021 Jonas S Karlsson
//         jsk@yesco.org
//

// various debug output
int trace= 0;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h>

int rows= 24, cols= 80;

// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
void getsize() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  rows = w.ws_row;
  cols = w.ws_col;
  if (1 || trace) printf("\t\trows=%d cols=%d\n", rows, cols);
}

// ansi
enum color{black, red, green, yellow, blue, magnenta, cyan, white, none};
enum color _fg= black, _bg= white;

void _color(c) {
  if (c >= 0) {
    printf("5;%dm", c);
  } else {
    c= -c;
    int b= c&0xff, g= (c>>8)&0xff, r=(c>>16);
    printf("2;%d;%d;%dm", r, g, b);
  }
}

void fg(int c) { printf("\e[38;"); _color(c); _fg= c; }
void bg(int c) { printf("\e[48;"); _color(c); _bg= c; }

int bold(int c /* 0-7 */) { return c+8; }
int rgb(int r,g,b /* 0-5 */) { return 0x10+ 36*r + 6*g + b; }
int gray(int c /* 0-7 */) { return 0xe8+  c; }
int RGB(int r,g,b /* 0-255 */) { return -(r*256+g)*256+b; }

// adjusted colors
void C(int n) { fg(n + 8*(n!=0 && n<8)); }
void B(int n) { bg(n); }

// hard space, hard newline
#define HS -32
#define HNL -10

// grouping of tags according to formatting
#define NL " br hr pre code h1 h2 h3 h4 h5 h6 h7 h8 h9 blockquote ul ol li dl dt dd table tr noscript address "
#define TB " td /td th /th "
#define TT " /td /th "
#define HL " strong u s q cite ins del caption noscript abbr acronym "
#define PR " pre code "
#define CD " pre code bdo kbd dfn samp var "
#define FM " form input textarea select option optgroup button label fieldset legend "
#define SKIP " script head "

// template for getting HTML
#define W "wget -O - %s 2>/dev/null"

// screen state
int _pre= 0, _ws= 0, _nl= 0, _tb= 0, _skip= 0, _indent= 0, _curx= 0, _cury= 0;

//printf("\e[44;3m");
// B: blue italics!

void cls() {
  printf("\e[H[2J[3J");
  getsize();
  _cury= 0; _curx= 0;
}

void nl() {
  printf("\e[K\n"); // workaround bug!
  _cury++; _curx= 0;
}

void nli() {
  nl();
  for(int i=_indent;i--;i>0) {
    putchar(' ');
  }
  _curx= _indent;
}

void p(int c) {
  if (_skip) return;
  // hard chars
  if (c < 0) {
    c= -c;
    if (c=='\n') {
      nli();
    } else {
      putchar(c);
      _curx++;
      if (_curx > cols-4)
        printf("||"); // TODO:
        nli();
    }
    return;
  }

  // TODO: handle hard \N \T
  int tb= (c=='\t');
  int ws= (c==' '||tb||c=='\n'||c=='\r');

  if (ws) {
    if (!_ws || _pre) putchar(' ');
    _ws= 1; if(_tb) _tb= tb;
  } else {
    putchar(c);
    _ws= 0; _tb= 0;
  }
}

typedef char TAG[32];

int parse(FILE* f, char* endchars, char* s) {
  int c; char* origs= s;
  if (s) *s++ = ' ';
  int l= 1;
  while (((c= fgetc(f)) != EOF)
    && (!strchr(endchars, c))) {
    if (s) {
      *s++= tolower(c);
      if (s-origs > sizeof(TAG)) {
        exit(7);
      }
    }
  }
  if (s) *s++ = ' ';
  if (s) *s= 0;
  return c<0? 0: c;
}

void process(TAG *end);

int level= 0;

void hi(TAG *tag, char* tags, enum color fg, enum color bg) {
  if (tag && !strstr(tags, *tag)) return;

  level++;
  if (trace)
    printf("--->%d %s %d %d\n", level, tag?*tag:NULL, fg, bg);

  // save colors
  int sfg= _fg, sbg= _bg, spre= _pre, sskip= _skip; {
    if (fg != none) _fg= fg;
    if (bg != none) _bg= bg;
    if (strstr(PR, tag)) _pre= 1;
    if (strstr(SKIP, tag)) _skip= 1;
    // underline links!
    if (strstr(" a ", tag)) { printf("\e[4m"); C(_fg); }
    // italics
    if (strstr(" i em ", tag)) { printf("\e[3m"); C(_fg); };

    C(_fg); B(_bg);
    
    TAG end = {0};
    if (tag && *tag) {
      end[0]= ' ';
      end[1]= '/';
      strcpy(end+2, *tag+1);
    }
    process(end);

    // off underline links!
    if (strstr(" a ", tag)) printf("\e[24m");
    // off italics
    if (strstr(" i em ", tag)) printf("\e[23m");


  } _pre= spre; _skip= sskip;
  if (strstr(NL, tag)) p(HNL);
  C(sfg); B(sbg); 


  level--;
  if (trace)
    printf("<--%d %s\n", level, tag?*tag:NULL);
}

#define HI(tags, fg, bg) hi(&tag, tags, fg, bg)

FILE* f;

void process(TAG *end) {
  int c;
  while ((c= fgetc(f)) != EOF) {
    if (c=='<') { // <tag...>
      TAG tag= {0};

      c= parse(f, "> \n\r", tag);
      if (trace) printf("\n---%s\n", tag);
      if (c!='>') parse(f, ">", NULL);

      // check if </endTAG>
      if (strstr(*end, tag)) return;

      if (strstr(NL, tag)) p(HNL);
      if (strstr(" p ", tag)) {p(HS);p(HS);}
      if (strstr(" h1 ", tag)) p(HNL);
      HI(" h1 ", white, black);
      HI(" h2 ", black, green);
      HI(" h3 ", black, yellow);
      HI(" h4 ", black, cyan);
      HI(" h5 h6 ", black, magnenta);

      HI(" b strong ", red, none);
      HI(" i em ", magnenta, none);
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
  if (trace) printf("URL=%s\n", url);

  C(white); B(black);
  cls();

  C(white);
  nl();
  //printf("./ω");
  //printf("ᎳᎳᎳ");
  //printf("🅆");
  //printf("./w");
  printf("🌍");
  //printf("🏠");

  putchar(' ');
  //printf("\e[4m"); // underlined

  char uu[strlen(url)+1];
  char* u= &uu;
  strcpy(u, url);

  if (strstr(u, "https://")==u) {
    printf("🔒"); u+= 8;
  } else if (strstr(u, "http://")==u) {
    printf(""); u+= 7;
  }
  if (u[strlen(u)-1]=='/')
    u[strlen(u)-1]=0;
  
  // 📂 📖 ⚠ ℯ 😱 🔓
  // ⌂ 🏠 🏡
  printf("%s", u); nl();
  printf("\e[9m"); // crossed out
  nl();

  printf("\e[29m"); // crossed end

  C(black); B(white);

  char* wget= calloc(strlen(url) + strlen(W) + 1, 0);
  sprintf(wget, W, url);
  f= popen(wget, "r");
  if (!f) return 404;

  C(_fg); B(_bg);

  TAG dummy= {0};
  process(&dummy);

  pclose(f);

  C(white); B(black);
  p(HNL); p(HNL); 
  return 0;
}
