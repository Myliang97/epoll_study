#include "common.h"
ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t ret;
    ret=write(fd,buf,count);
    if(ret==-1)
    {
        perror("write");
        exit(1);
    }
    return ret;
}
ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t ret;
    ret=read(fd,buf,count);
    if(ret==-1)
    {
        perror("read");
    }
    return ret;
}
int Socket(int domain, int type, int protocol)
{
	int ret=socket(domain,type,protocol);
	if(ret==-1)
	{
		perror("socket");
        exit(1);
	}
	return ret;
}
int Connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    int ret=connect(sockfd,addr,addrlen);
    if(ret==-1)
    {
        perror("connect");
        exit(1);
    }
    return ret;
}
int Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    int ret=bind(sockfd,addr,addrlen);
    if(ret ==-1)
    {
        perror("bind");
        exit(1);
    }
    return ret;
}
int Listen(int sockfd, int backlog)
{
    int ret=listen(sockfd,backlog);
    if(ret ==-1)
    {
        perror("listen");
        exit(1);
    }
    return ret;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;
    ret=accept(sockfd,addr,addrlen);
    if(ret==-1 && errno !=EAGAIN && errno !=EINTR)
    {
        perror("accept");
        exit(1);
    }
    return ret;
}

char *Fgets(char *s, int size, FILE *stream)
{
    s=fgets(s,size,stream);
    if(!s)
    {
        printf("fgets error \n");
        exit(1);
    }
    return s;
}