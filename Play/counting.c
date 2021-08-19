#include <stdio.h>

// -- generate sequense A:
//  a, b, c...z, aa, ba, ca, .. za, ab, bb, cb, ..zb, ac, ... zz, aaa, baa, caa, ... zaa, aba, ...
(mappningen behövs dock troligen inte)
// https://m.facebook.com/groups/242675022610339/permalink/1700651710145989/?comment_id=1700669053477588&notif_t=group_comment&notif_id=1629376009997714&ref=bookmarks
// recursion by Kjell Post:
void p26(int n) {
  if (n<26)
    putchar(n+'a');
  else {
    p26(n%26);
    p26(n/26-1);
  }
}

int main() {
  for(int i=0; i<1024*1024; i++) {
    p26(i); putchar('\n');
  }
}
