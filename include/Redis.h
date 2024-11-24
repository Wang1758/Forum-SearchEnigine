#pragma once
#include "mylogger.h"
#include <hiredis/hiredis.h>
#include <string>

namespace wd {
    class Redis {
    public:
        Redis() :_connect(nullptr), _reply(nullptr){};

        ~Redis() {
            if(_connect) {
                redisFree(_connect);
            }
            _connect = nullptr;
            _reply = nullptr;
        }
        
        //连接到指定的redis服务器
        bool connect(const std::string& ip, int port) {
            _connect = redisConnect(ip.c_str(), port);
            if (_connect == NULL || _connect->err){
                if (_connect){
                    LogError("Error: %s", _connect->errstr);
                } else {
                    LogError("Can't allocate redis context");
                }
                return false;
            }
            LogDebug("Connect to Redis server success");
            return true;
        }

        // 根据key获取对应的value，如果没有找到则返回“-1”
        std::string get(const std::string& key) {
            _reply = (redisReply*)redisCommand(_connect, "GET %s", key.c_str());
            LogDebug("Succeed to execute command: GET %s", key.c_str());

            if(_reply->type == REDIS_REPLY_NIL) {
                return string("-1");
            }
            string value = _reply->str;
            freeReplyObject(_reply);
            return value;
        }

        void set(const std::string& key, const std::string& value) {
            redisCommand(_connect, "SET %s %s", key.c_str(), value.c_str());
            LogDebug("Succeed to execute command: SET %s %s", key.c_str(), value.c_str());
        }
        
    private:
        redisContext* _connect;
        redisReply* _reply;
    };
}