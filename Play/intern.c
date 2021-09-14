#include <stdio.h>

#define QUIT "quit"
#define quiT "quit"

typedef intptr_t STR;

const STR FIRST=(STR)"FIRST";

const char * const Quit= QUIT;

const STR q= (STR)QUIT;
const STR Q= 42;

const long l = 43;
const long cl= l;

char *quit= QUIT;
char qUIt[] = QUIT;

const STR diff= QUIT-FIRST;

int main(void) {
  printf("diff = %lx\n", q-FIRST);
  printf("\n");
  printf("QUIT = %p\n", QUIT);
  printf("Quit = %p\n", Quit);
  printf("quit = %p\n", quit);
  printf("quiT = %p\n", quiT);
  printf("qUIt = %p\n", qUIt);
  printf("q    = %lx\n", q);
  printf("Q    = %lx\n", Q);

//  q= (STR)"foo";

  char *cmd= "QUIT";
  switch((STR)cmd) {
//  case q: printf("quit!\n"); break;
//  case Q: printf("quit!\n"); break;
//  case l: printf("quit!\n"); break;
//  case cl: printf("quit!\n"); break;
  default:
    printf("Don't know: %s\n", cmd);
  }
}

