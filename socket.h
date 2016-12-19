/*
 * @file socket.h
 * @brief Socket helper functions
 * @author Li Yanhao <1400012849@pku.edu.cn>
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

/**
 * @brief: Open a listener socket on a certain port.
 *
 * @param port: Port to listen on.
 * @param max_conn: Max number of connections allowed.
 *
 * @return: The listener socket if success.
 *         -1 if error occurs.
 */
int open_listener_socket(int port, int max_conn);

/**
 * @brief: Open a connection socket with remote server.
 *
 * @param fake_ip: Fake ip used locally
 * @param remote_ip: Remote ip of server.
 * @param remote_port: Remote port of server.
 *
 * @return: The socket just opened.
 *         -1 if error occurs.
 *
 * Port 0 is hard coded as local port.
 */
int open_server_socket(char* fake_ip, char* remote_ip, int remote_port);

/**
 * @brief: Open a udp listner socket on certain ip:port.
 *
 * @param ip: IP to bind.
 * @param port: Port to bind.
 *
 * @return: The udp listener socket if success.
 *         -1 if error occurs.
 */
int open_udp_listener_socket(char* ip, int port);

/**
 * @brief: Open a udp socket with remote server.
 *
 * @param fake_ip: Fake ip used locally
 * @param remote_ip: Remote ip of server.
 * @param remote_port: Remote port of server.
 *
 * @return: The socket just opened.
 *         -1 if error occurs.
 *
 * Port 0 is hard coded as local port.
 */
int open_udp_server_socket(char* fake_ip, char* remote_ip, int remote_port);

/**
 * @brief: str repr of sockaddr
 *
 * @param addr: Addr to be represented.
 *
 * The ret str is in a static buffer; not thread safe.
 */
const char* str_addr(const struct sockaddr_in* addr);

/**
 * @brief: Fill ip:port into addr.
 *
 * @param addr: Addr to be Filled in.
 * @param ip: IP to fill in.
 * @param port: Port to fill in.
 */
void fill_addr(struct sockaddr_in* addr, const char* ip, int port);

#endif
