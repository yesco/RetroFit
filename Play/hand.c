#undef MAIN
#include "../jio.c"
#include "../graphics.c"

#include <time.h>
#include <sys/time.h>

long utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + 1000000*tv.tv_sec;
}

long mstime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec/1000 + 1000*tv.tv_sec;
}

int main(void) {
  jio();

  keycode k, lastk;
  do {
    clear();
    gotorc(screen_rows/2, 0);
    fflush(stdout);
    gclear();
    gbg= black; gfg= white;

    k= key();
    int x= k & 0xff;
    int xx= x << 8; // higher precision
    int y= (k>>8) & 0xff;
    long ms= mstime();
    int dir= (x > screen_rows/2)?-1:+1;
    do {
      long msn= mstime(), d= msn-ms; ms= msn;
      printf("%5ld ", d);
      if (k & SCROLL_UP) y -= 1;
      if (k & SCROLL_DOWN) y += 1;
      x= (xx+128) >> 8;
      gset(x, y, gfg);
      gupdate();

      xx += dir* d*10;
      x= (xx+128) >> 8;
      // change direction
      // TODO: change when long time pass?
      if (x < screen_cols/8) {
        dir=-dir;
        xx= (screen_cols/8+3) << 8;
      }
      if (x > screen_cols*7/8) {
        dir=-dir;
        xx= (screen_cols*7/8-3) << 8;
      }
      lastk= k;
    } while(((k= key()) & 0xffff)
            ==(lastk &0xffff));
  } while(k!=CTRL+'C');
}
