#ifndef __COMMON_H__
#define __COMMON_H__
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#define SERV_PORT 6666
#define IP "127.0.0.1"

ssize_t Write(int fd, const void *buf, size_t count);

ssize_t Read(int fd, void *buf, size_t count);

int Socket(int domain, int type, int protocol);

int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);

int Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);

int Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

char *Fgets(char *s, int size, FILE *stream);

#endif