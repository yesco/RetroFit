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
  while (rows+3 < screen_rows) {
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

  for (int y=0; y < 8; y++) {
    //if (y==3 || y==5 || y==7) {r--;continue;}
    gotorc(r+y, c);
    char bits=def[y];
    for (int x=0; x < 8; x++) {
      //if (x==1 || x==4 || x==2) continue;
      B(bg);
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
  if (0)
    plots(0, 0, "Jonas S Karlsson\nis my name", 1, 1, black, yellow);
  else
    plots(0, 0, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 1,1, black, yellow);
  printf("\n");
}
#endif
