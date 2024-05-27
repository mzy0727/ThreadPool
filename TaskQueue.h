#pragma once

#include <queue>
#include <iostream>
#include <pthread.h>
using namespace std;

using callback = void (*)(void*);
//  任务结构体
template <typename T>
struct  Task{
    Task<T>(){
        function = nullptr;
        arg = nullptr;
    }
    Task<T>(callback f, void* arg){
        function = f;
        this->arg = (T*)arg;
    }
    callback function;
    T* arg;
};

template <typename T>
class TaskQueue{
public:
    TaskQueue();
    ~TaskQueue();

    // 取出一个任务
    Task<T> getTask();
    // 添加一个任务
    void addTask(Task<T>& task);
    void addTask(callback f, void* arg);    //重载
    inline int taskNumber(){
        return m_taskQ.size();
    }
private:
    pthread_mutex_t m_mutex;
    queue<Task<T>> m_taskQ; 

};
