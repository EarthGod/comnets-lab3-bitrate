#include "pool.h"



extern pool_t pool;


void init_pool(int listen_sock, pool_t *p, char** argv) 
{
    int i;
    gettimeofday(&(pool.start), NULL);
    pool.max_conn_idx = -1;
    pool.max_clit_idx = -1;
    pool.max_serv_idx = -1;
    pool.max_thru_idx = -1;
    for (i = 0; i < FD_SETSIZE; i++) 
	{
        pool.conn_l[i] = NULL;
        pool.client_l[i] = NULL;
        pool.server_l[i] = NULL;
    }

    pool.maxfd = listen_sock;
    pool.cur_conn = 0;
    pool.log_file = fopen(argv[1],"w");
    if (pool.log_file == NULL) 
	{
        DPRINTF("failed to open log file!\n");
        exit(-1);
    }
    pool.fake_ip = argv[4];
    sscanf(argv[2],"%f",&(pool.alpha));
    pool.cur_conn = 0;
    pool.cur_client = 0;
    pool.cur_server = 0;

    if(argv[7])
        pool.www_ip = argv[7];
    fprintf(stderr, "\n");
    FD_ZERO(&(pool.read_nrdy));
    FD_ZERO(&(pool.write_nrdy));
    FD_SET(listen_sock, &(pool.read_nrdy));
    pool.maxfd = listen_sock;
}


int open_listen_socket(int port) 
{
    int listen_socket;
    int yes = 1;        // for setsockopt() SO_REUSEADDR
    struct sockaddr_in addr;

    /* all networked programs must create a socket */
    if ((listen_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
	{
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    // lose the pesky "address already in use" error message
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* servers bind sockets to ports--notify the OS they accept connections */
    if (bind(listen_socket, (struct sockaddr *) &addr, sizeof(addr))) 
	{
        close_socket(listen_socket);
        fprintf(stderr, "Failed binding socket for port %d.\n", port);
        return EXIT_FAILURE;
    }


    if (listen(listen_socket, LISTENQ)) 
	{
        close_socket(listen_socket);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    return listen_socket;
}


int open_server_socket(char *fake_ip, char *www_ip, int port) 
{
    int serverfd;
    struct addrinfo *result = NULL;
    struct sockaddr_in fake_addr;
    struct sockaddr_in serv_addr;
    int rc;
    assert(fake_ip != NULL);

    /* Create the socket descriptor */
    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
        return -1;
    }

    //fcntl(serverfd, F_SETFL, O_NONBLOCK);
    
    memset(&fake_addr, '0', sizeof(fake_addr)); 
    fake_addr.sin_family = AF_INET;
    inet_pton(AF_INET, fake_ip, &(fake_addr.sin_addr));
    fake_addr.sin_port = htons(0);  // let system assgin one 
    rc = bind(serverfd, (struct sockaddr *)&fake_addr, sizeof(fake_addr));
    if (rc < 0) 
	{
        DPRINTF("Bind server sockt error!");
        return -1;
    }

    // server ip is specified
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    inet_pton(AF_INET, www_ip, &(serv_addr.sin_addr));
    serv_addr.sin_port = htons(port);
    rc = connect(serverfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (rc < 0) 
	{
        // handle error
        DPRINTF("Connect error!\n");
        return -1;
    }
    /* Clean up */
    if (result)
        free(result);
    int nonblock_flags = fcntl(serverfd,F_GETFL,0);
    fcntl(serverfd, F_SETFL,nonblock_flags|O_NONBLOCK);
    return serverfd;    
}

int add_client(int conn_sock, uint32_t addr) 
{
    int i;
    client_t* new_client;
    client_t** client_l = pool.client_l;

    for (i = 0; i < FD_SETSIZE; i++)
        if (client_l[i] == NULL) 
		{
            new_client = (client_t*)malloc(sizeof(client_t));
            new_client->cur_size = 0;
            new_client->size = BUF_SIZE;
            new_client->fd = conn_sock;
            new_client->addr = addr;
            new_client->num_serv = 0;
            client_l[i] = new_client;
            pool.cur_client++;

            FD_SET(conn_sock, &(pool.read_nrdy));
            FD_SET(conn_sock, &(pool.write_nrdy));
            if (conn_sock > pool.maxfd) 
                pool.maxfd = conn_sock;
            if (i > pool.max_clit_idx) 
                pool.max_clit_idx = i;
            return i;
        }


    /* failed to add new server */
    return -1;
}



int add_server(int sock, uint32_t addr) 
{
    server_t* new_server;
    server_t** serv_l = pool.server_l;
    int i = 0;

    for(; i < FD_SETSIZE;i++) 
	{

        if (serv_l[i] == NULL) 
		{
            new_server = (server_t*)malloc(sizeof(server_t));
            new_server->fd = sock;
            new_server->addr = addr;
            new_server->cur_size = 0;
            new_server->size = MAXBUF;
            new_server->num_clit = 0;
            serv_l[i] = new_server;
            pool.cur_server++;
            
            FD_SET(sock, &(pool.read_nrdy));
            FD_SET(sock, &(pool.write_nrdy));
            if (sock > pool.maxfd) 
                pool.maxfd = sock;
            if (i > pool.max_serv_idx) 
                pool.max_serv_idx = i;
            return i;
        }
    }

    /* failed to add new server */
    return -1;
}






int get_client(uint32_t addr) 
{
    int i = 0;
    client_t** cur_c = pool.client_l; 
    for( i = 0; i < FD_SETSIZE; i++) 
	{
        if (cur_c[i] == NULL)
            continue;
        if (cur_c[i]->addr == addr)
            return i;
    }
    return -1;
}

int get_server(int sock) 
{
    int i = 0;
    server_t** cur_s = pool.server_l;
    for( i = 0; i < FD_SETSIZE; i++) 
	{
        if (cur_s[i] == NULL)
            continue;
        if (cur_s[i]->fd == sock)
            return i;
    }
    return -1;
}


int update_client(int sock, uint32_t addr) 
{
        int i = 0;
    client_t** cur_c = pool.client_l; 
    for( i = 0; i < FD_SETSIZE; i++) 
	{
        if (cur_c[i] == NULL)
            continue;
        if (cur_c[i]->addr == addr) 
		{
            cur_c[i]->fd = sock;
            FD_SET(sock,&(pool.read_nrdy));
    
            return 0;
        }
    }
    // should never reach here!
    DPRINTF("wants to update, but no client found!\n");
    return -1;   
}

int update_server(int sock, uint32_t addr) 
{
    int i = 0;
    server_t** cur_s = pool.server_l;
    for( i = 0; i < FD_SETSIZE; i++) 
	{
        if (cur_s[i] == NULL)
            continue;
        if (cur_s[i]->addr == addr) 
		{
            cur_s[i]->fd = sock;
            FD_SET(sock,&(pool.read_nrdy));
            return 0;
        }
    }
    DPRINTF("wants to update, but no server found!\n");
    return -1;
}

void close_clit(int clit_idx) 
{
    DPRINTF("close client:%d\n",clit_idx);
    client_t *client = GET_CLIT_BY_IDX(clit_idx);
    close_socket(client->fd);
    FD_CLR(client->fd, &(pool.read_nrdy));
    FD_CLR(client->fd, &(pool.write_nrdy));
    free(client);
    GET_CLIT_BY_IDX(clit_idx) = NULL;
    pool.cur_client--;
}

void close_serv(int serv_idx) 
{
    DPRINTF("close server:%d\n",serv_idx);
    server_t *server = GET_SERV_BY_IDX(serv_idx);
    close_socket(server->fd);
    FD_CLR(server->fd, &(pool.read_nrdy));
    FD_CLR(server->fd, &(pool.write_nrdy));
    free(server);
    GET_SERV_BY_IDX(serv_idx) = NULL;
    pool.cur_server--;    
}


int close_socket(int sock) 
{
    DPRINTF("Close sock %d\n", sock);
    //log_write_string("Close sock %d\n", sock);
    if (close(sock)) 
	{
        DPRINTF("Failed closing socket.\n");
        return 1;
    }
    return 0;
}

void clean_state(pool_t *p, int listen_sock) 
{
    return;
}