#include "net/InetAddress.h"
#include <string.h>

namespace wd{
    InetAddress::InetAddress(unsigned short port) {
        ::memset(&_addr, 0,sizeof(struct sockaddr_in));
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        _addr.sin_addr.s_addr = INADDR_ANY;
    }

    InetAddress::InetAddress(const std::string& ip, unsigned short port) {
        ::memset(&_addr, 0, sizeof(struct sockaddr_in));
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        
        if(::inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr.s_addr) <= 0) {
            perror("inet_pton");
        }
    }

    InetAddress::InetAddress(const struct sockaddr_in& addr) :_addr(addr){};

    std::string InetAddress::ip() const {
        char ipStr[INET_ADDRSTRLEN];
        if(::inet_ntop(AF_INET, &_addr.sin_addr.s_addr, ipStr, INET_ADDRSTRLEN) == nullptr) {
            perror("inet_ntop");
            return "";
        }
        return std::string(ipStr);
    }

    unsigned short InetAddress::port() const {
        return ntohs(_addr.sin_port);
    }
}
