#include "mylogger.h"
#include <stdlib.h>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/RollingFileAppender.hh>

namespace wd{
    Mylogger* Mylogger::getInstance() {
        pthread_once(&_once, init);
        return _pInstance;
    }

    void Mylogger::init(){
        ::atexit(destroy);
        _pInstance = new Mylogger();
    }   

    void Mylogger::destroy() {
        if(_pInstance) {
            delete _pInstance;
            _pInstance = nullptr;
        }
    }

    Mylogger::Mylogger()
        : _mycategory(Category::getRoot().getInstance("mycategory")) {
        // 创建布局
        PatternLayout *ptnLayout1 = (new PatternLayout());
        PatternLayout *ptnLayout2 = (new PatternLayout());
        // 创建附加器，将日志输出到标准输出
        OstreamAppender *ostreamAppender =
            (new OstreamAppender("OstreamAppender", &std::cout));
        // 将日志输出到文件_filename
        FileAppender *fileAppender = (new FileAppender("FileAppender", _filename));
        // 设置转换模式，%d：日期时间,%p：优先级,%m：日志消息,
        ptnLayout1->setConversionPattern("%d [%p] %m%n");
        ptnLayout2->setConversionPattern("%d [%p] %m%n");
        // 设置附加器布局
        ostreamAppender->setLayout(ptnLayout1);
        fileAppender->setLayout(ptnLayout2);
        // 设置优先级，并添加附加器
        _mycategory.setPriority(Priority::DEBUG);
        _mycategory.addAppender(ostreamAppender);
        _mycategory.addAppender(fileAppender);
    }

    Mylogger::~Mylogger() {
        Category::shutdown();
    }

    Mylogger* Mylogger::_pInstance = nullptr;
    pthread_once_t Mylogger::_once = PTHREAD_ONCE_INIT;
    string Mylogger::_filename = "/home/桌面/search/Forum-SearchEnigine/log/search.log";
}