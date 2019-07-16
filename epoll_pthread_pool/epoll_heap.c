#include "epoll_heap.h"

void init_tcp()
{
    int sfd=Socket(AF_INET,SOCK_STREAM,0);
	int flag=fcntl(sfd,F_GETFL);
	flag |=O_NONBLOCK;
	fcntl(sfd,F_SETFL,flag);    //设置非阻塞
	printf("server fd:%d\n",sfd);
	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	int opt=1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  //IO复用
	Bind(sfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	Listen(sfd,100);
	pthread_t ptd;
	pthread_create(&ptd,NULL,accept_clients,(void*)sfd);
}

void set_event_node(eventNode *node,int fd,int event,void *arg,void (*callback)(int,int,void *))
{
	node->fd=fd;
	node->event=event;
	node->arg=arg;
	node->status=0;
	node->pthread_work=callback;
}
void add_event(eventNode *node)     //添加至监听红黑树
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
void *accept_clients(void *arg)               
{
	int fd=(int)arg;
	struct sockaddr_in client_addr;
	socklen_t len=sizeof(client_addr);
	int cfd=-1;
	while(1)
	{
		while((cfd=Accept(fd,(struct sockaddr *)&client_addr,&len))==-1);
		int flag=fcntl(cfd,F_GETFL);
		flag |=O_NONBLOCK;
		flag |=O_RDWR;
		fcntl(cfd,F_SETFL,flag);
		int i;
		for(i=0;i<=HEAPSIZ;++i)
		{
			if(!nodes[i].status)
				break;
		}
		char client_ip[BUFSIZ];
		inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,client_ip,len);
		printf("client: fd:%d,ip:%s,port:%d\n",cfd,client_ip,ntohs(client_addr.sin_port));
		int event=EPOLLIN;
		event |=EPOLLET;
		set_event_node(&nodes[i],cfd,event,&nodes[i],rev_data);
		add_event(&nodes[i]);
	}
}
void rev_data(int fd,int event,void *arg)
{
	eventNode *node=(eventNode *)arg;
	ssize_t size=0;
	printf("reading....\n");
	size=Read(fd,node->buf,BUFSIZ);
	if(size==0)
	{
		printf("quit\n");
		close(fd);
		return; 
	}
	del_event(node);
	node->buf[size]=0;      //添加'\0'
	node->size=size;
	event=EPOLLOUT;
	event|=EPOLLET;
	set_event_node(node,fd,event,node,send_data);//接收完数据后，把监听事件该为写
	add_event(node);
}
void send_data(int fd,int event,void *arg)
{
	printf("writing....\n");
	eventNode *node=(eventNode *)arg;
	Write(fd,node->buf,node->size);
	del_event(node);
	memset(node->buf,0,BUFSIZ);
	event=EPOLLIN;
	event |=EPOLLET;
	set_event_node(node,fd,event,node,rev_data);//写完数据把监听事件该为读
	add_event(node);
}
void del_event(const eventNode *node)
{
	if(!node->status)
		return;
	struct epoll_event evnt;
	evnt.events=node->event;
	evnt.data.ptr=node;
	epoll_ctl(epfd,EPOLL_CTL_DEL,node->fd,&evnt);
}