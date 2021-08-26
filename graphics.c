// Simple ANIM graphics.


// Usage: #include "anim.c" LOL

// TOOD: make independent?
// User extensible scripts!

#ifdef MAIN
  #include "jio.c"
#endif

void drawX() {
  for(int rr=1; rr<=screen_rows-2; rr++) {
    //B(white);
    //gotorc(rr, 0); clearend();
    int col= screen_cols*rr/screen_rows;
    gotorc(rr, col-1); B(black); spaces(3);
    gotorc(rr, screen_cols-col-1); B(black); spaces(3);
  }
  usleep(10*1000);
}

void drawPullDownMenu(color *colors, char **labels, int n) {
  int cc= 40*screen_cols/100;
  int rows= 0;
  while (rows < screen_rows) {
    rows++;
    // exponential decrease of slide
    usleep(70*SQR(rows)/SQR(screen_rows)/2*1000);

    for(int rr=0; rr<rows; rr++) {
      gotorc(rr, screen_cols-cc);
      color col= colors[n*rr/rows];
      B(col);
      spaces(cc);
      char* str= labels[n*rr/rows];

      // center label
      int w= (cc-strlen(str))/2;
      gotorc(rr, screen_cols-cc + w);
      readablefg();
      printf("%s", str);
    }
    while(haskey()) key();
  }
}

#include <stdio.h>
#include <stdlib.h>

#include "Graphics/font8x8/font8x8_basic.h"

void plot(int r, int c, char ch, int w, int h, int bg, int fg) {
  //printf("plot %d %d %c\n", r, c, ch); return;
  if (ch>127) return;
  char *def = font8x8_basic[ch];

  B(bg);
  for (int y=0; y < 8; y++) {
    //if (y==3 || y==5 || y==7) {r--;continue;}
    gotorc(r+y, c);
    char bits=def[y];
    for (int x=0; x < 8; x++) {
//      if (x==1 || x==4 || x==2 || x==7) continue;
      if (bits & 1 << x)
        B(fg);
      else
        B(bg);
      putchar(' ');
    }
  }
}

void plots(int r, int c, char *s, int w, int h, int bg, int fg) {
  int ch;
  while((ch= *s++)) {
    if (ch=='\n' || c+8>=screen_cols) {
      r+= 8;
      c = 0;
    }
    if (ch=='\n') continue;
    plot(r, c, ch, w, h, bg, fg);
    if ((c+= 8)>=screen_cols) {
      r+= 8;
      c = 0;
    }
  }
}

#ifdef MAIN
int main(void) {
  screen_init();

  switch(1) {

  case 0: plots(0, 0, "Jonas S Karlsson\nis my name", 1, 1, black, yellow); break;
  case 1:   plots(0, 0, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 1,1, black, yellow); break;
  case 2: // menu
    plots(0,0, "close", 1,1, yellow, black);
    plots(8,0, "star", 1,1, green, black);
    plots(16,0, "#hash", 1,1, cyan, black);
    plots(24,0, "undo", 1,1, blue, white);
    plots(32,0, "list", 1,1, magenta, white);
    plots(40,0, "history", 1,1, red, white);
    plots(48,0, "quit", 1,1, black, white);
    break;
  }
  printf("\n");
}
#endif

/*


     b             f     h   i  j k   l
 a   bb   cc  eee f  gg  hhh      k k l
a a  b b c    ee  f g  g h h i  j kk  l
 aaa bbb  cc  eee f  ggg h h i  j k k l
                       gg      j

                  t
m m nnn   o   ss tt u u v v x x
mmm n  n o o  s  t  u u v v  x
m m n  n  o  ss   t  uu  v  x x


 A  BB  CC DD  EEE FF  GG H H I   J K K
A A B  C   D D E   F  G   H H I   J Kk
AAA Bb C   D D EE  FF G G HhH I J J Kk
A A Bb  CC DD  EEE F   GG H H I  J  K K

M M N N  O  PP   Q  RRR SSS 
MMM NNN O O PPP Q Q RR  S
M M NNN O O P   QQQ R R  S
M M N N  O  P    QQ R R SS


*/

