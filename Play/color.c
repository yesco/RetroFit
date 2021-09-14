#include "../jio.c"

int main(void) {
  printf("FOO");
  bg(RGB(255,0,0));
  printf("BAR");
  bg(RGB(128,0,0));
  printf("FIE");
  bg(0);
  printf("FUM");
}
  
