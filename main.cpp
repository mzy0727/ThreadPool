#include<iostream>
#include<unistd.h>
#include "ThreadPool.h"
#include "ThreadPool.cpp"
using namespace std;



void taskFunc(void* arg){
    int num = *(int*)arg;
    printf("thread is working, number = %d, tid = %ld\n",num,pthread_self());
    usleep(1000);
}

int main(){

     // 创建线程池
    ThreadPool<int> pool(3,10);
    for(int i = 0; i < 10; i++){
        
        // threadPoolAdd(pool,taskFunc,&i);
        int* num = new int(i + 100);
        pool.threadPoolAddTask(Task<int>(taskFunc,num));
        


    }

    sleep(30);

    

    return 0;
}