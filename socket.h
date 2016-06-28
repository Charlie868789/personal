/*
 * socket.h
 *
 *  Created on: 2011-7-14
 *      Author: root
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdio.h>
#include <strings.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <fcntl.h>

#define MAX_CONNECTION	20
#define MAXDATASIZE 1024
int TCPClientInit( int *clientfd);
int TCPClientConnect( const int clientfd, const char *addr, int port);
int TCPWrite( int clientfd, unsigned char* buf, int size);
int TCPSendFile(char *fns,int socket_fd);
int TCPRevcFile(char *fns,int socket_fd);
int Delay(int times);
#endif
