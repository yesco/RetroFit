#include "jio.c"
#include "graphics.c"
#include <math.h>
#include <assert.h>

#define NKEYS 64

char keyc[NKEYS]= {0};
int keyy[NKEYS]= {0};
int keyx[NKEYS]= {0};
int nkeys= 1; // ahum, not 0?

// Search key positions to find where clicked
// Returns: 0 if not found, otherwise index
int findnkey(int x, int y) {
  int i;
  for(i=0; i<NKEYS; i++) { 
    if (x < keyx[i]) break;
    if (y < keyy[i]) break;
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

  int x= gx, y= gy;
  gputc(k);

  gputc(' ');

  // "invert"
  for(int dy=-1; dy<8; dy++) {
    for(int dx=-2; dx<8+2; dx++) { // TODO: wrong for 1 or more pad?
      pixel* p= gpixel(x+dx, y+dy);
      // inverse color?
      if (p) {
	switch(*p) {
	  //case 1 ... 7: *p= 7-*p; break;
	case black: *p= white; break;
	case white: *p= black; break;
	case none: *p= white; break; // TODO: depends background?
	  //case 9 ... 255: *p= 255-*p+9; break; // TODO: Hmmmm?
	default: *p= red; printf("ERROR: color=%d\n", *p);
	}
      }
    }
  }

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
    sprintf(r, "%32g", *s);
    
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
    int k= key();

    // take two -- see below
    if (s-stack<2 && strchr("+-*/^", k)) continue;

    // action
    switch(toupper(k)) {
    case '0' ... '9': *s= 10* *s + k-'0'; break;
    case '.': break; // TODO: enter string?
    case 'E': break; // TODO: 1E5

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

  } while(k!=CTRL+'C');
}

int main(void) {
  jio();
  cursoroff();
  
  calc();

  cursoron();
  exit(0);
}
