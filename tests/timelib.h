#include <time.h>

/**
 * @brief Find the difference in time between timeinitial and timefinal
 */ 
double timediff(struct timespec ti, struct timespec tf)
{
  return (double)((tf.tv_sec - ti.tv_sec)
      + (tf.tv_nsec - ti.tv_nsec)
      / 1E9);
}

void get_time(struct timespec *time)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, time);
}
