// https://www.gnu.org/software/libunistring/manual/libunistring.html

#include <stdio.h>

#ifndef FOO
#include <unistr.h>
#include <uniname.h>
#endif

// no special includes
#include <wchar.h>
// #include <stdlib.h>

//Function: uint32_t * u8_to_u32(const uint8_t *s, size_t n, uint32_t *resultbuf, size_t *lengthp)
// Converts an UTF-8 string to an UTF-32 string.
//Function: uint8_t * u32_to_u8 (const uint32_t *s, size_t n, uint8_t *resultbuf, size_t *lengthp)
//Converts an UTF-32 string to an UTF-8 string.


//UNINAME_MAX

//char* unicode_character_name(ucs4_t uc, char *buf)
//uc->name (uppercase ascii) or NULL
//ucs4_t unicode_name_character (const char *name)
//name->uc

char name[UNINAME_MAX] = {0};
ucs4_t c= 0;

void main(void) {
  printf("Hello Unicode!\n");

#ifdef FOO
  char* rr= unicode_character_name('&', name);
  printf("& = %s\n", rr);
  //strcpy(name, "AMP");

  uint8_t* s= "Ａ";
  size_t l= strlen(s)*5; // over
  uint32_t* u= calloc(l, 0);
  printf("11111\n");

  uint32_t* r= u8_to_u32(s, strlen(s), u, &l);

  printf("2222\n");
  printf("%s => 0x%04x\n", s, r[0]);

  size_t r8l= l*5;
  char r8[r8l]; memset(r8, 0, r8l);
  uint8_t* rs= u32_to_u8(u, l, r8, &r8l);

  printf("unicode -> utf8 '%s'", &r8);

  // counting characters
  // https://github.com/gagern/gnulib/blob/master/lib/unistr/u8-mbsnlen.c

 //Function: size_t u8_mbsnlen (const uint8_t *s, size_t n)
#endif

  // - simple interface stdlib !
  // https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/wchar.h.html
  putwchar(0xff21);
  putwchar(0xff22);
  putwchar(0xff23);
  wchar_t cc= getwchar();
  printf("%lc, 0x%x", cc, cc);

  // #include <stdlib.h>
  //int mbtowc(wchar_t *restrict pwc, const char *restrict s, size_t n);
  int ri = mbtowc(&cc, s, strlen(s));
}
