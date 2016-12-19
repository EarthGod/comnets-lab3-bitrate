/*
 * @file socket.c
 * @brief Implementation of socket.h
 * @author Li Yanhao <1400012849@pku.edu.cn>
 */

#include "macro.h"
#include "socket.h"

int open_listener_socket(int port, int max_conns){
    // create listener socket
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Failed creating listener socket. Server not started.\n");
        return -1;
    }

    // allow addr reuse
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind port
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0){
        fprintf(stderr, "Failed binding the port %d. Server not started.\n", port);
        close(sock);
        return -1;
    }

    // listen on sock
    if (listen(sock, max_conns) < 0){
        fprintf(stderr, "Error listening on socket. Server not started.\n");
        close(sock);
        return -1;
    }

    // set non-blocking
    int flag = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flag|O_NONBLOCK);
    return sock;
}

int open_server_socket(char* fake_ip, char* remote_ip, int remote_port){
    int sock;
    struct sockaddr_in fake_addr;
    struct sockaddr_in srv_addr;
    bzero(&fake_addr, sizeof(fake_addr));
    bzero(&srv_addr, sizeof(srv_addr));

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "[open_server_socket] Failed to create socket!\n");
        return -1;
    }

    // allow addr reuse
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind addr
    fake_addr.sin_family = AF_INET;
    fake_addr.sin_port = htons(0);
    inet_pton(AF_INET, fake_ip, &fake_addr.sin_addr);
    if (bind(sock, (struct sockaddr*) &fake_addr, sizeof(fake_addr)) < 0){
        fprintf(stderr, "[open_server_socket] Failed to bind %s:%d!\n", fake_ip, 0);
        fprintf(stderr, "[open_server_socket] %s\n", strerror(errno));
        errno = 0;
        close(sock);
        return -1;
    }

    // connect to server
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_ip, &srv_addr.sin_addr);
    if (connect(sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0){
        fprintf(stderr, "[open_server_socket] Failed to connect to %s:%d\n", remote_ip, remote_port);
        close(sock);
        return -1;
    }

    // set non-blocking
    int flag = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flag|O_NONBLOCK);
    return sock;
}

int open_udp_listener_socket(char* ip, int port){
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
        fprintf(stderr, "[open_udp_listener_socket] Failed to create socket!\n");
        return -1;
    }

    // allow addr reuse
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind addr
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0){
        fprintf(stderr, "[open_udp_listener_socket] Failed to bind %s:%d!\n", ip, 0);
        fprintf(stderr, "[open_udp_listener_socket] %s\n", strerror(errno));
        errno = 0;
        close(sock);
        return -1;
    }

    // set non-blocking
    int flag = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flag|O_NONBLOCK);
    return sock;
}

int open_udp_server_socket(char* fake_ip, char* remote_ip, int remote_port){
    int sock;
    struct sockaddr_in fake_addr;
    struct sockaddr_in srv_addr;
    bzero(&fake_addr, sizeof(fake_addr));
    bzero(&srv_addr, sizeof(srv_addr));

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0){
        fprintf(stderr, "[open_udp_server_socket] Failed to create socket!\n");
        return -1;
    }

    // allow addr reuse
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind addr
    fake_addr.sin_family = AF_INET;
    fake_addr.sin_port = htons(0);
    inet_pton(AF_INET, fake_ip, &fake_addr.sin_addr);
    if (bind(sock, (struct sockaddr*) &fake_addr, sizeof(fake_addr)) < 0){
        fprintf(stderr, "[open_udp_server_socket] Failed to bind %s:%d!\n", fake_ip, 0);
        fprintf(stderr, "[open_udp_server_socket] %s\n", strerror(errno));
        errno = 0;
        close(sock);
        return -1;
    }

    // connect to server
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_ip, &srv_addr.sin_addr);
    if (connect(sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0){
        fprintf(stderr, "[open_server_socket] Failed to connect to %s:%d\n", remote_ip, remote_port);
        close(sock);
        return -1;
    }

    // set non-blocking
    int flag = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flag|O_NONBLOCK);
    return sock;
}

#define ADDRLEN 256
const char* str_addr(const struct sockaddr_in* addr) {
    static char buf[ADDRLEN + 1];
    snprintf(buf, ADDRLEN, "%s:%hu", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
    return buf;
}

void fill_addr(struct sockaddr_in* addr, const char* ip, int port) {
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);
}
