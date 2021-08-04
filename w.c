//
//               ./w 
//
//    a minimal web-browser in C
//
//
//  (>) CC-BY 2021 Jonas S Karlsson
//
//          jsk@yesco.org
//

// various debug output
int trace= 0;

// general formatting
int rmargin= 1; // 1 or more (no 0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <assert.h>

#include <unistr.h>
#include <uniname.h>
#include <wchar.h>

int rows= 24, cols= 80;

// https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
void getsize() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  rows = w.ws_row;
  cols = w.ws_col;
  if (trace) printf("\t\trows=%d cols=%d\n", rows, cols);
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
#define SNL -11

// grouping of tags according to formatting
#define NL " br hr pre code h1 h2 h3 h4 h5 h6 h7 h8 h9 blockquote li dl dt dd table tr noscript address "
#define HD " h1 h2 h3 h4 h5 h6 "
#define TB " td /td th /th "
#define TT " /td /th "
#define HL " u s q cite ins del caption noscript abbr acronym "
#define PR " pre code "
#define CD " pre code "
#define TT " bdo kbd dfn samp var tt "
#define FM " form input textarea select option optgroup button label fieldset legend "
#define SKIP " script head "

// template for getting HTML
#define WGET "wget -O - \"%s\" 2>/dev/null"

// https://html.spec.whatwg.org/entities.json
// (reworked using entities.js and hand-edit)

// one long string repated (&name;? UU?)*\&
// the file is 27K
// - I think we can just do strstr, LOL!
#include "entities.h"

// name must be "&name;" ';' is optional :-(
// returns unicode string 1-2 bytes, or NULL
// the string is static, so use it fast!
// - &amp;    - HTML named Entity
// - &#4711;  - decimal numbered char
// - &#xff21; - fullwidth 'A'
char* decode_entity(char* name) {
  char fnd[32]= {0};
  strcpy(fnd, name);

  // return pointer to fixed static string
  static char result[2*4*2];
  memset(result, 0, sizeof(result));

  // numbered entity?
  uint32_t c = 0;
  assert(sizeof(int)==4);
  if (sscanf(fnd, "&#x%x;", &c) ||
      sscanf(fnd, "&#%i;", &c)) {
    size_t len=sizeof(result)-1;
    return u32_to_u8(&c, 1, result, &len);
  }

  // search for '&name; '
  fnd[strlen(fnd)]= ' ';
  if (trace) printf("[>> \"%s\" <<]", fnd);

  char* m= strcasestr(ENTITIES, fnd);
  if (!m) return NULL;
  
  // skip '&name;? '
  m+= strlen(name)+1;
  char* p= result;
  // copy first char at least (might be '&')
  do {
    *p++ = *m++;
  } while ('&' != *m); // until '&...';
  if (trace) printf("{$s}", result);
  return result;

// TODO: maybe hardcode the common ones?
// &nbsp;&gt;&lt;&quot;&.squo;&dquo;&lsaquo;&rsaquo;&ndash;&mdash;&circ;&tilde;&permil;&amp;&eur;&em;&emm;&raquo;&copy;
}

// screen state
int _pre= 0, _ws= 0, _nl= 0, _tb= 0, _skip= 0, _indent= 0, _curx= 0, _cury= 0, _fullwidth= 0;

//printf("\e[44;3m");
// B: blue italics!

void cls() {
  printf("\e[H[2J[3J");
  getsize();
  _cury= 0; _curx= 0; _ws= 1; _nl= 1;
}

void nl();
void indent();

void inx() {
  _curx++; _nl= 0;
  if (!_pre && _curx+rmargin == cols) nl();
}

// internal putchar that buffers words/&#4711; and does wordwrap
#define WORDLEN 10
char word[WORDLEN+1] = {0};
int _overflow= 0;

void _pc(int c) {
  // TODO: break on '&' this doesn't work:
  //if (strlen(word) && (c<=' ' || c==';' || c=='\n' || c=='\r' || c=='\t' || c=='&')) {
  if (c<=' ' || c==';' || c=='\n' || c=='\r' || c=='\t') { // output word
    // output word
    // html entity?
    char* e_result= NULL;
    if (word[0]=='&') {
      if (c==';') word[strlen(word)]= c;
      e_result= decode_entity(word);
    }
    if (e_result) {
      printf("%s", e_result);
      //if (c!=';') putchar(c);
    } else {
      // no entity, just output word
      printf("%s", word);
      if (c>=0) putchar(c);
    }
    memset(word, 0, sizeof(word));
    _overflow= 0;
    //_ws= (c==' '||c=='\t'); // TODO: doesn't matter!
  } else if (_overflow) {
    if (_curx+rmargin+1 >= cols) {
      putchar('-');
      nl();
    }
    indent();
    putchar(c); inx();
  } else {
    int l= strlen(word);
    if (l>=WORDLEN) {
      _overflow= 1;
      // flush
      for(int i=0; i<strlen(word); i++)
        putchar(word[i]);

      putchar(c); inx();
      memset(word, 0, sizeof(word));
      return;
    }
    word[l]= c;
    // word too long for this line?
    if (_curx+rmargin+1 >= cols) {
      nl();
      indent(); // this affects <li> second line indent
      _curx+= strlen(word);
    }

    // html entity?
    //   TODO: this isn't correct
    //   complex: &amp followed by ';' or NOT!
    if (0 && word[0]=='&') {
      char* e= decode_entity(word);
      printf(" { %s } ", e);
    }
  }
}

void nl() {
  //printf("\\N");
  printf("\e[K\n"); // workaround bug!
  _cury++; _curx= 0; _ws= 1; _nl= 1;
}

void indent() {
  while(_curx < _indent) {
    putchar(' '); inx();
  }
  _ws= 1;
}

void p(int c) {
  if (_skip) return;

  // TODO: handle &...;
  if (_pre) {
    _pc(c); inx();
    if (c=='\n') _curx= 0;
    return;
  }

  // hard chars
  if (c < 0) {

    if (c==SNL) {
      if (_nl) return;
      if (_curx < _indent*2) {
        indent();
      } else {
        nl();
      }
      return;
    } else {
      c= -c;
    }
    
    if (c=='\n') {
      //printf("\\n");
      nl();
    } else {
      _pc(c); _ws= 0; inx();
    }
    return;
  }

  // TODO: handle hard \N \T
  int tb= (c=='\t');
  int ws= (c==' '||tb||c=='\n'||c=='\r');

  if (ws) {
    if (!_curx) return;
    if (!_ws || _pre) {
      _pc(' '); inx();
      if (_fullwidth) { _pc(' '); inx(); }
    }
    _ws= 1; if(_tb) _tb= tb;
  } else {
    indent();
    if (_fullwidth && c < 128) {
      // https://en.m.wikipedia.org/wiki/UTF-8
      // modify unicode:
      // ef  bc "10 xx xxxx" == "!"
      char fw[4]; strcpy(fw, "！");
      int cc= (int)fw[2] + c - 33;
      fw[2]= (cc & 63) + 128;
      if (cc >= 128+64) fw[1]++;
      printf("%s", fw); inx(); inx();
    } else {
      _pc(c); inx();
    }
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
    if (strstr(" ul ol ", tag)) _indent+= 2;
    if (strstr(" li ", tag)) _indent+= 3;
    // underline links!
    if (strstr(" a ", tag)) { printf("\e[4m"); C(_fg); }
    // italics
    if (strstr(" i em ", tag)) { printf("\e[3m"); C(_fg); };
    // fullwidth
    if (strstr(" h1 ", tag)) _fullwidth++;
    if (strstr(HD, tag)) _fullwidth++;

    // content
    C(_fg); B(_bg);
    if (strstr(" h1 ", tag)) p(HNL);
    if (strstr(HD, tag)) p(HS);
    if (strstr(TT, tag)) p(HS);
    
    TAG end = {0};
    if (tag && *tag) {
      end[0]= ' ';
      end[1]= '/';
      strcpy(end+2, *tag+1);
    }
    process(end);
    // end content

    // - cleanups
    if (strstr(" ul ol ", tag)) { p(SNL); _indent-= 2; }
    if (strstr(" li ", tag)) _indent-= 3;
    if (strstr(TT, tag)) p(HS);
    // off underline links!
    if (strstr(" a ", tag)) printf("\e[24m");
    // off italics
    if (strstr(" i em ", tag)) printf("\e[23m");
    // off fullwidth
    if (strstr(" h1 ", tag)) _fullwidth--;
    if (strstr(HD, tag)) _fullwidth--;

  } _pre= spre; _skip= sskip;
  if (strstr(NL, tag)) p(SNL);
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
    if (c!='<') {
      p(c);
    } else { // <tag...>
      TAG tag= {0};
      _pc(-1); // flush word

      // parse tag
      c= parse(f, "> \n\r", tag);
      if (0 ||trace) printf("\n---%s\n", tag);
      if (strstr(" !-- ", tag)) {
        char com[4] = "1234";
        while ((c= fgetc(f)) != EOF) {
          strcpy(&com[0], &com[1]);
          com[2]= c;
          if (!strcmp("-->", com)) break;
        }
        if (c==EOF) return;
        continue;
      } else if (c!='>') {
        parse(f, ">", NULL);
      }

      // check if </endTAG>
      if (strstr(*end, tag)) return;

      if (strstr(NL, tag)) p(SNL);
      if (strstr(" p ", tag)) {
        if (_curx>_indent) p(HNL);
        p(HS);p(HS);
      }
      //if (strstr(" h1 ", tag)) p(HNL);
      if (strstr(" li dt ", tag)) { p(SNL); indent(); printf(" ● "); inx(); inx(); inx(); }

      // these require action after
      HI(" h1 ", white, black);
      HI(" h2 ", black, green);
      HI(" h3 ", black, yellow);
      HI(" h4 ", black, blue);
      HI(" h5 ", black, cyan);
      HI(" h6 ", black, white);

      HI(" b strong ", red, none);
      //HI(" i em ", magnenta, none);
      HI(" i em ", none, none);

      HI(HL, magnenta, none);

      HI(FM, yellow, black);
      HI(CD, green, black);
      HI(TT, black, rgb(3,3,3));

      HI(" a ", blue, none);

      HI(" ul ol li ", none, none);
      HI(SKIP, none, none);
    }
  }
}

int main(int argc, char**argv) {
  char* url= argv[1];
  if (trace) printf("URL=%s\n", url);

  C(white); B(black);
  cls();

  C(white); B(black);
  //printf("./ω");
  //printf("ᎳᎳᎳ");
  //printf("🅆");
  //printf("./w");
  printf("🌍");
  putchar(' ');
  B(8);  // gray() ??? TODO
  //printf("🏠");
  putchar(' ');

  //printf("\e[4m"); // underlined

  char uu[strlen(url)+1];
  char* u= &uu;
  strcpy(u, url);

  if (strstr(u, "https://")==u) {
    printf("🔒"); u+= 8;
  } else if (strstr(u, "http://")==u) {
    int f=_fg;
    C(red);
    printf("🚩");
    fg(f);
    u+= 7;
  } 
  if (u[strlen(u)-1]=='/')
    u[strlen(u)-1]=0;
  
  // 📂 📖 ⚠ ℯ 😱 🔓
  // ⌂ 🏠 🏡
  printf("%s", u); nl();
  //printf("\e[9m"); // crossed out
  //nl();
  //printf("\e[29m"); // crossed end

  C(black); B(white);

  // get HTML
  char* wget= calloc(strlen(url) + strlen(WGET) + 1, 0);
  sprintf(wget, WGET, url);
  f= fopen(url, "r");
  if (!f) // && strstr(url, "http")==url)
    f= popen(wget, "r");
  if (!f) return 404; // TODO:

  C(_fg); B(_bg);

  TAG dummy= {0};
  process(&dummy);

  pclose(f);

  C(white); B(black);
  p(HNL); p(HNL); 
  return 0;
}
