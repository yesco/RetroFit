/*
Fixed-sized ASCII-records a 64 bytes, last in \n
----------------------------------------------------------------
------------
BaSe64BaSe64 BaSe64BaSe64 BaSe64BaSe64 BaSe64BaSe64 BaSe64BaSe64

1631449374 1631449374 1631449374 1631449374 1631449374
xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxxx x\n

2021-09-09T19:46:04+0700 tab=4711 ohi=471142 state="FUSEDC*$#TWRH" top=42 o=1717 len=3232
1234567812345678123456781234567812345678123456781234567812345678
(8x8 longs, no newline)
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../jio.c"

int main(void) {
  FILE *ftabstate= fopen(".wtabstate", "r+");
  if (!ftabstate) {
    if (errno!=ENOENT) {
      perror("Couldn't open");
      exit(1);
    }
    ftabstate= fopen(".wtabgstate", "w+");
  }

  // Sanity of variables
  assert(sizeof(int)>=sizeof(int32_t));
  assert(sizeof(long)>=sizeof(int64_t));

  // 16*4=64 bytes per entry!
  int64_t read_mstime= mstime();

  int32_t tab= 19; // line# in .whistory
  int32_t ohi= 19; // offset in -"-

  int32_t top= 42; // line# in .ANSI
  int32_t o= 4711; // offset in .DOWN

  int32_t bytes= 0; // bytes in .DOWN
  int32_t unusedint= 0;

  int64_t checked_mstime= 0;
  int32_t check_interval= 0;

  char failed='F';  // 0
  char unread='U';  // 1
  char seen='S';    // 2
  char end='E';     // 3
  char deleted='D'; // 4
  char changed='C'; // 5
  char starred='*'; // 6
  char commented='$'; // 7
  char hashtag='#'; // 8
  char type='T';    // 9 Text Html Image Sound Video sVg Xml Mime Mail Folder Pdf hOst(ip) Word(shudder)
  char monitor='M'; // 10
  char reload='R';  // 11
  char hidden='H';  // 12
  char unused3=' '; // 13
  char unused2=' '; // 14
  char unused1=' '; // 15
  // 4 flags remain
  
  // 2 int32_t remains
  char reserved[2*4]={0};

  fseek(ftabstate, 0, SEEK_SET);
  do {
    long p=ftell(ftabstate);
    char buf[9]= {0};
    int r=fread(buf, sizeof(buf), 1, ftabstate);
    printf("%ld: r=%d\n", p, r);
  } while (!feof(ftabstate));

  fclose(ftabstate);
}
