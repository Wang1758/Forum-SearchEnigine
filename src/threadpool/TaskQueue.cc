#include "threadpool/TaskQueue.h"

namespace wd {
    TaskQueue::TaskQueue(size_t queSize) :_queSize(queSize),
        _mutex(),
        _notFull(_mutex),
        _notEmpty(_mutex),
        _flag(true){}

    bool TaskQueue::empty() const {return _que.size() == 0;}

    bool TaskQueue::full() const {return _que.size() == _queSize;}

    void TaskQueue::push(ElemType elem) {
        MutexGuard lock(_mutex);
        while(full()) { //使用while防止虚假唤醒
            _notFull.wait();
        }
        // _que.emplace(std::forward<ElemType>(elem));  // 完美转发
        _que.push(elem);
        _notEmpty.notify();
    }

    ElemType TaskQueue::pop() {
        MutexGuard lock(_mutex);
        while(_flag && empty()){
            _notEmpty.wait();
        }

        if(_flag) {
            // ElemType elem = std::move(_que.front()); // 移动语义
            ElemType elem = _que.front();
            _que.pop();
            _notFull.notify();
            return elem;
        } else {
            return nullptr;
        }
    }
    
    void TaskQueue::wakeup() {
        _flag == false;
        _notEmpty.notifyall();
    }
}