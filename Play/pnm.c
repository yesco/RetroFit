#include <assert.h>

#undef MAIN
#include "../jio.c"
#include "../graphics.c"

char *gloadpnm(char *fname, int tw, int th, int method) {
  FILE *f= fopen(fname, "r");
  char buf[80];
  
  if (0!=fscanf(f, "P6")) return "Not P6---";

  int w=0, h=0, cols=0;
  if (3!=fscanf(f, "%d %d %d\n", &w, &h, &cols)) return "w h cols missing";
  if (cols!=255) return "cols!=255";

  printf("w=%d h=%d cols=%d\n", w, h, cols);
  int size= w*h* 3;
  unsigned char *pic= calloc(size, 1);
  int n= fread(pic, 1, size, f);
  if (n!=size) {
    printf("size=%d n=%d\n", size, n);
    return "size differs";
  }
  fclose(f);

  // process image
  gclear(); gfg= rgb(5,5,5);

  // TODO: malloc?
  struct sumpix {
    long r,g,b,n;
  } rgbn[gsizey*gsizex];
  memset(rgbn, 0, sizeof(rgbn));

  unsigned char *p= pic;

  for(int y=0; y<h; y++) {
    for(int x=0; x<w; x++) {
      int r= *p++, g= *p++, b= *p++;
      //printf("%4d %4d:   %3d %3d %3d\n", y, x, r, g, b);
      int xx= x*tw/w, yy= y*th/h;
      int rr= r*5/256, gg= g*5/256, bb= b*5/256;
      switch(method) {
      case 0: // just keep size
        gset(x, y, rgb(rr,gg,bb));
        break;
      case 1: // fit/overwrite
        gset(xx, yy, rgb(rr,gg,bb));
        break;
      case 2: { // sum it up/dest
        int i= yy*gsizex+xx;
        rgbn[i].n++;
        rgbn[i].r+= r;
        rgbn[i].g+= g;
        rgbn[i].b+= b;
        break; }
      }
    }
  }
  if (p-pic!=size) {
    printf("size=%d p-pic=%ld\n", size, p-pic);
    return "wrong loop size";
  }

  // process accumlated pixels
  for(int y= 0; y<gsizey; y++) {
    for(int x= 0; x<gsizex; x++) {
      int i= y*gsizex+x;
      long n= rgbn[i].n;

      switch(method) {
      case 2: {// savg
        long r= rgbn[i].r/n;
        long g= rgbn[i].g/n;
        long b= rgbn[i].b/n;
        
        int rr= r*5/256, gg= g*5/256, bb= b*5/256;

        //gset(x, y, rgb(rr,gg,bb));
        if (r+g+b>255/2)
          gset(x, y, yellow);

        break; }
      }
    }
  }

  gupdate();

  free(pic);
  return NULL;
}

int main(int argc, char *argv[]) {
  printf("argc=%d\n", argc);
  assert(argc==3);
  jio();

  //clear();

  int tw= 16, th= 8; // another method?

  int f= atoi(argv[1]);
  int method= atoi(argv[2]);

  char *files[]={
    "Graphics/fil.pnm", "Graphics/x.pnm", "Graphics/u.pnm"};
  char *err= gloadpnm(files[f], tw, th, method);

  gotorc(screen_rows/2, 0);
  printf("error=%s\n", err?err:"OK");
}
