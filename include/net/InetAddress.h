#pragma once
#include <arpa/inet.h>
#include <string>

namespace wd{
    class InetAddress{
    public:
        // 监听所有地址
        explicit InetAddress(unsigned short port);
        // 监听指定地址
        InetAddress(const std::string &ip, unsigned short port);
        InetAddress(const struct sockaddr_in &addr);

        std::string ip() const;
        unsigned short port() const;
        struct sockaddr_in *getInetAddressPtr() {
            return &_addr;
        }

    private:
        struct sockaddr_in _addr;
    };
}
