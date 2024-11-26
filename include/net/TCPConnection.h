#pragma once
#include <string>
#include <memory>
#include <functional>
#include "InetAddress.h"
#include "Nocopyble.h"
#include "Socket.h"
#include "SocketIO.h"
using std::function;
using std::string;

namespace wd
{
    class TCPConnection;
    using TCPConnectionPtr = std::shared_ptr<TCPConnection>;
    using TCPConnectionCallback = function<void(const TCPConnectionPtr &)>;

    class EventLoop;

    class TCPConnection : Nocopyble,
        public std::enable_shared_from_this<TCPConnection>
    {
    public:
        // 传入客户端/服务端？的文件描述符,以及
        TCPConnection(int fd, EventLoop *loop);
        ~TCPConnection();

        // 从fd中读取一行数据并返回
        string receive();
        // 向fd中写入数据msg
        void send(const string &msg);
        // 
        void sendInLoop(const string &msg);

        // 返回该TCP连接的两端地址
        string toString() const;
        // 关闭fd的写端，析构时被调用
        void shutdown();

        // 设置回调函数
        void setConnectionCallback(const TCPConnectionCallback &cb);
        void setMessageCallback(const TCPConnectionCallback &cb);
        void setCloseCallback(const TCPConnectionCallback &cb);

        // 处理回调函数
        void handleConnectionCallback();
        void handleMessageCallback();
        void handleCloseCallback();

    private:
        InetAddress getLocalAddr();
        InetAddress getPeerAddr();

    private:
        Socket _sock;
        SocketIO _socketIO;
        InetAddress _localAddr;
        InetAddress _peerAddr;
        bool _isShutdownonWrite;
        EventLoop *_loop;

        // 绑定的回调函数
        TCPConnectionCallback _onConnection;
        TCPConnectionCallback _onMessage;
        TCPConnectionCallback _onClose;
    };

}
