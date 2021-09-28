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

// Generic description:
//
// No. If state=0 and I get '<' I
// read a tag name, extract
// attributes till '>" , for some
// special tags I do some formatting
// actions, like newline, if it's a
// "bracketing" tag, like <ul></ul>
// or <h1></h2> <a></a> I do recurse
// and wait for the terminator.
//
// But you'd be surprised how many
// webpages have unterminated <a>
// tags!
// 
// Also, <li> <td> <tr> doesn't need
// the ternimating tag, same with <p>
// so you need to have "recovery"
// mechanism or treat them like
// "separators".
// 
// When reading a tag or word, it
// buffers, only when it received a
// full "symbol" does it take
// decision.
//
// It is true that surprising many
// tags have names that are
// non-overlapping, but I don't use
// this fact.
//
// Instead my formatter is looking
// for symbols that fulfils a
// criteria for a specific
// formatting. So some tags require
// "clear" (content start on
// newline), some have clear-behind,
// etc.
//
// Unless you sanitize and "fix"the
// html before processing it, you'll
// have to "hack it". I belive most
// big webbrowsers do that. That that
// (almost) requires almost to keep
// all of it in memory and do
// extensive processing on it,
// rewriting it as needed. I guess
// one could create a generic
// statemachine that ahdnles this,
// together with a stack and methods
// to "pop" unmatched tags. But it's
// rather complicated I think.
//
// In this case I keep track of a
// running state: fg color, bg color,
// last was whitespace, last was
// newline, AND tags that "need" to
// be matched.  The formatter and
// extractor is 616 LOCs
// C-code.(comments, empty lines, {
// }, else etc not counted).  Tables
// is one issue where you may need to
// keep extensive state, maybe read
// the whole table before formatting,
// unless you institute a policy of
// giving up, or fixing the
// formatting, OR have really simple
// formatting output. I'm trying to
// do the best under limited
// effort. I need to get back to
// tables one day...
//
// - jsk
//
// posted at in facebook discussion

// various debug output
int trace= 0, trace_content= 0;

// sexy words! bold words that are 6 letter or longer ("six-rule by my Swedish teacher)
// TODO: this should be a UI filtering option (in wless)...

int sexy= 1;

// 0 : not sexy
// 1 : hilite words len>=6 or not/n't
// 2 : also hihlite Foo Bar
// 3 : summary (remove not choosen)

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

// general formatting
const int rmargin= 1; // 1 or more (no 0)
const int lmargin= 2;

//const int reverse= 1; // white on black

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

#include <signal.h>

#include "jio.h"

// generally used for parse() of symbols
typedef char TAG[40]; // largest entity 35!

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
  if (!name) return dflt;
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

// TODO: <nobr> - how to do if not fit?
// TODO: <link rel=next|prev|alternate|author|help|icon|prefetch|preload|prerender|search|stylesheet|shortcut|canonical>

// hard space, hard newline
#define HS -32
#define HNL -10
#define SNL -11

// -- groups of tags according to format
#define NL " br hr div pre title h0 h1 h2 h3 h4 h5 h6 blockquote li dt dd table tr noscript address tbody channel feed author item entry pubdate lastbuilddate updated "
#define XNL " br /ul /ol /dl hr tbody /item /entry "
#define HD " title h0 h1 h2 h3 h4 h5 h6 "
//#define CENTER " center caption " // TODO

#define BD " b strong dt label "
#define IT " i em caption legend pubdate lastbuilddate updated "
// TODO: #define UL " u a " ????

// TODO: ins=green, del=red
#define HL " u s q cite ins del noscript abbr acronym "

#define PR " pre "
#define TT " bdo kbd dfn samp var tt code "

// thead tfoot tbody /tbody-optional!
// td/th: rowspan colspan

#define FM " form input textarea select option optgroup button label fieldset legend "

#define SKIP " option table link guid id svg "

// attribute captures
#define TATTR " a body table th td tr font img base iframe frame colgroup span div p link "

// TODO:
#define ATTR " href hreflang src alt aria-label title aria-hidden name id type value size accesskey align valign colspan rowspan span color bgcolor target start role rel "

// TODO:
#define CSS " liststyle color background background-color width height max-height min-height max-width min-width linebreak hypens overflow clip white-space word-break word-spacing word-wrap left right top bottom text-align align-content clear break-before break-after floatdisplay visibility font-size font-weight text-decoration text-shadow text-indent text-justify text-overflow table-layout "

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
    //printf("---jsk: '%s' len=%zu\n", r, len);
    if (r==result) return result;
    if (r==NULL) return NULL;
    error(r!=result, 77, "BAD ptr from u32_to_u8");
  }

  // search for '&name; '
  fnd[strlen(fnd)]= ' ';
  TRACE("[>> \"%s\" <<]", fnd);

  char* m= strstr(ENTITIES, fnd);
  // I think case matters but if not found...
  if (!m) m= strstr(ENTITIES, fnd);
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
//  _cury= 0; _curx= 0; _ws= 1; _nl= 1;
//}

void nl();
void indent();

// TODO: (linebreak chars!)
//  - https://metacpan.org/pod/Unicode::LineBreak

// track visible chars printed
// (assumes one byte == 1 char)
// trick?
//
// TODO:
//   - 32..126 == 1 char
//   - 0b11xx xxxx == 1 char
//   - 0b10xx xxxx == (no count!)
//
// Unicode problem (wide chars):

// ALT: 0) print it in terminal and ask it!
// ALT: 2) wcwidth.c !!!!
// - https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
// AL2: 1) parse and handle this file
// - https://stackoverflow.com/questions/3634627/how-to-know-the-preferred-display-width-in-columns-of-unicode-characters
// - http://www.unicode.org/reports/tr11/#Relation
// - https://www.unicode.org/Public/UCD/latest/ucd/EastAsianWidth.txt
// - http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt
// inx(-2) will step forward 2 steps
int inx(int c) {
  if (c<0) _curx-= c; // add!
  // TODO: accumulate utf8 sequence and decode and check...
  if (iszerowidth(c) || isinsideutf8(c)) return c;
  if (isfullwidth(c)) _curx++;

  _curx++; _nl= 0;
  // TODO: handle overflow? It'll break display assumptions of lines
  if (!_pre && _curx+rmargin +1*!!_fullwidth >= screen_cols) {
    nl();
  }
  return c;
}

// _pc buffers word, and word-wrap+ENTITIES
#define WORDLEN 10
char word[WORDLEN+1] = {0};
int _overflow= 0;

#define FLUSH_WORD -1

int isdelimiter(int c) {
  if (c==FLUSH_WORD || c==NL || c==SNL || c==HS || isspace(c)) return 1;
  if (isutf8(c)) return 0;
  if (strchr(".,:;", c)) return 0; // keept with
  return c<33 || isspace(c);
  return c<'0' || (c>'9' && c<'A') || strchr("[\\\{|}`~", c);
}

void _pc(int c) {
  // TODO: break on '&' this doesn't work:
  //if (strlen(word) && (c<=' ' || c==';' || c=='\n' || c=='\r' || c=='\t' || c=='&')) {

  if (c==FLUSH_WORD || isdelimiter(c)) {
    // output word (if at break char)
    // (sexy)
    if (sexy && (strlen(word)>=6) ||
        strcasestr(word, "not") ||
        strcasestr(word, "n't") || 
        (sexy>=2 && isalpha(word[0]) &&
         word[0]==toupper(word[0]))) {
      if (sexy<=2) {
        printf("\e[1m%s\e[0m", word);
        recolor();
      } else {
        printf("%s", word);
      }
    } else if (sexy<3) {
      printf("%s", word);
    } else {
      // already counted--remove
      _curx-= strlen(word);
      putchar(inx('_'));
    }

    if (c>=0) putchar(c);
    memset(word, 0, sizeof(word));
    _overflow= 0;

  } else if (_overflow) {
    if (!isinsideutf8(c)) {
      if (_curx+rmargin+1 >= screen_cols) {
        // TODO: *overflow* may leave a single char on next line
        putchar('-');
        nl();
      }
      indent();
    }
    putchar(c);

  } else { // add char to word
    int l= strlen(word);
    if (l+1>=WORDLEN) {
      _overflow= 1;
      // flush
      // (must be WORDLEN < screen_cols)
      for(int i=0; i<strlen(word); i++)
        putchar(word[i]);

      putchar(c);
      memset(word, 0, sizeof(word));
      return;
    }
    word[l]= c;

    // word too long for this line?
    if (_curx+rmargin+1 >= screen_cols && !isutf8(c)) {
      nl();
      indent(); // this affects <li> second line indent
      _curx+= strlen(word);
    }
  }
}

long offset= 0;

int myfgetc(FILE* f) {
  long n= ftell(f);
  offset= n<0 ? offset+1 : n;
  return fgetc(f);
}

void myungetc(int c, FILE* f) {
  offset--;
  ungetc(c, f);
}

#define fgetc myfgetc
#define ungetc myungetc

///////////////////////////////////
// state of document parsed

// --- keys for links

char _keys[15]= {'a'-1, 0};
int _nkeys= 0;

// -- generate sequense A:
//  a, b, c...z, aa, ba, ca, .. za, ab, bb, cb, ..zb, ac, ... zz, aaa, baa, caa, ... zaa, aba, ...
//
// https://m.facebook.com/groups/242675022610339/permalink/1700651710145989/?comment_id=1700669053477588&notif_t=group_comment&notif_id=1629376009997714&ref=bookmarks
// recursion by Kjell Post:
// (modified from printf)
void p26(int n, int i) {
  if (n<26)
    _keys[i]= n+'a';
  else {
    p26(n%26, i);
    p26(n/26-1, i+1);
  }
}

void step_key() {
  p26(_nkeys++, 0);
}

void p(int c);

dstr *_rel= NULL;

void print_link_shortcut() {
  if (_rel) return;

  step_key();

  // prelink + part of link fit?
  // let's say 6 text chars...
  if (6+_curx+strlen(_keys)*2+1+rmargin+1 > screen_cols) nl();

  // output [ab] link key selector
  indent();
  int ff=_fg, bb=_bg, ws=_ws; {
    B(rgb(1,2,4)); C(white);
    //B(blue); C(white);
    //B(red); C(white); // retro!
    _fullwidth++;
    char* k= (char*)_keys;
    while (*k) p(*k++);
    _fullwidth--;
  } fg(ff), bg(bb);
  //p(' ');
}

TAG link_tag;
dstr *_url= NULL;

void print_hidden_url() {
  if (!_url) return;
  printf("\e]:A:{%s %s}\e\\", _keys, _url->s);
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

void metadata(char* type, char* a, char* b, char* c, char* d) {
  printf("\n#=%s", type);
  safe_print(a, 1, 0);
  safe_print(b, 1, 1);
  safe_print(c, 1, 1);
  putchar('\n');
  if (c) {
    putchar('\n');
    putchar('#');
    safe_print(d, 0, 0);
    putchar('\n');
  }
}

int lines=0;

dstr *linetags= NULL;

void logtag(char *tag, char *attr, char *val) {
  if (!tag) return;
  if (_skip) return;

  // TODO: if reparse the output it'll be interpreted as the tag :-(
  // (any clever way of quoting?)
  // &amp;amp; will have similar problems...

  if (!attr)
    linetags= dstrprintf(linetags, "\e]:T{ <%s }\e\\", tag+1);
  else
    linetags= dstrprintf(linetags, "\e]:T{ <%.*s.%.*s=%s}\e\\", strlen(tag+1)-1, tag+1, strlen(attr+1)-1, attr+1, val);
}

void print_searchables() {
  if (linetags) {
    printf("%s", linetags->s);
    linetags->s[0]= 0;
  }
}

void nl() {
  lines++;
  printf("\e[K\n"); // workaround bug!
  _cury++;

  // output offset of current location
  // ( diff -I @[[;digit;]]  == ignore ! )
  printf("@%d\n", offset);

  // recolor for each new line
  recolor();
  clearend();
  // indent(); print_state();
  // print at newline if still inside <a>
  print_hidden_url();
  _curx= 0; _ws= 1; _nl= 1;
}

// indent at least to current level
// if posy at higher, don't do any
// Note: first 5 rows leaves room for icon
void indent() {
  if (_pre) return;
  if (lines<=5) _indent+= 7;
  _indent= MAX(0, MIN(10, _indent));
  while(_curx < _indent) {
    putchar(inx(' '));
  }
  if (lines<=5) _indent-= 7;
  _ws= 1;
}

////////////////////////////////////////
// WARNING: table handling and rendering
//          is just a hack for now, bad
//          explorative hack!
#include "table.c"
// Typically, you do not include .c-files:
// TODO: when it'scomplete, move the code here; I want it to be one C-file.

void p(int c) {
  char b= c;
  if (_table)   table= dstrncat(table, &b, 1);
  if (_capture) content= dstrncat(content, &b, 1);

  if (_skip) return;

  // preformatted
  if (_pre) {
    if (c=='\n') {
      nl(); _curx= 0;
    } else {
      // TODO: how to handle non-wrap?
      putchar(inx(c));
    }
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
        _pc(inx(c)); _ws= 0;
      }
    }
    return;
  }

  // TODO: remove?
  if (_fullwidth) _pc(FLUSH_WORD);
  // collapse whitespace
  int tb= (c=='\t'); // TODO: table?
  int ws= (c==' '||tb||c=='\n'||c=='\r');
  if (ws) {
    if (!_curx) return;
    if (!_ws || _pre) {
      _pc(inx(' '));
      // shrink spaces!
      if (_fullwidth) _pc(inx(' '));
    }
    _ws= 1; if (_tb) _tb= tb;
    return;
  }

  // visible chars
  indent();
  if (_fullwidth) {
    // cheat, no word-wrap, print now!
    if (c<128) {
      wchar_t w= 0xff01 + c-33;
      //inx(w); indent(); // might wrap!
      inx(w); indent();
      putwchar(w);
    } else {
      // compensate
      if (isstartutf8(c)) inx(' ');
      putchar(inx(c));
    }
  } else {
    // normals visible: word accumulate
    _pc(inx(c));
  }
  _ws= 0; _tb= 0;
}

// steps one char in input stream
// == true if "have next" (not EOF)
// c is set to character read
FILE* f;

#define STEP ((c= fgetc(f)) != EOF)

// parse chars till one of ENDCHARS
// S: if not NULL, LOWERCASE char, append
// Returns non-matching char
int parse(FILE* f, char* endchars, char* s, int size) {
  int c; char* origs= s;
  if (s) *s++ = ' ';
  while (STEP && (!strchr(endchars, c))) {
    if (s && --size>2) {
      *s++= tolower(c);
      // TODO: error here for google.com
      if (s-origs>sizeof(TAG)) s--;
      //error(s-origs > sizeof(TAG), 7, "TAG waiting for >>>%s<<< too long=%s\n", endchars, origs);
    }
  }
  if (s && size>=2) { *s++ = ' '; *s= 0; }
  return c<0? 0: c;
}

///////////////////////////////////
// specific web-page parsing

// WARNING: don't free these, as they may be shared or not (not big leak)
char *file=NULL, *url= NULL;
char *hosturl= NULL; // ends before '/'

int urlIsFile= 0;

dstr *dsbase= NULL;

// must be absolute URI
// ensures it ends with '/'
// and truncates path filename
//   http://foo.com/bar/index.html
//   => http://foo.com/bar/
// examples:
//   yesco.org       =>  yesco.org/
//   yesco.org/      =>  yesco.org/
//   yesco.org/foo   =>  yesco.org/
//   yesco.org/foo/  =>  yesco.org/foo/
//
// We cannot prefix with http[s]://
//   as we don't know which!
void setBase(dstr* d) {
  if (dsbase) free(dsbase);
  dsbase= d;
  char *s= dsbase->s;
  // last slash
  char *last= strrchr(s+1, '/');

  // last char=='/' already
  // 0) yesco.org/
  // 0) http://yesco.org/
  if (last && last==s+strlen(s)-1)
    ; // OK

  // add if no single '/'
  // 1) yesco.org
  // 2) http://yesco.org
  else if (!last || *(last-1)=='/')
    dsbase= dstrncat(dsbase, "/", 1);

  // truncate after last '/'
  // 3) yesco.org/foo
  // 3) http://yesco.org/foo
  else
    s[last-s+1]= 0;

  // extract host url
  s= dsbase->s;
  char* end= strstr(s, "//");
  end= end ? end+2 : s;
  end = strchr(s, '/');
  // copy upto but not including '/'
  hosturl= strndup(s, end-s);
}

void setLinkUrl(dstr* val) {
  if (_url) free(_url);
  _url= val;
  if (!_url || !dsbase || !hosturl) return;

  //fprintf(stderr, "\nFIXURL:  %s\n", _url->s);

  // absolute url
  if (strchr(_url->s, ':')) return;
  char* dslash= strstr(_url->s, "//");
  if (dslash > _url->s) return;

  dstr *n= NULL;
  // absolute path /foo...
  if (dslash) 
    n= dstrncat(NULL, "http:", -1);
  else if (_url->s[0]=='/')
    n= dstrncat(NULL, hosturl, -1);
  else
    n= dstrncat(NULL, dsbase->s, -1);
  
  n= dstrncat(n, _url->s, -1);
  free(_url);
  _url= n;

  //fprintf(stderr, "FIXURL=> %s\n", _url->s);
}

void addContent();

// extracted by process
char *style= NULL, *sc= NULL, *sb= NULL;

int process(TAG *end);

// Use HI macro! (passes in tag)

#define HI(tags, fg, bg) hi(&tag, tags, fg, bg)

// hilight TAG if it's part of TAGS
// using FG color and BG color, and other
// formatting.

// After the matching </TAG> is reached:
// restore colors and undo formatting.
// Returns true if tag was triggered, thus consumed
int hi(TAG *tag, char* tags, enum color f, enum color b) {
  static int level= 0;
  if (!tag || !*tag || !strstr(tags, *tag)) return 0;

  level++;
//printf("{%d %s %d %d}", level, tag?*tag:"(null)", f, b);
  TRACE("--->%d %s %d %d\n", level, tag?*tag:NULL, f, b);

  // save colors
  int sfg= _fg, sbg= _bg, spre= _pre, sskip= _skip, sindent=_indent, sreverse= _reverse; {
    // - START highlight
    if (f != none) C(f);
    if (b != none) B(b);
    if (strstr(PR, tag)) _pre++;

    if (strstr(" ul ol dl ", tag)) _indent+= 2;

    // TODO: _capture <title>?
    if (strstr(" a link ", tag)) {
      underline();
      fg(_fg); bg(_bg); // needed?
      _capture++;

      if (_url) {
        print_hidden_url();
      } else if (!_rel) {
        // non-XHTML supression of link
        print_link_shortcut();
        p('l'),p('i'),p('n'),p('k');
      }

    }
    if (strstr(" table ", tag)) {
      table= dstrncat(NULL, NULL, 1024);
      _table++;
    }
    // bold
    if (strstr(BD, tag)) { printf("\e[1m"); }

    // italics
    if (strstr(IT, tag)) { printf("\e[3m"); fg(_fg); bg(_bg); };
    // fullwidth
    if (strstr(HD, tag)) {
      _fullwidth++;
      // TODO: clean this hack
      // this achieves bold white on black for h1
      if (strstr(" h1 ", tag)) {
        _reverse= !_reverse;
        printf("\e[27;1m");
        B(black); C(white);
      } else {
        // bold
        printf("\e[7;1m");
      }
      recolor();
    }

    // -- style overrides
    if (sc) C(decode_color(sc, _reverse?_bg:_fg));
    if (sb) B(decode_color(sb, _reverse?_fg:_bg));
    FREE(style); FREE(sc); FREE(sb);

    // -- content
    {
      // actions just before content
      if (strstr(" h1 ", tag)) p(HNL);
      if (strstr(HD, tag)) p(HS);
      if (strstr(TT, tag)) p(HS);
      if (strstr(SKIP, tag)) _skip++;
    
      // find end tag (recurse)
      TAG end = {' ', '/'};
      strcpy(end+2, *tag+1);
      process(end);
      if (strstr(SKIP, tag)) _skip--;
    }
    // -- end content

    // -- ENDing highlight/formatting
    // TODO: make state changes clearer
    if (strstr(TT, tag)) p(HS);

    // off underline links!
    if (strstr(" a link ", tag)) {
      end_underline();
      // TODO: make again, but at next new line...
      //metadata("LINK", link_tag, &_keys[0], content->s, _url->s);

      if (!--_capture) addContent();
      setLinkUrl(NULL);
    }
    if (strstr(" table ", tag)) {
      if (!--_table) renderTable();
    }

    // off bold
    if (strstr(BD, tag)) { printf("\e[0m"); recolor(); }

    // off italics
    if (strstr(IT, tag)) printf("\e[23m");

    // off fullwidth
    if (strstr(HD, tag)) {
      _fullwidth--;
      //printf("\e[0m");
      //recolor();
    }

    // restore saved state (colors/pre/skip)
  } _pre= spre; _skip= sskip; _indent= sindent, _reverse= sreverse;
  if (strstr(NL, tag)) p(SNL);
  fg(sfg); bg(sbg); // TODO: recolor

  // hack for bold headlines :-(
  if (strstr(HD, tag)) {
    printf("\e[0m");
    recolor();
  }

  level--;
  TRACE("<--%d %s\n", level, tag?*tag:NULL);
  return 1;
}

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
  if  (!tag || !attr || !val) return;

 // - https://www.w3.org/TR/html4/appendix/notes.html#non-ascii-chars

  // B.2.2 Ampersands in URI attribute values The URI that is constructed when a form is submitted may be used as an anchor-style link (e.g., the href attribute for the A element). Unfortunately, the use of the "&" character to separate form fields interacts with its use in SGML attribute values to delimit character entity references. For example, to use the URI "http://host/?x=1&y=2" as a linking URI, it must be written <A href="http://host/?x=1&#38;y=2"> or <A href="http://host/?x=1&amp;y=2">.
  // We recommend that HTTP server implementors, and in particular, CGI implementors support the use of ";" in place of "&" to save authors the trouble of escaping "&" characters in this manner.

  // TODO: decode val! &amp:....
  // and %xx and + for ' ', lol?

  entity* e= lastentity;
  if (trace) printf("\n---%s.%s=%s\n", tag, attr, val->s);

  if (strstr(" id name tabindex key accesskey ", attr)) {
    // TODO: this might become too long...
    logtag(tag, attr, val->s);
  }

  // guardian doesn't terminate <link>
  if (strstr(" rel ", attr) &&
      strstr(" link ", tag)) {
    // capture rel and don't display link
    // at is has a "function" for the browser...
    FREE(_rel);
    _rel= val;

    // just ignore preload/cache links..
    return;

    // just print it
    p(' ');
    char *s= val->s;
    while(*s) p(*s++);
    p(' ');
    return;
  }

  // embed URL as hidden message
  if (strstr(" href src ", attr)) {
    if (strstr(" base ", tag)) {
      // TODO: conflicted, only first should be honored...
      // - https://url.spec.whatwg.org/#concept-url-serializer
      // - https://html.spec.whatwg.org/multipage/urls-and-fetching.html#fallback-base-url
      setBase(val);
      return;
    }

    // frame - make a link
    // TODO: render as a tag, but no content?
    //if (strstr(" frame iframe "))

    // don't put in link for images
    // TODO: render differnt?
    if (strstr(" a link ", tag)) {
      memcpy(link_tag, tag, sizeof(link_tag));
      setLinkUrl(val);
      //print_hidden_url();

      print_link_shortcut();

      return;
    }
  }    

  // dstr * not used
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

// unquote HTML inside tag (RSS/ATOM)
// TODO: DESCRIPTION SUMMARY CONTENT
//       (IFF have type='xhtml')
int _unquote= 0; 

int process(TAG *end) {
  int c;
  while (STEP) {
    print_searchables();
 
    if (c==CTRL+'D') {
      // == EOF (interactive)
      // exit the hard way!
      printf("\n---EOF\n");
      fflush(stdout);
      // TODO: maybe not so good?
      exit(0);
    } else if (c=='\e') { // let ansi through
      _pc(FLUSH_WORD);
      do {
        putchar(c);
      } while (STEP && !isalpha(c) && c!='\\' && c!=7);
      putchar(c);
    } else if (c!='<' && c!='&') { // content
      p(c);

    } else if (c=='&') { // &amp;
      TAG entity= {0};
      char *e= &entity[0], *s= e;
      do {
        *e++= tolower(c);
        // check if start of entity
        if (entity[1]!='#' && !strcasestr(ENTITIES, entity)) break;
      } while (e-s+2<sizeof(entity) && STEP && (isalnum(c) || c=='#'));
      if (c==';') *e++= c; else ungetc(c, f);
      char* d= decode_entity(entity);
      s= d ? d : s;

      // single char
      // TODO: doesn't really work as we take different path for reading '>', unless..., we move content decoding to STEP (and fgetc()). LOL
      if (_unquote && *s && !*(s+1)) {
        ungetc(*s, f);
        continue;
      }

      // print - it's all printables
      while(*s) p(*s++);

    } else { // '<' tag start
      TAG tag= {0};
      FREE(_rel);

      _pc(FLUSH_WORD);

      // parse tag
      c= parse(f, "> \n\r", tag, sizeof(tag));
      TRACE("\n---%s\n", tag);

      if (strstr(" !-- ", tag)) { fscan(f, "-->"); continue; }

      // process attributes till '>'
      // TODO:move out to function
      if (c!='>' && c!='/') {
        // <TAG attr>
        // any tag may contain id/name
        if (1 || strstr(TATTR, tag)) {
          newTag(tag);
          // TODO: CSS cheat: match nay
          while (STEP) {
            // TODO: hmmm
            ungetc(c, f);
            ungetc(skipspace(f), f); //hmm
            // read attribute
            TAG attr= {0};
            c= parse(f, "= />\"\'", attr, sizeof(attr));
            if (c=='>' || c=='/' || c==EOF) break;
            //printf("\n---%s.%s\n", tag, attr);
            // do we want it?
            if (strstr, ATTR, attr) {
              int q= skipspace(f);
              // TODO: move out?
              // merge w parse?
              dstr *v = NULL;
              char ch;
              if (q=='"' || q=='\'') {
                // id='foo' id="foo"
                while (STEP && c!=q) {
                  ch= c;
                  v = dstrncat(v, &ch, 1);
                }
              } else {
                // TODO: not correct... misses first char
                // id=foo
                ungetc(q, f);
                while (STEP && !isspace(c) && c!='>') {
                  ch= c;
                  v= dstrncat(v, &ch, 1);
                }
                ungetc(c, f);
              }

              if (v) {
                // capcture colors
                if (strstr(" color ", attr))
                  sc= strdup(v->s);
                if (strstr(" bgcolor ", attr))
                  sb= strdup(v->s);

                if (strstr(" style ", attr)) {
                  // TODO: conflict with bgcolor/color being substring...
                  sc= getcolonval("color", v->s);
                  sb= getcolonval("background-color", v->s);
                  if (!sb) sb= getcolonval("background", v->s);
                  style= strdup(v->s);
                }

                // addAttr can store/free v
                addAttr(tag, attr, v);
              }
            }
          }
        }
      }

      // TODO :don't if done (in addAttr)
      logtag(tag, NULL, NULL);

      // XML single tag
      if (c=='/') {
        c= skipspace(f);
        while (c!='>') {
          printf("{%c}", c);
          if (!STEP) return 0;
        }
        FREE(_rel);
        continue;
      } else if (strstr(" link ", tag) && _rel) {
        // NOT: xhtml... 
        // guardian NO '/' <link href=>
        // (this only matters as we extract content of <link> for RSS/ATOM)
        // already got link, maybe no content- terminate
        //printf("\n{_url=%s}", _url);
        continue;
      }

      FREE(_rel);

      // end of tag
      if (c!='>') c= parse(f, ">", NULL, 0);

      // skippers
      if (strstr(" script ", tag)) { fscan(f, "</script>"); continue; }
      if (strstr(" style ", tag)) { fscan(f, "</style>"); continue; }

      // pre action for tags (and </tag>)
      // TODO: /th /td /tr tags are optional.. :-(
//      if (strstr(" /th /td  ", tag)) { /*p(-'\t'); */ p(' '); p('|'); p(' '); }
//      if (strstr(" /tr ", tag)) p(SNL);

      if (strstr(XNL, tag)) p(HNL);
      if (strstr(" td th tr /td /th /tr /table ", tag)) {
        //if (strstr(" td th tr /table ", tag)) {
        if (0) {
          end_underline();
          if (strstr(" td th ", tag))
            putchar('|');
        }
        //printf("\n[===%s===]\n", tag);
        handle_trd(strstr(" tr ", tag), strchr(tag, '/'), tag);
      }

      // check if </endTAG>
      if (strstr(*end, tag)) return 1;

      // pre action for some tags
      if (strstr(NL, tag)) {
        p(SNL);
        // restore color (missing </a>)
        if (strstr(" a ", end)) {
          B(white); C(black);
        }
      }

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
      if (strstr(" li dd dt ", tag)) {
        p(SNL);
        // first line special indent
        _indent-= 3; indent(); _indent+= 3;
        if (strstr(" dd ", tag)) {
          printf("   "); inx(-3);
        } else if (strstr(" dt ", tag)) {
          // dt becomes shifted left
          //   dd here
        } else {
          printf(" ● "); inx(-3);
        }
        inx(-3);
        _nl=1, _ws=1;
      }

      // these require action after
      // NOTE: only one should be trigggered as they consume input!
      // these recurses on process and will return after matching tag found
      // - HI(tag, fg, bg)
      HI(" title subtitle summary ", white, 20) || // html,atom*2
        HI(" h0 ", black, white) ||
        HI(" h1 ", white, black) ||
        HI(" h2 ", black, green) ||
        HI(" h3 ", black, yellow) ||
        HI(" h4 ", white, blue) || 
        HI(" h5 ", black, cyan) ||
        HI(" h6 ", black, white) ||

        HI(BD, none, none) ||
        HI(IT, none, none) ||

        HI(HL, magenta, none) ||
        HI(FM, red, 229) ||

        //TODO: <input/> not handled
        // HI expects end-tag: </...
        //HI(FM, yellow, black);
        
        HI(PR, green, black) ||
        HI(TT, black, rgb(3,3,3)) ||

        // TODO: if change from 27 edit jio.c
        HI(" a link ", 27, none) || // ldarkblue

        // specialized xml types
        HI(" rss atom feed ", none, 215) || // orange
        HI(" xml ", none, 247) || // boring gray

        // formatting only
        HI(" ul ol dl ", none, none) ||
        HI(SKIP, none, none) ||
        HI(" table ", none, none) ||

        // apply color/bgcolor changes
        // (ignore color on link-tag!)
        ((sc || sb) && !strstr(" link meta ", tag) && HI(tag, none, none)) ||
        0;
    }
  }
  // except for toplevel we end up here for not properly terminated tags
  // but only for tags we've done HI() on.
  return 0;
}

int main(int argc, char**argv) {
  // get width for formatting
  screen_init(0);

  if (argc<2 || !strlen(argv[1])) {
    fprintf(stderr, "Usage:\n  ./w [FIL] URL [COLS] # cols must be 3rd arg\n");
    return 0;
  }
  file= argc>1 ? argv[1] : "-";
  url= argc>2 ? argv[2] : file;
  // override if run from script
  if (argc>3) screen_cols= atoi(argv[3]);

  // metadata
  printf("\n#=DATE "); fflush(stdout);
  system("date --iso=s");

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
    int ff=_fg, bb=_bg;
    C(red);
    printf("🚩");
    fg(ff); bg(bb);
    u+= 7;
  } 
  if (u[strlen(u)-1]=='/')
    u[strlen(u)-1]=0;
  printf("%s", u); nl();

  // detect preformatted text
  if (endswith(u, ".txt") || endswith(u, ".ANSI") || endswith(u, ".ANSI")) {
    _pre++;
  }
  
  // get HTML
  char* wget= calloc(strlen(url) + strlen(WGET) + 1, 1);
  sprintf(wget, WGET, url);
  // if stdin, make "interactive"
  if (!strcmp(file, "-")) {
    jio();
    f= stdin;
    // TODO: maybe
    // _pre++;
  } else {
    f= fopen(file, "r");
  }
  urlIsFile= f && file==url;
  // TODO: remove? make pure renderer?
  if (!f)
    f= popen(wget, "r");
  if (!f) return 404; // TODO: better error

  // if is file, keep it null
  if (urlIsFile) {
    metadata("FILE", file, NULL, NULL, NULL);
    // base==NULL
  } else {
    metadata("URL", url, NULL, NULL, NULL);
    setBase(dstrncat(NULL, url, -1));
    //printf("base=%s\n", dsbase?dsbase->s:"(null)");
    //printf("hosturl=%s\n", hosturl?hosturl:"(null)"); exit(0);
  }

//  fprintf(stderr, "\nurlIsFile=%s\nequal: %s\nfile=%s\nurl=%s\nbase=%s\n", urlIsFile?"true":"false", file==url?"==":"!=", file, url?url:"(null)", dsbase&&dsbase->s?dsbase->s:"(null)"); exit(0);

  // render HTML
  C(black); B(white);

  process((TAG){0});

  p(HNL);
  C(white); B(black); p(HNL); p(HNL);

  pclose(f);
  return 0;
}
