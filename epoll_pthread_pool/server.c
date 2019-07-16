#include "pthread_pool.h"

int main()
{
    epfd=epoll_create(HEAPSIZ);
	init_tcp();
	struct epoll_event events[HEAPSIZ];
    init_pool();
	printf("server start...\n");
    int i;
	int num;
	while(1)
	{
		num=0;
		num=epoll_wait(epfd,events,HEAPSIZ,-1);
		for(i=0;i<num;++i)           //调用回调函数
		{
			eventNode *node=(eventNode*)events[i].data.ptr;
			add_task(node);  //添加至任务队列
		}
	}	
	return 0;
}