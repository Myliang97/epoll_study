
#include "pthread_pool.h"

//工作线程的回调函数
void *pthread_callback(void *arg)
{
    pthread *self=(pthread *)arg;
    while(1){
        pthread_mutex_lock(&p_cntl.lock_self);
        while(!p_cntl.del_pthread_num && p_cntl.cur_task_num==0)        //任务数为0时，阻塞等待条件变量
            pthread_cond_wait(&p_cntl.lock_tasks_que,&p_cntl.lock_self);
        int i;
        for(i=0;i<p_cntl.max_pthread_num;++i)   //找到没有被执行的任务，取出
        {
            if(p_cntl.task_queue[i])
                break;
        }
        if(i!=p_cntl.max_pthread_num)   
        {
            //printf("pthread %lu working\n",self->pid);
            eventNode *task=(eventNode *)p_cntl.task_queue[i];
            p_cntl.task_queue[i]=NULL;  //取出,置空
            p_cntl.cur_task_num--;
            p_cntl.busy_pthread_num++;
            self->work=1;
            task->pthread_work(task->fd,task->event,task->arg);
            self->work=0;
            p_cntl.busy_pthread_num--;
            pthread_cond_signal(&p_cntl.lock_que_full);
        }
        if(!self->work && p_cntl.del_pthread_num >0)
        {
            p_cntl.cur_pthread_num--;
            p_cntl.del_pthread_num--;
            self->live=0;
            printf("pthread %lu dead\n",self->pid);
            pthread_mutex_unlock(&p_cntl.lock_self);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&p_cntl.lock_self);
        sleep(1);
    }
}
void add_thread()
{
    if(!p_cntl.pool[p_cntl.cur_pthread_index].live)   //找到没有创建线程的位置
    {
        printf("add pthread...\n");
        pthread_create(&p_cntl.pool[p_cntl.cur_pthread_index].pid,NULL,pthread_callback,
                                (void *)&p_cntl.pool[p_cntl.cur_pthread_index]);
        p_cntl.cur_pthread_num++;
        p_cntl.cur_pthread_index++;
        p_cntl.cur_pthread_index %=p_cntl.max_pthread_num;
    }
    else
    {
        p_cntl.cur_pthread_index++;
        p_cntl.cur_pthread_index %=p_cntl.max_pthread_num;
        add_thread();
    }
}
///管理者线程的回调函数
void *manage_pthread(void *arg)
{
    while(!p_cntl.exit)
    {
        pthread_mutex_lock(&p_cntl.lock_self);
        int cur_busy=p_cntl.busy_pthread_num;
        int cur_ptd=p_cntl.cur_pthread_num;
        //printf("cur_busy:%d,cur_pthread:%d\n",cur_busy,cur_ptd);
        if(cur_busy *2 >cur_ptd )    //当忙线程数超过线程一半时，增加线程
        {
            int num;
            if(cur_busy<cur_ptd)
                num=cur_ptd / 4 + 1;   //至少增加1个线程
            if(cur_busy>cur_ptd)
                num=cur_busy-cur_ptd;
                
            num=(cur_ptd+num)>p_cntl.max_pthread_num?(p_cntl.max_pthread_num-cur_ptd):num;
            for(;num>0;--num)
                add_thread();
        }
        if(cur_busy *3 < cur_ptd)
        {
            int num=cur_ptd/3 +1;
            num=(cur_ptd-num)>p_cntl.min_pthread_num?num:(cur_ptd-p_cntl.min_pthread_num);
            p_cntl.del_pthread_num=num;
        }
        if(p_cntl.cur_task_num>0)
            pthread_cond_signal(&p_cntl.lock_tasks_que);
        pthread_mutex_unlock(&p_cntl.lock_self);
        sleep(1);
    }
    ////没有设置退出条件
    pthread_mutex_lock(&p_cntl.lock_self);
    printf("cur_task:%d,cur_pthread:%d\n",p_cntl.cur_task_num,p_cntl.cur_pthread_num);
    p_cntl.del_pthread=1;
    pthread_mutex_unlock(&p_cntl.lock_self);
    pthread_cond_broadcast(&p_cntl.lock_tasks_que);
    printf("manage pthread dead\n");
    pthread_exit(NULL);
}
///添加任务至任务队列
void add_task(const eventNode *node)
{
    pthread_mutex_lock(&p_cntl.lock_self);
    if(p_cntl.task_index == p_cntl.max_pthread_num)  //任务队列已满
    {
        pthread_cond_wait(&p_cntl.lock_que_full,&p_cntl.lock_self);
    }
    p_cntl.task_index %= p_cntl.max_pthread_num;
    if(!p_cntl.task_queue[p_cntl.task_index])
    {
        p_cntl.task_queue[p_cntl.task_index]=node;
        p_cntl.cur_task_num++;
        p_cntl.task_index++;
        ///唤醒线程池，执行任务
        pthread_cond_signal(&p_cntl.lock_tasks_que);
        pthread_mutex_unlock(&p_cntl.lock_self);
    }
    else                 //当前任务队列位置非空，递归
    {
        p_cntl.task_index++;
        pthread_mutex_unlock(&p_cntl.lock_self);
        add_task(node);
    }
}
void init_pool()
{
    printf("init start ...\n");
    ///线程锁和条件变量初始化
    pthread_mutex_init(&p_cntl.lock_self,NULL);
    pthread_cond_init(&p_cntl.lock_tasks_que,NULL);
    pthread_cond_init(&p_cntl.lock_que_full,NULL);

    p_cntl.min_pthread_num=MIN_PTHREAD;
    p_cntl.max_pthread_num=MAX_PTHREAD;

    ///任务队列初始化
    int i;
    for(i=0;i<MAX_TASK;++i)
    {
        p_cntl.task_queue[i]=(eventNode*)malloc(sizeof(eventNode)*MAX_TASK);
        p_cntl.task_queue[i]=NULL;
    }
    p_cntl.task_index=0;
    p_cntl.cur_task_num=0;
    p_cntl.busy_pthread_num=0;
    p_cntl.cur_pthread_num=MIN_PTHREAD;
    p_cntl.del_pthread=0;
    p_cntl.del_pthread_num=0;
    p_cntl.exit=0;

    ///线程池初始化
    p_cntl.pool=(pthread *)malloc(sizeof(pthread)*MAX_PTHREAD);
    if(!p_cntl.pool)
    {
        printf("error\n");
        return;
    }
    for(i=p_cntl.max_pthread_num;i>0;--i)   //未创建的线程，置0
    {
        p_cntl.pool[i-1].live=0;
    }
    p_cntl.cur_pthread_index=0;
    i=0;
    for(;i<p_cntl.min_pthread_num;i++)
    {
        p_cntl.pool[i].work=0;
        p_cntl.pool[i].live=1;
        pthread_create(&p_cntl.pool[i].pid,NULL,pthread_callback,
                            (void *)&p_cntl.pool[i]);
    }
    p_cntl.cur_pthread_index=i;
    ///管理者线程
    pthread_create(&p_cntl.p_man,NULL,manage_pthread,NULL);
    printf("init...success\n");
}
void free_resource()
{
    pthread_join(p_cntl.p_man,NULL);
    int i=0;
    for(i;i<p_cntl.max_pthread_num;++i)
    {
        if(p_cntl.pool[i].live)
            pthread_join(p_cntl.pool[i].pid,NULL);
    }
    printf("resource release\n");
    pthread_mutex_lock(&p_cntl.lock_self);
    pthread_mutex_destroy(&p_cntl.lock_self);
    pthread_cond_destroy(&p_cntl.lock_tasks_que);
    free(p_cntl.pool);
    for(i=0;i<MAX_TASK;++i)
    {
        free(p_cntl.task_queue[i]);
    }
}