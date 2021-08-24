// TODO: keep rereading file to displaY?
// tail -f source:
// - http://git.savannah.gnu.org/cgit/coreutils.git/tree/src/tail.c

// partly from old imacs.c
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

#include "jio.h"

#define LOADING_FILE ".w/Cache/loading.html.ANSI"

// --- limits
int nlines= 0, nright= 10;

int ntab= 1; // number of newly openeed tabs + last

int rows;

// --- state
FILE *fhistory, *fbookmarks;

int top, right, tab, start_tab;

// DONT free (partof hit)
char *file= NULL;
char *url= NULL;

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
#define LINKS_MAX 25*25*25
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
    else
      error(LINKS_MAX>=nlinks, 77, "Getmore links! %d");
  }
  fclose(flinks);
}

// TODO: search prefixkey, or string
void listshortcuts() {
  clear();
  printf("\n=== BOOKMARKS ===\n\n");
  for(int i=0; i<nlinks; i++) 
    printf("%s\n", links[i]);
  printf("\n===Press key to continue\n");
  key();
}

// TODO: research universal XML bookmarks format called XBEL, also supported by e.g. Galeon, various "always-have-my-bookmarks" websites and number of universal bookmark converters.

void logbookmark(int k, char *s) {
  //log(bms, url, offset, top, s);
  gotorc(screen_rows-2, 0);
  cleareos();
  dstr *ds= dstrprintf(NULL, "%s %s %d %d %c%s\n",
    isotime(), url,  -1, top, k, s);
  // print to screen w/o newline
  fputs(ds->s, fbookmarks);
  free(ds);
}

void listbookmarks(char *url, char *s) {
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
          C(blue);
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

  printf("\n%d Matching Lines", mcount);
  printf("\n(press key to continue)");
  fflush(stdout);
  key();
}

void bookmark(int k) {
  int cpos= -1; // TODO

  if (k=='*' || k==CTRL+'D') {
    // TODO: to save the seekpos too!
    logbookmark('*', "");
    return;
  }

  gotorc(screen_rows-2, 0);
  char prompt[2]= {k, 0};
  char* s= input(prompt);
  if (!s) return;

  // also logs searches!
  logbookmark(k, s);

  // search
  if (k=='=') {
    // TODO: make a loop around it allowing "incremental" search
    listbookmarks(NULL, s);
  }

  free(s);
}

// start download in background
void download(char* url, int force) {
  if (!url || !*url) return;
  char *end= strpbrk(url, " \t\n");
  int ulen= end? end-url : strlen(url);

  // TODO: srip the script?
  dstr *cmd= dstrprintf(NULL, "./wdownload %s \"%.*s\" %d %d &",
    force?"-d":"", ulen, url, screen_rows, screen_cols);
  system(cmd->s);
  free(cmd);

  // wait enough for .whistory to be updated...  and .TMP to be created
  usleep(1000*1000);
}

void reload(char* url) {
  download(url, 1);
}


// --- Display
void message(char* format, ...) {
  va_list argp;
  va_start(argp, format);

  // last line
  reset();
  gotorc(screen_rows-1, 0);
  vprintf(format, argp);
  clearend(); fflush(stdout);
}

void display(int k) {

  // -- header
  reset();
  gotorc(0, 0);
  clearend();
  B(black); C(white);
  printf("./w %.*s", screen_cols-5, url);
  gotorc(0, 0);
  fflush(stdout);

  // wait for open of ANSI file
  FILE *fansi= fopen(file?file:".stdout", "r");
  FILE *ftmp= fopenext(file, ".TMP", "r");
  
  // file missing in cache
  // TODO: don't do every time...
  if (!fansi && !ftmp) {
    // clear content with white (so no flicker)
    gotorc(1, 0);
    for(int i=rows; i; i--) {
      B(white); C(blue);
      for(int j=(rows-i)*(screen_cols-20-1)/rows; j; j--)
        putchar(' ');
      printf("\e[1m... reloading ...\n"); clearend();
    }
    gotorc(1, 0);
    download(url, 0);
    
    fansi= fopen(file?file:".stdout", "r");
    ftmp= fopenext(file, ".TMP", "r");
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
    
    ftmp= fopenext(file, ".TMP", "r");
  }
  nlines= fansi? flines(fansi) : -1;
  if (ftmp) fclose(ftmp);

  fansi= fopen(file?file:".stdout", "r");

  // --- print header for real!
  reset();
  gotorc(0, 0);
  if (url) {
    clearend();
    // TODO: app header reserved for tab-info?
    char *u= url, col;
    col= printf("./w ");
    gotorc(0, col);
    // nprintf !!!
    char parts[10];
    int w= snprintf(parts, sizeof(parts), " %d/%d", (top+2)/rows+1, (nlines+0)/rows+1);
    while (*u) {
      // TODO: unicode?
      putchar(*u++);
      col++;
      if (col+1 >= screen_cols-w) break;
    }
    clearend();
    // space out
    while (col++ < screen_cols-w) putchar(' ');
    printf("%s", parts);
  }

  // -- main content
  B(white); C(black);
  gotorc(1, 0);
  fflush(stdout);

  if (fansi) {
    int c, n=top;
    fseek(fansi, 0, SEEK_SET);
    while((c= fgetc(fansi)) != EOF) {
      // TODO: cleanup when make the hidden lines simplier...
      if (c=='\n') {
        c= fgetc(fansi);
        if (c!='\n' && c!='#') {
          if (n<0) putchar('\n');
          n--;
        } else {
          while(c=='\n' || c=='#') {
            // skip comment line
            if (c=='#')
              while((c= fgetc(fansi)) != EOF && c!='\n');
            c= fgetc(fansi);
            if (c==EOF) break;
          }
        }
      }
      // TODO: hidden escape codes?
      if (n<0) putchar(c);
      if (n<=-rows) break;
    }

    fclose(fansi);
  }

  // read keyboard shortcuts, page links
  trunclinks(0);
  loadshortcuts(".wkeys");
  loadshortcuts(file);

  //loadbookmarks(file?file:".wlinks");

//  reset();
  clearend(); C(white); B(black); clearend();
  cleareos();

  // -- footer
  reset();

  // TODO: set message and show for X s?
  if (0) message("%3d %3d/%d = %d %d", top, right, tab, ntab-1, 4711, 12);
  
  gotorc(screen_rows-2, 0);
}

// --- ACTIONS

// update status
void deltab() {
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
  reload(url);

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
      return newtab(u);
    }
  }
  message("\[31mNo such link: %s", keys);
  return 0;
}

int command(keycode k, dstr *ds) {
  char *line= ds->s;
  int len= strlen(line);
  if (!*line) return k;

  if (k==RETURN) {
    // all a-z, maybe link click?
    if (strspn(line, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")==len) {
      tab= click(line);
      line[0]= 0;
      return CTRL+'L';
    }
    // url? or file? have /:.?
    if (strchr(line+1, '/') || strchr(line+1, ':') || strchr(line, '.')) {
      tab= newtab(line);
      line[0]= 0;
      return CTRL+'L';
    }
  }

  return k;
}

// --- MAIN LOOP

int main(void) {
  // TODO: mouse init/jio
  fprintf(stderr, "\e[?1000;1003;1006;1015h");
  cursoroff();
  system("echo '`date --iso=ns` #=WLESS`' >> .wlog");
  screen_init();
  rows = screen_rows-2;

  // --- Open files for persistent state
  // (append+ will create if need)
  fhistory= fopen(".whistory", "a+");
  fbookmarks= fopen(".wbookmarks", "a+");
  start_tab= flines(fhistory);
  
  char *hit= NULL; // FREE!
  int k= 0, q=0, last_tab;
  // string for editing/command
  dstr *line= dstrncat(NULL, NULL, 1);
  while(1) {

    // load right page data
    int t= start_tab+tab;
    if (hit) {
      free(hit);
      file= url= hit= NULL;
    }
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
      file= LOADING_FILE;

      //error(!hit, 10, "history log entry not found: %d", t);
    }

    display(k);
    visited();

    // - read key & decode
    //testkeys();
do {
    gotorc(screen_rows-2, 0); clearend();

    cursoron();
    // TODO: only " ', add commands?
    k= edit(&line, -1, NULL, " *#@=");
    cursoroff();

    if (k & MOUSE) {
      // TODO: make function/macro/API?
      int b= (k>>16) & 0xff, r= (k>>8) & 0xff, c= k & 0xff;
      k= -1;

      // Adjusted Row and Column
      int ar= r-1, ac= c-1;
      char LEVELS[]= "Nn   CC   sS";
      char DIST[]= "W CCC E";

      if(0)
      for(int rr=0; rr<screen_rows; rr++){
        for(int cc=0; cc<screen_cols; cc++){
          int l= (sizeof(LEVELS)-1)*rr/screen_rows;
          int d= (sizeof(DIST)-1)*cc/screen_cols;
          if (LEVELS[l]==' ' || DIST[d]==' ')
            continue;
          gotorc(rr, cc);
          B(yellow);spc();
        }
      }

      int l= (sizeof(LEVELS)-1)*ar/screen_rows;
      int d= (sizeof(DIST)-1)*ac/screen_cols;
      if (l<0) l=0; if (d<0) d=0;
      if (LEVELS[l]==' ' || DIST[d]==' ')
        continue;
      for(int rr=0; rr<screen_rows; rr++){
        for(int cc=0; cc<screen_cols; cc++){
          int ll= (sizeof(LEVELS)-1)*rr/screen_rows;
          int dd= (sizeof(DIST)-1)*cc/screen_cols;
          if (ll!=l || dd!=d) continue;
          gotorc(rr, cc);
          B(red);spc();
        }
      }

      gotorc(ar, ac-1);
      B(red);C(white);

      char dir[3]={LEVELS[l], DIST[d],0};
      printf("%s",dir);

      // click buttons
      if (!strcmp(dir, "sE")) k= META+' ';
      if (!strcmp(dir, "SE")) k= ' ';

      if (!strcmp(dir, "CW")) k= LEFT;
      if (!strcmp(dir, "CE")) k= RIGHT;

      if(0)
      for(int rr=-3; rr<=1; rr++) {
        for(int cc=-5; cc<=3; cc++) {
          gotorc(r+rr, c+cc);
          if (k && MOUSE_DOWN)
            B(red);
          else 
            B(green);
          putchar(' ');
        }
      }

      B(white); C(black); gotorc(ar, ac-2);
      //printf(" %c%c ", LEVELS[l], DIST[d]);

      B(white);
    }
} while (k<0);

    int kc= k & 0x7f; // only char

    k= command(k, line);

    // action
    if (k==CTRL+'U') opentab();
    if (k==CTRL+'L') clear();

    //if (k==CTRL+'S' || k=='/' || k=='%') searchPage();

    //if (k==CTRL+'H') showHistory();
    //if (k==CTRL+'J') showDownloads();


    // --- Do stuff with page
    // CTRL-P: print current webbpage ? save?
    // CTRL-S: save current webpage
    // ESC: stop loading webpage
    // ^O - open file on computer
    // ^U - display HTML
    // ^D - save current page as bookmark
    // ^S-D- save all open tabs as "folder"
    
    // --- page functions
    // print
    // email
    // save
    // ! run command
    // | pipe to program
    // open file (^O)
    // edit file (^E) in emacs!
    // html (full, formatted, outline, loading log, headers))



    // w3m: Esc-b	View bookmarks
    // w3m: Esc-a	To bookmark
    // elinks: v load bookmark, ESc b
    // elinks: a, ESC a add current bookmark
    // ^S_B: show/hide bookmarks (chrome)
    // ^S_A: open bookmarks manager (chrome)
    // ^D - save current page as bookmark
    // ^S-D- save all open tabs as "folder" (chrome)
    if (k==CTRL+'D' || strchr("=*#$", k)) bookmark(k);
    if (k==CTRL+'A') listbookmarks(NULL, NULL);
    if (k==CTRL+'Q') listbookmarks(url, NULL);

    if (k==CTRL+'X') listshortcuts();

    // navigation
    if (k==CTRL+'C') break;
    if (k==CTRL+'Z') kill(getpid(), SIGSTOP);
    // navigation
    if (k=='<' || kc==',') top= 0; // top
    if (k=='>' || kc=='.') top= nlines-rows; // bottom
    if (k==META+'V' || k==META+' ' || k==BACKSPACE || k==DEL) if ((top-= rows) < 0) top= 0;
    if (k==CTRL+'V' || k==' ') if ((top+= rows) > nlines) top= nlines-1;

    // -- TABS

    if (k=='?' || k==CTRL+'H' || k==FUNC+1) {
      push(tab);
      // open already existing?
      tab= newtab("wless.html");
    }

    // clicking (new tab) a-z0-9
    //  a  open link in new tab and go
    //  A  open in background tab
    //  M-A open shortcut page 
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

    if (k==CTRL+'R') reload(url);

    // Up/Down LOL?
    if (k==CTRL+'U'); // Unread

    // Chrome keybaord short-cut
    // - https://support.google.com/chrome/answer/157179?hl=en&co=GENIE.Platform%3DDesktop#zippy=%2Ctab-and-window-shortcuts

    // ^1-8 goto tab #
    // ^9 go to righ most tab

    // ^TAB; goto next open tab
    // ^S-TAB: goto prev open tab

    // M-LEFT: back browsing history
    // M-RIGHT: forward browsing histor

    // M-F, M-E: open menu
    // ^S_B: show/hide bookmarks
    // ^S_A: open bookmarks manager

    // ^H: history page in new tab
    // ^J: downloads manager
 
    // S-ESC: task manager
    // ^F, F3: search bar
    // ^G: next match
    // ^S-G: prev match
    // ^S-J: open dev tools
    
    // ^S-DEL: open delete history
    // F1: help in new tab
    // ^S-M: login as different user
    // M-S-i: feedback form
    // F7 : turn on caret browsing

    // CTRL-P: print current webbpage ? save?
    // CTRL-S: save current webpage
    // ESC: stop loading webpage
    // ^O - open file on computer
    // ^U - display HTML

    // ^D - save current page as bookmark
    // ^S-D- save all open tabs as "folder"

    // ALT-link (mouse) download link

    // M-F, M-E: open menu

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

    COUNT(top, DOWN, UP, nlines);
    COUNT(top, CTRL+'N', CTRL+'P', nlines);
    COUNT(top, RETURN,META+RETURN, nlines);
    COUNT(top, SCROLL_UP, SCROLL_DOWN, nlines);

    //COUNT(top, CTRL+'F', CTRL+'B', nlines);
    if (k==LEFT) tab--;
    if (k==RIGHT) tab++;
    if (start_tab+tab<=1) tab= -start_tab+1;
    if (tab>=ntab) tab= ntab-1;
    //COUNT_WRAP(right, RIGHT, LEFT, nright);

    //TODO: field? nfield
    // remove "right"
    //COUNT_WRAP(field, TAB, S_TAB, nfield);

    // quit?
    q= k<33 ? 0 : q*2 + k;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");

  gotorc(9999,9999);
  printf("\nExiting...\n");
  cursoron();
  // TODO: deinit mouse/jio
  fprintf(stderr, "\e[?1000;1003;1006;1015l");

  return 0;
}
