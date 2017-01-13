#ifndef _TIMER_H
#define _TIMER_H

#include <sys/time.h>
#include <stdlib.h>

double get_time_since(struct timeval* start);
double get_diff(struct timeval* start, struct timeval* end);
#endif