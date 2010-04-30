#ifndef __STOPWATCH_H
#define __STOPWATCH_H

//from the HPX project

#include <sys/time.h>

/* simple stopwatch class */
class HStopWatch {
 private:
  struct timeval tv;
  bool running;
    
 public:
  HStopWatch(void);
  void start(void); /* start the stopwatch */
  long split(void); /* get value, but don't stop */
  long stop(void); /* stop the stopwatch and get the value in usecs */
};

#endif /* __STOPWATCH_H */
