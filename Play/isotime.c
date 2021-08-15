#include <stdio.h>
#include <time.h>
#include <locale.h>
 
int main(void) {
    char buff[70];
    struct tm my_time = {
      .tm_year=112, // = year 2012
      .tm_mon=9,    // = 10th month
      .tm_mday=9,   // = 9th day
      .tm_hour=8,   // = 8 hours
      .tm_min=10,   // = 10 minutes
      .tm_sec=20    // = 20 secs
    };
 
    time_t tm= time(NULL);
    my_time= *localtime(&tm);
    // https://man7.org/linux/man-pages/man3/strftime.3.html
    if (strftime(buff, sizeof buff, "%A %c", &my_time)) {
      puts(buff);
    } else {
      puts("strftime failed");
    }
 
    //setlocale(LC_TIME, "el_GR.utf8");
 
    // > date --iso=s
    // 2021-08-15T02:50:58+07:00
    if (1 && strftime(buff, sizeof buff, "%FT%T%z", &my_time)) {
      puts(buff);
    } else {
      puts("strftime failed");
    }

    struct tm xxx;
    time_t t= time(NULL);
    xxx= *localtime(&t);
    if (strftime(buff, sizeof(buff), "%FT%T%z", &xxx)) {
      puts(buff);
    } else {
      puts("strftime failed");
    }
}
