#include "common.h"
#define  HEAPSIZ 100
int epfd;

/*
	这个结构体用于保存从客户端读取的数据，线程要执行的回调函数
	当监听树上触发了epoll_event，我们将通过epoll_event的指针
	找到该结构体，然后把回调函数添加至任务队列
*/
typedef struct
{
	int fd;           //文件描述符
	int event;		  //事件
	void *arg;
	void (*pthread_work)(int fd,int event,void *arg);   //线程任务函数
	char buf[BUFSIZ];    //保存读取的数据
	int size;      //实际读到buf的size
	int status;   //是否在树上
}eventNode;

static eventNode nodes[HEAPSIZ+1];

/*  初始化tcp  
	服务端socket的初始化
*/
void init_tcp();

/*  设置事件结构体基本属性   
	参数node:为要设置的eventNode指针类型
	参数fd:文件描述符
	参数event:为node设置的事件
	参数callback:和node关联的回调函数
*/
void set_event_node(eventNode *node,int fd,int event,void *arg,void (*callback)(int,int,void *));

/*  把事件挂到红黑树  
	参数node:把node中的event挂载到监听树
*/
void add_event(eventNode *node);

/*  从红黑树上删除   
	参数node:要从监听树上摘下的node
*/
void del_event(const eventNode *node);

/*     
	独立子线程阻塞等待客户端的回调函数
	参数arg:服务端的文件描述符
*/
void *accept_clients(void *arg);

/*  从socket读取数据  
	参数fd:文件描述符
	参数event:对文件描述符执行的事件
	参数arg:eventNode，用于保存从fd读取的数据
	参照send_data()
*/
void rev_data(int fd,int event,void *arg);

/*  向socket写数据  
	参数fd:文件描述符
	参数event:对文件描述符执行的事件
	参数arg:eventNode，从eventNode中取出数据，写到fd
	参照rev_data()	
*/
void send_data(int fd,int event,void *arg);
