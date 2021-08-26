#include <stdio.h>
#include <stdlib.h>

#include "Graphics/font8x8/font8x8_basic.h"

// Simple ANIM graphics.


// Usage: #include "anim.c" LOL

// TOOD: make independent?
// User extensible scripts!

#ifdef MAIN
  #include "jio.c"
#endif

// Terminal graphic characters
// (they have non-logical order)
const char* BLOCKS[]= {
  " ","▘","▝","▀",   "▖","▌","▞","▛",
  "▗","▚","▐","▜",   "▄","▙","▟","█"};

typedef unsigned char pixel;
typedef pixel *gscreen;

int pixels_per_col= 2;
int pixels_per_row= 2;

gscreen _gscreen= NULL, _gscold= NULL;
int gsizex= 0, gsizey= 0;
int gx= 0, gy= 0;
int gbg= black, gfg= white;

gscreen gcanvas() {
  if (!_gscreen) {
    gsizex= screen_cols*pixels_per_col;
    gsizey= screen_rows*pixels_per_row;
    _gscreen= calloc(gsizex*gsizey, 1);
    _gscold= calloc(gsizex*gsizey, 1);
  }
  return _gscreen;
}

// return pointer to pixel or NULL
pixel *gpixel(int x, int y) {
  gscreen g= gcanvas();
  if (x<0 || y<0 || x>=gsizex || y>=gsizey) return NULL;
  return &(g[y*gsizex + x]);
}

pixel gset(int x, int y, int c) {
  pixel *p= gpixel(x, y);
  if (!p) return 0;
  // blend?
  pixel v= *p;
  if (c!=none) *p= c;
  return v;
}

void gupdate() {
  pixel lastbg= black, lastfg= white;
  int lastr= -1, lastc= -1;

  B(black); C(white);
  for(int y=0; y<gsizey; y+=pixels_per_row) {
    for(int x=0; x<gsizex; x+=pixels_per_col) {
      pixel a= *gpixel(x, y), b= *gpixel(x+1, y);
      pixel c= *gpixel(x, y+1), d= *gpixel(x+1, y+1);
//      pixel *p= _gscold + (gpixel(x, y) - _gscreen);
//      if (a==*p && b==*++p &&
//          c==*(p+=gsizex-1) && d==*++p)
//        continue;
      long p= _gscold - _gscreen;

      if(1)
      if (a==*(p+gpixel(x, y)) && b==*(p+gpixel(x+1, y)) &&
          c==*(p+gpixel(x, y+1)) && d==*(p+gpixel(x+1, y+1)))
        continue;
      
      int nr= y/pixels_per_row, nc= x/pixels_per_col;
      if (lastr!=nr || lastc!=nc-1)
        gotorc(nr, nc);
      lastr= nr; lastc= nc;

      // decide fg color
      pixel fg= MAX(a, MAX(b, MAX(c, d)));
      // decide bg color
      pixel bg= MIN(a, MIN(b, MIN(c, d)));
      // TODO: messed up?
      if (fg>7) fg= 7;
      if (bg>7) bg= 7;
      fg= gfg;
      bg= black;

      // decide if fg
      // TODO: make better
      int fa= !!a, fb= !!b;
      int fc= !!c, fd= !!d;

      // print quad-block
      const char* s= BLOCKS[fa*1+fb*2+fc*4+fd*8];
      if (bg!=lastbg) B(lastbg= bg);
      if (fg!=lastfg) C(lastfg= fg);
      printf("%s", s);
    }
  }
  memcpy(_gscold, _gscreen, gsizey*gsizex);
}

void gputc(char c) {
  if (c<0 || c>127) return;
  char *def = font8x8_basic[c];

  for (int y=0; y<8; y++) {
    char bits=def[y];
    for (int x=0; x<8; x++) {
      if (bits & 1<<x)
        gset(gx+x, gy+y, gfg);
      //else
      //gset(gx+x, gy+y, gbg);
    }
  }
  gupdate();
}

void gputs(char *s) {
  int c;
  while((c= *s++)) {
    if (c=='\n' || gx+8>=gsizex) {
      gy+= 8;
      gx = 0;
    }
    if (c=='\n') continue;
    gputc(c);
    if ((gx+= 8)>=gsizex) {
      gy+= 8;
      gx = 0;
    }
  }
}



////////////////////////////////////////
// figures




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
      const char* str= labels[n*rr/rows];

      // center label
      int w= (cc-strlen(str))/2;
      gotorc(rr, screen_cols-cc + w);
      readablefg();
      printf("%s", str);
    }
    while(haskey()) key();
  }
}

void plot(int r, int c, char ch, int w, int h, int bg, int fg) {
  //printf("plot %d %d %c\n", r, c, ch); return;
  if (ch>127) return;
  char *def = font8x8_basic[ch];

  // print using 1 char = 1 pixel
  // directly to screen
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
//  jio();
  screen_init();
  cursoroff();

  gset(10, 10, 255);

  switch(4) {

  case 0: // X
    for(int y=0; y<gsizey; y++) {
      int x= gsizex*y/gsizey;
      // unsafe
      gset(x, y, 255);
      gset(gsizex-1-x, y, 255);
      gupdate();
    }
    break;
  case 1: // line
    for(int x=0; x<gsizex; x++) {
      gset(x, 0, 255);
      gupdate();
    }
    break;
  case 2: // block
    for(int x=0; x<gsizex; x++) {
      for(int y=0; y<gsizey; y++) {
        gset(x, y, 255);
      }
      gupdate();
    }
    break;
  case 3: // block horiz=faster
    for(int y=0; y<gsizey; y++) {
      for(int x=0; x<gsizex; x++) {
        gset(x, y, 255);
      }
      gupdate();
    }
    break;
  case 4: // plots
    // 23 s => 0.856s in highest reso
    // (opt of only update delta)
    for(int i=0; i<100; i++) {
      gputs("abcdefghijklmnopqrstuvwxyz");
      //gupdate();
    }
    
    break;
  case 5:
      gset(30,30,255);
      gputc('X');
      break;
  }
  gupdate();
  
  gotorc(screen_rows-10, 0);
  printf("\n<<<(%d %d)>>>\n", gsizex, gsizey);
  printf("x=%d y=%d bg=%d fg=%d\n", gx, gy, gbg, gfg);

  cursoron();
  exit(0);


  switch(1) {

  case 0: plots(0, 0, "Jonas S Karlsson\nis my name", 1, 1, black, yellow); break;
  case 1:   plots(0, 0, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 1,1, black, yellow); break;
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
