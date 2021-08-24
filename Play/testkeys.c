#include "../jio.c"

int main() {
  // off
  fprintf(stderr, "\e[?1000;1003;1006;1015l");
  // on
  //fprintf(stderr, "\e[?1000;1003;1006;1015h");
  fprintf(stderr, "\e[?1000;1006h");

  testkeys();

  fprintf(stderr, "\e[?1000;1003;1006;1015l");

}
