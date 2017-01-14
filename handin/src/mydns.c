#include "mydns.h"
#include <errno.h>
/* globle varibal for dns service */
dns_t dns;

/**
 * Initialize your client DNS library with the IP address and port number of
 * your DNS server.
 *
 * @param  dns_ip    The IP address of the DNS server.
 * @param  dns_port  The port number of the DNS server.
 * @param  local_ip  The local ip address client sockets should bind to
 *
 * @return 0 on success, -1 otherwise
 */
 
int init_mydns(const char* dns_ip, unsigned int dns_port, const char* local_ip) 
{
	int sock, yes = 1;
	struct sockaddr_in myaddr;
	
	DEBUGPRINT("Entering init_mydns\n");
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) 
	{
		DEBUGPRINT("init_mydns could not create socket");
		exit(-1);
	}
	
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	inet_pton(AF_INET, local_ip, &(myaddr.sin_addr));
	myaddr.sin_port = htons(0);
	
	// lose the pesky "address already in use" error message
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) 
	{
		DEBUGPRINT("init_mydns could not bind socket, %s\n", strerror(errno));
		exit(-1);
	}
	
	/* fill the address of DNS server */
	dns.sock = sock;
	memset((char*)&(dns.servaddr), 0, sizeof(dns.servaddr));
	dns.servaddr.sin_family = AF_INET;
	dns.servaddr.sin_port = htons(dns_port);
	inet_pton(AF_INET, dns_ip, &(dns.servaddr.sin_addr));
	
	DEBUGPRINT("Exiting init_mydns\n");
	return 0;
}



void freeMyAddrinfo(struct addrinfo* addr) 
{
	free(addr->ai_addr);
	free(addr);
}

//generating query packet
data_packet_t* make_query_pkt(const char* node) 
{
	data_packet_t* tmp = (data_packet_t*) malloc(sizeof(data_packet_t));
	tmp->header = (header_t*) malloc(sizeof(header_t));
	tmp->query = (query_t*) malloc(sizeof(query_t));
	tmp->query->qname = (char*)malloc(strlen(node)+2);  // one for head and one for tail
	tmp->query->question = (question_t*) malloc(sizeof(question_t));
	tmp->response = NULL;

	header_t* header = tmp->header;
	query_t* query = tmp->query;
	question_t* question = query->question;
	char* name = query->qname;

	// generate header
	srand(time(NULL));
	header->id = (uint16_t)rand();

	//header->FLAG = 0; // 0 0000 0 0 0 0 000 0000
	header->qr = 0;  // a query
	header->opcode = 0;
	header->aa = 0;
	header->tc = 0;
	header->rd = 0;

	header->ra = 0;
	header->z = 0;
	header->rcode = 0;

	header->qdcount = 1;
	header->ancount = 0;
	header->nscount = 0;
	header->arcount = 0;
	// generate data 
	dot2len(name,node);

	// set up data attribute
	question->qtype = 1;
	question->qclass = 1;
	
	return tmp;
}



int parse_res(char* req_buf, char* res_buf, struct addrinfo* tmp, int length) 
{
	char* ip;
	uint32_t* ip_int;
	char* qname;
	header_t* hdr = (header_t*)req_buf;
	int offest;

	hdr->ancount = htons(1);
	hdr->qr = 1;
	hdr->aa = 1;

	if (memcmp(req_buf, res_buf, length)) 
	{
		DEBUGPRINT("parse_res: Incorrect response!\n");
		return -1;
	}


	qname = (char*)(req_buf + sizeof(header_t));
	offest = length + strlen(qname) + sizeof(answer_t) - 1; // padding
	ip = res_buf + offest;
	ip_int = (uint32_t*)ip;
	((struct sockaddr_in*)tmp->ai_addr)->sin_addr.s_addr = *ip_int;
	return 0;
}

//dot to len
void dot2len(char* name, const char* const_src) 
{
	int i = 0, pos = 0;
	int length = strlen(const_src);
	char src[length+1];
	memcpy(src, const_src, length);
	src[length] = '.';

	for(i = 0; i <= length; i++) 
	{
		if (src[i] == '.') 
		{
			*name = i - pos;
			name++; 
			for (; pos < i; pos++) 
			{
				*name = src[pos];
				name++;
			}
			pos++;
		}
	}
	*name = '\0';
}

//packet to buf
int pktToBuf(char* buf, data_packet_t* pkt) 
{
	int index  = 0, length = 0;
	dns_response_t* res = pkt->response;

	hostToNet(pkt);

	length = sizeof(header_t);
	memcpy(buf+index,pkt->header,length);	
	index += length;

	length = strlen(pkt->query->qname)+1;
	memcpy(buf+index,pkt->query->qname,length);
	index += length;

	length = sizeof(question_t);
	memcpy(buf+index,pkt->query->question,length);
	index += length;

	if (res) 
	{
		length = strlen(res->name) + 1;
		memcpy(buf+index,res->name,length);
		index += length;

		length = sizeof(answer_t);
		memcpy(buf+index,res->answer,length);
		index += length;

		length = sizeof(uint32_t); // ipv4 only
		memcpy(buf+index,res->data,length);
		index += length;
	}

	return index;
}




void hostToNet(data_packet_t* pkt) 
{
	header_t* hdr = pkt->header;
	query_t* qry = pkt->query;
	question_t *q = qry->question;
	dns_response_t* res = pkt->response;

	hdr->id = htons(hdr->id);
	hdr->rcode = htonl(hdr->rcode);
	hdr->qdcount = htons(hdr->qdcount);
	hdr->ancount = htons(hdr->ancount);

	q->qtype = htons(q->qtype);
	q->qclass = htons(q->qclass);

	if (res) 
	{
		answer_t* ans = res->answer;
		ans->atype = htons(ans->atype);
		ans->aclass = htons(ans->aclass);
		ans->attl = htons(ans->attl);
		ans->ardlength = htons(ans->ardlength);
	}
}


void netToHost(data_packet_t* pkt) 
{
	header_t* hdr = pkt->header;
	query_t* qry = pkt->query;
	question_t *q = qry->question;
	dns_response_t* res = pkt->response;

	hdr->id = ntohs(hdr->id);
	hdr->rcode = ntohl(hdr->rcode);
	hdr->qdcount = ntohs(hdr->qdcount);
	hdr->ancount = ntohs(hdr->ancount);

	q->qtype = ntohs(q->qtype);
	q->qclass = ntohs(q->qclass);

	if (res) 
	{
		answer_t* ans = res->answer;
		ans->atype = ntohs(ans->atype);
		ans->aclass = ntohs(ans->aclass);
		ans->attl = ntohs(ans->attl);
		ans->ardlength = ntohs(ans->ardlength);
	}
}

void free_pkt(data_packet_t* pkt) 
{
	free(pkt->query->question);
	free(pkt->query->qname);
	free(pkt->query);
	free(pkt->header);
	if (pkt->response) 
	{
		free(pkt->response->name);
		free(pkt->response->answer);
		free(pkt->response->data);
		free(pkt->response);
	}
	free(pkt);
}


int resolve(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) 
{
	int recvlen = 0;
	int pkt_len = 0;
	struct addrinfo *tmp;
	data_packet_t* pkt;
	char buf[BUFSIZE];
	char res_buf[BUFSIZE];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);


	*res = malloc(sizeof(struct addrinfo));
	tmp = *res;
	memset(tmp, 0, sizeof(struct addrinfo)); // make sure the struct is empty
	tmp->ai_flags = AI_PASSIVE;
	tmp->ai_family = AF_INET;
	tmp->ai_socktype = SOCK_STREAM;
	tmp->ai_protocol = 0;
	tmp->ai_addrlen = sizeof(struct sockaddr_in);
	tmp->ai_addr = malloc(sizeof(struct sockaddr_in));
	tmp->ai_canonname = NULL;
	tmp->ai_next = NULL;

	// generate query pkt
	if((pkt	= make_query_pkt(node)) == NULL) 
	{
		DEBUGPRINT("failed to generate query!\n");
		return 0;
	}

	pkt_len = pktToBuf(buf, pkt);
	DEBUGPRINT("About to send\n");
	// send query to DNS server
	sendto(dns.sock, buf, pkt_len, 0, (struct sockaddr *)&dns.servaddr, 
				sizeof(dns.servaddr));
	DEBUGPRINT("Send DNS\n");

	// recv response from DNS server
	recvlen = recvfrom(dns.sock, res_buf,
		BUFSIZE, 0, (struct sockaddr *)&from, &fromlen);
	DEBUGPRINT("Recv DNS\n");
	if (recvlen == -1) 
	{
		DEBUGPRINT("DNS recv error: %s\n", strerror(errno));
		free_pkt(pkt);
		return -1;
	}

	// parse response
	if (parse_res(buf, res_buf, tmp, pkt_len) != 0 ) 
	{
		DEBUGPRINT("Fail to parse DNS response!");
		free_pkt(pkt);
		return -1;
	}
	free_pkt(pkt);

	// fill up port info
	((struct sockaddr_in*)tmp->ai_addr)->sin_port = htons(atoi(service));
	return 0; 
}
