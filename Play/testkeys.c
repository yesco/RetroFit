#include "../jio.c"

int main() {
  fprintf(stderr, "\e[?1000;1003;1006;1015h");
  testkeys();
  fprintf(stderr, "\e[?1000;1003;1006;1015l");

}
