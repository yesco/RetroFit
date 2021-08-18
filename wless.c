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

#define LOADING_FILE ".w/Cache/loading.html.ansi"


// - limits
int nlines= 12000, nright= 10;

int ntab= 1; // number of newly openeed tabs + last

int rows;

// - state
int top, right, tab, start_tab;

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

// TODO: cleanup
// TODO: dynamic array? or read from file and match!
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

// Load bookmarks/keyboards from FILE
// Returns number of items added
int loadbookmarks(char *file) {
  FILE *flinks;

  // .ansi file - extract links
  if (endswith(file, ".ansi")) {
    //const char CMD[]= "./wlinks %s";
    //char cmd[sizeof(CMD)+1 + 1024]= {0};
    //int lcmd= snprintf(cmd, sizeof(cmd), CMD, file);
    //assert(lcmd+5 < sizeof(cmd));
    //system(cmd);
    // TODO: use popen instead, but it doesn't work with fgetline
    //FILE* flinks= popen(cmd, "r");

    // TODO: murky...
    // ./wdisplay already wrote it...
    file= ".wlinks";
  }

  flinks= fopen(file, "r");
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
void displaybookmarks() {
  clear();
  printf("\n=== BOOKMARKS ===\n\n");
  for(int i=0; i<nlinks; i++) 
    printf("%s\n", links[i]);
  printf("\n===Press key to continue\n");
  key();
}

// DONT free (partof hit)
char *file= NULL;
char *url= NULL;

// --- Display

void display(int line, int k) {
  // header
  reset();
  gotorc(0, 0);
  if (url) {
    clearend();
    // TODO: app header reserved for tab-info?
    char *u= url, col;
    col= printf("./w ");
    gotorc(0, col);
    // nprintf !!!
    while (*u) {
      putchar(*u++);
      col++;
      if (col+1 >= screen_cols) break;
    }
    clearend();
  }

  // main content
  gotorc(1, 0);
  fflush(stdout);

  // build ./wdisplay command
  const char command[]= "./wdisplay %s %d %d";
  char buf[sizeof(command)+1 + 1024]= {0};
  // top+2 because skips the header line
  int lbuf= snprintf(buf, sizeof(buf), command, file?file:".stdout", top+2, rows);
  assert(lbuf+5 < sizeof(buf));

  fgcolor(0); bgcolor(7);

  system(buf);

  trunclinks(0);
  loadbookmarks(".wkeys");
  loadbookmarks(".wlinks");
  //loadbookmarks(file?file:".wlinks");

//  reset();
  clearend(); C(white); B(black); clearend();
  cleareos();

  gotorc(rows, 0);
//  cleareos();

  reset();

//cursoron();return;

  // last line
  gotorc(screen_rows-1, 0);

  // pretty print state & key
  // TODO: remove
  if (1) { // debug
    printf("%3d %3d %3d/%d ", top, right, tab, ntab-1);
    //clearend();
    puts(key_string(k));
    clearend();
  }
}

void help() {
  // TODO: fix
  system("./w wless.html");
}

// --- ACTIONS

// Find match to key INPUT extract link and ./wdonwload it in, allocate new tab.
//
// Return 0 if no match
//        N the new tab number
//
// TODO: make it match a string!
int click(int k) {
  for(int i=0; i<nlinks; i++) {
    char *u = links[i];
    if (*u++==k) {
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
      
      // start download in background
      char *end= strchr(u, ' ');
      int llen= end? end-u : strlen(u);
      char cmd[30+llen];
      cmd[0]= 0;
      strcat(cmd, "./wdownload \"");
      strncat(cmd, u, llen);
      sprintf(cmd+strlen(cmd), "\" %d %d &", screen_rows, screen_cols);
      system(cmd);

      //sleep(3); // testing
      // TODO: write log entry here!
      // wait enough for .whistory to be updated...
      usleep(100*1000);

      // open new tab, go to
      return ++ntab;
    }
  }
  return 0;
}

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

void reload() {
}


void logbookmark(int k, char *s) {
  //log(bms, k, url, cpos, top, s);
  printf("\n=== ./wbookmark %c %s %d %d %s\n", k, url, -1, top, s?s:"");
  key();
}

void bookmark(int k) {
  int cpos= -1; // TODO

  if (k=='*' || k==CTRL+'D') {
    logbookmark('*', "");
    return;
  }

  char prompt[2]= {k, 0};
  //char *s= readline((char*)prompt);
  char buf[256]= {0};
  char *s= fgets(buf, sizeof(buf), stdin);
  if (!s) return;

  // search
  if (s=='=') {
    // TODO: seach/grep
    return;
  }

  // add bookmark data
  logbookmark(k, s);
  free(s);
}

// --- MAIN LOOP

int main(void) {
  cursoroff();
  system("echo '`date --iso=ns` #=WLESS`' >> .wlog");
  screen_init();
  rows = screen_rows-2;

  FILE *history= fopen(".whistory", "a+");
  assert(history);
  start_tab= flines(history);

  char *hit= NULL; // FREE!
  int k= 0, q=0, last_tab;
  while(1) {

    // load right page data
    int t= start_tab+tab;
    if (hit) {
      free(hit);
      file= url= hit= NULL;
    }
    hit= fgetlinenum(history, t);

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

    display(top, k);
    visited();

    // - read key & decode
    k= key();
    int kc= k & 0x7f; // only char

    // action
    if (k=='?' || k==CTRL+'H') help();

    if (k==CTRL+'D' || strchr("=*#$", k)) bookmark(k);
    if (k==CTRL+'X') displaybookmarks();

    // navigation
    if (k==CTRL+'C') break;
    if (k==CTRL+'Z') kill(getpid(), SIGSTOP);
    // navigation
    if (k=='<' || kc==',') top= 0; // top
    if (k=='>' || kc=='.') top= nlines-1; // bottom
    if (k==META+'V' || k==BACKSPACE || k==DEL) if ((top-= rows) < 0) top= 0;
    if (k==CTRL+'V' || k==' ') if ((top+= rows) > nlines) top= nlines-1;

    // clicking (new tab) a-z0-9
    //  a  open link in new tab and go
    //  A  open in background tab
    //  M-A open shortcut page 
    if (isalnum(kc) && (k<127 || kc<='Z')) {
      if (k==toupper(kc)) { // A-Z
        // open behind
        click(k);
      } else { // a-z, M-A -- M-Z
        // open now
        push(tab);
        tab= click(kc);
      }
    }

    if (k==CTRL+'R') reload();

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
    // ^S-B: show/hide bookmarks
    // ^S-A: open bookmarks manager

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
    COUNT(top, CTRL+'F', CTRL+'B', nlines);

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
  return 0;
}
