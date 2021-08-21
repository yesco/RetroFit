#include <stdio.h>
#include <time.h>
#include <assert.h>

char *timeago(time_t epoch) {
  static char r[50];
  time_t now= time(NULL);

  long ss= now-epoch;
  long s= ss<0? -ss: ss;
  long m= (s+30)/60;
  long h= (m+30)/60;
  int d= (h+12)/24;
  int w= (2*d+7)/14;
  int M= (d+15)/30;
  int Y= (d+365/2)/365;
  int D= (Y+5)/10;
  int H= (Y+50)/100;

  char* p= ss<0? strcpy(r, "in ")+3 : r;

  if (0) ;
  else if (s <= 90) sprintf(p, "%ld s", s);
  else if (m <= 90) sprintf(p, "%ld minutes", m);
  else if (h <= 72) sprintf(p, "%ld hours", h);
  else if (d <= 30) sprintf(p, "%d day", d);
  else if (w <= 11) sprintf(p, "%d weeks", w);
  else if (M <= 18) sprintf(p, "%d months", M);
  else if (Y <= 64) sprintf(p, "%d years", Y);
  else if (D <= 9) sprintf(p, "%d decades", D);
  else sprintf(p, "%d hundred years", H);
  if (ss>0) strcat(r, " ago");
  return r;
}

char *isoago(char *iso) {
  struct tm tm = {0};

  // fill in current time so we can parse partials (hh:mm)!
  time_t now= time(NULL);
  struct tm *p = localtime(&now);
  if (p) tm= *p;

  // it parses as much as it can
  if (strptime(iso, "%H:%M", &tm))
    return timeago(mktime(&tm));
  else if (strptime(iso, "%Y-%m-%dT%H:%M:%S", &tm) || 
      strptime(iso, "%Y-%m-%d %H:%M:%S", &tm) ||
      strptime(iso, "%Y", &tm) )
    return timeago(mktime(&tm));
  else
    return iso;
}

// https://stackoverflow.com/questions/1002542/how-to-convert-datetime-to-unix-timestamp-in-c

int main(void) {
  char *timestamps[]= {
    "2017-03-21T01:00:00",
    "1968-07-12T16:34:00",
    "2021-08-12T16:34:00",
    "1000-01-01T00:00:00",
    "2021-03-01T00:00:00",
    "2022-03-01T00:00:00",
    "2020-02-19",
    "1968",
    "17:34",
  };

  for(int i=0; i<sizeof(timestamps)/sizeof(timestamps[0]); i++) {
    char* ago= isoago(timestamps[i]);
    printf("%s -- %s\n", timestamps[i], ago);
  }
}

