// TODO: keep rereading file to displaY?
// tail -f source:
// - http://git.savannah.gnu.org/cgit/coreutils.git/tree/src/tail.c

// partly from old imacs.c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "jio.h"

// lazy include no .h
#include "graphics.c"

#define LOADING_FILE ".w/Cache/loading.html.ANSI"
#define INTERNET_SEARCH "https://duckduckgo.com/html?q=%s"


// generic functions?


void message(char* format, ...) {
  va_list argp;
  va_start(argp, format);

  save();
  // last line
  gotorc(screen_rows-2, 0);
  int sbg= B(black), sfg= C(green);
  vprintf(format, argp);
  B(sbg); C(sfg);
  clearend();
  restore();
  fflush(stdout);
}


// set k=NO_REDRAW to not update screen
// (good for showing temporary information like menus/hilite)
#define NO_REDRAW -1
#define REDRAW (CTRL+'L')

// --- limits
int nlines= 0, nright= 10;

int ntab= 1; // number of newly openeed tabs + last

int rows;

// --- "global" browser state
FILE *fhistory, *fbookmarks;

int top, right, tab, start_tab;

char *_search= NULL;
int _only= 0;

int _click_r=-100, _click_c=-100;


char *hit= NULL; // FREE!
// DONT free (partof hit)
char *file= NULL;
char *url= NULL;

dstr *line= NULL;

void clearcmd() {
  if (line) line->s[0]= 0;
}


int incdec(int v, int k, int ikey, int dkey, int min, int max, int min2val, int max2val) {
  if (k==ikey) v++;
  if (k==dkey) v--;
  // fix
  if (v<min) v= min2val;
  if (v>max) v= max2val;
  return v;
}

#define COUNT(var, ikey, dkey, limit) var= incdec(var, k, ikey, dkey, 0, limit-1, 0, limit-1)
#define COUNT_WRAP(var, ikey, dkey, limit) var= incdec(var, k, ikey, dkey, 0, limit-1, limit-1, 0)

///////////////////////////////////
// bookmarks + links

// TODO: dynamic array? or read from file and match on every keystroke?
// (some newspapers have 26*26*5++ links! [cee] )
#define LINKS_MAX 26*26*26
int nlinks= 0;
char *links[LINKS_MAX] = {0};

void trunclinks(int n) {
  // free old links
  while(nlinks > n) {
    char *l= links[--nlinks];
    if (l) free(l);
    links[nlinks]= NULL;
  }
}

FILE *fopenext(char *fname, char *ext, char *mode) {
  char tmp[strlen(fname)+1+strlen(ext)+1];
  strcpy(tmp, fname);
  char *ldot= strrchr(tmp, '.');
  if (ldot)
    strcpy(ldot, ext);
  else
    strcat(tmp, ext);
  //printf("....open: %s\n", tmp); key();
  return fopen(tmp, mode);
}

// Load bookmarks/keyboards from FILE
// Returns number of items added
int loadshortcuts(char *file) {
  if (!file) return 0;
  FILE *flinks;

  // .ansi file - extract links
  if (endswith(file, ".ANSI")) {
    flinks= fopenext(file, ".LINKS", "r");
  } else {
    flinks= fopen(file, "r");
  }
  // TODO: display error message?
  if (!flinks) return 0;

  // TODO: this doesn't work with popen
  char *lk;
  while (lk=fgetline(flinks)) {
    if (nlinks<LINKS_MAX)
      links[nlinks++]= lk;
    else if (LINKS_MAX>=nlinks) {
      message("loadshortcuts: LINKS_MAX reached!");
      //error(LINKS_MAX>=nlinks, 77, "Getmore links! %d");
    }
  }
  fclose(flinks);
}

// score of string A matched to B
//    INT_MAX  "FOO" to "FOO"
//  INT_MAX/2  "foo" to "FOO"
//      10000  "foo" to "fo"
//       9993  "foo bar" to "bar"
//       9990  "fo ba bar" to "bar"
//          1  "foo" "foO"
//          2  "fie foo fum" "foo fum"
//          2  "fie foo foo bar" "foo bar"
//
// Returns  0  if no match
//    INT_MAX  if equal (case match)
//  IMT_MAX/2  if equal (case not)
//      10000  if it's a prefix
//       1000+ if substring (>= 1000)
//          n  prefixwords matched
int match(char* a, char* b) {
  // equal
  if (!strcmp(a, b)) return INT_MAX;
  if (!strcasecmp(a, b)) return INT_MAX/2;

  // prefix (TODO: case)
  int l= lprefix(a, b);
  if (l==strlen(b)) return 10000;

  // substring
  char* m= strcasestr(a, b);
  // but start of word
  if (m && m-a>0 && isalnum(*(m-1))) return 0;
  if (m) return 9999- (m-a);

  // matches prefixes to words in order
  char* s= strchr(b, ' ');
  int n= s? s-b : strlen(b);
  int r= 0;
  while (a && *a && b && *b) {
    char* wd= NULL;
    int l= 0;
    // TODO: malloc/free expensive?
    sscanf(b, "%ms%n", &wd, &l);

    // find it
    m= strcasestr(a, wd);
    if (wd) free(wd);
    if (!m) return 0;
    // but start of word
    if (m && m-a>0 && isalnum(*(m-1))) return 0;
    
    // matched one
    r++;

    // next word
    while(*b && *b!=' ') b++;
    while(*b && *b==' ') b++;
    if (!*b) return r;
    
    a= m+l;
    while(*a && isalnum(*a)) a++;
    while(*a && *a==' ') a++;
    if (!*a) return 0;
  }
  return r;
}

// TODO: search prefixkey, or string
keycode listshortcuts() {
  clear();
  printf("\n=== ShortCuts/Links ===\n\n");
  for(int i=0; i<nlinks; i++) 
    printf("%s\n", links[i]);

  // try clever matching of command line
  if (line->s[0]) {
    for(int i=0; i<nlinks; i++) {
      int m= match(links[i], line->s);
      if (0) ;
      else if (m>10000) C(green);
      else if (m>1000) C(yellow);
      else if (m) C(cyan);
      else continue;
      printf("==> %5d %s\n", m, links[i]);
    }
  
    C(white);
  }

  printf("\n\n(press CTRL-L to see browser page)\n");
  return NO_REDRAW;
}

// TODO: research universal XML bookmarks format called XBEL, also supported by e.g. Galeon, various "always-have-my-bookmarks" websites and number of universal bookmark converters.

void logbookmark(int k, char *s) {
  //log(bms, url, offset, top, s);
  gotorc(screen_rows-1, 0);
  cleareos();
  dstr *ds= dstrprintf(NULL, "%s %s %d %d %c%s\n",
    isotime(), url,  -1, top, k, s);
  // print to screen w/o newline
  fputs(ds->s, fbookmarks);
  free(ds);
  message("Saved bookmark '%c' = '%s'", k, s);
}

void loghistory(char *url, int ulen, char *file) {
  dstr *ds= dstrprintf(NULL, "%s #=W %.*s %s\n",
    isotime(), ulen, url, file);
if (strstr(ds->s, "LINKTEXT")) {
  printf("\n\n\n-------------loghistory- LINKTEXT!---->%s<\n", ds->s);
  if (key()==CTRL+'C') assert(!"stop");
}
  fputs(ds->s, fhistory);
  fflush(fhistory);
  free(ds);
}

keycode listbookmarks(char *url, char *s) {
  char *line;
  clear();
  C(black); B(white);
  if (url)
    printf("./wbookmarks %s", url);
  else if (s && *s)
    printf("./wbookmarks %s", s);
  else 
    printf("./wbookmarks");
  clearend(); printf("\n\n");
  C(white); B(black);
  
  fseek(fbookmarks, 0, SEEK_SET);
cursoron();
  int mcount= 0;
  while(line= fgetline(fbookmarks)) {
    char *m=NULL;
    if ((!url || !*url || strstr(line, url))
        && (!s || !*s || (m= strcasestr(line, s)))) {

      // it's a match!
      //printf("= %s\n", line);

      char *date, *u, *data;
      int offset, top;
      int n= sscanf(line, "%ms %ms %d %d %m[^]",
        &date, &u, &offset, &top, &data);

      // simplify url to show
      char *simple= u;
      simple= sskip(simple, "https://");
      simple= sskip(simple, "http://");
      simple= sskip(simple, "www.");
      simple= strunc(simple, "?");
      simple= sdel(simple, ".html");
      simple= sdel(simple, ".htm");

      //printf("matched=%d (5)\n", n);
      if (n == 5 && data[0]!='=') {
        mcount++;

        // --- got a matched result

        const int right= 16;
        int cols= screen_cols-right-2;

        if (strlen(data)<=cols)
          printf("%-*s ", cols, data);
        else
          printf("%-*s ", cols, "");

        // print right column
        if (url) {
          C(yellow);
          printf("%.16s\n", isoago(date));
        } else {
          C(cyan);
          if (strlen(simple)>right)
            printf("%.13s...\n", simple);
          else
            printf("%.16s\n", simple);
        }
        C(white);

        if (strlen(data)>cols)
          printf("%s\n", data);

      }

      free(line);
      free(date); free(u); free(data);
    } else {
      free(line);
    }
  }

  printf("\n%d Matching Lines\n", mcount);
  printf("(CTRL-L to redraw)\n");
  return NO_REDRAW;
}

keycode bookmark(int k, char *text) {
  int cpos= -1; // TODO

  if (k=='*' || k==CTRL+'D') {
    k= '*'; text= "";
  }

  // log all including searches!
  logbookmark(k, text);

  // search
  if (k=='=') listbookmarks(NULL, text);

  return NO_REDRAW;
}

// search is performed in printAnsiLines()
void search(int k, char* text) {
  if (!text || !strlen(text)) {
    FREE(_search);
  } else {
    _search= strdup(text);
  }
}

// start download in background
void download(char* url, int force, int dolog) {
  if (!url || !*url) return;
  char *end= strpbrk(url, " \t\n");
  int ulen= end? end-url : strlen(url);

  // quote & log
  dstr *file= dstrncat(NULL, ".w/Cache/", -1);
  file= dstrncaturi(file, url, ulen);
  file= dstrncat(file, ".ANSI", -1);
  if (dolog)
    loghistory(url, ulen, file->s);

  FILE *f= force ? NULL : fopen(file, "r");
  free(file);

  // if .ANSI file exists, exit
  if (f) {
    fclose(f);
  } else {
    dstr *cmd= dstrprintf(NULL, "./wdownload %s \"%.*s\" %d %d &",
      force?"-d":"", ulen, url, screen_rows, screen_cols);
    system(cmd->s);
    free(cmd);
    // wait a little for .TMP to be created
    // TODO: fix this timing issue, create the .TMP file?
    usleep(300*1000);
  }
}

void reload(char* url) {
  download(url, 1, 0);
}

int netErr() {
  // net down?
  FILE *f= fopen(".wnetdown", "r");
  if (f) {
    gtoasterr("Net Down");
    fclose(f);
    return 1;
  }
  return 0;
}

// opens ansi file, or reloads it and waits, any key will return
FILE *openOrWaitReloadAnsi() {
  // wait for open of ANSI file
  FILE *fansi= fopen(file, "r");
  FILE *ftmp= fopenext(file, ".TMP", "r");
  
  //
  if (!fansi && !ftmp) {
    FILE *ferr= fopenext(file, ".ERR", "r");
    // TODO: easy way to remov all this code: on error generate an .ANSI with error?
    // error previously - report again
    // this red error message is too much in the face?
    if (ferr) {
      wclear();
      char buf[80];
      fgets(buf, sizeof(buf), ferr);
      char *err= strstr(buf, "ERROR ");
      if (err) {
        char *end= strchr(err, '.');
        if (end) *end= 0;
        gtoasterr(err);
      } else {
        gtoasterr("Load Err");
      }
      fclose(ferr);
    } else {
      gotorc(1,0);
      // file missing in cache
      gtoast(" Download? ");

      // if waited 1s then reload
      if (keywait(1000)>1000) {
        gtoast("  Loading  ");
        gotorc(1, 0); // place of >>>
        reload(url);
        keywait(300);
        ftmp= fopenext(file, ".TMP", "r");
      }
    }
  }
  // wait if have .TMP till not there
  // that signals the end of .ANSI created
  gotorc(1, 0);
  int zlast= 0;
  while (ftmp && !haskey()) {
    usleep(300*1000);
      
    fseek(ftmp, 0, SEEK_END);
    int z= ftell(ftmp);
    if (z!=zlast)
      printf(" %d ", z);
    zlast= z;
    fclose(ftmp);
      
    putchar('>'); fflush(stdout);
    // TODO: ?
    if (netErr()) return NULL;

    ftmp= fopenext(file, ".TMP", "r");
  }
  if (ftmp) fclose(ftmp);

  fansi= fopen(file?file:".stdout", "r");
  if (!fansi && netErr()) return NULL;
  
//  -- no difference in speed...
// wc: takes 0.01s
// cat: takes 0.11s
//
//  static char *buffer= NULL;
//  static int *size= 100*100*3;
//  if (!buffer) buffer= malloc(size);
//  if (fansi) setbuffer(fansi, buffer, size); // _IOFBF, size);

  nlines= fansi? flines(fansi) : -1;
  return fansi;
}

int findlink(char *ln) {
  // We find beginning of link by hidden text
  char *p= strcasestr(ln,"\e]:A:");
  if (!p) return 0;
  // TODO: find better:
  // end of link: end underline, LOL
  char *end= strcasestr(ln, "\e[24m");
  int len= !end? 0xff : end-p;
  return ((p-ln)<<8) + MIN(0xff, len);
}

// limited to matching 255 chars
// Returns: pos*256 + len
//          0 if no match
//  if len==0xff -> end of string
int matchfinder(char *ln, char *pat) {
  if (!ln || !*ln || !pat || !*pat) return 0;
  if (!strcmp(pat, "LINKS"))
    return findlink(ln);
  char *p= strcasestr(ln, pat);
  if (!p) return 0;
  int len= strlen(pat);
  return ((p-ln)<<8) + MIN(0xff, len);
}

int visCol(char *ln, char *end) {
  if (!ln) return -100;
  int col= 0;
  char c;
  while ((c= *ln) && ln++<=end) {
    if (c=='\r') col= 0; // ^M
    else if (c=='\e') {
      if ((c= *ln)==']') {
        // skip hidden text
        while ((c= *ln++) && c!='\e');
        c=*ln++; // skip \\
      } else {
        // skip till letter
        while ((c= *ln++) && !isalpha(c));
      }
    } else if (c<32) ;
    else if (c>127) {
      if (isstartutf8(c)) col++;
    } else {
      if (c>=' ') col++;
      // TODO: detect fullwidth char? :-(
    }
  }
  return col;
}

// inject "codes" to rest color (hilite) after any code terminal co
int _screen_top= 0;
int _screen_left= 0;
int _curx= 0;

// poor mans clear end for limited width window!
void _clearend() {
  spaces(screen_cols-_curx);
  _curx= screen_cols;
}

void printansi(int len, char *ln, char *codes) {
  static int ch= 0, chk= 0;
  if (!ln || len<=0) return;
  char c;
  _curx= 0;
  while ((c= *ln++) && len-->0) {
    if (c=='\n' || c=='\r') {
      _curx= 0; ch= 0;
      printf("\e[%dG", _screen_left);
      printf("\e[24;0m"); // reset
      continue;
    }
    if (_curx>= screen_cols) continue;

    // decode utf-8 inline!
    if (isstartutf8(c)) {
      putchar(c);
      ch= c;
      chk= 0;
      int m=0x80;
      while(m & ch) {
        ch &= ~m;
        m>>= 1;
        chk++;
      }
      chk--;
    } else if (isinsideutf8(c)) {
      putchar(c); ch= (ch<<6) + (c & 0x3f);
      if (--chk==0 && isfullwidth(ch))
        _curx++;
    } else
      putchar(c); ch= 0;
    if (c>31 && !isinsideutf8(c))
      // TODO: wide chars isfullwidth()
      // but that requires "whole char"
      _curx++;
    else if (c=='\r' || c=='\n')
      _curx= 0;
    else if (c=='\e') {
      c= *ln++;
      if (!c) return;
      putchar(c);
      if (c==']') {
        // TODO: not needed? - delete?
        // hidden text
        while ((c= *ln++) && c!='\e')
          putchar(c);
        if (!c) return;
        putchar(c);
        c= *ln++; // read \\
        if (!c) return;
        putchar(c);
      } else {
        // ansi codes-skip till letter
        while ((c= *ln++) && (!isalpha(c) && c!='\\' && c!=7))
          putchar(c);
        if (!c) return;
        // if "clear" ignore...
        if (c=='K')
          putchar(' ');
        else
          putchar(c);
      }
      // after possible change reapply codes
      if (codes) printf("%s", codes);
    } 
  }
}

int _clicked= 0;

// print ansi line and hilite matches
int printansiln(char *ln, int n, int matchLink) {
  char *s= ln;

  int m= matchLink? findlink(s) : matchfinder(s, _search);
  char *f= m ? s+(m>>8) : NULL;
  int len= m & 0xff;

  char *found= f; // a flag!

  // trunacte if only and no match
  if (_only && !f) *s= 0;

  // reset codes before each text char!
  char *codes= -n==screen_rows/2-1?"\e[27m":"";
  codes= "";

  // find and hilite each match
  while(f) {
    printansi(f-s, s, codes);

    // -- print match
    // test if click on
    if (_search && !strcmp(_search, "links")) {
      int r= -n-1, c= visCol(ln, f);
      int vend= visCol(ln, f+len);
      if (_click_r==r+1 && c<=_click_c && _click_c<=vend) {
        char *p= sskip(f, "\e]:A:{");
        // copy "shortcut" to cmd line
        line->s[0]= 0;
        while(*p && !isspace(*p))
          line= dstrncat(line, p++, 1);
        // go!
        tab= click(line);
        _clicked= 1;
        // make click pos inactive!
        // todo: cleaner?
        FREE(_search);

        B(red); C(white);
      }
    } else if (_search) {
      // hilite every match
      B(red); C(white);
    }

    //printf("\e ");
    printansi(len, f, codes);
    printf("\e[24;0m"); // reset

    if (len==0xff) { f=NULL; break; } // all
    s= f + len;

    // assume color (todo search back?)
    B(white); C(black);
    // find next match
    m= matchLink? findlink(s) : matchfinder(s, _search);
    len= m & 0xff;
    f= len ? s+(m>>8) : NULL;
  }
  // print remainder
  // (this will always be called, even for lines not to be displayed)
  printansi(strlen(s), s, codes);

  return found;
}

// Return: true if clicked (askin redo page)

int printAnsiLines(FILE *fansi, int top, int rows) {
  _clicked= 0;

  int matchLink = _search && !strcmp(_search, "LINKS");
  int c, n=top;
  rows-=1;

  fseek(fansi, 0, SEEK_SET);
  dstr *ln= dstrncat(NULL, NULL, 160);
  char *found= NULL;
  while(c= fgetc(fansi)) {
    // TODO: cleanup when make the hidden lines simplier...
    if (c=='\n' || c==EOF) {
      // -- print accumulated line
      if (ln->s[0]) {
        gotorc(_screen_top-n, _screen_left);
        found= printansiln(ln->s, n, matchLink);
        ln->s[0]= 0;
      }
      if (c==EOF) break;
      c= fgetc(fansi);
      if (c!='\n' && c!='#') {
        // count of lines printed
        if (n<0 && (!_only || found || matchLink)) {
          _clearend();
          putchar('\n');
        }
        if (n>=0 || !_only || found || matchLink)
          n--;
         if (n<-rows) break;
      } else {
        // skip comment line(s)
        while(c=='\n' || c=='#') {
          if (c=='#')
            while((c= fgetc(fansi)) != EOF && c!='\n');
          c= fgetc(fansi);
          if (c==EOF) break;
        }
      }
      _clearend();
    }

    // accumulate actual char to print
    if (n<0) {
      char ch= c;
      ln= dstrncat(ln, &c, 1);
    }
  }

  // mark click position
  if (matchLink) {
    save();
    gotorc(_click_r, _click_c);
    B(red); C(white);
    putchar('*');
    restore();
  }

  reset();
  fflush(stdout);

  return _clicked;
}

void displayScrollbar(int vis) {
  int v= MAX(0, (100-vis-10)*4/100)+1;
  gclear(); gbg=white; gfg=rgb(v,v,v);
  int pc= 100*screen_rows/nlines;
  int nr= MAX(1, pc*gsizey/100);
  int y0= gsizey*top/nlines+2;
  for(int y=0; y<nr; y++) {
    if (y0+y>=gsizey-2) break;
    gset(gsizex-1, y0+y, gfg);
    gset(gsizex-2, y0+y, gfg);
  }
  gupdate();
}

void displayPageNum() {
  // show page numbers
  gclear();
  char parts[15];
  // TODO: same calculation as display()
  // (but here no " L4711 ")
  snprintf(parts, sizeof(parts), " %d/%d ", (top+2)/(rows-4)+1, (nlines-rows+2)/(rows-4)+1);

  drawCenteredText(parts);
  gupdate();
  //TODO: too much obtrustive
  //keywait(300);
}

// Returns a malloced string (no-NULL)
char* getTitle(char *url, char *file) {
  if (!file || file==LOADING_FILE) return strdup("");
  char *r= NULL;
  FILE *f= fopenext(file, ".TITLE", "r");
  if (f) {
    r= fgetline(f);
    fclose(f);
  }
  // try use URL (hostname)
  if (!r && url) {
    // get hostname
    r= sskip(url, "http://");
    r= sskip(r, "file://");
    char *end= r= sskip(r, "https://");
    if (1) {
      // full path
      end+= strlen(end);
    } else {
      // just hostname
      while(*end && *end!='/') end++;
    }
    r= strndup(r, end-r);
  }
  return r ? r : strdup("");
}

void displayTabInfo(keycode k) {
 gclear();
 char buf[10]; sprintf(buf, " Tab%+d ", tab);

 // print Tab-3 center white o black
 // TODO: a bit too much on the nose?
 if (0) {
   gy= 10;
   gx= (gsizex-strlen(buf)*8)/2;
   gputs(buf);
   gnl(); gy+= 4;
   int sgy= gy;
   gupdate();
   if (url) {
     // print host center black on white
     gclear(); gbg= white; gfg= black;
     gy= sgy;
     // clear previous host
     for(int i=gsizex/8; i; i--) gputc(' ');
     // TODO: use fullwidth? or small font
     char *u= url, *end;
     u= sskip(u, "http://");
     u= end= sskip(u, "https://");
     while(*end && *end!='/') end++;
     gx= (gsizex-8*(end-u))/2; gx= MAX(0, gx);
     while(*u && *u!='/' && gx<gsizex) gputc(*u++);
     // print path
     //if (*u) gputc(*u++);
     //gnl();
     //gputs(u);
     //while(gy<gsizey) gputc(' ');
   }
 
   // line of <<<<< or >>>>>
   gy= (gsizey-8)/2; // middle line
   gx= (gsizex%8)/2; // center text
   for(int i=gsizex/8; i; i--)
     gputc(k==LEFT?'<':'>');
   gnl();

   gupdate();
 }

 fflush(stdout);
}

// --- Display

// Return true if tab changed (by click)
// TODO: elegance? hmmm
int displayWin(keycode k, char *url, char *file, int top, int rows, int clreos) {
  static int wpage= 0;
  // -- update header
  // (as it may have changed)
  reset();
  gotorc(0, 0);
  B(black); C(white);
  printf("./w %.*s  ", screen_cols-wpage-5, url);
  fflush(stdout);

  FILE *fansi= openOrWaitReloadAnsi();
  // display trunced by key-press
  if (!fansi) return 0; 
    
  // --- print header for real!
  // (we needed to wait for nlines)
  reset();
  gotorc(0, 0);
  if (url) {
    // TODO: app header reserved for tab-info?
    char *u= url, col;
    B(black); C(white);
    col= printf("./w ");
    // nprintf !!!
    char parts[15];
    // TODO: same calculation as in displaPageNum
    wpage= snprintf(parts, sizeof(parts), " L%d %d/%d ", top, (top+2)/(rows-4)+1, (nlines-rows+2)/(rows-4)+1);
    while (*u) {
      putchar(*u++);
      col++;
      if (col+1 >= screen_cols-wpage) break;
    }
    // space out
    while (col++ < screen_cols-wpage) putchar(' ');
    printf("%s", parts);
  }

  // -- print page
  const long tms= 280, sms= mstime();
  const int width= 4;
  if (k==LEFT || k==RIGHT) { // back
    int scols= screen_cols;

    // "scroll it sideways"
    for(int i=0; i<scols; i++) {
      int c= k==RIGHT? scols-width-i: i;
      // print slice of page
      _screen_left= k==RIGHT? c+width+1: 0;
      // TODO: i => garbage on last  2col
      // i+1 -> "scroll" up
      screen_cols= k==RIGHT? i-1: i-width;
      printAnsiLines(fansi, top, rows);
      fflush(stdout);

      // draw black bar 
      for(int r=0; r<rows; r++) {
        gotorc(1+r, c<0? 0: c);
        B(black); spaces(c<0? width+c : width);
        gotorc(r, c+width);
        B(white);
      }
      fflush(stdout);

      // eat up scrolls, bread on key
      keycode pk= peekey();
      if (0 && pk!=-1) {
        if (0)
        while(pk!=-1 && (pk & SCROLL) && (pk==_prevkey) && (mstime()-_prevkeyms<200)) {
          key();
          pk= peekey();
        }
        if (pk!=-1) break;
      }

      // catch up/cheat
      long actual= mstime()-sms;
      long expected= tms*i/scols;
      if (actual>expected) {
        // skip
        i= actual*scols/tms;
      } else {
        usleep((expected-actual)*1000);
      }
    }

    _screen_left= 0;
    screen_cols= scols;
  }

  // full redraw
  gotorc(1, 0);
  int redo= printAnsiLines(fansi, top, rows);
  fclose(fansi);

  //  reset();
  clearend(); // avoid ragged
  C(white); B(black); clearend();

  // click requesting redo page
  if (redo) return redo;

  // need to do before icon...
  if (clreos) cleareos();

  // favicon?
  if (top==0 && !_search) {
    save(); reset(); fflush(stdout);
    // no icon - use 2 first from nost
    if (!gicon(url)) {
      gclear(); gbg= white; gfg= black; gy= 3; gx= 1;
      char *u= url;
      u= sskip(u, "https://");
      u= sskip(u, "http://");
      char two[3]= {u[0], u[1], 0};
      gputs(two);
      gupdate();
    }
    restore(); fflush(stdout);
  }

  // read keyboard shortcuts, page links
  trunclinks(0);
  loadshortcuts(".wkeys");
  // TODO: very expensive to do every time... (try Tests/links-many.html) 
  // TODO: keep file open and delete/reload if changed?
  loadshortcuts(file);
}

// TODO: remove k?
int display(int k) {
  int r= displayWin(k, url, file, top, rows, 1);

  // -- footer
  reset();
  // TODO: set message and show for X s?
  if (0) message("%3d %3d/%d = %d %d", top, right, tab, ntab-1, 4711, 12);
  gotorc(screen_rows-1, 0);

  return r;
}

// --- ACTIONS

keycode copyurl() {
  if (!url) return REDRAW;
  printf(" copying URL"); fflush(stdout);
  dstr *cmd= dstrprintf(NULL, "printf \"%s\" | termux-clipboard-set", url);
  system(cmd->s);
  free(cmd);
  return REDRAW;
}

// update status
keycode deltab() {
  // TODO: mark as "deleted"
  tab--;
  return REDRAW;
}


keycode gohistory() {
  // open already existing?
  system("perl whi2html.pl > .whistory.html");
  tab= newtab(".whistory.html");
  return REDRAW;
}

void visited() {
}

// queue/stack readers
void push(int t) {
}

int pop() {
  return 0;
}

void delpush(int t) {
}

int delpop() {
  return 0;
}

void read_next() { // till end
}

void queue_read() {
}

// creates a new tab from URL.
//
// Note: URL can be terminated by whitespace. This allows this
// function to be called with a pointer of an url within another
// string.
//
// Return newly opened tab number
int newtab(char* url) {
  // TODO: force or not?
  // TODO: maybe click shouldn't open new tab, at least not if already loaded in >tab! (only log if history and not future?)
  download(url, 0, 1);

  // open new tab, go to
  return ntab++;
}
      
void opentab() {
  gotorc(0, 0);
  char* u= input("./w ");
  if (!u) return;
  push(tab);
  tab= newtab(u);
}

// Find match to key INPUT extract link and ./wdonwload it in, allocate new tab.
//
// Return 0 if no match
//        N the new tab number
//
// TODO: make it match a string!
int click(char *keys) {
  int len= strlen(keys);
  for(int i=0; i<nlinks; i++) {
    char *u = links[i];
    if (!strncmp(u, keys, len) && u[len]==' ' || u[len]=='\t') {
      u+= strlen(keys);
      // one key match found!
      //fprintf(stderr, "\n===========FOUND match key='%c' for:\n'%s'\n", k, links[i]);
      // TODO: make a safe system/popen
      // TODO: handle gracefully
      assert(!strchr(links[i], '\\'));
      assert(!strchr(links[i], '"'));
      assert(!strchr(links[i], '\''));
      assert(!strchr(links[i], '\`'));
      assert(!strchr(links[i], '\n'));

      // skip spaces
      while(*u && (*u==' ' || *u=='\t')) u++;

      // TODO: force or not?
      // TODO: maybe click shouldn't open new tab, at least not if already loaded in >tab! (only log if history and not future?)
      int n= newtab(u);

      clearcmd();
      return n;
    }
  }
  message("\[31mNo such link: %s", keys);
  return 0;
}

keycode command(keycode k, dstr *ds) {
  char *ln= ds->s;
  int len= strlen(ln);
  if (!*ln) return k;

  // SEARCHING in page
  if (k==RETURN || k==CTRL+'S' || k==CTRL+'O') {
    switch(ln[0]) {

    case '=':
      bookmark(ln[0], &ln[1]);
      if (ln[0]!='=') clearcmd();
      return NO_REDRAW;

    case '^': // TODO: ^top search
    case '_': // TODO: _end search
      break;

    case '/':  case '&':
      // TODO: O only?
      _only= (k==CTRL+'O');
      search(ln[0], &ln[1]);
      k=0;
      return k;

    default: // page search
      if (k==RETURN) break;
      _only= (k==CTRL+'O');
      search(k, ln);
      return k;
    }
  }
  
  // ACTIONS /  bookmarks (save store go)
  if (k==RETURN) {

    switch(ln[0]) {

    case '#': case '@': case '$':
      bookmark(ln[0], &ln[1]);
      clearcmd();
      return NO_REDRAW;

    case '/':  case '&':
      search(ln[0], &ln[1]);
      // keep line to allow for more search
      break;
      
    case '!':
      // TODO: can't input ls -l *.html LOL
      // TODO: replace %u w URL
      reset();
      clear();
      B(white); C(black);
      printf("./w %s\n", ln);
      B(black); C(white);
      printf("--- CTRL-L to redraw scree");
      C(green);
      cursoron(); fflush(stdout);
      _jio_exit();
      system("clear");
      system(&ln[1]);
      cursoroff();
      jio();
      printf("\n\n--- CTRL-L to redraw screen\n\n");
      return NO_REDRAW;

    case '|': // TODO: pipe HTML/text
    case '^': // TODO: move to top?
    case '_': // TODO: bury? _ underline something?
      break;
    }

    // If have SPC then NOT link/url
    if (!strchr(ln, ' ')) {

      // CLICK link
      // all a-z, maybe link click?
      if (strspn(ln, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")==len) {
        int rtab= click(ln);
        // short-cut matched
        if (rtab) {
          tab= rtab;
          clearcmd();
          return REDRAW;
        }

        // not a link name - do search!
      }

      // Calculate expression
      if (strspn(ln, "+-0123456789.eE+*/%^=-lastijng<>!()[] ")==len 
          && !strchr(ln, '"') && !strchr(ln, '\'')) {
        dstr *cmd= dstrprintf(NULL, "printf \"`echo \\\"%s\\\" | bc -l`\"", ln);
        // delete "microphone prompt"
        display(k);

        // calculate and print inline
        gotorc(screen_rows-1, 0);
        reset();
        B(black); C(white);
        printf("%s", ln);
        C(yellow);
        printf("  ==> ");
        clearend();
        C(green); fflush(stdout);
        system(cmd->s);
        free(cmd);
        clearend();
        C(white); fflush(stdout);

        return NO_REDRAW;
      }

      // OPEN url
      // (url? or file? have /:.? )
      if (strchr(ln+1, '/') || strchr(ln+1, ':') || strchr(ln, '.')) {
        tab= newtab(ln);
        clearcmd();
        return REDRAW;
      }

      // Fallthrough (even if no space)
    }

    // SEARCH the web (duckduckgo)
    // TODO: change query to use dstr!
    dstr *q= dstrncaturi(NULL, ln, -1);
    char query[strlen(INTERNET_SEARCH)+strlen(q->s)+1];
    sprintf(query, INTERNET_SEARCH, q->s);
    tab= newtab(query);
    free(q);
    return REDRAW;
  }

  return k;
}



////////////////////////////////////////
// CLICK ACTIONS

char *speech() {
  // show microphone
  gotorc((screen_rows-screen_cols/2)/2, 0);
  fflush(stdout);
  system("Graphics/imcat/imcat Graphics/microphone-wired.png");

  char *r= NULL;
  // only works on Android
  // ... -p  # gives continous output
  // TODO: but it seem buffered?
  // https://github.com/termux/termux-api/blob/master/app/src/main/java/com/termux/api/SpeechToTextAPI.java
  FILE *f= popen("termux-speech-to-text -p", "r");
  char buf[1024]= {0};
  if (f) {
    // read all lines, show, but pick last
    while (fgets(buf, sizeof(buf), f)) {
      //printf("%s", buf); clearend();
    }

    r= strdup(buf);
    if (r && r[strlen(r)-1]=='\n')
      r[strlen(r)-1]= 0;
    fclose(f);
  }

  // remove pending strokes (duplicate click?)
  while(haskey()) key();

  return r;
}

// Definition of clickable regions
char LEVELS[]= "N n   CC   s S";
char DIST[]= "W CCC E";

void showClickableRegions(int l, int d) {
  for(int rr=0; rr<screen_rows; rr++){
    for(int cc=0; cc<screen_cols; cc++){
      int ll= (sizeof(LEVELS)-1)*rr/screen_rows;
      int dd= (sizeof(DIST)-1)*cc/screen_cols;
      if (LEVELS[ll]==' ' || DIST[dd]==' ') continue;
      //if (ll!=-1 && ll!=l && dd!=d) continue;
      gotorc(rr, cc);
      //B((ll==l && dd==d)? black : rgb(3,3,3));
      B(rgb(3,3,3)); // gray
      spc();
    }
  }
}

void showClick(keycode k, int r, int c) {
  // show a click block
  for(int rr=-3; rr<=1; rr++) {
    for(int cc=-5; cc<=3; cc++) {
      // make ir round (need round rr?)
      //if (rr*rr+cc*cc>4*4) continue;
      gotorc(r+rr, c+cc);
      //B((k && MOUSE_DOWN)? black : yellow);
      B(black);
      putchar(' ');
    }
  }
}

int clickDispatch(int k) {
  // Don't redraw as we want hilite links
  if (k & MOUSE_DOWN) return NO_REDRAW;

  // TODO: make function/macro/API?
  int b= (k>>16) & 0xff, r= (k>>8) & 0xff, c= k & 0xff;
  int save_k= k;
  k= REDRAW;

  // Adjusted Row and Column (calibrate?)
  int ar= r-1, ac= c-1;

  _click_r= ar; _click_c= ac;
  _search= strdup("LINKS"); // secret signal!
  //return REDRAW;

  if (ar<0) ar= 0; if (ac<0) ac= 0;
  if (ar>=screen_rows) ar= screen_rows-1;
  if (ac>=screen_cols) ac= screen_cols-1;

  // continue if not clickable region
  int l= (sizeof(LEVELS)-1)*ar/screen_rows;
  int d= (sizeof(DIST)-1)*ac/screen_cols;
  if (LEVELS[l]==' ' || DIST[d]==' ')
    return k;

  char dir[3]={LEVELS[l], DIST[d],0};
  printf("%s",dir);

  // --- Click Buttons

  // MENU      SPEACH      CLOSE
  if (!strcmp(dir, "NW")) {
    // (bookmkar) info for page
    return CTRL+'Q';
  }
  if (!strcmp(dir, "NC")) {
    char *ln= line->s;
    char *r= speech();
    gtoast(r);
    keywait(1000);
    // TODO: "generalify" more (void) commands
    // use prefix? "duc duc dcu"
    if (!strcmp("clear", r)) {
      *ln= 0;
      free(r);
      return REDRAW;
    }
    if (!r) return REDRAW;

    int empty = !*ln;
    // append to current edits
    if (ln[0] && ln[strlen(ln)-1]!=' ')
      ln= dstrncat(ln, " ", -1);
    ln= dstrncat(ln, r, -1);
    free(r);
    return empty? RETURN : REDRAW;
  }
  if (!strcmp(dir, "NE")) drawX(),k= LEFT;

  // xxxx      xxxx        xxxx
  if (!strcmp(dir, "nW")) ;
  if (!strcmp(dir, "nC")) ;
  if (!strcmp(dir, "nE")) ;

  // HistoryBACK  ???   HistoryFORW
  if (!strcmp(dir, "CW")) k= LEFT;
  //   (CC: main content area)
  if (!strcmp(dir, "CE")) k= RIGHT;

  // xxxx      xxxx        PAGE UP
  if (!strcmp(dir, "sW")) k= CTRL+'H';
  if (!strcmp(dir, "sC")) ;
  if (!strcmp(dir, "sE")) k= META+' ';
  // xxxx      xxxx        PAGE DOWN
  if (!strcmp(dir, "SW")) clearcmd();
  if (!strcmp(dir, "SC")) ;
  if (!strcmp(dir, "SE")) k= ' ';

  if (k==NO_REDRAW)
    showClickableRegions(l, d);

  showClick(k, r, c);

  // show exactly where clicked
  B(white); C(black); gotorc(ar, ac-2);
  printf(" %c%c ", LEVELS[l], DIST[d]);
  restore();

// TODO: move to scroller?
  // simplify for up/down counters
  if (save_k & SCROLL) k = save_k & SCROLL;

  return k;
}




////////////////////////////////////////
// DRAG SCROLLERS ACTIONS

#define FAILIF(exp, msg...) if(exp){message(msg); return NO_REDRAW;}


void scrollBookmarks() {
  // first show current page bookmark info
  // second scroll bookmarks file backwards (reverse order)
  // can scroll a page info in 3 rows movement! (number to indicate?)
  // IDEA: have a diagonal background, this would make scrolling clear!
}

void scrollHistory() {
  // Bottom Left
  // scrollback of list in time, show when last visited?
  // date headers like ./whi?
}

void scrollStack() {
}

void scrollReadings() {
}

void scrollTabs() {
  // this is more like normal viewing
  // show an alternative history!
  // ALT-LEFT / ALT-RIGTH ? LOL
  // this is pages in the order you saw them, so like the browser BACK/FORWARD

  // Now, WHAT happens when you create a branch by going back, clicking on a link. There is no forward from that branch?
}

keycode showScroll(keycode k, int r, int c) {
  int lxy, kxy=k & 0x0000ffff;
  int n= 0, up= 0, dn= 0;

  // loop until different event
  do {
    // count scrolls
    if (k & SCROLL_UP) up++;
    if (k & SCROLL_DOWN) dn++;
    n++;

    int d=dn-up;

    // draw line follow scroll
    gotorc(r-up+dn, c);
    B(yellow); spc();

    // print info
    B(black); C(white);
    gotorc(0, 0);
    printf("---SCROLL n=%d dn=%d up=%d     ", n, dn, up);
    fflush(stdout);
            
    // location is stored in 2 lowest
    lxy= kxy;
    k= key();
    gotorc(r-up+dn, c);
    spc();
    kxy = k & 0x0000ffff;
  } while(lxy==kxy);

  // not ours!
  return k;
}

keycode flickMenu(keycode k) {
  // MENU
  color COLORS[]={
    yellow, green, cyan, blue, magenta, red, black};
  char* TEXT[]={
    "CLOSE", "STAR", "HASHTAB", "UNDO",  "LIST", "HISTORY", "QUIT"};
  assert(ALEN(COLORS)==ALEN(TEXT));
  drawPullDownMenu(COLORS, TEXT, ALEN(COLORS), ALEN(TEXT), 1);

  return waitScrollEnd(k);
}

keycode flickHamburger(keycode k) {
  // MENU
  color COLORS[]={black, white};
  char* TEXT[]={
    "Home", "About", "Search", "Content", "Nothing", "Settings", "Contacts"};
  drawPullDownMenu(COLORS, TEXT, ALEN(COLORS), ALEN(TEXT), 0);

  return waitScrollEnd(k);
}

void loadPageMetaData();

// future is +1, past is -1
keycode panHistory(keycode k, int future) {
  cursoroff();
  // TODO: merge with gohistory()
  //system("perl whi2html.pl > .whistory.html ; ./w.x .whistory.html > .whistory.html.ANSI");
  //FILE *fansi= fopen(file, "r");
  //FILE *f= fopen(".whistory.html.ANSI", "r");
  //FAILIF(!f || !fansi, "Can't view history");
  int top0= top, n= future;
  while (n && (k= key()) & SCROLL) {
    if (k==CTRL+'C') exit(0);

    // scroll 
    int kr= (k>>8) & 0xff, kc= k & 0xff;
    if (((kr<screen_rows/2)?+1:-1)==future) {
      tab+= k & SCROLL_UP? -1 : +1;
      tab= MAX(-start_tab, MIN(ntab-1, tab));
    } else {
      n+= k & SCROLL_UP? +1 : -1;
    }

    int tab1= tab;
    if (future==+1) {
      //n= MIN(ntab+1, MAX(0, n));
      n= MIN(rows/2, MAX(0, n));
      gotorc(1, 0);
      //printAnsiLines(fansi, top0, rows-n);
      loadPageMetaData();
      displayWin(k, url, file, top, rows-n+1, 0);
      cursoroff();
      gotorc(rows-n+1, 0);
      spaces((screen_cols-11)/2);
      B(black); C(orange);
      printf("F U T U R E"); clearend();

      // -- The Future
      //printAnsiLines(f, 0, n);
      for(int i=0; i<n-1; i++) {
        putchar('\n');
        if (i==tab) {
          B(purple); C(brightorange);
        } else {
          B(black); C(orange);
        }

        tab= i;
        loadPageMetaData();

        char *title= getTitle(url, file);
        char status= 'U';

        printf("%c%3d  %.*s", status, i, screen_cols-7, title); clearend();
        free(title);
      }
      B(black); C(white);
      fflush(stdout);

    } else { // -1 The Past
      //n= MIN(0, MAX(-start_tab, n));
      n= MIN(0, MAX(-rows/2, n));

      gotorc(0, 0);
      for(int i=0; i<-n; i++) {
        int t= i+n+1;
        if (t==tab) {
          B(purple); C(brightorange);
        } else {
          B(black); C(orange);
        }

        tab= t; loadPageMetaData();
        char *title= getTitle(url, file);
        char status= 'R';

        printf("%c%3d  %.*s", status, t, screen_cols-7, title); clearend(); putchar('\n');
        free(title);

      }
      B(black); C(orange);
      spaces((screen_cols-7)/2);
      printf("P A S T"); clearend();
      putchar('\n');
      
      gotorc(n, 0);

      tab= tab1;
      loadPageMetaData();
      FILE *fansi= fopen(file, "r");
      if (fansi) {
        printAnsiLines(fansi, top0, rows+n);
        fclose(fansi);
      }
      // TODO: starts from rc=0,0
      //displayWin(k, url, file, top, rows+n+1, 0);
      cursoroff();
    }
    tab = tab1;

    cleareos();
    fflush(stdout);
  }
  //fclose(f);
  //fclose(fansi);
  cursoron();
  while(haskey() && ((k= key()) & SCROLL));
  //return k & SCROLL? REDRAW : k;
  return REDRAW;
}

keycode newPanHistory(keycode k, int future) {
  cursoroff();
  // TODO: merge with gohistory()
  //system("perl whi2html.pl > .whistory.html ; ./w.x .whistory.html > .whistory.html.ANSI");
  //FILE *fansi= fopen(file, "r");
  //FILE *f= fopen(".whistory.html.ANSI", "r");
  //FAILIF(!f || !fansi, "Can't view history");
  int top0= top, n= future;
  while ((k= key()) & SCROLL) {
    if (k==CTRL+'C') exit(0);

    // scroll 
    int kr= (k>>8) & 0xff, kc= k & 0xff;
    //if (((kr<screen_rows/2)?+1:-1)==future) {
    //tab+= k & SCROLL_UP? -1 : +1;
    //tab= MAX(-start_tab, MIN(ntab-1, tab));
    //} else {
    const step=5;
    n+= k & SCROLL_UP? +step : -step;
    //}

    int tab1= tab;
    if (n>=0) {
      //n= MIN(ntab+1, MAX(0, n));
      n= MIN(rows/2, MAX(0, n));
      gotorc(1, 0);
      //printAnsiLines(fansi, top0, rows-n);
      loadPageMetaData();
      displayWin(k, url, file, top, rows-n, 0);
      cursoroff();
      gotorc(rows-n+1, 0);

      // -- The Future
      //printAnsiLines(f, 0, n);
      for(int i=0; i<n; i++) {
        putchar('\n');
        B(red); C(white);
        tab= i;
        loadPageMetaData();
        char *title= getTitle(url, file);
        char status= 'U';

        printf("%c%3d  %.*s", status, i, screen_cols-7, title); clearend();
        free(title);
      }
      B(black); C(white);
      fflush(stdout);

    } else { // -1 The Past
      //n= MIN(0, MAX(-start_tab, n));
      if (n < -rows*2/3) tab1--;
      n= MIN(0, MAX(-rows*2/3, n));

      gotorc(0, 0);
      if (0) {
        int lns= 7;

        int i= 0;
        while(i<lns) {
          tab= tab1 + n/step - 1 + i;
          loadPageMetaData();
          char *title= getTitle(url, file);
          char status= 'R';
          B(black); C(orange);
          printf("%c%3d  %.*s", status, tab, screen_cols-7, title); clearend(); putchar('\n');
          free(title);

          i++;
        }
        while(i<-n) {
          tab= tab1 + (i+n)/step - 1;
          loadPageMetaData();
          char *title= getTitle(url, file);
          char status= 'R';
          B(black); C(orange);
          printf("%c%3d  %.*s", status, tab, screen_cols-7, title); clearend(); putchar('\n');
          free(title);

          FILE *fansi= fopen(file, "r");
          if (fansi) {
            printAnsiLines(fansi, top0, lns);
            fclose(fansi);
          }
          i+= lns;

          i++;
        }

      } else
      for(int i=0; i<-n; i++) {
        tab= tab1 + n/step + 1 + i;
        loadPageMetaData();
        char *title= getTitle(url, file);
        char status= 'R';
        B(black); C(orange);
        printf("%c%3d  %.*s", status, tab, screen_cols-7, title); clearend(); putchar('\n');
        free(title);

      }
      gotorc(n, 0);
      clearend(); printf("\n");

      tab= tab1;
      loadPageMetaData();
      FILE *fansi= fopen(file, "r");
      if (fansi) {
        printAnsiLines(fansi, top0, rows+n+1);
        fclose(fansi);
      }
      // TODO: starts from rc=0,0
      //displayWin(k, url, file, top, rows+n+1, 0);
      cursoroff();
    }
    tab = tab1;

    cleareos();
    fflush(stdout);
  }
  //fclose(f);
  //fclose(fansi);
  cursoron();
  while(haskey() && ((k= key()) & SCROLL));
  //return k & SCROLL? REDRAW : k;
  return REDRAW;
}

// A "touch" is a scroll event.
// It has a start position anda up=down (row) trail.
// There is no end-event.
keycode touchDispatch(keycode k) {
  // loop till no more scroll,
  // or exit middle if content scroll
  while (k & SCROLL) {
    int kb= (k>>16) & 0xff, kr= (k>>8) & 0xff, kc= k & 0xff;

    // Adjusted Row and Column
    int ar= kr-1, ac= kc-1;

    // % start areas of interest
    int pr= kr*100/screen_rows;
    int pc= kc*100/screen_cols;

    //     |  top   |         /m
    // ----+--------+--------/ i
    // left|        | right  - d
    // ----+--------+--------\ d
    //     | bottom |         \l
    //     /|||||||||\         e
    //    /c e n t e r\


    // HORIZONTAL:
    //                  1
    // -1-2-3-4-5-6-7-9-0
    // LLLLL        RRRRR
    // lllllllll rrrrrrrr
    // WWwwcccccccccceeEE
    //         CCC

    // VERTICAL:
    //
    //-10 u N U
    //-20 u n U
    //-30 u m U
    //-40 u m
    //-45 u m M
    //-50   m M
    //-55 d m M
    //-60 d m
    //-70 d m
    //-80 d m D
    //-90 d s D
    //-00 d S D


    int cd= ABS(pc-50);
    int md= ABS(pr-50);

    int u= pr<45, d= pr>55;
    int U= pr<25, D= pr>75;
    int l= pc<45, r= pc>55;
    int L= pc<25, R= pc>75;

    int N= pr<10, n= pr<20 && !N;
    int S= pr>=90, s= pr>=80 && !S;
    int W= pc<10, w= pc<20 && !W;
    int E= pc>=90, e= pr>=80 && !E;

    int C= cd<=10, c= cd<=80 && !C;
    int M= md<=10, m= md<=80 && !M;


    // -- menu flick down
    if (N && E) return flickMenu(k);

    // -- hamburger menu 
    // TODO: show user "menu"
    if (U && L) return flickHamburger(k);

    // -- scrolling/bar
    if (1 && E) {
      int dd= (nlines-screen_rows)*4/screen_rows/3;
      dd+= 1;
      top+= k & SCROLL_UP? -dd : +dd;
      return REDRAW;
    }

    // -- fast back/forward scroll
    if (M) {
      while(haskey()) key();
      if (mstime()-_prevkeyms<200)
        return NO_REDRAW; // ignore

      return !!L==!!(k & SCROLL_DOWN)? LEFT: RIGHT;
    }

    // -- flick back/forward
    if (0 && M && R) {
      static long t0= 0;
      keycode k0= k;
      long t= mstime();
      if (t-t0<300) return NO_REDRAW;
      t0= t;
      while(haskey()) key();

      return k0 & SCROLL_DOWN  ? RIGHT : LEFT;
    }

    // -- flick page up/down
    if (d && R) {
      static long t0= 0;
      keycode k0= k;
      long t= mstime();
      if (t-t0<300) return NO_REDRAW;
      t0= t;

      while(haskey()) key();
      return k0 & SCROLL_DOWN ? META+' ' : ' ';
    }

    // -- pull down past/future scroller
    // TODO: neat idea but not work well: touchy-2-3-step process :-(
    //if (C & N) return panHistory(k, -1);
  //if (C & S) return panHistory(k, +1);
    
    if (0 && M) {
      if (k & SCROLL_UP)
        return newPanHistory(k, -1);
      else 
        return newPanHistory(k, +1);
    }
    
    // TODO: remove
    // OLD ONES
    int top= pr<10, bottom= pr>90;
    int left= pc<10, right= pc>=90;
    int Left= pc<20, Right= pc>=80;

    int center= !left && !right;
    int middle= !top && !bottom;


    // scrolling content region
    if (center && middle) return k;

    return k;

    // TODO: remove
    // -- Drag starting locations:

    //else if (top && center) k= scrollStack();
    if (top && center) {
      // empty scroll events...
      //while(haskey()) key();

      // TODO: hmmm, conflicts with scroll...
      //return CTRL+'R';
    }

    //else if (top && left) k= scrollBookmarks();

    else if (M & W) {
      return (k & SCROLL_UP) ? LEFT : RIGHT;
    } else if (u && E) {
      return (k & SCROLL_UP) ? META+'V' : CTRL+'V';
    }
    //else if (bottom && left) k= scrollHistory();
    // else if (bottom && center) k= scrollReadings();
    //else if (bottom && right) k= scrollTabs();
    else {
      // No specific drag-down defined
      k= showScroll(k, ar, ac);
    }
  }

  return NO_REDRAW;
}

////////////////////////////////////////
//

void loadPageMetaData() {
  // TODO: No need to read if tab didn't change
  int t= start_tab+tab;
  file= url= FREE(hit);

  // handle other manipulator of .whistory - now just get confused and out of sync when opening, need to continously "tail -f" the file!

  // TODO: keep all "new tabs" in memory
  hit= fgetlinenum(fhistory, t);
  if (hit) {
    const char *W= "#=W ";
    // extract file
    file= strstr(hit, W);
    if (file) {
      int c;
      file+= strlen(W);
      // skip spaces
      while(*file==' ') file++;

      url= file;
      // skip URL
      while(*file && *file!=' ') file++;
      if (*file) *file++= 0;

      // skip spaces
      while(*file==' ') file++;
    }
    error(!file, 10, "history log entry bad: '%s'\n", hit);
  } else {
    // TODO: add a way to reload or signal when done!
    //printf("\n----NOT LOADED! (press key)\n"); key();

    file= LOADING_FILE;

    //error(!hit, 10, "history log entry not found: %d", t);
  }
}

// CTRL
// ==== 
// ^A    bookmarks
// ^B    back, chrome:bookmarks
// ^C    EXIT
// ^D    ? del-tab, chrome: bookmark
// ^E    emacs, edit HTML and ANSI
// ^F    forward tab (history), chrome: search bar
// ^G    cancel (clear-line/draw), chrome: next match ^S-G previous
// ^H    history, chrome: history
// ^I    TAB (completion-list of URLs)
// ^J    RETURN (can't change?), chrome: download-manager
// ^K    KILL tab
// ^L    redraw screen,  chrome: location - go edit URL
// ^M    RETURN
// ^N    scroll-down, chrome: new window
// ^O    ? , chrome: open file on computer
// ^P    scroll-up, chrome: print file
// ^Q    quotable info about page
// ^R    reload
// ^S    search (search-next?)
// ^T    ? tabs list, chrome: new tab
// ^U    (urls) list-links/shortcuts, chrome: display HTML (use ^E)
// ^V    page-down, M-V page-up
// ^W    close-tab, chrome: close tab
// ^X    ? reserve for extra
// ^Y    ? yank (undo tab kill)
// ^Z    ZUSPEND/ZLEEP

// CTRL-X
// ======
// ^X ^A    ? emacs: none
// ^X ^B    list history (buffers/tabs)
// ^X ^C    exit
// ^X ^D    ? emacs: list-directory
// ^X ^E    emacs edit HTML
// ^X ^F    ? open-file, emacs: find-file
// ^X ^G    cancel, emacs: none
// ^X ^H    ? emacs: none
// ^X ^I    ? emacs: indent-regidly
// ^X ^J    ? emacs: none
// ^X ^K    ? emacs: prefix
// ^X ^L    ? emacs: downcase-region
// ^X ^M    RET: long commands?
// ^X ^N    ! ? emacs: set-goal-column
// ^X ^O    ! ? emacs: delete-blank-lines
// ^X ^P    ? emacs: mark-page
// ^X ^Q    ? emacs: read-only-mode
// ^X ^R    ? emacs: find-file-read-only
// ^X ^S    ? emacs:save
// ^X ^T    ? emacs: transpose-lines
// ^X ^U    copyUrl
// ^X ^V    ? emacs: find-alternative-file
// ^X ^W    ? emacs: write-file
// ^X ^X    ?eXchange mark and point
// ^X ^Y    ? emacs: none
// ^X ^Z    zuspend

void listCXActions() {
  wclear();
  // TODO: open help screen for CTRL-X?
}

// call emacs/editor on page files
void editor(char *file) {
  char *editor = getenv("EDITOR");
  dstr *run= dstrprintf(NULL, "tidy -i -q --force-output yes  %s > .tidy.html 2> .tidy.errors ; %s .tidy.html .tidy.errors %s ", file, editor?editor:"emacs", file);
  // fix endings, lol TODO: fix..
  memcpy(strstr(run->s, ".ANSI"), ".DOWN", 5);
  memcpy(strstr(run->s, ".ANSI"), ".DOWN", 5);
  // add ANSI file
  run= dstrprintf(run, " %s", file);

  reset(); B(black); C(white); clear(); fflush(stdout); B(black); C(white);
  _jio_exit();
  system(run->s);
  free(run);
  jio();
}

keycode ctrlXAction(keycode xk) {
  keycode k= REDRAW; // default

  fprintf(stdout, "%s", keystring(xk));
  switch(xk){

  case CTRL+'G': break; // cancel

  case CTRL+'?': listCXActions(); return NO_REDRAW;

  case CTRL+'R': // real-reload
    // TODO: delete cached files!
    gtoast("Reloading");
    reload(url);
    return REDRAW;

  case CTRL+'O': { // open in chrome
    printf("Opening in external browser"); fflush(stdout);
    int http= strstr(url, "http");
    dstr *run= dstrprintf(NULL, "termux-open-url \"%s%s\"", http?"":"http://", url);
    system(run->s);
    free(run);
    return REDRAW;
  }

  case 'k': return deltab();
  case CTRL+'B': return gohistory();

  case CTRL+'S': { // save
    FILE *h= fopenext(file, ".DOWN", "r");
    FAILIF(!h, "Can't open");
    FAILIF(!line->s[0], "No filename");
    FILE *s= fopen(line->s, "w");
    FAILIF(!s, "Can't create");
    int c, n= 0;
    while((c= fgetc(h))!=EOF) {
      fputc(c, s); n++;
    }
    fclose(s);
    fclose(h);
    message("Saved to file: %s [%d bytes]\n", line->s, n);
    clearcmd();
    return NO_REDRAW;
  }

  case CTRL+'U': return copyurl();
  case 'u': { // edit url
    clearcmd();
    line= dstrncat(line, url, -1);
    return NO_REDRAW;
  }

  // TODO: generalize all to systemf()
  // Edit HTM (and ANSI)L with emacs
  case CTRL+'E': editor(file);
    
    // Map to themselves:
  case CTRL+'Z': case CTRL+'C':
    return xk;

    // TODO:------------:TODO
  case CTRL+'X': // exchange point&mark
  case CTRL+'T': // tail -f file
  case CTRL+'D': // directory

  default:
    FAILIF(1, "C-x %s not recognized", keystring(xk));
  }

  return k;
}

keycode keyAction(keycode k) {
  // already processed
  if (k==NO_REDRAW) return k;

  int kc= k & ~META & ~ CTRL;
  
  if (k==CTRL+'L') clear();
    
  // Do this first so it can map to other key actions!
  if (k==CTRL+'X') {
    reset();
    gotorc(screen_rows-1, 0); clearend();
    printf("C-x "); cursoron(); fflush(stdout);

    k= ctrlXAction(key());
    cursoroff();
  }

  // -- bookmarks
  // w3m: Esc-b	View bookmarks
  // w3m: Esc-a	To bookmark
  // elinks: v load bookmark, ESc b
  // elinks: a, ESC a add current bookmark
  // ^S_B: show/hide bookmarks (chrome)
  // ^S_A: open bookmarks manager (chrome)
  // ^D - save current page as bookmark
  // ^S-D- save all open tabs as "folder" (chrome)
  if (k==CTRL+'D' || (k<127 && strchr("=*#$", k))) {
    bookmark(k, line->s);
    k= NO_REDRAW;
  }
  if (k==CTRL+'A') k= listbookmarks(NULL, NULL);
  if (k==CTRL+'Q') k= listbookmarks(url, NULL);

  if (k==CTRL+'U') k= listshortcuts();

  // -- page action

  // TODO: how to only do RE-RENDER (w.x)?
  // (twice in a row?, lol)
  if (k==CTRL+'R') {
    gtoast("Reloading");
    reload(url);
    k= REDRAW;
  }
  // chrome: CTRL-P: print current webbpage ? save?
  // chrome: CTRL-S: save current webpage
  // chrome: ESC: stop loading webpage
  // chrome: ^U - display HTML
  // chrome: ^O - open file on computer
  // chrome: ALT-link (mouse) download link

  // chrome:F7 : turn on caret browsing

  // -- search
  // chrome: ^F, F3: search bar
  // chrome: ^G: next match
  // chrome: ^S-G: prev match

  // -- small navigation
  COUNT(top, DOWN, UP, nlines);
  COUNT(top, CTRL+'N', CTRL+'P', nlines);
  COUNT(top, RETURN,META+RETURN, nlines);
  COUNT(top, SCROLL_UP, SCROLL_DOWN, nlines);

  // -- big navigation
  if (k==META+'<' || k==META+',' || k==META+UP) top= 0; // top
  if (k==META+'>' || k==META+'.' || k==META+DOWN) top= nlines;
  if (k==META+'V' || k==META+' ' || k==CTRL+UP) top-= rows-3;
  if (k==CTRL+'V' || k==' ' || k==CTRL+DOWN) top+= rows-3;

  if (top>nlines-rows) top= nlines-rows;
  if (top<0) top= 0;

  // -- TABS
  // list history
  if (k==CTRL+'H') return gohistory();

  // help
  if (k=='?' || k==FUNC+1) {
    push(tab);
    // open already existing?
    tab= newtab("wless.html");
  }

  // TODO: cleanup (a-z A-Z used in editing), now abc RET is how to select link
  // clicking (new tab) a-z0-9
  //  a  open link in new tab and go
  //  A  open in background tab
  //  M-A open shortcut page 
  //
  // TODO: could use M-a too?
  if (isalnum(kc) && (k<127 || kc<='Z') && !(k&TERM)) {
    char keys[2]={kc,0};
    if (k==toupper(kc)) { // A-Z
      // open behind
      click(keys);
    } else { // a-z, M-A -- M-Z
      // open now
      push(tab);
      tab= click(keys);
    }
  }

  // chrome: ^1-8 goto tab #
  // chrome: ^9 go to righ most tab

  // pop from deleted (chrome: ^S-T
  if (k==CTRL+'Y') {
    push(tab);
    //tab= delpop();
  }

  if (k==CTRL+'W') { // close tab (^F4)
    delpush(tab);
    deltab();
    //tab= pop();
  }
  if (k==CTRL+'K') { // kill forward
    delpush(tab);
    deltab();
    //tab++;
  }
  if (k==CTRL+'D') { // delete tab back
    delpush(tab);
    deltab();
    //tab++;
  }

  // TODO: open empty has no meaning...
  //if (k==CTRL+'T') {
  //push(tab);
  //tab= ntab++;
  //}

  //COUNT(top, CTRL+'F', CTRL+'B', nlines);
  // chrome: ^TAB; goto next open tab
  // chrome: ^S-TAB: goto prev open tab
  // chrome: M-LEFT: back browsing history
  // chrome: M-RIGHT: forward browsing histor
  // chrome: ^H: history page in new tab
  
  if (k==LEFT || k==CTRL+'B') tab--;
  if (k==RIGHT || k==CTRL+'F') tab++;

  if (start_tab+tab<=1) tab= -start_tab+1;
  if (tab>=ntab) tab= ntab-1;

  //COUNT_WRAP(right, RIGHT, LEFT, nright);

  //TODO: field? nfield
  // remove "right"
  //COUNT_WRAP(field, TAB, S_TAB, nfield);

  // --- Chrome keybaord short-cut
  // - https://support.google.com/chrome/answer/157179?hl=en&co=GENIE.Platform%3DDesktop#zippy=%2Ctab-and-window-shortcuts

  // TODO: shortcuts "missing", to consider
  // chrome: ^J: downloads manager
  // chrome: S-ESC: task manager
  // chrome: ^S-J: open dev tools
  // chrome: ^S-DEL: open delete history
  // chrome: ^S-M: login as different user
  // chrome: M-S-i: feedback form
  return k;
}

keycode editTillEvent() {
  int k;

  // loops capture menu drag events
  do {

    // Update TABS info
    char buf[32];
    int w= snprintf(buf, sizeof(buf), "  T%d/%d", tab, ntab);
    gotorc(screen_rows-1, 0);
    spaces(screen_cols-w-1);
    printf("%s", buf);

    // EDIT
    gotorc(screen_rows-1, 0);
    B(black); C(white);
    cursoron();
    k= edit(&line, -1, NULL, NULL, " *");
    cursoroff();

    // terminal may have been resized!
    rows = screen_rows-1;

    char *ln= line->s;

    // Safe-way out!
    if (!strcmp(ln, "QUIT")) exit(0);

    // --- Editor extension!

    // clear line
    if (k==CTRL+'G' || !*ln) {
      _only= 0;
      *ln= 0;
      FREE(_search);
    }

    // space!
    // ( if no test - PAGE DOWN )
    // ( if standing at space - PAGE DOWN )
    //
    // this is almost DWIM!
    if (k==' ' && *ln && ln[strlen(ln)-1]!=' ') {
      line= dstrncat(line, " ", 1);
      k= NO_REDRAW;
      continue;
    }
    if (k=='*' && *ln && isdigit(ln[strlen(ln)-1])) {
      line= dstrncat(line, "*", 1);
      k= NO_REDRAW;
      continue;
    }

    // --- Capture some events here
    // (those that don't want redraw)
    // TODO: may no longer be needed!)

    // TOUCH DRAG
    if (k & SCROLL)
      k= touchDispatch(k);

    // up/down main body
    if (k>0 && (k & SCROLL)) {
      // Simplify to SCROLL_UP/DOWN
      k &= SCROLL;
      break;
    }

    // (MOUSE) CLICK
    if ((k & MOUSE) && !(k & SCROLL))
      k= clickDispatch(k);

    // url completion
    if (k==TAB) {
      printf("\n====================\n");
      if (1) {
        dstr *cmd= dstrprintf(NULL, "cut -d\\  -f3 .whistory | sort | uniq -c | sort -n | GREP_COLORS='mt=01;32' grep --color=always -iP \"%s\" ", line->s);
        system(cmd->s);
        printf("\n\n(ctrl-L to redraw)\n");
        free(cmd);
      }

      if (1) {
        printf("\n====================\n");
        dstr *cmd= dstrprintf(NULL, "cut -d\\  -f3 .whistory  | sort | uniq -c | sort -n | GREP_COLORS='mt=01;32' grep --color=always -P \" %s\" ", line->s);
        system(cmd->s);
        printf("\n\n(ctrl-L to redraw)\n");
        free(cmd);
        k= NO_REDRAW;
      }
    }
  } while (k==NO_REDRAW);

  return k;
}

// --- MAIN LOOP

int main(void) {
  jio();
  rows = screen_rows-1;

  cursoroff();
  system("echo '`date --iso=ns` #=WLESS`' >> .wlog");
  wclear();

  // --- Open files for persistent state
  // (append+ will create if need)
  fhistory= fopen(".whistory", "a+");
  fbookmarks= fopen(".wbookmarks", "a+");
  start_tab= flines(fhistory);
  
  int k= 0, q=0, lasttab=-1, lasttop=-1;
  // string for editing/command
  line= dstrncat(NULL, NULL, 1);

  // main loop
  while(1) {
    loadPageMetaData();

    // avoid update if events waiting
    if (!haskey() && k!=NO_REDRAW) {
      if (display(k)) {
        //keywait(40);
        // vis confirm of click link
        keywait(100);
        k= REDRAW; // quiet
        continue;
      }
      visited();

      displayScrollbar(100);

      // --- display various meta info
      if (tab!=lasttab && k!=REDRAW) {
        displayTabInfo(k);
        keywait(300);
        lasttab= tab;
        k= REDRAW;
        continue;
      }

      if (ABS(top-lasttop)>rows-5) {
        // display pagenum over new page, wait and do redraw after 300ms
        displayPageNum();
        // if key in queue don't wait 
        // = instant action!
        keywait(300);
        // make sure not loop
        lasttop= top;
        k= REDRAW;
        continue;
      }

      // fade out scrollbar
      int pc= 100;
      while(keywait(100)>100 && pc>=0) {
        displayScrollbar(pc-=10);
      }
      if (!haskey()) display(k);
    }

    // print "key" acted on
    //gotorc(screen_rows-1, 0);
    //printf("---%8x---%s\n", k, keystring(k));

    // - read special event & decode
    k= editTillEvent(line);

    k= command(k, line);
    k= keyAction(k);

    // -- system
    if (k==CTRL+'C') break;
    if (k==CTRL+'Z') {
      reset(); 
      _jio_exit();
      gotorc(9999,9999);
      printf("\nSuspending...\n");
      printf("\n(Type another ^Z ; fg to come back)\n");
      // TODO: why kill doesn't work?
      //kill(getpid(), SIGSTOP);
      kill(getpid(), SIGTSTP);
      printf("\nWaking up...\n");
      jio();
      
    }
  }
  printf("\r\n");

  gotorc(9999,9999);
  printf("\nExiting...\n");
  // implicity calls _jio_exit();
  return 0;
}
