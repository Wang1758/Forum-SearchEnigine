### 项目简介
本项目是一个简单的搜索引擎，直接使用TCP套接字实现了一个服务器，通过解析xml文件获取网页信息，建立网页库的倒排索引，将查找到的结果通过余弦相似度算法排序后封装成`json`字符串发送到前端。

### 已实现功能
- 使用`log4cpp`库实现日志系统，通过单例模式实现日志对象的唯一性
- 使用`redis`数据库实现数据的持久化存储
- 基于POSIX线程库，使用reactor模型实现了一个TCP服务器
- 使用`cppjieba`分词库实现中文分词功能
- 对查找到的网页使用余弦相似度进行排序
- 通过TF-IDF算法计算文档的权重，建立网页库的倒排索引
- 将查询到的结果封装成`json`字符串发送到前端

### 项目依赖
##### 环境要求
- Ubuntu 22.04
- C++ 11
- Redis 6.0.6

##### 第三方库
- [log4cpp](https://log4cpp.sourceforge.net/)
- [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
- [hiredis](https://github.com/redis/hiredis)
- [TinyXML-2](https://github.com/leethomason/tinyxml2)
- [CppJieba](https://github.com/yanyiwu/cppjieba)中文分词器
- [simhash](https://github.com/yanyiwu/simhash)算法库
> cppjieba、simhash、jsoncpp、tinyxml-2库已经包含在项目中，无需额外安装

### 项目结构
```shell
.
├── CMakeLists.txt
├── README.md
├── SearchEngine.jpg    项目类图
├── src                 源代码
│   ├── net             TCP服务器
│   ├── threadpool      线程池
│   ├── Web             网页库
│   ├── JsonCpp
│   ├── mylogger
│   ├── tinyxml2
│   ├── Configuration   配置读取
│   ├── WordQueryServer
│   ├── TestOnline      主程序
├── include             头文件
│   ├── cppjieba
│   ├── json
│   ├── net
│   ├── threadpool
│   ├── Web
│   ├── Redis
│   ├── mylogger
│   ├── tinyxml2
│   ├── Configuration
│   ├── WordQueryServer
├── conf                配置文件
│   ├── online.conf
├── data                前置数据
├── log                 日志文件
│   ├── search.log
├── php_TCP             前端文件
└── bin                 可执行文件
```

### 项目启动
**1. 配置第三方库**
- hiredis
```shell
git clone git@github.com:redis/hiredis.git
cd hiredis
make
./test.sh
sudo make install
```
- log4cpp下载地址：[log4cpp](https://sourceforge.net/projects/log4cpp/files/)
```shell
$ tar xzvf log4cpp-1.1.4rc3.tar.gz
$ cd log4cpp
$ ./configure  //进行自动化构建，自动生成makefile
$ make
$ sudo make install //安装  把头文件和库文件拷贝到系统路径下
    
//安装完后
//默认头文件路径：/usr/local/include/log4cpp
//默认lib库路径：/usr/local/lib
```

**2. 前端搭建**
```shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install apache2
sudo apt-get install php7.2 libapache2-mod-php7.2
sudo cp php_TCP /var/www/html/
```

**3. 更改配置**
- **conf/online.conf**:将所有的目录修改为自己的目录
- **src/TestOnline.cc**:将下面这条语句的目录更改为自己的conf/online.conf目录
```cpp
Configuration::getInstance(
    "/home/wjj/桌面/search/Forum-SearchEnigine/conf/online.conf");
```

**4. 启动服务器**
```shell
mkdir bin
make
./bin/SearchEngine.exe
```

**5. 打开网页**
在浏览器中输入http://127.0.0.1/php_TCP/index.html

### 压力测试
```shell
./WebBench/webbench -c 1000 -t 10 http://127.0.0.1:5080/
```
> -c 连接数量
> -t 连接时间

### 参考
[@stdbilly](https://github.com/stdbilly/SearchEnigine)
