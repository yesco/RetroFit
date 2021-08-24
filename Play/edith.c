#include "../jio.c"

int main() {
  char *allowed= NULL;
  char *not= NULL; //"0123456789";

  dstr *line= NULL;
  int k=0;
  do {
    B(cyan); C(black);
    gotorc(10,10);
    printf("NAME: ");

    B(white); C(black);
    spaces(10);

    gotorc(10,10+6);
    fflush(stdout);

    B(white); C(black); fflush(stdout);
    // try -1 for width!
    k= edit(&line, 10, allowed, not);

    B(black); C(white);
    printf("\nBREAK: %s\n", keystring(k));
  } while (k!=CTRL+'C');
}

