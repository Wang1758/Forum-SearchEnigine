#pragma once
#include <pthread.h>
#include <functional>
#include <iostream>
#include <string>
#include "Nocopyble.h"

namespace wd {
    using ThreadCallback = std::function<void()>;

    class Thread : Nocopyble
    {
    public:
        Thread(ThreadCallback &&cb)
            : _pthid(0), _isRunning(false), _cb(std::move(cb)) {}

        ~Thread();
        // 创建新线程，执行threadFunc函数
        void start();
        void join();
        static void *threadFunc(void *);

    private:
        pthread_t _pthid;
        bool _isRunning;
        ThreadCallback _cb;
    };
}
