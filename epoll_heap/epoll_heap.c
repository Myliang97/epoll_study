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

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"
#define HEAPSIZE 1000
static int epfd;   //红黑树根节点
typedef struct
{
	/* data */
	int fd;           //文件描述符
	int event;		  //事件
	void *arg;
	void (*callback)(int fd,int event,void *arg);
	char buf[BUFSIZ];
	int size;
	int status;   //是否在树上
}eventNode;
static eventNode nodes[HEAPSIZE+1];
/*
反应堆，利用epoll函数，和结构体指针
设置回调函数，使得每个连接独自调用回调函数
 */

void initTCP();
void setEventNode(eventNode *node,int fd,int event,void *arg,void (*callback)(int,int,void *));
void addEvent(eventNode *node);
void delEvent(eventNode *node);
void revData(int fd,int event,void *arg);
void sendData(int fd,int event,void *arg);
void acceptClients(int fd,int event,void *arg);

int main()
{
	epfd=epoll_create(HEAPSIZE);
	initTCP();
	struct epoll_event events[HEAPSIZE];
	int i;
	printf("server start...\n");
	while(1)
	{
		int num=epoll_wait(epfd,events,HEAPSIZE,-1);
		for(i=0;i<num;++i)           //调用回调函数
		{
			eventNode *node=(eventNode*)events[i].data.ptr;
			if(events[i].events ==EPOLLIN && node->event==EPOLLIN)
				node->callback(node->fd,node->event,node->arg);
			if(events[i].events==EPOLLOUT && node->event==EPOLLOUT)
				node->callback(node->fd,node->event,node->arg);
		}
	}	
	return 0;
}

void setEventNode(eventNode *node,int fd,int event,void *arg,void (*callback)(int,int,void *))
{
	node->fd=fd;
	node->event=event;
	node->arg=arg;
	//memset(node->buf,0,BUFSIZ);     //*****读完数据，在memset导致buf为空
	//node->size=0;
	node->status=0;
	node->callback=callback;
}
void addEvent(eventNode *node)     //添加至监听红黑树
{
	struct epoll_event evnt={0,{0}};
	evnt.events=node->event;
	evnt.data.ptr=node;
	if(node->status)     //已经在树上
	{
		epoll_ctl(epfd,EPOLL_CTL_MOD,node->fd,&evnt);
	}
	else
	{
		epoll_ctl(epfd,EPOLL_CTL_ADD,node->fd,&evnt);
		node->status=1;
	}
}
void delEvent(eventNode *node)
{
	if(!node->status)
		return;
	struct epoll_event evnt;
	evnt.events=node->event;
	evnt.data.ptr=node;
	epoll_ctl(epfd,EPOLL_CTL_DEL,node->fd,&evnt);
}
void revData(int fd,int event,void *arg)
{
	printf("reading data...\n");
	eventNode *node=(eventNode *)arg;
	ssize_t size=0;
	size=read(fd,node->buf,BUFSIZ);
	delEvent(node);
	if(size==0)    //客户端退出
	{
		printf("quit\n");
		close(fd);
	}
	else if(size>0)      //接收完数据后，把监听事件该为写
	{
		printf("data: %s",node->buf);
		node->buf[size]=0;    
		node->size=size;
		setEventNode(node,fd,EPOLLOUT,node,sendData);
		addEvent(node);
	}
	else
	{
		perror("read");
	}
}
void sendData(int fd,int event,void *arg)
{
	printf("writing data...\n");
	eventNode *node=(eventNode *)arg;
	ssize_t size=write(fd,node->buf,node->size);
	delEvent(node);
	if(size>0)      //写完数据把监听事件该为读
	{
		memset(node->buf,0,BUFSIZ);
		setEventNode(node,fd,EPOLLIN,node,revData);
		addEvent(node);
	}
	else if(size==-1)
	{
		perror("write");
	}
	else
	{
		close(fd);
		delEvent(node);
	}
	
}
void acceptClients(int fd,int event,void *arg)               
{
	struct sockaddr_in client_addr;
	socklen_t len=sizeof(client_addr);
	int cfd=accept(fd,(struct sockaddr *)&client_addr,&len);

	int flag=fcntl(cfd,F_GETFL);
	flag |=O_NONBLOCK;
	fcntl(cfd,F_SETFL,flag);

	int i;
	for(i=0;i<=HEAPSIZE;++i)
	{
		if(!nodes[i].status)
			break;
	}
	printf("cfd:%d\n",cfd);
	char client_ip[BUFSIZ];
	inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,client_ip,len);
	printf("client: ip:%s,port:%d\n",client_ip,ntohs(client_addr.sin_port));
	setEventNode(&nodes[i],cfd,EPOLLIN,&nodes[i],revData);
	addEvent(&nodes[i]);
}
void initTCP()
{
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	fcntl(sfd,F_SETFL,O_NONBLOCK);    //设置非阻塞
	setEventNode(&nodes[HEAPSIZE],sfd,EPOLLIN,&nodes[HEAPSIZE],acceptClients);
	addEvent(&nodes[HEAPSIZE]);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET,SERV_IP,&serv_addr.sin_addr.s_addr);
	bind(sfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	listen(sfd,100);
}


