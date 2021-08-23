#include "../jio.c"

int main() {
  // Conclusion: \e21m does NOT unbold
  // need to reset, and restablish.
  // TODO: Need a TERM state struct that can be copied/saved/restored and applied.

  printf("\e[7m"); fg(white); bg(black);
  printf("noBOLD \e[1mBOLD");
  printf("\e[0;7m"); fg(white); bg(black);
  printf("noBOLD\n");

  printf("\e[37;40m\eHELLO\e[1mBOLD\e[0;37;40m\e[K  ANDNOT");
  B(black); C(blue); printf("HELLO \e[1mBOLD\e[21m\n");
}


