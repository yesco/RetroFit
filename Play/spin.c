#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

void hide() { printf("\e[?25l"); }
void show() { printf("\e[?25h"); }

void save() { printf("\e7"); }
void restore() { printf("\e8"); }

void gotorc(int r, c) { printf("\e[%d;%dH", r, c); }

int spin_n= 0;

void spin(int sig) {
  static char* states[]= {
    "🌍",
    "🌎",
    "🌏",
  };
  spin_n++;

  hide(); save(); {
    gotorc(0, 0);
    if (!sig || spin_n < 0)
      printf("😡"),
      printf("🏠");
    else
      printf("%s", states[spin_n % 3]);
  } restore(); show();
  fflush(0);
}

#define SEC 0
#define USEC 250000

struct itimerval new, old= {0};

void start_spin() {
  signal (SIGALRM, spin);

  new.it_interval.tv_sec = SEC;
  new.it_interval.tv_usec = USEC;
  new.it_value.tv_sec = SEC;
  new.it_value.tv_usec = USEC;
  
  if (setitimer(ITIMER_REAL, &new, &old) < 0)
    printf("timer init failed\n");
}

void stop_spin() {
  spin_n = -10;
  spin(0);
  setitimer(ITIMER_REAL, &old, &new);
}

int main(void) {
  start_spin();

  while(1);
  //while(spin_n < 10) ;

  // TODO: stop at signal
  //stop_spin();
  //while(1) ;

  return 0;
}
