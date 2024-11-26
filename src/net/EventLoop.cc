#include "net/EventLoop.h"
#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>
#include <net/Acceptor.h>
#include <net/TCPConnection.h>

namespace wd {
    EventLoop::EventLoop(Acceptor& acceptor) 
                        :_efd(createEpollFd()),
                         _eventfd(createEventFd()),
                         _acceptor(acceptor),
                         _eventList(1024),
                         _isLooping(false) {
        addEpollFdRead(_acceptor.fd());
        addEpollFdRead(_eventfd);
    }

    // 设置isLooping为true，启动事件循环
    void EventLoop::loop() {
        _isLooping = true;
        while(_isLooping) {
            waitEpollFd();
        }
    }

    // 设置isLooping为false，关闭事件循环
    // 运行在另一个线程
    void EventLoop::unloop() {
        if(_isLooping) {
            _isLooping = false;
        }
    }

    // TCPConnection对象添加任务的接口，将该连接上的写事件放入待处理队列中
    void EventLoop::runInloop(function<void()>&& cb) {
        {
            MutexGuard lock(_mutex);
            _pendingFunctors.push_back(std::move(cb));
        }
        wakeup();   // 通过eventfd通知epollfd有事件需要处理
    }

    // 处理epollfd上监听到的事件，将在主循环中调用
    void EventLoop::waitEpollFd() {
        int event_num;
        do {
            // epoll_wait将阻塞5s等待事件到来
            event_num = epoll_wait(_efd, &*_eventList.begin(), _eventList.size(), 5000);
        } while(event_num == -1 && errno == EINTR);

        if(event_num == -1) {
            perror("epoll_wait");
            return;
        } else if(event_num == 0) {
            std::cout << ">> epoll_wait timeout"<< std::endl;
        } else {
            // 监听到的事件数量大于2/3的eventlist最大容量，则扩容
            if(3 * event_num > 2 * _eventList.size()) {
                _eventList.resize(2*event_num);
            }

            for(int i = 0; i < event_num; ++i) {
                int fd = _eventList[i].data.fd;
                if(fd ==_acceptor.fd()) {
                    // 来自服务器接收器的事件
                    // 有新连接到来
                    if(_eventList[i].events & EPOLLIN) {
                        handleNewConnection();
                    }
                } else if (fd == _eventfd) {
                    // 来自eventfd上的事件，说明有tcp连接将写事件放入了待处理队列
                    if(_eventList[i].events & EPOLLIN) {
                        handleRead();
                        std::cout << ">> do pending functors"<< std::endl;
                        doPendingFunctors();
                        std::cout << ">> finish do pending functors" << std::endl;
                    }
                } else {
                    // 处理来自客户端发送的数据，当客户端关闭时也将触发
                    if(_eventList[i].events & EPOLLIN){
                        handleMessage(fd);
                    }
                }
            }
        }
    }

    // 处理连接事件
    // 为新连接构造一个新的TCPConnection对象，并触发新连接的连接回调函数
    void EventLoop::handleNewConnection() {
        int peerfd = _acceptor.accept();
        addEpollFdRead(peerfd); // 添加到事件监听器中
        // 构造一个tcp连接对象，设置回调函数，并加入到conns中
        TCPConnectionPtr conn = std::make_shared<TCPConnection>(peerfd, this);
        conn->setConnectionCallback(_onConnection);
        conn->setMessageCallback(_onMessage);
        conn->setCloseCallback(_onClose);
        _conns.insert({peerfd, conn});
        conn->handleConnectionCallback();   //触发连接回调函数
    }

    // 处理客户端发送消息事件
    // 检查连接是否关闭,若没关闭则触发这个tcp连接的消息处理回调函数
    // 否则将该客户端fd从监听事件集中删除，触发这个tcp连接的关闭回调函数
    void EventLoop::handleMessage(int fd) {
        bool isClose = isConnectionClosed(fd);
        auto it = _conns.find(fd);
        assert(it != _conns.end());
        if(!isClose) {
            it->second->handleMessageCallback();
        } else {
            delEpollFdRead(fd);
            it->second->handleCloseCallback();
            _conns.erase(it);
        }
    }

    bool EventLoop::isConnectionClosed(int fd) {
        int ret;
        do{
            char buf[1024];
            ret = recv(fd, buf, sizeof(buf), MSG_PEEK);
        }while(ret == -1 && errno == EINTR);
        return (ret == 0 || ret == -1);
    }

    int EventLoop::createEpollFd() {
        int ret = ::epoll_create1(0);
        if(ret == -1) {
            perror("epoll_create1");
        }
        return ret;
    }

    void EventLoop::addEpollFdRead(int fd) {
        struct epoll_event evt;
        evt.data.fd = fd;
        evt.events = EPOLLIN;
        if(epoll_ctl(_efd, EPOLL_CTL_ADD, fd, &evt) == -1) {
            perror("epoll_ctr");
        }
    }

    void EventLoop::delEpollFdRead(int fd) {
        struct epoll_event evt;
        evt.data.fd = fd;
        if(epoll_ctl(_efd, EPOLL_CTL_DEL, fd, &evt) == -1) {
            perror("epoll_ctl");
        }
    }

    int EventLoop::createEventFd() {
        int fd = ::eventfd(0,0);
        if(fd == -1 ){
            perror("eventfd");
        }
        return fd;
    }

    void EventLoop::handleRead() {
        uint64_t count;
        int ret = ::read(_eventfd, &count, sizeof(count));
        if(ret == -1) {
            perror("read");
        }
    }

    void EventLoop::wakeup() {
        uint64_t one = 1;
        int ret = ::write(_eventfd, &one, sizeof(one));
        if(ret != sizeof(one)) {
            perror("write");
        }
    }

    // 处理待处理的任务
    void EventLoop::doPendingFunctors() {
        vector<function<void()>> temp;
        {
            // 获取锁，将待处理队列中的任务移交到temp中，然后释放锁
            // 这样是为了防止当前线程获取共享资源_pendingFunctors的时间太久
            MutexGuard lock(_mutex);
            _pendingFunctors.swap(temp);
        }

        for(auto& functor : temp) {
            functor();
        }
    }
}