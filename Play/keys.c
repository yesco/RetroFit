// Unix TTY and emulates the DOS <conio.h> functions kbhit() and getch():

// (jsk) modified from:
// - https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input
// Alternative:
// - https://stackoverflow.com/questions/3962263/how-to-check-if-a-key-was-pressed-in-linux
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

  // bracketed paste display
  fprintf(stderr,"\e[2004l");

/*

Ps = 1 0 0 0  -> Send Mouse X & Y on button press and
release.  See the section Mouse Tracking.  This is the X11
xterm mouse protocol.

Ps = 1 0 0 1  -> Use Hilite Mouse Tracking.
Ps = 1 0 0 2  -> Use Cell Motion Mouse Tracking.
Ps = 1 0 0 3  -> Use All Motion Mouse Tracking.
Ps = 1 0 0 4  -> Send FocusIn/FocusOut events.
Ps = 1 0 0 5  -> Enable UTF-8 Mouse Mode.
Ps = 1 0 0 6  -> Enable SGR Mouse Mode.
Ps = 1 0 0 7  -> Enable Alternate Scroll Mode.
Ps = 1 0 1 0  -> Scroll to bottom on tty output (rxvt).
Ps = 1 0 1 1  -> Scroll to bottom on key press (rxvt).
Ps = 1 0 1 5  -> Enable urxvt Mouse Mode.
*/

  // xterm mouse init
  // - 1003 doesn't seem to give contious move... on termux... :-(

  fprintf(stderr, "\e[?1000;1003;1006;1015h");
  //fprintf(stderr, "\e[?1003;1006;1015h");

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
    if (c==27) fprintf(stderr, "\r\n");
    if (c < 32)
      fprintf(stderr, "^%c", c+64);
    else
      fputc(c, stderr);
      
    // quit?
    q= c<33 ? 0 : q*2 + c;
    if (q=='q'*8+'u'*4+'i'*2+'t') break;;
  }
  printf("\r\n");

  fprintf(stderr, "\e[?h");

  // deinit mouse/jio
  fprintf(stderr, "\e[?1000;1003;1006;1015l");

}

