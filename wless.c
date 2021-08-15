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

// - limits
int nlines= 12000, nright= 10, ntab= 0;

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

// --- Display

void display(char* file, int line, int k) {
  // home
  // (no cleear screen - no flicker)
  cursoroff();
  reset();
  gotorc(0, 0);
  printf("\t%3d %3d %s", start_tab+tab, line, file?file:"(null)");
  gotorc(1, 0);
  fflush(stdout);

  // build command
  const char command[]= "./wdisplay %s %d %d";
  char buf[sizeof(command)+1 + 1024]= {0};
  int len= snprintf(buf, sizeof(buf), command, file?file:".stdout", top+2, rows);
  assert(len+5 < sizeof(buf));

  fgcolor(0); bgcolor(7);

  system(buf);
  
//  reset();
  clearend(); C(7); B(0); clearend();
  cleareos();

  gotorc(rows, 0);
//  cleareos();

//  reset();

//cursoron();return;

  gotorc(screen_rows-1, 0);

  // pretty print state & key
  // TODO: remove
  if (1) { // debug
    printf("%3d %3d %3d ", top, right, tab);
    //clearend();
    print_key(k);
  }
      
  cursoron();
}

void help() {
  system("./w wless.html");
}

// --- ACTIONS

void click(int k) {
}

int newtab() {
  return ++tab;
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

void bookmark() {
}

void reload() {
}

// --- MAIN LOOP

int main(void) {
  system("echo '`date --iso=ns` #=WLESS`' >> .wlog");
  screen_init();
  rows = screen_rows-2;

  char* history= fopen(".whistory", "r");
  start_tab= flines(history);

  char* file= NULL;

  int k= 0, q=0, last_tab;
  while(1) {

    // load right page data
    int t= start_tab+tab;
    char* hit= fgetlinenum(history, t);

    if (hit) {
      if (tab!=last_tab) free(file), file= NULL;
      const char* W= "#=W ";
      // extract file
      file= strstr(hit, W);
      if (file) {
        int c;
        file+= strlen(W);
        // skip spaces
        while(*file==' ') file++;
        // skip URL (TODO: extract)
        while(*file!=' ' && *file!=EOF) file++;
        // skip spaces
        while(*file==' ') file++;
      }
      error(!file, 10, "history log entry bad: '%s'\n", hit);
    }
    error(!hit, 10, "history log entry not found: %d", t);

    display(file, top, k);
    visited();

    // - read key & decode
    k= key();

    // action
    //if (c==CTRL+'H') help();
    if (k==CTRL+'C' ||k=='q') break;
    if (k==CTRL+'Z') kill(getpid(), SIGSTOP);
    // navigation
    if (k=='<') top= 0; // top
    if (k=='>') top= nlines-1; // bottom
    if (k==META+'V' || k==BACKSPACE || k==DEL) if ((top-= rows) < 0) top= 0;
    if (k==CTRL+'V' || k==' ') if ((top+= rows) > nlines) top= nlines-1;

    if (k>='a' && k<='z') {
      push(tab);
      tab= newtab();
      click(k);
    }

    // tab mgt
    if (k>='A' && k<='Z') {
      push(tab);
      int t= newtab();
      click(k+32);
      tab= pop();
    }
    if (k>=META+'A' && k<=META+'Z') {
      push(tab);
      tab= newtab();
      click(k-META);
      tab= pop();
    }

    if (k==CTRL+'D') bookmark();
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
      tab= delpop();
    }

    if (k==CTRL+'W') { // close tab (^F4)
      delpush(tab);
      deltab();
      tab= pop();
    }
    if (k==CTRL+'K') { // kill forward
      delpush(tab);
      deltab();
      tab++;
    }
    if (k==CTRL+'D') { // delete tab back
      delpush(tab);
      deltab();
      tab++;
    }

    if (k==CTRL+'T') {
      push(tab);
      tab= newtab();
    }

    COUNT(top, DOWN, UP, nlines);
    COUNT(top, CTRL+'N', CTRL+'P', nlines);
    COUNT(top, CTRL+'F', CTRL+'B', nlines);

    if (k==LEFT) tab--;
    if (k==RIGHT) tab++;
    //COUNT_WRAP(right, RIGHT, LEFT, nright);

    //TODO: field?
    //COUNT_WRAP(tab, TAB, S_TAB, ntab);

    // quit?
    q= k<33 ? 0 : q*2 + k;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");

  gotorc(9999,9999);
  printf("\nExiting...\n");
  return 0;
}
