#include "timer.h"
#include "debug.h"

double get_time_since(struct timeval* start) 
{
	struct timeval now;
	double t1 = start->tv_sec+(start->tv_usec/1000000.0);
	double t2;
	gettimeofday(&now, NULL);
	t2=now.tv_sec+(now.tv_usec/1000000.0);
	t2 = t2 - t1;
	//printf("T2 = %f", t2);
	return t2;
}

double get_diff(struct timeval* start, struct timeval* end) 
{
	double t1 = start->tv_sec+(start->tv_usec/1000000.0);
	double t2 = end->tv_sec+(end->tv_usec/1000000.0);
	return (t2 - t1);	
}