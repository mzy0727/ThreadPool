#pragma once
#include "TaskQueue.h"
#include "TaskQueue.cpp"

template <typename T>
class ThreadPool
{

   
public:
   
    
    //  创建线程池并初始化
     ThreadPool(int min, int max);
    //  销毁线程池
    ~ThreadPool();
    //  给线程池添加任务
    void threadPoolAddTask(Task<T> task);
    //  获取线程池中工作的线程的个数
    int  getBusyNum();
    //  获取线程池中活着的线程的个数
    int getAliveNum();
    //////
private:
    static void* worker(void* arg);

    static void* manager(void* arg);

    void threadExit();
    //////
private:
    //任务队列
    TaskQueue<T> *taskQ;

    //管理者线程
    pthread_t   managerID;  //管理者线程ID
    //工作线程
    pthread_t   *threadIDs; //工作的线程ID
    int minNum;     //最小线程数量
    int maxNum;     //最大线程数量
    int busyNum;    //忙的线程个数
    int liveNum;    //存活的线程个数
    int exitNum;    //要销毁的线程个数
    //互斥锁
    pthread_mutex_t mutexPool;  //锁整个的线程池
    //条件变量
    pthread_cond_t notEmpty;    //任务队列是不是空了

    bool shutdown = false;   //是不是要销毁线程池，销毁为1，不销毁为0
    static const int NUMBER = 2;
};

