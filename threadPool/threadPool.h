#ifndef THREADPOOL
#define THREADPOOL
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

enum threadPoolFlag{RUN=0,END=1};
typedef enum threadPoolFlag TPF;
//任务节点
struct TaskQueue
{
    void* (*fun)(void*);
    void* agrv;
    struct TaskQueue* next;
};
typedef struct TaskQueue TAQ;

struct ThreadPool
{
    TPF flag;                     //线程池存在的标志
    TAQ *taskHead;                //任务队列头指针
    pthread_cond_t taskCond;      //任务条件
    pthread_mutex_t taskMutex;    //任务互斥锁
    pthread_t *threadID;          //线程ID号
    int maxThreadNum;             //线程池中最大线程数
    int curThreadNum;             //线程池中活跃线程数
    int curTaskNum;               //当前任务数
};
typedef struct ThreadPool THP;

//防止外部文件失误调用。（必须调用函数返回）
static THP *threadPool=NULL;

//创建线程池
//参数说明：需要创建的线程总数目
THP* createThreadPool(int threadNum);

//处理线程函数
void* threadHeadle(void* argv);

//向任务队列中添加新任务，线程自动执行
//参数说明：需要执行的函数指针；函数参数
void addTask(void* (*fun)(void*),void* argv);

//销毁线程池
void destroyThreadPool();
#endif