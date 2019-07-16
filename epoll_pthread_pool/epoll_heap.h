#include "common.h"
#define  HEAPSIZ 100
int epfd;
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

/*  初始化tcp  */
void init_tcp();

/*  设置事件结构体基本属性   */
void set_event_node(eventNode *node,int fd,int event,void *arg,void (*callback)(int,int,void *));

/*  把事件挂到红黑树  */
void add_event(eventNode *node);

/*  从红黑树上删除   */
void del_event(const eventNode *node);

/*  子线程阻塞等待客户端    */
void *accept_clients(void *arg);

/*  从socket读取数据  */
void rev_data(int fd,int event,void *arg);

/*  向socket写数据  */
void send_data(int fd,int event,void *arg);
