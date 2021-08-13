// https://www.gnu.org/software/libunistring/manual/libunistring.html

#include <stdio.h>

#include <unistr.h>
#include <uniname.h>

// #include <stdlib.h>

#define LEN 32
int main(void) {
  //uint8_t* s= "Ａ";

//  uint32_t uc = 0xFF21;
  uint32_t uc = 65;
  size_t l = LEN;
  uint8_t b[LEN] = {0};
  uint8_t* r= u32_to_u8(&uc, 1, &b[0], &l);

  printf("unicode -> utf8 '%s' len=%zu", r, l);
}
