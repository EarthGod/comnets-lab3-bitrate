#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdint.h>
#include <stdlib.h>

#define GET_CONN_BY_IDX(idx) (pool.conn_l[idx]);
#define GET_THRU_BY_IDX(idx) (pool.thru_l[idx]);

#define MAX_CONN 2048 
#define MAX_FILE_NAME 8192 




typedef struct client_s 
{
    int fd;     
    unsigned int cur_size; 
    unsigned int size;   
    uint32_t addr;
    int num_serv;
} client_t;

typedef struct server_s 
{
	int fd;
	unsigned int cur_size; 
    unsigned int size;     
	uint32_t addr;
	int num_clit;
} server_t;

typedef struct conn_s 
{
	int serv_idx;
	int clit_idx;
	int t_put; 
	int avg_put; //ewma, kBps
	int cur_bitrate;
	char cur_file[MAX_FILE_NAME];
	char cur_size;
	struct timeval start;
	struct timeval end;
	int alive; /* 1 when connection is alive; 0 when connection is closed */
} conn_t;

typedef struct response_s 
{
	int length;
	int type;
	char *hdr_buf;
	int hdr_len;
	int close;
} response_t;
	
typedef struct thruputs_s 
{
	uint32_t clit_addr;
	uint32_t serv_addr;
	int avg_put; // ewma, kBps
} thruputs_t;


int server_get_conn(int );
int client_get_conn(int ,uint32_t);
int add_conn(int, int);
int update_conn(int clit_idx, int serv_idx);
void close_conn(int);
int update_thruput(int, conn_t*, thruputs_t* thru);

int get_thru_by_addrs(uint32_t clit_addr, uint32_t serv_addr);
int add_thru(uint32_t clit_addr, uint32_t serv_addr);
#endif