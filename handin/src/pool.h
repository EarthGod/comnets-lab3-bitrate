// modified from ics proxy lab

#ifndef _POOL_H
#define _POOL_H

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "connection.h"
#include "mydns.h"
#include "debug.h"

#define BUF_SIZE 8192 /* Initial buff size */
#define MAXLINE  8192
#define MAXBUF   8192
#define MAX_SIZE_HEADER 8192 /* Max length of size info for the incomming msg */
#define LISTENQ 1024 /* second argument to listen() */



#define GET_SERV_BY_IDX(idx) (pool.server_l[idx])
#define GET_CLIT_BY_IDX(idx) (pool.client_l[idx])


typedef struct pool_s 
{
	FILE* log_file;
	struct timeval start;
	int maxfd;
	int serv_sock;
	float alpha;
	char *fake_ip;
	char *www_ip;
	
	int nready; 
	int cur_conn; 
	int cur_client;	
	int cur_server; 
	int max_conn_idx, max_clit_idx, max_serv_idx, max_thru_idx;
	
	fd_set read_nrdy; /* before recving */
	fd_set read_rdy; /* ready to recv */
	fd_set write_nrdy; /* before sending*/
	fd_set write_rdy; /* ready to write */
	
	client_t* client_l[FD_SETSIZE];
 	server_t* server_l[FD_SETSIZE];
	conn_t* conn_l[FD_SETSIZE]; /* array of points to all connections */
	thruputs_t* thru_l[FD_SETSIZE];
} pool_t;

void init_pool(int, pool_t *,char**);
int open_listen_socket(int);
int open_server_socket(char *, char *, int);
int add_client(int sock, uint32_t addr);
int add_server(int sock, uint32_t addr);
void close_clit(int clit_idx);
void close_serv(int serv_idx);
int close_socket(int);
void clean_state(pool_t *, int);
int get_client(uint32_t addr);
int update_client(int sock, uint32_t);
int get_server(int sock);
int update_server(int sock, uint32_t);
#endif




