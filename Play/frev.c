#include <stdio.h>
#include <assert.h>

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

void faster() {
  FILE *f= fopen(".w/Cache/Tests%2flinks-many.html.ANSI", "r");
  //FILE *f= fopen("Play/1-10.txt", "r");

  // read lines backwards
  // (bigger than 1024 not notic faster)
  char nlbuf[1024];

  fseek(f, 0, SEEK_END);
  long max= ftell(f);
  long pos= max;
  long bufpos= pos*2; // dummy val

  while(!feof(f) && pos>0) {

    // find prev line
    int c= 0;
    while (pos>0 && c>= 0 && c!='\n') {
      // read previous block
      pos--;
      if (pos<0) break;
      
      if (pos<=bufpos) {
        bufpos= (pos/sizeof(nlbuf))*sizeof(nlbuf);
        fseek(f, bufpos, SEEK_SET);
        int r= fread(nlbuf, 1, sizeof(nlbuf), f);
        //printf("%% @%d r=%d\n", bufpos, r);
        assert(r==sizeof(nlbuf) || r==max%sizeof(nlbuf));
      }
      assert(pos>=bufpos);
      assert(pos-bufpos<sizeof(nlbuf));

      c= nlbuf[pos-bufpos];
    }
    if (pos<0) break;
    // no newline before last
    fseek(f, pos?pos+1:0, SEEK_SET);

    // TODO: reads fewer bytes?
    // a./ouit | wc less than wc orig
    char buf[1024]={0};
    if (fgets(buf, sizeof(buf), f)) {
      printf("- %s", buf);
    }

    // go "back"
    fseek(f, pos, SEEK_SET);
  }

  fclose(f);
}

int main() {
  //  FILE *f= fopen(".whistory", "r");
  faster();
}
