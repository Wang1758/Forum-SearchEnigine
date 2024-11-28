#include "Web/WebPage.h"
#include <iostream>
#include <sstream>
using std::istringstream;

namespace wd {
    WebPage::WebPage(int id, const string &title, const string &link,
                     const string &content)
        : _docid(id), _title(title), _link(link), _content(content) {}

    // 计算一个字符ch的字节数
    size_t WebPage::getBytes(const char ch) {
        int nByte = 1;
        // 检查最高位（8）是否为1
        if(ch & (1 << 7)) {
            // 循环检查第7位到第2位是否为1
            for(int i = 0; i<6; ++i) {
                if(ch & (1 << (6 - i))) {
                    ++nByte;    // 如果是1，则增加字节数
                }else {
                    break;  // 遇到0则不用继续检查
                }
            }
            return nByte;
        }
        // 当最高位为0时，则字节数为1
        return nByte;
    }

    // 计算一个utf-8的字符串的字符数量
    size_t WebPage::length(const std::string& str) {
        std::size_t ilen = 0;
        for(std::size_t i = 0;i != str.size(); ++i) {
            int nByte = getBytes(str[i]);
            i += (nByte - 1);
            ++ilen;
        }
        return ilen;
    }

    // 提取_contend中包含查询词的行，最多寻找两行，返回结果字符串
    string WebPage::summary(const vector<string>& queryWords) {
        vector<string> summaryVec;
        string period = "。";
        istringstream iss(_content);
        string line;
        while(iss >> line) {
            for(auto word : queryWords) {
                string result;
                size_t pos = line.find(word);
                // 如果在当前line中找到了查询词，则继续
                if(pos != string::npos) {
                    // 长度太长，需要分割
                    if(length(line) >= 100) {
                        // 寻找查询词之前和之后的第一个句号
                        size_t p1 = line.rfind(period, pos);
                        size_t p2 = line.find(period, pos);

                        // word前后都有句号
                        if(p1 != string::npos && p2!= string::npos) {
                            p1 += getBytes(line[p1]);
                            p2 -= getBytes(line[p2]);
                            result = line.substr(p1, p2-p1);
                        } else if(p1!=string::npos && p2 == string::npos) {
                            // word 之后没有句号
                            p1 += getBytes(line[p1]);
                            string temp = line.substr(p1);
                            size_t len = length(temp);
                            if(len > 100) {
                                len = 100;
                            }
                            p2 = p1;
                            for(size_t ilen = 0; ilen < len; ++ilen,++p2) {
                                size_t bytes = getBytes(temp[p2]);
                                p2 += (bytes - 1);
                            }
                            result = line.substr(p1, p2-p1);
                        } else if(p1 == string::npos && p2 != string::npos) {
                            // word之前没有句号
                            p2 -= getBytes(line[p2]);
                            result = line.substr(0,p2);
                        } else {
                            // word前后都没有句号
                            size_t start = pos > 50 ? pos - 50 : 0;
                            size_t end = pos + 50 <line.size() ? pos + 50 : line.size();
                            result = line.substr(start, end-start);
                        }
                    } else {
                        // 当前行长度小于100个字
                        result = line;
                    }
                    result.append("...");
                    summaryVec.push_back(result);
                } else {
                    // line中没找到这个word，继续下一个
                    continue;
                }
            }

            if(summaryVec.size() >=2) {
                break;
            }
        }

        string summary;
        for(auto s : summaryVec) {
            summary.append(s).append("\n");
        }
        return summary;
    }
    
}