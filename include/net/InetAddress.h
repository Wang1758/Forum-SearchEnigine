#pragma once
#include <arpa/inet.h>
#include <string>

namespace wd{
    class InetAddress{
    public:
        explicit InetAddress(unsigned short port);
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
