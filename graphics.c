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
int gbytes= 0;

void gallocate(int x, int y, int clear) {
  gsizex= x;
  gsizey= y;
  int size=gsizex*gsizey;
  if (gbytes!=size) {
    FREE(_gscreen);
    FREE(_gscold);
  }
  gscreen old= _gscreen;
  if (!old) {
    _gscreen= malloc(size);
    _gscold= malloc(size);
    gbytes= size;
    clear= 1;
  }
  if (clear) {
    memset(_gscreen, none, size);
    memset(_gscold, none, size);
    gx= 0, gy= 0, gbg= black, gfg= white;
  }
}

// TODO: generalize
gscreen gclear() {
  gallocate(
    screen_cols*pixels_per_col,
    screen_rows*pixels_per_row,
    1);
  return _gscreen;
}

gscreen gcanvas() {
  gallocate(
    screen_cols*pixels_per_col,
    screen_rows*pixels_per_row,
    0);
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

void gline(int x, int y, int dx, int dy, int c) {
  int ax= abs(dx), ay= abs(dy);
  if (ax >= ay) { // x
    printf("Fiefum\n");
    int s= dy>0?1:-1;
    for(int d=0; d<=ax; d++) {
      usleep(200*1000);
      gset(x+s*d, y-s*ay*d/dx, c);
      void gupdate();
      gupdate();
    }
  } else { // y
    printf("Foobar\n");
    int s= dy>0?1:-1;
    for(int d=0; d<ay; d++)
      gset(x+s*ax*d/dy, y+s*d, c);
  }
}

void glineto(int x, int y, int tx, int ty, int c) {
  gline(x, y, tx-x, ty-y, c);
}



// Update the screen to the current canvas
void gupdate() {
  assert(pixels_per_row<=2);
  assert(pixels_per_col<=2);
  save();
  pixel lastbg= black, lastfg= white;
  int lastr= -1, lastc= -1;

  B(black); C(white);
  for(int y=0; y<gsizey; y+=pixels_per_row) {
    for(int x=0; x<gsizex; x+=pixels_per_col) {
      // Only applies if 2?? what if row:2,col:1 ???
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

      // TODO: improve? choose policy?
      // decide fg color
      pixel fg= MAX(a, MAX(b, MAX(c, d)));
      // decide bg color
      pixel bg= MIN(a, MIN(b, MIN(c, d)));

      bg= gbg;

      // decide if fg
      // TODO: make better
      int fa= a!=bg && a!=none;
      int fb= b!=bg && b!=none;
      int fc= c!=bg && c!=none;
      int fd= d!=bg && d!=none;

      // colors?
      fg= fa? a : fb? b : fc? c : fd? d : gfg;

      // print quad-block
      const char* s= BLOCKS[fa*1+fb*2+fc*4+fd*8];
      if (bg!=lastbg) B(lastbg= bg);
      if (fg!=lastfg) C(lastfg= fg);
      B(bg); C(fg);
      printf("%s", s);
    }
  }
  memcpy(_gscold, _gscreen, gsizey*gsizex);
  restore();
  fflush(stdout);
}

void gnl() {
  // TODO: should it paint over lines?
  // not if bg= none
  gy+= 8;
  gx = 0;
}

// Graphics put CHAR on screen, move cursor
// (\n wraps line)
void gputc(char c) {
  if (c<0 || c>127) return;
  char *def = font8x8_basic[c];

  if (c=='\n' || gx+8>gsizex) gnl();
  if (c=='\n') return;

  for (int y=0; y<8; y++) {
    char bits=def[y];
    for (int x=0; x<8; x++) {
      if (bits & 1<<x)
        gset(gx+x, gy+y, gfg);
      else
        gset(gx+x, gy+y, gbg);
    }
  }
  if ((gx+= 8)>gsizex) gnl();
}

// Graphics put STRING on screen, move cursor
// (\n wraps line)
void gputs(char *s) {
  if (!s) return;
  int c;
  while((c= *s++)) gputc(c); 
}

// Graphics put TEXT centered on screen using char length
void drawCenteredText(char *s) {
  if (!s) return;
  int len= strlen(s);
  gy= (gsizey-8*1)/3; // upper 3rd
  gx= (gsizex-8*len)/2;
  if (gx<0) gy-=4,gx=0; // overflow... wrap
  gputs(s);
}


////////////////////////////////////////
// figures ??? TODO:

// TODO: remove? maybe these are only for speed/demo?


// TODO: what does it do?
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

// clear actual screen content area
void wclear() {
  gclear();
  B(black); C(white);
  for(int r=0; r<screen_rows-2; r++) {
    clearend(); putchar('\n');
  }
  fflush(stdout);
}

// Show a toast TEXT centered on empty screen
void gtoast(char *s) {
  gclear(); gfg= black; gbg= white;
  drawCenteredText(s);
  gupdate();
  gotorc(0,0);
}

// Show a read toast TEXT centered in red
void gtoasterr(char *s) {
  gclear(); gfg= red; gbg= white;
  drawCenteredText(s);
  gupdate();
  gotorc(0,0);
}

// Print ANSI host icon at (1,0)
// (download+generate if not cached)
//
// Returns 0 if no icon
int gicon(char *url) {
  if (!url) return 0;

  // extract hostname
  char *host= url;
  host= sskip(host, "https://");
  host= sskip(host, "http://");
  char *end= strchr(host, '/');
  if (!end) end= host+strlen(host);
  if (!host) return 0;

  // dangerous?
  if (strchr(host, '`') || strchr(host, '\\') || strchr(host, '"')) return 0;

  gotorc(1, 0); fflush(stdout);

  // cached?
  dstr *ansi= dstrprintf(NULL, ".w/Favicons/%.*s.ANSI", end-host, host);
  FILE *f= fopen(ansi->s, "r");
  free(ansi);
  if (f) {
    int c, n= 0;
    while((c= fgetc(f))!=EOF) {
      putchar(c);
      n++;
    }
    fclose(f);
    fflush(stdout);
    return n;
  } else {
    // generate
    dstr *cmd= dstrprintf(NULL, "./wicon \"%.*s\"",  end-host, host);

    int r= system(cmd->s);
    free(cmd);
    return !r;
  }
}

void drawPullDownMenu(color *colors, char **labels, int ncol, int n, int right) {
  int cc= 40*screen_cols/100;
  int c0= right? screen_cols-cc : 0;
  int rows= 0;
  while (rows < screen_rows) {
    rows++;
    // exponential decrease of slide
    usleep(70*SQR(rows)/SQR(screen_rows)/2*1000);

    for(int rr=0; rr<rows; rr++) {
      int i= n*rr/rows;
      gotorc(rr, c0);
      B(colors[i % ncol]); spaces(cc);

      // center label
      const char* str= labels[i];
      int w= (cc-strlen(str))/2;
      gotorc(rr, c0 + w);
      readablefg();
      printf("%s", str);
    }
  }
}

// Plot at ROW COL the CHaracter with Width Height using BackGround color and ForeGround color
// Plots on actual screen, not canvas
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

// Plot string at ROW COL the String with Width Height using BackGround color and ForeGround color
// Plots on actual screen, not canvas
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
int main(void) { /* ENDWCOUNT */

//  jio();
  screen_init();
  cursoroff();

  switch(7) {
  case -1:
    gset(10, 10, 255);
    break;

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

  case 5: // X
    gfg= green;
    gset(30,30,255);
    gputc('X');
    break;

  case 6:
    gtoast("Reloading");
    //exit(1);
    break;

    case 7: {
      int xx= 0;

      clear();
      gclear(); gfg= rgb(5, 1, 0); //RGB(0xff, 0xa5, 0x00);
      gputs("Duck");
      xx= gx;
      gupdate();

      gclear(); gfg= rgb(5, 3, 0); //RGB(0xff, 0xa5, 0x00);
      gx= xx;
      gputs("Duck");
      xx= gx;
      gupdate();

      gclear(); gfg= rgb(0, 3, 0); //RGB(0xff, 0xa5, 0x00);
      gx= xx;
      gputs("Go");
      gputc('\n');
      xx= gx;
      int yy= gy;
      gupdate();

      gclear(); gfg= rgb(5, 1, 0); //RGB(0xff, 0xa5, 0x00);
      gx= xx;
      gy= yy;
      gputs("D");
      xx= gx;
      gupdate();

      gclear(); gfg= rgb(5, 3, 0); //RGB(0xff, 0xa5, 0x00);
      gx= xx;
      gy= yy;
      gputs("D");
      xx= gx;
      gupdate();

      gclear(); gfg= rgb(0, 3, 0); //RGB(0xff, 0xa5, 0x00);
      gx= xx;
      gy= yy;
      gputs("G");
      xx= gx;
      gupdate();
      break; }
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

