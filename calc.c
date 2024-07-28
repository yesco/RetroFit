#include "jio.c"
#include "graphics.c"
#include <math.h>

void dkey(char k) {
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
  gcanvas();

  // clear actual screen
  wclear();
  
  double stack[16]= {0}, *s= stack;
  int k;
  do {
    double v= *s, b= v, a= s[-1];
    
    switch(k) {
    case '0' ... '9': v= v*10+v; break;
    case '.': break; // TODO: enter string?
    case 'E': break; // TODO: 1E5

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

    gclear();

    int w= 5*8*3;
    // draw result panel
    char* r= "4711";
    gx= w-8*strlen(r);
    gputs(r);
    gputc('\n'); gy+= 4;

    // draw keys
    row("( ) Q L X S");
    row("7 8 9 * D C");
    row("4 5 6 / B T");
    row("1 2 3 - I ");
    row(". 0 < + =");

    gupdate();
  } while((k= key())!=CTRL+'C');
}

int main(void) {
  jio();
  cursoroff();
  
  calc();

  cursoron();
  exit(0);
}
