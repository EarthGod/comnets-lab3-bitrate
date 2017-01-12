#include "mydns.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>




int main() {
	//data_packet_t* pkt = make_query_pkt("video.pku.edu.cn");
	//char buf[BUFSIZE];
	//pktToBuf(buf, pkt);
	int res;
	struct addrinfo *servinfo;
	char ipstr[INET_ADDRSTRLEN];

	res = init_mydns("127.0.0.1", 9999);

	if (res == -1) {
		fprintf(stderr, "Init dns error\n");
	}

	res = resolve("video.pku.edu.cn", "8888", NULL, &servinfo);

	if (res == -1) {
		fprintf(stderr, "resolve error\n");
	}
 
	struct sockaddr_in *ipv4 = (struct sockaddr_in*)servinfo->ai_addr;

	inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
	fprintf(stderr, "%s\n", ipstr);
	
	return 0;
}