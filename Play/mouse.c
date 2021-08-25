// jsk: not doing anything???
// - https://stackoverflow.com/questions/59864485/capturing-mouse-in-virtual-terminal-with-ansi-escape
//now have some working code that spits out the mouse events.
//
//Here is the updated code from applying the accepted answer to this question:

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() {
  system("stty -echo"); //don't show mouse events on screen
  system("stty -icanon");
  fprintf(stderr, "\e[?1003h\e[?1015h\e[?1006h");
  char buffer[16] = " ";
  char previousBuffer[16] = " ";

  //Make standard in not be blocking
  int flags = fcntl(0, F_GETFL, 0);
  fcntl(0, F_SETFL, flags | O_NONBLOCK);

  printf("starting...");fflush(stdout);
  //Run for 50 seconds
  for (int hunSeconds = 0; hunSeconds < 500; hunSeconds++) { 
    //read mouse input
    read(0, buffer, 16);
    if (strcmp(buffer, previousBuffer)) {
      // only show event if it is different
      fprintf(stderr, "%s", buffer);
      strncpy(previousBuffer, buffer, 16);
    }
    // sleep for .1 seconds
    usleep(100*1000); 
  }
  printf("ending...");
  //turn off mouse events
  printf("\e[?1000l"); fflush(stdout);
  //turn on screen echo again
  system("stty echo"); 
  return 0;
}
