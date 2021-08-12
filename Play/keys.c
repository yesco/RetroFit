// Unix TTY and emulates the DOS <conio.h> functions kbhit() and getch():

// (jsk) modified from:
// - https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

struct termios orig_termios;

void reset_terminal_mode() {
  tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode() {
  struct termios new_termios;

  // two copies
  tcgetattr(0, &orig_termios);
  new_termios= orig_termios;

  // register cleanup handler
  atexit(reset_terminal_mode);

  // new terminal mode
  cfmakeraw(&new_termios);
  tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch() {
  int r;
  unsigned char c;
  return read(0, &c, 1) < 0 ? -1 : c;
}

int main(int argc, char *argv[]) {
  fprintf(stderr, "\n[Type '^C q u i t' to quit!\r\n");

  set_conio_terminal_mode();
  // jsk: after this the \n isn't \r\n 1

  int c= 0, q=0;
  while(c>=0) {
    // wait for key, print .
    while (!kbhit()) {
      for(int i=256*1024*1024; i>0; i--);
      fputc('.', stderr);
    }

    // pretty
    c= getch();
    if (c < 32)
      fprintf(stderr, "^%c", c+64);
    else
      fputc(c, stderr);
      
    // quit?
    q= c<33 ? 0 : q*2 + c;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");
}
