#include "stopwatch.h"

#include <sys/time.h>
#include <iostream>

HStopWatch::HStopWatch(void) {
  running = false;
}

void HStopWatch::start(void) {
  if (gettimeofday(&tv, NULL) < 0) {
    std::cout << "stopwatch failed" <<std::endl;
    return;
  }
  running = true;
}

long HStopWatch::split(void) {
  struct timeval tv_stop;
  if (gettimeofday(&tv_stop, NULL) < 0) {
    std::cout << "stopwatch failed" <<std::endl;
    return -1;
  }
  if (tv_stop.tv_usec < tv.tv_usec) {
    tv_stop.tv_sec--;
    tv_stop.tv_usec += 1000000;
  }

  return ((tv_stop.tv_sec - tv.tv_sec) * 1000000 
	  + tv_stop.tv_usec - tv.tv_usec);
}

long HStopWatch::stop(void) {
  if (running == false) {
    std::cout << "Stopwatch already not running." << std::endl;
    return -1;
  }
  running = false;
  return split();
}
