#include "../jio.c"

// Edits a string VALUE of max-SIZE until a non-ALLOWED or a NOT allowed key is typed.
//
// Returns key not allowed or EOF
//
// Edit keys:
//   For now, only BACKSPACE   
//
// Usage:
//   Normal usage is to envelope the envelope the function with a while loop
//   until RETURN si returned.
//    This allows the user program to still handle short-cut keys!
//    
int edith(dstr **dsp, int width, char *allowed, char *not) {
  // make sure have a pointer
  *dsp= dstrncat(*dsp, NULL, 1);

  printf("%s", (*dsp)->s); fflush(stdout);

  int k=0;
  while ((k= key())!=EOF) {
    int len= strlen((*dsp)->s);
    if (k>=32 && k<127) {
      if (allowed && !strchr(allowed, k)) break;
      if (not && strchr(not, k)) break;
    } else if (isutf8(k))
      ; // ok
    else if (k==BACKSPACE || k==DEL) {
      if (len>0) {
        putchar(8); putchar(' '); putchar(8); fflush(stdout);
        (*dsp)->s[len-1]= 0;
      }
      k= ' ';
      continue;
    } else if (k<32 || k>=256)
      break; // CTRL or META or TERM

    // ignore when full
    if (len>=width) continue;
    // accept and output
    char c= k;
    putchar(c); fflush(stdout);
    *dsp= dstrncat(*dsp, &c, 1);
  }
  return k;
}

int main() {
  char *allowed= NULL;
  char *not="0123456789";

  dstr *line= NULL;
  int k=0;
  do {
    B(blue); C(yellow);
    gotorc(10,10);
    printf("NAME: [%10s]", "");
    gotorc(10,10+7);
    fflush(stdout);

    B(white); C(black);
    k= edith(&line, 10, allowed, not);
    printf("\nBREAK: %s\n", keystring(k));
  } while (k!=CTRL+'C');
}

