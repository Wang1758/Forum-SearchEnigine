#pragma once
#include <queue>
#include "Condition.h"
#include "MutexLock.h"
#include <functional>
using std::queue;

namespace wd
{
    using ElemType = std::function<void()>;
    class TaskQueue
    {
    public:
        TaskQueue(size_t quesize);

        bool empty() const;
        bool full() const;
        // 添加任务
        void push(ElemType);
        // 获取任务
        ElemType pop();
        // 终止任务，唤醒所有阻塞在pop的线程，立即返回nullptr，
        void wakeup();

    private:
        size_t _queSize;
        queue<ElemType> _que;
        MutexLock _mutex;
        Condition _notFull;
        Condition _notEmpty;
        bool _flag; 
    };
} // namespace wd
