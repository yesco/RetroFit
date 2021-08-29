#include <stdio.h>
#include <stdlib.h>

#include "../jio.c"

#undef MAIN
#include "../graphics.c"

int main(void) {
  jio();
  cursoroff();
  
  FILE *f= fopen(".whistory", "r");

  keycode k;
  int n=0;
  while((k= key()) != CTRL+'C') {
    if (k & SCROLL_UP) n--;
    if (k & SCROLL_DOWN) n++;

    char *ln= fgetlinenum(f, n);
    gotorc(3*screen_rows/4, 0);
    B(white); C(black); clearend();
    printf("%s", ln?ln:"(null)");
    clearend();

    gclear();

    // print date
    if (ln) ln[10]=0;
    // gbg= white doesn't update corectly!
    gbg= yellow; gfg= black;
    gx= 0; gy= 0;
    if (ln) gputs(ln);
    gputc('\n');

    // print time
    if (ln) ln[19]=0;
    // gbg= white doesn't update corectly!
    gbg= yellow; gfg= black;
    if (ln) gputs(&ln[11]);

    gy= 8*2; gx= 0;
    // print url
    if (ln) *strchr(&ln[30], ' ')= 0;
    // gbg= white doesn't update corectly!
    gbg= yellow; gfg= black;
    if (ln) gputs(&ln[30]);
    gputs("                \n");
    //while(gy<gsizey/2-8) gputc(' ');

    // print num
    char buf[30];
    sprintf(buf, "  %d  ", n);
    gbg= yellow; gfg= black;
    drawCenteredText(buf);

    gupdate();
    free(ln);
  }

  fclose(f);
}
