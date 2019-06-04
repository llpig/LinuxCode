#include "threadPool.h"

void* threadHeadle(void* argv)
{
    while(threadPool->flag==RUN)
    {
        pthread_mutex_lock(&(threadPool->taskMutex));
        //如果线程池处于运行中，且任务队列中无任务，则线程睡眠等待被唤醒。
        while(threadPool->flag==RUN && threadPool->curTaskNum==0)
        {
            //等待任务来时被唤醒
            pthread_cond_wait(&(threadPool->taskCond),&(threadPool->taskMutex));
        }
        //执行任务队列中的任务
        if(threadPool->flag==RUN)
        {
            //领取任务
            TAQ *p=threadPool->taskHead;
            threadPool->taskHead=threadPool->taskHead->next;
            //任务数减1
            --threadPool->curTaskNum;
            //活跃线程数加1
            ++threadPool->curThreadNum;
            pthread_mutex_unlock(&(threadPool->taskMutex));
            //执行任务
            (*(p->fun))(p->agrv);
            free(p);
            p=NULL;
            //存在没有同步的风险
            --threadPool->curThreadNum;
        }
        //销毁线程池
        else if(threadPool->flag==END)
        {
            --threadPool->maxThreadNum;
            pthread_mutex_unlock(&(threadPool->taskMutex));
            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }
    }
}

THP* createThreadPool(int threadNum)
{
    threadPool=(THP*)malloc(sizeof(THP));
    threadPool->flag=RUN;
    pthread_mutex_init(&(threadPool->taskMutex),NULL);
    pthread_cond_init(&(threadPool->taskCond),NULL);
    threadPool->maxThreadNum=threadNum;
    threadPool->curThreadNum=0;
    threadPool->curTaskNum=0;
    threadPool->threadID=(pthread_t*)malloc(threadNum*sizeof(pthread_t));
    int i=0;
    for(;i<threadNum;++i)
    {
        pthread_create(&(threadPool->threadID[i]),NULL,threadHeadle,NULL);
    }
    return threadPool;
}

void addTask(void* (*fun)(void*),void* argv)
{
    //将新来的任务，插入任务队列的头节点的位置。
    TAQ *newTask=(TAQ*)malloc(sizeof(TAQ));
    newTask->fun=fun;
    newTask->agrv=argv;
    newTask->next=threadPool->taskHead;
    threadPool->taskHead=newTask;

    pthread_mutex_lock(&(threadPool->taskMutex));
    //任务队列中任务数加1。
    ++threadPool->curTaskNum;
    //当线程池还有任务时，唤醒线程池中的一个睡眠线程，去执行任务。
    if(threadPool->curThreadNum<threadPool->maxThreadNum)
    {
        pthread_cond_signal(&(threadPool->taskCond));
    }
    pthread_mutex_unlock(&(threadPool->taskMutex));
}

void destroyThreadPool()
{
    if(threadPool->flag==RUN)
    {
        threadPool->flag=END;
        pthread_cond_broadcast(&(threadPool->taskCond));
        TAQ *p=threadPool->taskHead;
        while(p)
        {
            --threadPool->curTaskNum;
            threadPool->taskHead=threadPool->taskHead->next;
            free(p);
            p=threadPool->taskHead;
        }
    }
}

void* testFun(void* argv)
{
    int *p= (int*)argv;
    printf("线程号%lu：开始执行函数。argv=%d\n",pthread_self(),*p);
    sleep(1);
    printf("线程号%lu：函数执行完毕！argv=%d\n",pthread_self(),*p);
    return NULL;
}

int main()
{
    THP *thp=createThreadPool(3);
    int i=0;
    for(;i<20;++i)
    {
        addTask(testFun,(void*)(&i));
        sleep(1);
    }
    while(thp->curTaskNum!=0);
    printf("任务执行完毕\n");
    destroyThreadPool();
    return 0;
}