#ifndef UTIL_H
#define UTIL_H

#include <sys/time.h>
#include <time.h>

inline double tv_to_double(struct timeval *tv) {
  return tv->tv_sec + (double) tv->tv_usec / 1000000;
}

inline void double_to_tv(double val, struct timeval *tv) {
  long long secs = (long long) val;
  long long usecs = (long long) ((val - secs) * 1000000);

  tv->tv_sec = secs;
  tv->tv_usec = usecs;
}

inline double get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv_to_double(&tv);
}

#endif
