# Forum-SearchEnigine
深圳大学本科生竞赛交流平台的搜索服务器

# 项目简介

# 已实现功能
使用log4cpp库实现日志系统，通过单例模式实现日志对象的唯一性
使用redis数据库实现数据的持久化存储
使用POSIX多线程库实现线程池，实现了线程、条件变量、互斥锁等多线程编程的基本功能
实现了一个计算线程，处理业务逻辑，通知IO线程进行数据的发送

# 项目结构

# 项目启动

# 项目实现日志
[11.23-11.24]一些功能函数：mylogger -> Nocopyable -> redis -> MutexLock -> Condition ->
[11.24]线程池 Thread -> TaskQueue -> ThreadPool ->
[] SocketIO -> Socket -> InetAddress -> Acceptor -> TCPConnection -> EventLoop -> TCPServer