#include <stdio.h>
#include <sys/time.h>

void hide() { printf("\e[?25l"); }
void show() { printf("\e[?25h"); }

void save() { printf("\e7"); }
void restore() { printf("\e8"); }

void gotorc(int r, c) { printf("\e[%d;%dH", r, c); }

int spin_n= 0;

void spin(int sig) {
  //if (spin_n < 0) return;

  static char* states[]= {
    "🌍",
    "🌎",
    "🌏",
  };
  spin_n++;

  hide(); save(); {
    gotorc(0, 0);
    printf("%s", states[spin_n % 3]);
  } restore(); show();
  fflush(0);
  signal(sig, spin);
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
  else 
    printf("timer init succeeded\n");
}

void stop_spin() {
  spin_n = -10;
}

int main(void) {
  start_spin();

  while(1);

  return 0;
}
