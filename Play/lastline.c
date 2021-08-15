#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep

int flines(FILE* f) {
  fseek(f, 0, SEEK_SET);
  int n= 0, c;
  while((c= fgetc(f)) != EOF)
    if (c=='\n') n++;
  return n++;
}

// Read full line from FILE
// Return NULL, if none, or malloced line without '\n'
char* fgetline(FILE* f) {
  // save pos, figure out how long line
  long p= ftell(f), len= 2, c;
  while((c= fgetc(f)) != EOF && c!='\n')
    len++;
  // reset, allocate and read
  fseek(f, p, SEEK_SET);
  char* r= calloc(len, 1);
  if (!fgets(r, len, f)) free(r), r= NULL;
  if (r && r[strlen(r)-1]=='\n')
    r[strlen(r)-1]= 0;
  return r;
}

// In FILE Find and return numbered LINE.
// Result: see fgetline()
char* fgetlinenum(FILE* f, long line) {
  fseek(f, 0, SEEK_SET);
  int c;
  while(line > 1 && (c= fgetc(f)) != EOF)
    if (c=='\n') line--;
  if (line > 1) return NULL;
  return fgetline(f);
}

int main(void) {
  // count lines
  FILE* f= fopen(".whistory", "r");
  size_t n= flines(f);
  printf("lines=%lu\n", n);

  // get last
  char* l= fgetsnum(f, n);
  printf("last:\n%s<<<\n", l);
  free(l);

  // get last-1
  char* lm1= fgetsnum(f, n-1);
  printf("last minus 1:\n%s<<<\n", lm1);
  free(lm1);

  //while((tailf= fgetsnum(f, n+1))==NULL);

  // tail -f the file until QUIT in it
  char* tailf= NULL;
  do {
    // wait for line
    while((tailf= fgetline(f))==NULL) {
      putchar('.');
      fflush(stdout);
      usleep(300*100);
    }
    
    printf("tail:\n%s<<<\n", tailf);
  } while (tailf && !strstr(tailf, "QUIT"));
  if (tailf) free(tailf);
  fclose(f);
}
