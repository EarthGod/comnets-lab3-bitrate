#include "log.h"
#include <stdlib.h>
#include <time.h>

extern pool_t pool;

//write down activities on logfile as required
void loggin(conn_t* conn, thruputs_t* thru) 
{
	time_t since_epoch;
	since_epoch = time(NULL);
	double secs = get_diff(&(conn->start),&(conn->end));
	FILE* logfile = pool.log_file;
	server_t* server = pool.server_l[conn->serv_idx];
	
	struct in_addr ip_addr;
	ip_addr.s_addr = server->addr;

	fprintf(logfile, "%ld ", since_epoch);
	fprintf(logfile, "%lf ", secs);
	fprintf(logfile, "%d ", conn->t_put);
	fprintf(logfile, "%d ", thru->avg_put);
	fprintf(logfile, "%d ", conn->cur_bitrate);
	fprintf(logfile, "%s ", inet_ntoa(ip_addr));
	fprintf(logfile, "%s\n", conn->cur_file );

	fflush(pool.log_file);
}



