#include <stdio.h>
#include <assert.h>

#include "../jio.c"

// bytes: 32194112
//   cat >/dev/null => 0.031
//   cat | wc       => 0.7
//   tac >/dev/null => 0.132
//   cat | wc       => 0.7
//   slow()         => 1m19s !!!

void slow() {
  FILE *f= fopen(".w/Cache/Tests%2flinks-many.html.ANSI", "r");

  // read lines backwards
  fseek(f, 0, SEEK_END);
  long pos= ftell(f);

  while(!feof(f) && pos>0) {
    // find prev line
    int c= 0;
    while (pos>0 && c>= 0 && c!='\n') {
      fseek(f, --pos, SEEK_SET);
      c= fgetc(f);
    }
    if (c<0) break;
    // no newline before last
    if (!pos) fseek(f, 0, SEEK_SET);

    char buf[1024]={0};
    if (fgets(buf, sizeof(buf), f)) {
      printf("- %s", buf);
    }

    // go "back"
    fseek(f, pos, SEEK_SET);
  }

  fclose(f);
}

// Read line from file
// Returns: line (without \n)
//          NULL if at EOF (d free:d)
dstr *dstrfgetln(dstr *d, FILE *f) {
  if (d) d->s[0]= 0;
  int c; char ch;
  do {
    c= fgetc(f);
    if (c==EOF) return d && strlen(d->s) ? d : FREE(d);
    ch= c;
    d= dstrncat(d, &ch, 1);
  } while (ch!='\n');
  // remove newline
  int len= d? strlen(d->s) : 0;
  if (len && d->s[len-1]=='\n')
    d->s[len-1]= 0;
  return d;
}

// read lines backwards from pos
dstr *dstrfrgetln(dstr *d, FILE *f) {
  // (bigger than 1024 not notic faster)
  static char nlbuf[1024];
  static long max;
  max= ftell(f);
  //printf("MAX=%ld\n", max);
  if (max<=1) return FREE(d);
  static long pos;
  pos= max;
  static long bufpos;
  bufpos= pos*2; // after pos

  pos--;
    
  // find prev line
  int c= 0;
  while (pos>0 && c>= 0 && c!='\n') {
    pos--;

    // read previous block
    if (pos<0) return FREE(d);
      
    if (pos<=bufpos) {
      bufpos= (pos/sizeof(nlbuf))*sizeof(nlbuf);
      fseek(f, bufpos, SEEK_SET);
      int r= fread(nlbuf, 1, sizeof(nlbuf), f);
      //printf("%% @%d r=%d\n", bufpos, r);
      //assert(r==sizeof(nlbuf) || r==max%sizeof(nlbuf));
    }
    assert(pos>=bufpos);
    assert(pos-bufpos<sizeof(nlbuf));

    c= nlbuf[pos-bufpos];
  }
  if (pos<0) return FREE(d);

  // no newline before real "first"
  fseek(f, pos?pos+1:0, SEEK_SET);

  // TODO: reads fewer bytes?
  // a./ouit | wc less than wc orig
  //printf("@%ld ", pos);
  if (0) {
    char buf[1024]={0};
    if (fgets(buf, sizeof(buf), f)) {
      printf("- %s", buf);
    }
  } else {
    d= dstrfgetln(d, f);
    //printf("- >%s<\n", d?d->s:"(null)");
  }

  // go "back"
  fseek(f, pos+1, SEEK_SET);
  return d;
}

int main() {
  //printf("BUFSIZ=%d\n", BUFSIZ);

  //  FILE *f= fopen(".whistory", "r");
  //FILE *f= fopen(".w/Cache/Tests%2flinks-many.html.ANSI", "r");

  FILE *f= fopen("Play/1-10.txt", "r");

  dstr *d = NULL;
  fseek(f, 0, SEEK_SET);
  while((d= dstrfgetln(d, f))) {
    printf("= %s\n", d->s);
  }
  FREE(d);
  printf("============\n");
  fseek(f, 0, SEEK_END);
  while((d= dstrfrgetln(d, f))) {
    printf("= %s\n", d->s);
  }
  printf("-----------\n");
  while((d= dstrfrgetln(d, f))) {
    printf("= %s\n", d->s);
  }
  FREE(d);

  fclose(f);
}
