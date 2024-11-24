#pragma once
#include "Thread.h"
#include "TaskQueue.h"
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
using std::unique_ptr;
using std::vector;

namespace wd
{
    using Task = std::function<void()>;
    class Threadpool
    {
    public:
        Threadpool(size_t, size_t);
        ~Threadpool();
        void start();
        void stop();
        void addTask(Task &&task);

    private:
        void threadFunc(); // 子线程执行的任务
        Task getTask();

    private:
        size_t _threadNum;
        size_t _queSize;
        vector<unique_ptr<Thread>> _threads;
        TaskQueue _taskque;
        bool _isExit;
    };
}
