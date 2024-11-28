#include "Configuration.h"
#include "mylogger.h"
#include <fstream>
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

namespace wd {

    Configuration* Configuration::_pInstance = nullptr;

    Configuration* Configuration::getInstance() {
        if(_pInstance) {
            return _pInstance;
        } else {
            return nullptr;
        }
    }

    Configuration* Configuration::getInstance(const std::string& filepath) {
        if(_pInstance == nullptr) {
            ::atexit(destroy);
            _pInstance = new Configuration(filepath);
        } else {
            LogInfo("configuration已经存在,路径是%s,无法为新路径%s创建新对象",_pInstance->_filepath, filepath);
        }
        return _pInstance;
    }

    void Configuration::destroy() {
        if(_pInstance) {
            delete _pInstance;
        }
    }

    Configuration::Configuration(const std::string& filepath) {
        _filepath = filepath;
        readFile(filepath);
        std::cout <<"Configuration(const string&)" <<std::endl;
    }

    void Configuration::readFile(const std::string& filename) {
        std::ifstream ifs(filename);
        if(!ifs) {
            perror("fopen");
            return;
        }
        
        std::string line, key, value;
        while(getline(ifs, line)) {
            std::istringstream iss(line);
            iss >> key >> value;
            _configMap.insert(make_pair(key, value));
        }
        LogDebug("read config file success");
    }

    unordered_set<string>& Configuration::getStopWords() {
        if(_stopWords.size() >0) {
            return _stopWords;
        }

        std::ifstream ifs(_configMap[STOP_WORD_PATH]);
        if(!ifs) {
            perror("fopen stopwordpath");
        }

        string line;
        while(getline(ifs, line)) {
            _stopWords.insert(line);
        }
        return _stopWords;
    }
}