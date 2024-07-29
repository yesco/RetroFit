#include "jio.c"
#include "graphics.c"
#include <math.h>
#include <assert.h>

// -- keys position mapping from MOUSE click
// Limited. Stores only char of key and
// (x,y) of starting position. Returns index
// into keyc[], keyy[],keyx[].
//
// Assumption: keys are added in x-> and then y-> order.
//
// Matching starts from right bottom, first key (kx,ky)
// that is less than click position (x,y) is returned.

#define NKEYS 64

char keyc[NKEYS]= {0};
int keyy[NKEYS]= {0};
int keyx[NKEYS]= {0};
int nkeys= 0; // ahum, not 0?

// Search key positions to find where clicked
// Returns: 0 if not found, otherwise index
int findnkey(int x, int y) {
  int i;
  for(i=nkeys; i; i--) { 
    //printf("  %d: '%c' x%d y%d for %d %d\n", i, keyc[i], keyx[i], keyy[i], x, y);
    if (keyx[i] > x) continue;
    if (keyy[i] > y) continue;
    //printf("---passed: %d\n", i);
    return i;
  }
  return 0;
}

void dkey(char k) {
  // remember coordinates
  nkeys++;
  assert(nkeys<NKEYS);
  keyc[nkeys]= k;
  keyx[nkeys]= gx;
  keyy[nkeys]= gy;

  // draw
  gputc(' ');

  // save pos of char for inverse
  int x= gx, y= gy;
  gputc(k);

  gputc(' ');

  ginvert(x-2, y-1, 8+3, 8+1);
}
  
void row(char* keys) {
  int c;
  while((c= *keys++)) {
    if (c==' ') continue;
    dkey(c);
  }
  gputc('\n'); gy+= 3;
}

void calc() {
  int firsttime= 1;
  gcanvas();

  // clear actual screen
  wclear();
  
  double stack[16]= {0}, *s= stack+1;
  int k;
  do {
    gx= 0; gy= 0;
    int wc= 6*3;
    int w= wc*8;

    // draw result panel
    char r[64]= {0};
    sprintf(r, "%*.*g", wc, wc-5, *s);
    
    // right adjust
    gx= w-8*strlen(r);
    gputs(r);
    gputc('\n'); gy+= 4;

    if (firsttime) {
      // draw keys
      row("( ) Q L X S");
      row("7 8 9 * D C");
      row("4 5 6 / B T");
      row("1 2 3 - I ");
      row(". 0 E + < >");
      firsttime= 0;
    }

    gupdate();

    // action by key
    k= key();
    if ((k & MOUSE)==MOUSE_DOWN) {
      int r= mouserow(k), c= mousecol(k);
      int i= findnkey(c*pixels_per_col, r*pixels_per_row);
      if (i) {
	k= keyc[i];
	// show key pressed down for a while
	ginvert(keyx[i]+6, keyy[i]-1, 8+5, 8+2);
	gupdate();
	usleep(100*1000);
	key(); // read MOUSE_UP
	ginvert(keyx[i]+6, keyy[i]-1, 8+5, 8+2);
	gupdate();
      }
    }

    // take two -- see below
    if (s-stack<2 && strchr("+-*/^", k)) continue;

    // action
    switch(toupper(k)) {
    case '0' ... '9': *s= 10* *s + k-'0'; break;
    case '.': break; // TODO: enter string?
    case 'E': break; // TODO: 1E5
    case 8: case 127: break; // TODO: backspace

    case ' ': case '\n': case '\r': // or <enter> or space
    case '<': s++; *s= 0; break; // push/enter
    case '>': if (s>stack) s--; break; // pop
      // case ' TODO: dup??? 

      // take two - see above
    case '+': s--; *s= *s+s[1]; break;
    case '-': s--; *s= *s-s[1]; break;
    case '*': s--; *s= *s*s[1]; break;
    case '/': s--; *s= *s/s[1]; break;
    case '^': s--; *s= pow(*s, s[1]); break;

    case 'Q': *s= sqrt(*s); break;
    case 'L': *s= log(*s); break;
    case 'X': *s= exp(*s); break;
    case 'D': *s= log10(*s); break;
    case 'B': *s= log2(*s); break;

    case 'I': *s= 1/ *s; break;

    case 'S': *s= sin(*s); break;
    case 'C': *s= cos(*s); break;
    case 'T': *s= tan(*s); break;
    }

    //printf(" %s %d %x | ", keystring(k), k, k);
  } while(k!=CTRL+'C');
}

int main(void) {
  jio();
  cursoroff();
  
  calc();

  cursoron();
  exit(0);
}
