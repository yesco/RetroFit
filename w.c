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
int trace= 0, trace_content= 0;

// TODO: configure options:
// - use fancy unicode glpyhs (allow testing)
// - map uncode -> ascii glyphs
// - asni 8/16/255 colors
// - (page/elements) color/background
// - colorize tables
// - ansi underline
// - ansi italic
// - bold
// - paagraph indent/empty line

#define TRACE(exprs...) if (trace) printf(exprs);

#define error(exp, exitcode, msg...) \
  do if (exp) { fprintf(stderr, "%%ERROR: %s:%d in %s(...)\n", __FILE__, __LINE__, __func__); fprintf(stderr, msg); fputc('\n', stderr); exit(exitcode); } while(0)

// general formatting
const int rmargin= 1; // 1 or more (no 0)
const int lmargin= 2;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
//#include <unistd.h>
#include <assert.h>

#include <unistr.h>
#include <uniname.h>
#include <wchar.h>

// normally don't include .c...
#include "jio.c"

// Dynamic STRings (see Play/dstrncat.c)
#define DSTR_STEP 64

typedef struct dstr {
  int max;
  char s[0];
} dstr;

// Usage: d= dstrncat(d, "foo", -1);
//   d, add: can be NULL (both->alloc N)
//   n: -1) strlen(add), or copy N chars
// 
// Returns: d or newly allocated dstr
dstr* dstrncat(dstr* d, char* add, int n) {
  int len= d ? strlen(d->s) : 0, max= d ? d->max : 0;
  n= (n<0 && add)? strlen(add) : n;
  if (!d || len+n+1>max) {
    max= ((max+n)/DSTR_STEP + 1)*DSTR_STEP*13/10;
    d= realloc(d, sizeof(dstr)+max);
    d->s[len]= 0; // if new allocated
  }
  d->max= max;
  if (add) strncat(d->s, add, n);
  return d;
}



// generally used for parse() of symbols
typedef char TAG[32];

// w3c colors
// - https://www.w3.org/wiki/CSS/Properties/color/keywords
#include "colors.h"

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

// hard space, hard newline
#define HS -32
#define HNL -10
#define SNL -11

// -- groups of tags according to format
#define SKIP " script head "

#define NL " br hr pre code h1 h2 h3 h4 h5 h6 h7 h8 h9 blockquote li dl dt dd table tr noscript address tbody "
#define XNL " br /ul /ol hr tbody "
#define HD " h1 h2 h3 h4 h5 h6 "
//#define CENTER " center caption " // TODO

#define BD " b strong "
#define IT " i em caption "
// TODO: #define UL " u a "

#define HL " u s q cite ins del noscript abbr acronym "

#define PR " pre code "
#define TT " bdo kbd dfn samp var tt "

// thead tfoot tbody /tbody-optional!
// td/th: rowspan colspan

#define FM " form input textarea select option optgroup button label fieldset legend "

// attribute captures
#define TATTR " a body table th td tr font img a base iframe frame colgroup span div p "

#define ATTR " href src alt aria-label title aria-hidden name id type value size accesskey align valign colspan rowspan span color bgcolor "

// -- template for getting HTML
// TODO: use "tee" to save to cache
// - https://www.gnu.org/software/coreutils/manual/html_node/tee-invocation.html#tee-invocation
// TODO: read headers from wget/curl and show loading status
#define WGET "wget -O - \"%s\" 2>/dev/null"

// - HTML Name Entities
#include "entities.h"

// decodes a HTML Named Entity to UTF-8
// name must be "&ID" (';' is optional)
//   - &amp;    - HTML named Entity
//   - &#4711;  - decimal numbered char
//   - &#xff21; - fullwidth 'A'
// Return unicode string 1-2 bytes, or NULL
// the string is static, so use it fast!
char* decode_entity(char* name) {
  TAG fnd= {0};
  strncpy(fnd, name, sizeof(fnd));
  // return pointer to fixed static string! (safe!)
  static char result[2*4*2];
  memset(result, 0, sizeof(result));

  // numbered entity?
  uint32_t c = 0;
  assert(sizeof(int)==4);
  if (sscanf(fnd, "&#x%x;", &c) ||
      sscanf(fnd, "&#%i;", &c)) {
    size_t len=sizeof(result)-1;
//    if (c<128) {
//      result[0]= c;
//      return result;
//    }
    // TODO: -- see tests forthis fuction:
    // - https://github.com/coreutils/gnulib/blob/master/tests/unistr/test-u32-to-u8.c
    // TODO: use https://man7.org/linux/man-pages/man3/wctomb.3.html ???
    char *r= u32_to_u8(&c, 1, result, &len);
    printf("---jsk: '%s' len=%zu\n", r, len);
    if (r==result) return result;
    if (r==NULL) return NULL;
    error(r!=result, 77, "BAD ptr from u32_to_u8");
  }

  // search for '&name; '
  fnd[strlen(fnd)]= ' ';
  TRACE("[>> \"%s\" <<]", fnd);

  char* m= strcasestr(ENTITIES, fnd);
  if (!m) return NULL;
  
  // skip '&name;? '
  m+= strlen(name)+1;

  char* p= result;
  // copy first char at least (might be '&')
  do {
    *p++ = *m++;
  } while ('&' != *m && *m); // until '&...';
  *p= 0;
  TRACE("{$s}", result);
  return result;
}

// screen state
int _pre= 0, _ws= 1, _nl= 1, _tb= 0, _skip= 0, _indent= lmargin;
int _curx= 0, _cury= 0, _fullwidth= 0, _capture= 0, _table= 0;

//void cls() {
//  cls();
//  screen_init();
//  _cury= 0; _curx= 0; _ws= 1; _nl= 1;
//}

void nl();
void indent();

// track visible chars printed
void inx() {
  _curx++; _nl= 0;
  if (!_pre && _curx+rmargin == screen_cols) nl();
}

// _pc buffers word, and word-wrap+ENTITIES
#define WORDLEN 10
char word[WORDLEN+1] = {0};
int _overflow= 0;

void _pc(int c) {
  // TODO: break on '&' this doesn't work:
  //if (strlen(word) && (c<=' ' || c==';' || c=='\n' || c=='\r' || c=='\t' || c=='&')) {

  // output word (if at break char)
  if (c<=' ' || c==';' || isspace(c)) {
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
    if (_curx+rmargin+1 >= screen_cols) {
      putchar('-');
      nl();
    }
    indent();
    putchar(c); inx();

  } else { // add char to word
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
    if (_curx+rmargin+1 >= screen_cols) {
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

int offset= 0;

int myfgetc(FILE* f) {
  int n= ftell(f);
  offset= n<0 ? offset+1 : n;
  return fgetc(f);
}

void myungetc(int c, FILE* f) {
  offset--;
  ungetc(c, f);
}

#define fgetc myfgetc
#define ungetc myungetc



TAG link_tag;
dstr* _url= NULL;

void print_url() {
  if (!_url) return;
  printf("\e]:A:{%s}\e\\", _url->s);
}

void safe_print(char* s, int space, int quote) {
  if (!s) return;
  if (space) putchar(' ');
  if (quote) putchar('"');
  while(*s) {
    if (*s=='"') printf("\\\"");
    else if (*s>32) space= !putchar(*s);
    else if (!space) space= putchar(' ');
    s++;
  }
  if (quote) putchar('"');
}

void metadata(char* type, char* a, char* b, char* c) {
  printf("\n#=%s", type);
  safe_print(a, 1, 0);
  safe_print(b, 1, 1);
  putchar('\n');
  if (c) {
    putchar('\n');
    putchar('#');
    safe_print(c, 0, 0);
    putchar('\n');
  }
}

void nl() {
  printf("\e[K\n"); // workaround bug!
  // hidden source file offset
  printf("@%d:\r", offset);
  B(_bg); C(_fg); // fix "less -r" scroll up
  printf("\e[K");
  // indent(); print_state();
  print_url();
  _cury++; _curx= 0; _ws= 1; _nl= 1;
}

void indent() {
  while(_curx < _indent) {
    putchar(' '); inx();
  }
  _ws= 1;
}

////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
// WARNING: table handling and rendering
//          is just a hack for now, bad
//          explorative hack!
#include "table.c"
// Typically, you do not include .c-files:
// TODO: when it'scomplete, move the code here; I want it to be one C-file.

void p(int c) {
  char b= c;
  if (_table)
    table= dstrncat(table, &b, 1);
  if (_capture)
    content= dstrncat(content, &b, 1);

  if (_skip) return;

  // preformatted
  if (_pre) {
    _pc(c); inx();
    if (c=='\n') _curx= 0;
    return;
  }

  // soft+hard chars
  if (c<0) {
    // soft char
    if (c==SNL) {
      if (!_nl) nl();
    } else {
      // hard chars
      c= -c;
      if (c=='\n') {
        nl();
      } else {
        _pc(c); _ws= 0; inx();
      }
    }
    return;
  }

  if (_fullwidth) _pc(-1);
  // collapse whitespace
  int tb= (c=='\t'); // TODO: table?
  int ws= (c==' '||tb||c=='\n'||c=='\r');
  if (ws) {
    if (!_curx) return;
    if (!_ws || _pre) {
      _pc(' '); inx();
      if (_fullwidth) { _pc(' '); inx(); }
    }
    _ws= 1; if (_tb) _tb= tb;
    return;
  }

  // visible chars
  indent();
  if (_fullwidth) {
    // cheat, no word-wrap, print now!
    if (c<128) {
      putwchar(0xff01 + c-33); inx();
    } else {
      putchar(c);
    }
  } else {
    _pc(c);
  }
  inx(); _ws= 0; _tb= 0;
}

// steps one char in input stream
// == true if "have next" (not EOF)
// c is set to character read
#define STEP ((c= fgetc(f)) != EOF)

// parse chars till one of ENDCHARS
// S: if not NULL, lowercase char, append
// Returns non-matching char
int parse(FILE* f, char* endchars, char* s) {
  int c; char* origs= s;
  if (s) *s++ = ' ';
  while (STEP && (!strchr(endchars, c))) {
    if (s) {
      *s++= tolower(c);
      // TODO: error here for google.com
      error(s-origs > sizeof(TAG), 7, "TAG too long=%s\n", origs);
    }
  }
  if (s) { *s++ = ' '; *s= 0; }
  return c<0? 0: c;
}

void addContent();

void process(TAG *end);

// Use HI macro! (passes in tag)

#define HI(tags, fg, bg) hi(&tag, tags, fg, bg)

// hilight TAG if it's part of TAGS
// using FG color and BG color, and other
// formatting.

// After the matching </TAG> is reached:
// restore colors and undo formatting.
void hi(TAG *tag, char* tags, enum color fg, enum color bg) {
  static int level= 0;
  if (!tag || !*tag || !strstr(tags, *tag)) return;

  level++;
  TRACE("--->%d %s %d %d\n", level, tag?*tag:NULL, fg, bg);

  // save colors
  int sfg= _fg, sbg= _bg, spre= _pre, sskip= _skip, sindent=_indent; {
    // - START highlight
    if (fg != none) _fg= fg;
    if (bg != none) _bg= bg;
    if (strstr(PR, tag)) _pre= 1;
    if (strstr(SKIP, tag)) _skip= 1;

    if (strstr(" ul ol ", tag)) _indent+= 2;

    // underline links!
    if (strstr(" a ", tag)) {
      underline(); C(_fg);
      _capture++;
    }
    if (strstr(" table ", tag)) {
      table= dstrncat(NULL, NULL, 1024);
      _table++;
    }
    // italics
    if (strstr(IT, tag)) { printf("\e[3m"); C(_fg); };
    // fullwidth
    if (strstr(HD, tag)) _fullwidth++;

    // content
    C(_fg); B(_bg);
    if (strstr(" h1 ", tag)) p(HNL);
    if (strstr(HD, tag)) p(HS);
    if (strstr(TT, tag)) p(HS);
    
    // find end tag (recurse)
    TAG end = {' ', '/'};
    strcpy(end+2, *tag+1);
    process(end);

    // end content

    // - ENDing highlight/formatting
    if (strstr(TT, tag)) p(HS);
    // off underline links!
    if (strstr(" a ", tag)) {
      end_underline();
      if (content && _url)
        metadata("LINK", link_tag, content->s, _url->s);
      if (_url) free(_url); _url= NULL;
      if (!--_capture) addContent();
    }
    if (strstr(" table ", tag)) {
      if (!--_table) renderTable();
    }
    // off italics
    if (strstr(IT, tag)) printf("\e[23m");
    // off fullwidth
    if (strstr(HD, tag)) _fullwidth--;

    // restore saved state (colors/pre/skip)
  } _pre= spre; _skip= sskip; _indent= sindent;
  if (strstr(NL, tag)) p(SNL);
  C(sfg); B(sbg); 

  level--;
  TRACE("<--%d %s\n", level, tag?*tag:NULL);
}

FILE* f;

int skipspace() {
  int c;
  while (STEP && isspace(c));
  return c;
}

// capture deatails of an entity rendering
typedef struct pos{
  int x, y;
} pos;

typedef struct entity {
  TAG tag;
  //dhash* attr;
  dstr* content;

  pos start, end;
} entity;

entity* lastentity= NULL;

void newTag(TAG tag) {
  entity* e= calloc(sizeof(entity), 1);
  memcpy(e->tag, tag, sizeof(tag));

  lastentity= e; // TODO: add to list/stack?
}

void addAttr(TAG tag, TAG attr, dstr* val) {
  entity* e= lastentity;
  if (trace) printf("\n---%s.%s=%s\n", tag, attr, val->s);
  // embed URL as hidden message
  if (strstr(" href src ", attr)) {
    memcpy(link_tag, tag, sizeof(link_tag));
    if (_url) free(_url);
    _url= val;
    print_url();
    return;
  }
  // not used
  free(val);
}

void addContent() {
  if (trace_content) {
    printf("\n---%s y=%d x=%d", lastentity->tag, _cury, _curx);
    printf("\n---content: \"%s\" y=%d, x=%d", content->s, _cury, _curx);
  }

  if (lastentity) {
    lastentity->content= content;
    lastentity->end= (pos){_cury, _curx};
  } else {
    free(content);
  }
  content= NULL;
}

void process(TAG *end) {
  int c;
  while (STEP) {

    if (c!='<') { // content
      p(c);

    } else { // '<' tag start
      TAG tag= {0};
      _pc(-1); // flush word

      // parse tag
      c= parse(f, "> \n\r", tag);
      TRACE("\n---%s\n", tag);

      // comment
      if (strstr(" !-- ", tag)) {
        // shift 3 characters
        char com[4] = "1234";
        while (STEP) {
          strcpy(&com[0], &com[1]);
          com[2]= c; // add at end
          if (!strcmp("-->", com)) break;
        }
        continue;
      }
      
      // process attributes till '>'
      // TODO:move out to function
      if (c!='>') {
        // <TAG attr>
        if (strstr(TATTR, tag)) {
          newTag(tag);
          // TODO: CSS cheat: match nay
          // - color:\s*\S+[ ;}]
          // - background(-color)?:\s*\S+[ ;}]
          // - width/height/max-height/min-height
          // - linebreak/hypens/overflow/clip/white-space/word-break/word-spacing/word-wrap
          // - left/right/top/bottom
          // - text-align/align-content
          // - clear/break-before/break-after
          // - float
          // - display/visibility/
          // - font-size/font-weight/text-decoration/text-shadow/
          // - text-indent/text-justify/text-overflow
          // - table-layout
          while (STEP) {
            // TODO: hmmm
            ungetc(c, f);
            ungetc(skipspace(f), f); //hmm
            // read attribute
            TAG attr= {0};
            c= parse(f, "= >\"\'", attr);
            if (c=='>' ||c==EOF) break;
            //printf("\n---%s.%s\n", tag, attr);
            // do we want it?
            if (strstr, ATTR, attr) {
              int q= skipspace(f);
              // TODO: move out?
              // merge w parse?
              dstr *v = NULL;
              if (q=='"' || q=='\'') {
                // id='foo' id="foo"
                while (STEP && c!=q)
                  v = dstrncat(v, &c, 1);
                ungetc(' ', f);
              } else {
                // id=foo
                while (STEP && !isspace(c) && c!='>')
                  v = dstrncat(v, &c, 1);
              }
              addAttr(tag, attr, v);
            }
          }
        }
      }
      if (c!='>') c= parse(f, ">", NULL);

      // pre action for tags (and </tag>)
      // TODO: /th /td /tr tags are optional.. :-(
//      if (strstr(" /th /td  ", tag)) { /*p(-'\t'); */ p(' '); p('|'); p(' '); }
//      if (strstr(" /tr ", tag)) p(SNL);

      if (strstr(XNL, tag)) p(HNL);
      if (strstr(" td th tr /td /th /tr /table ", tag)) {
        //if (strstr(" td th tr /table ", tag)) {
        end_underline();
        //printf("\n[===%s===]\n", tag);
        handle_trd(strstr(" tr ", tag), strchr(tag, '/'), tag);
      }

      // check if </endTAG>
      if (strstr(*end, tag)) return;

      // pre action for some tags
      if (strstr(NL, tag)) p(SNL);

      // table hacks
      // TODO: at TD TH set indent to _curx, reset at <tr></table> to saved before <table> can HI() store _indent?
      // TODO: inside td/th handle \n differently
      // <COLGROUP align="center">
      // <COLGROUP align="left">
      // <COLGROUP align="center" span="2">
      // <COLGROUP align="center" span="3">
      if (strstr(" th ", tag)) underline();
      if (strstr(" td th ", tag)) if (!_nl) ; // { p('|'); p(' '); }

      if (strstr(" p ", tag)) {
        if (_curx>_indent) p(HNL);
        indent(); p(HS);p(HS);
      }
      // TODO: dt
      if (strstr(" li dt ", tag)) {
        p(SNL);
        _indent-= 3; indent(); _indent+= 3;
        printf(" ● "); inx(); inx(); inx();
      }

      // these require action after
      HI(" h1 ", white, black);
      HI(" h2 ", black, green);
      HI(" h3 ", black, yellow);
      HI(" h4 ", black, blue);
      HI(" h5 ", black, cyan);
      HI(" h6 ", black, white);

      HI(BD, red, none);
      HI(IT, none, none);

      HI(HL, magnenta, none);

      HI(FM, yellow, black);
      HI(PR, green, black);
      HI(TT, black, rgb(3,3,3));

      HI(" a ", 27, none);

      // formatting only
      HI(" ul ol ", none, none);
      HI(SKIP, none, none);
      HI(" table ", none, none);
    }
  }
}

int main(int argc, char**argv) {
  if (argc<2 || !strlen(argv[1])) {
    fprintf(stderr, "Usage:\n  ./w URL\n");
    return 0;
  }
  char* url= argv[1];

  // metadata
  printf("\n#=DATE ");
  system("date --iso=s");
  metadata("URL", url, NULL, NULL);
  // TODO: metadata("BASE", 
  TRACE("URL=%s\n", url);

  // get width for formatting
  screen_init();

  // print header line
  C(white); B(black);
  printf("🌍 ");
  B(8); // gray() ??? TODO
  //printf("🏠");
  putchar(' ');

  // simplify url shown
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
  printf("%s", u); nl();

  if (strstr(u, ".txt")==u+strlen(u)-4) {
    // TODO: yesco.org/resume.txt
    _pre= 1;
    //_raw= 1;
  }

  // get HTML
  char* wget= calloc(strlen(url) + strlen(WGET) + 1, 1);
  sprintf(wget, WGET, url);
  f= fopen(url, "r");
  if (!f) // && strstr(url, "http")==url)
    f= popen(wget, "r");
  if (!f) return 404; // TODO: better error

  // render HTML
  C(black); B(white);

  process((TAG){0});

  p(HNL);
  C(white); B(black); p(HNL); p(HNL);

  pclose(f);
  return 0;
}
