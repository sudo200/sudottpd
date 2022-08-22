#include "http_utils.h"

static const char *day2string[] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

static char buffer[31];

http_header_t get_date_header()
{
  time_t t;
  struct tm pt = {};
  time(&t);
  gmtime_r(&t, &pt);
  sprintf(buffer, "%s, %02d %02d %04d %02d:%02d:%02d GMT",
      day2string[pt.tm_wday],
      pt.tm_mday,
      pt.tm_mon,
      pt.tm_year + 1900,
      pt.tm_hour % 24,
      pt.tm_min,
      pt.tm_sec
      );
  return (http_header_t) {
    .name = "Date", .value = buffer
  };
}

