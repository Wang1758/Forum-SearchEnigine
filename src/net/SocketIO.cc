#include "net/SocketIO.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


namespace wd {
    SocketIO::SocketIO(int fd) :_fd(fd) {};

    int SocketIO::readn(char* buf, int len) {
        int left = len;
        char *p = buf;
        while(left > 0) {
            int ret = ::read(_fd, p, left);
            if(ret == -1&& errno == EINTR) {    // 发生系统中断
                continue;
            } else if(ret == -1) {  // 读取出现错误
                perror("read");
                return len-left;
            } else if(ret == 0) {   // 数据已经读取完
                return len-left;
            } else {    // 再读一轮
                left -= ret;
                p += ret;
            }
        }
        return len - left;
    }

    int SocketIO::recvPeek(char* buf, int len){
        int ret;
        do {
            ret = ::recv(_fd, buf, len, MSG_PEEK);
        }while(ret == -1&& errno == EINTR);
        return ret;
    }

    int SocketIO::readLine(char* buf, int maxLen) {
        int left = maxLen - 1;
        char * p = buf;
        int totalLen = 0;
        // 这个循环存在bug，如果缓冲区的数据没有\n,并且长度小于maxLen，
        // 那么将会阻塞在recv
        while(left > 0)
        {
            int ret = recvPeek(p,left);
            // 查找\n
            for(int i = 0;i != ret ; ++i){
                if(p[i]=='\n') {
                    int sz = i +1;
                    readn(p,sz);
                    totalLen +=sz;
                    p+=sz;
                    *p = '\0';
                    return totalLen;
                }
            }
            // 没有找到\n,继续寻找
            readn(p,ret);
            left -= ret;
            p += ret;
            totalLen += ret;
        }
        // 最终没有发现\n
        *p = '\0';
        return totalLen;
    }

    int SocketIO::writen(const char *buf, int len) {
        int left = len;
        const char *p = buf;
        while (left > 0) {
            int ret = ::write(_fd, p, left);
            if (ret == -1 && errno == EINTR) {
                continue;
            } else if (ret == -1){
                perror("write");
                return len - left;
            } else {
                left -= ret;
                p += ret;
            }
        }

        printf(">> writen finish\n");
        return len - left;
    }
}