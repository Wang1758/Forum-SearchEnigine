#pragma once
#include "InetAddress.h"
#include "Socket.h"

namespace wd{
    class Acceptor{
    public:
        Acceptor(unsigned short port);
        Acceptor(const std::string &ip, unsigned short port);
        // 设置地址端口号可复用，调用bind绑定地址，调用listen监听
        void ready();
        int accept();
        int fd() const { 
            return _listensock.fd(); 
        }

    private:
        // 设置地址可复用
        void setReuseAddr(bool on);
        // 设置端口号可复用
        void setReusePort(bool on);
        void bind();
        void listen();

    private:
        InetAddress _addr;
        Socket _listensock;
    };
} // namespace wd
