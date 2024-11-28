#pragma once 
#include <iostream>
#include <string>
#include <vector>
#include "Configuration.h"
#include "cppjieba/Jieba.hpp"

namespace wd {
    class WordSegmentation {
    public:
        WordSegmentation() : _jieba(CONFIG[DICT_PATH], CONFIG[HMM_PATH], 
            CONFIG[USER_DICT_PATH], CONFIG[IDF_PATH], CONFIG[STOP_WORD_PATH]) {
            cout << ">> jieba init" << endl;
        }

        std::vector<std::string> operator()(const std::string& str) {
            std::vector<std::string> words;
            _jieba.Cut(str, words, true);
            auto stopWords = Configuration::getInstance()->getStopWords();
            for(auto it = words.begin(); it != words.end(); ++it){
                auto uit = stopWords.find(*it);
                if(uit != stopWords.end()) {
                    words.erase(it);
                }
            }
            return words;
        }

    private:
        cppjieba::Jieba _jieba;
    };
}