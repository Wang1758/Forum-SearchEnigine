#pragma once

namespace wd {
    class SocketIO {
    public:
        explicit SocketIO(int fd);

        // 从_fd中读取长度为len的数据到buf中，返回实际读取的字节长度
        int readn(char *buf, int len);
        int readLine(char *buf, int maxLen);
        int writen(const char *buf, int len);

    private:
        int recvPeek(char *buf, int len);

    private:
        int _fd;
    };

}
