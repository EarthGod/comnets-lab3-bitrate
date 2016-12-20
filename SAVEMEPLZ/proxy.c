/*
 * @file proxy.cpp
 * @brief Proxy mentioned
 * @author Li Yanhao <1400012849@pku.edu.cn>
 */

#include <signal.h>
#include <sys/wait.h>
#include "socket.h"
#include "macro.h"
#include "pool.h"
// define configuration
// refrence from lab3cp1ref.py
#define ARGCNT 7
typedef struct conf_struct{
	char* log;
	double alpha;
	int port_listen;
	char* ip_fake;
	char* ip_dns;
	int port_dns;
	char* ip_www;
	int port_www;
}conf_type;
static conf_type conf;
static int sock = -1;

int main(int argc, char** argv){
	// parsing configuration from argv
	if (argc != ARGCNT + 1 && argc != ARGCNT){
		fprintf(stderr, "Usage: %s <log> <alpha> <listen-port> <fake-ip> <dns-ip> <dns-port> [<www-ip>]\n", argv[0]);
		return -1;
	}
	conf.log = argv[1];
	conf.alpha = atof(argv[2]);
	conf.port_listen = atoi(argv[3]);
	conf.ip_fake = argv[4];
	conf.ip_dns = argv[5];
	conf.port_dns = atoi(argv[6]);
	conf.ip_www = (argc == ARGCNT + 1) ? argv[7] : NULL;
	conf.port_www = 8080; 

	//open listener socket 
	// TODO: IMPLEMENT CONNECTION POOL!!!!!!!!
	sock = open_listener_socket(conf.port_listen, MAXCONN);
	if (sock < 0){
		fprintf(stderr, "open socket error!\n");
		return -1;
	}
	return 0;
}