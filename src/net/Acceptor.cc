#include "net/Acceptor.h"

namespace wd {
    Acceptor::Acceptor(unsigned short port) : _addr(port), _listensock() {};

    Acceptor::Acceptor(const std::string& ip, unsigned short port)
        : _addr(ip,port), _listensock() {};

    void Acceptor::ready() {
        setReuseAddr(true);
        setReusePort(true);
        bind();
        listen();
    }

    void Acceptor::setReuseAddr(bool ison) {
        int on = ison;
        if(setsockopt(_listensock.fd(), SOL_SOCKET, SO_REUSEADDR, 
                      &on,sizeof(on)) < 0) {
            perror("setsockopt");
        }
    }

    void Acceptor::setReusePort(bool ison) {
        int on  = ison;
        if(setsockopt(_listensock.fd(),SOL_SOCKET, SO_REUSEPORT,
                      &on, sizeof(on)) < 0) {
            perror("setsockopt");
        }
    }

    void Acceptor::bind() {
        if(::bind(_listensock.fd(), (struct sockaddr*)_addr.getInetAddressPtr(),
           sizeof(struct sockaddr)) == -1) {
            perror("bind");
        }
    }

    void Acceptor::listen() {
        if(::listen(_listensock.fd(), 10) == -1) {
            perror("listen");
        }
    }

    int Acceptor::accept() {
        int peerfd = ::accept(_listensock.fd(), nullptr, nullptr);
        if(peerfd == -1) {
            perror("accept");
        }
        return peerfd;
    }
}