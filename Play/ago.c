#include <stdio.h>
#include <time.h>
#include <assert.h>

// https://stackoverflow.com/questions/1002542/how-to-convert-datetime-to-unix-timestamp-in-c
int main(void) {
  char *timestamp;
  timestamp= "2017-03-21T01:00:00";
  timestamp= "1968-07-12T16:34:00";
  timestamp= "1000-01-01T00:00:00";
  struct tm tm;
  time_t epoch;

  if ( strptime(timestamp, "%Y-%m-%dT%H:%M:%S", &tm) != NULL ) {
    epoch = mktime(&tm);
    printf("%s   --   %ld\n", timestamp, epoch);

    time_t now= time(NULL);
    long s= now-epoch;
    long m= (s+30)/60;
    long h= (m+30)/60;
    int d= (h+12)/24;
    int w= (2*d+7)/14;
    int M = (d+15)/30;
    int Y = (d+365/2)/365;
    int D = (Y+5)/10;
    int H = (Y+50)/100;
    if (0) ;
    else if (s <= 90) printf("%ld s", s);
    else if (m <= 90) printf("%ld minutes", m);
    else if (h <= 72) printf("%ld hours", h);
    else if (d <= 30) printf("%d day", d);
    else if (w <= 11) printf("%d weeks", w);
    else if (M <= 18) printf("%d months", M);
    else if (Y <= 64) printf("%d years", Y);
    else if (D <= 9) printf("%d decades", D);
    else printf("%d hundred years", H);
    printf(" ago\n");
  } else {
    assert(!"strptime failed");
  }
}
