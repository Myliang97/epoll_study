#include "epoll_heap.h"

#define  MAX_TASK 100
#define  MIN_PTHREAD 10
#define  MAX_PTHREAD 100
typedef struct
{
    pthread_t pid;
    int work;                 //是否在工作，用于判断杀死进程
    int live;                //是否存活，当live==0时，在线程池中会被新线程覆盖
}pthread;
typedef struct 
{
    pthread_mutex_t lock_self;         //用于锁住整个结构体
    pthread_cond_t lock_tasks_que;     //条件变量，通知线程执行任务
    pthread_cond_t lock_que_full;     //条件变量，判断任务队列是否有空位置，添加新任务判断

    pthread *pool;              //线程池
    eventNode *task_queue[MAX_TASK];         //任务队列
    pthread_t p_man;            //线程管理者

    int cur_task_num;               //当前任务数
    int task_index;                      //当前任务队列索引
    int cur_pthread_index;          //线程池索引
    int cur_pthread_num;                 //当前线程数，包括空闲
    int busy_pthread_num;               //正在工作的线程数
    int max_pthread_num;               //最大能创建的线程数
    int min_pthread_num;               //最少保证存在的线程数
    int del_pthread;                  //减少线程标志
    int del_pthread_num;              //减少的线程数
    int exit;                         //退出
}pthread_cntl;
pthread_cntl p_cntl;    //全局的控制器

/*  线程池初始化  */
void init_pool();

/*  工作线程的回调函数  */
void *pthread_callback(void *arg);

/*  管理者线程的回调函数 */
void *manage_pthread(void *arg);

/*  添加任务至任务队列 */
void add_task(const eventNode *node);

/*  添加线程  */
void add_thread();

/*  释放资源  */
void free_resource();
