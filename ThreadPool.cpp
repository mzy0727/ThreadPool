
#include "ThreadPool.h"
#include <iostream>
#include <string.h>
#include <string>
#include <pthread.h>
#include <unistd.h>
using namespace std;

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
   
    do{
        // 实例化任务对列
        taskQ = new TaskQueue<T>;
        if(taskQ == nullptr){
            cout<<"malloc taskQ fail..."<<endl;
            break;
        }
        minNum = min;
        maxNum = max;
        busyNum = 0;
        liveNum = min;    //与最小个数相等
        exitNum = 0;
        threadIDs = new pthread_t[max];
        if(threadIDs == nullptr){
            cout<<"malloc threadIDS  fail..."<<endl;
            break;
        }
        if(pthread_mutex_init(&mutexPool,NULL) != 0 || pthread_cond_init(&notEmpty,NULL)  != 0){
            printf("mutex or condition init fail...\n");
            break;
        }
        shutdown = false;
         //创建线程
        pthread_create(&managerID,NULL,manager,this);
        for(int i = 0; i < min; i++){
            pthread_create(&threadIDs[i],NULL,worker,this);
        }
    }while(0);

}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    // 关闭线程池
    shutdown = 1;
    // 阻塞回收管理者线程(销毁管理者线程)
    pthread_join(managerID,NULL);
    // 唤醒阻塞的消费者线程
    for(int i = 0; i < liveNum; i++){
        pthread_cond_signal(&notEmpty);
    }
    if(taskQ){
        delete taskQ;
    }
    if(threadIDs){
        delete[] threadIDs;
    }

    // 销毁锁和条件变量
    pthread_mutex_destroy(&mutexPool);
    pthread_cond_destroy(&notEmpty);
   

   
}
template <typename T>
void ThreadPool<T>::threadPoolAddTask(Task<T> task){
    if(shutdown){
        //pthread_mutex_unlock(&mutexPool);
        return ;
    }
    // 添加任务
    taskQ->addTask(task);
    // 唤醒工作的线程
    pthread_cond_signal(&notEmpty);

    
}
template <typename T>
int ThreadPool<T>::getBusyNum(){
    pthread_mutex_lock(&mutexPool);
    int busyNum = this->busyNum;
    pthread_mutex_unlock(&mutexPool);
    return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum(){
    pthread_mutex_lock(&mutexPool);
    int liveNum = this->liveNum;
    pthread_mutex_unlock(&mutexPool);
    return liveNum;
}

template <typename T>
// 消费者线程
void* ThreadPool<T>::worker(void* arg){
    ThreadPool* pool = (ThreadPool*)arg;

    while(1){
        pthread_mutex_lock(&pool->mutexPool);
        // 当前任务队列是否为空
        while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)
        {
            // 阻塞工作线程（阻塞消费者）
            pthread_cond_wait(&pool->notEmpty,&pool->mutexPool);
        
            // 判断是不是要销毁线程
            if(pool->exitNum > 0){
                pool->exitNum--;
                if(pool->liveNum > pool->minNum){
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);    // 销毁前，先解锁，防止死锁
                    pool->threadExit();
                }
                
                //pthread_exit(NULL);
            }
        }
        
        // 判断线程池是否被关闭
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->mutexPool);     //防止死锁
            pool->threadExit();
            //pthread_exit(NULL);
        }

        // 从队列中取出一个任务
        Task<T> task = pool->taskQ->getTask();
        

        //解锁
       // pthread_cond_signal(&pool->notFull);    //唤醒生产者

       //开始工作
        printf("thread %ld  start working...\n",pthread_self());
       // pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum++;
       // pthread_mutex_unlock(&pool->mutexBusy);

        pthread_mutex_unlock(&pool->mutexPool);

        

        //调用任务里的函数
        task.function(task.arg);        //参数建议传堆内存
        //(*task.function)(task.arg);   //另一种写法
        delete task.arg;
        task.arg = nullptr;

        printf("thread %ld  end working...\n",pthread_self());
        pthread_mutex_lock(&pool->mutexPool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexPool);

    }

    return NULL;
}

template <typename T>
void* ThreadPool<T>::manager(void* arg){
    ThreadPool* pool = (ThreadPool*)arg;
    while(!pool->shutdown){
        // 每3s检测一次
        sleep(3);

        // 取出线程池中日任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->taskQ->taskNumber();
        int liveNum = pool->liveNum;

        // 取出忙的线程的数量
        // pthread_mutex_lock(&pool->mutexBusy);
        int busyNum = pool->busyNum;
        // pthread_mutex_unlock(&pool->mutexBusy);

        pthread_mutex_unlock(&pool->mutexPool);
        
       
        
        //添加线程
        //自定义规则：任务的个数>存活的线程个数 && 存活的线程个数 < 线程的最大个数
        if(queueSize > liveNum && liveNum < pool->maxNum){
            pthread_mutex_lock(&pool->mutexPool);
            int count = 0;
            for(int i = 0; i < pool->maxNum && count < NUMBER && pool->liveNum < pool->maxNum; ++i){
                if(pool->threadIDs[i] == 0){
                    pthread_create(&pool->threadIDs[i],NULL,worker,pool);
                    count++;
                    pool->liveNum++;
                }
            }
             pthread_mutex_unlock(&pool->mutexPool);
        }

        //销毁线程
        //自定义规则：忙的线程*2 < 存活的线程数 && 存活的线程 > 最小线程数
        if(busyNum * 2 < liveNum && liveNum > pool->minNum){
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);
            // 让工作的线程自杀
            for(int i = 0; i < NUMBER; ++i){
                // 唤醒工作线程
                pthread_cond_signal(&pool->notEmpty);
            }
        
        }

    }
    return NULL;
}

template <typename T>
// 线程退出
void ThreadPool<T>::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < maxNum; ++i)
    {
        if (threadIDs[i] == tid)
        {
            cout << "threadExit() function: thread " 
                << to_string(pthread_self()) << " exiting..." << endl;
            threadIDs[i] = 0;
            break;
        }
    }
    pthread_exit(NULL);
}
