#include "threadpool/Threadpool.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include "threadpool/Thread.h"

namespace wd {
    Threadpool::Threadpool(size_t threadnum, size_t quesize)
        : _threadNum(threadnum),
          _queSize(quesize),
          _taskque(_queSize),
          _isExit(false) {
            _threads.reserve(_threadNum);
    }

    Threadpool::~Threadpool() {
        std::cout << "~Threadpool()" << std::endl;
        if(!_isExit) {
            stop();
        }
    }

    void Threadpool::addTask(Task&& task) {
        _taskque.push(std::move(task));
    }

    Task Threadpool::getTask() {
        return _taskque.pop();
    }

    void Threadpool::start() {
        for(size_t i = 0; i<_threadNum; ++i) {
            std::unique_ptr<Thread> thread(
                new Thread(std::bind(&Threadpool::threadFunc, this))
            );
            _threads.push_back(std::move(thread));
        }

        for(auto& thread: _threads) {
            thread->start();
        }
    }

    void Threadpool::threadFunc() {
        while(!_isExit) {
            Task task = getTask();
            if(task) {
                task();
            }
        }
    }

    void Threadpool::stop() {
        if(!_isExit) {
            while(!_taskque.empty()) {
                ::usleep(1);
            }
            _isExit = true;
            _taskque.wakeup();
            for(auto& thread: _threads) {
                thread->join();
            }
        }
    }
}