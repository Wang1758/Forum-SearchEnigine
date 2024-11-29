#include "WordQuery.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include "Redis.h"
#include "json/json.h"
#include "tinyxml2.h"
using namespace tinyxml2;
using std::ifstream;
using std::istringstream;
using std::make_pair;
using std::stoi;

namespace wd {
    namespace current_thread {
        extern thread_local Redis* predis;
    }

    // 比较两个向量与基向量的相似度，使用了余弦相似度作为判断依据
    struct SimilarityCompare {
        vector<double> _base;   // 基向量
        SimilarityCompare(vector<double>& base) : _base(base) {};

        // 比较对象lhs和rhs的余弦相似度，若lhs>rhs返回true
        bool operator()(const pair<int, vector<double>>& lhs,
                        const pair<int, vector<double>>& rhs) {
            double lhsCrossProduct = 0; //lhs与基向量的点积
            double rhsCrossProduce = 0; //rhs与基向量的点积
            double lhsVectorLength = 0; //lhs的范数（长度平方）
            double rhsVectorLength = 0; //rhs的范数

            for(int i = 0; i != _base.size(); ++i) {
                lhsCrossProduct += (lhs.second)[i] * _base[i];
                rhsCrossProduce += (rhs.second)[i] * _base[i];
                lhsVectorLength += pow((lhs.second)[i], 2);
                rhsVectorLength += pow((rhs.second)[i], 2);
            }

            if(lhsCrossProduct / sqrt(lhsVectorLength) < 
               rhsCrossProduce / sqrt(rhsVectorLength)) {
                return true;
            } else {
                return false;
            }
        }
    };

    WordQuery::WordQuery() {
        _pageLib.reserve(300);
        loadLibrary();
    }

    // 加载网页库与倒排索引表
    void WordQuery::loadLibrary(){
        // 加载网页库
        XMLDocument doc;
        doc.LoadFile(CONFIG[NEW_RIPEPAGE_PATH].c_str());
        XMLElement *page = doc.FirstChildElement("doc");
        XMLError errid = doc.ErrorID();
        if (errid){
            LogError("tinyxml load ripepage error");
            return;
        }

        // 解析每个doc元素，将提取到的每个doc元素创建一个WebPage对象
        do{
            string docid = page->FirstChildElement("docid")->GetText();
            string title = page->FirstChildElement("title")->GetText();
            string link = page->FirstChildElement("link")->GetText();
            string content = page->FirstChildElement("content")->GetText();

            WebPage webpage(stoi(docid), title, link, content);
            _pageLib.insert(make_pair(stoi(docid), webpage));
        } while (page = page->NextSiblingElement());
        LogDebug("load rigepage success, size = %d", _pageLib.size());

        // 加载倒排索引表
        ifstream ifsIndex(CONFIG[INDEX_PATH]);
        if (!ifsIndex.good()){
            LogError("ifstream load invert index error");
            return;
        }
        string word, line;
        int docid;
        double weight;
        while (getline(ifsIndex, line)){
            istringstream iss(line);
            iss >> word;
            set<pair<int, double>> tempSet;
            while (iss >> docid >> weight)
            {
                tempSet.insert(make_pair(docid, weight));
            }
            _invertIndex.insert(make_pair(word, tempSet));
        }
        ifsIndex.close();
        LogDebug("load invert index success, size = %d", _invertIndex.size());
    }

    string WordQuery::doQuery(const string &str) {
        // 检查查询字符串是否为空
        if (str.size() == 0) {
            return returnNoAnswer();
        }
        // 查询redis缓存是否已经有结果
        string ret = current_thread::predis->get(str);
        if (ret != "-1") {
            LogDebug("cache hit: %s", str.c_str());
            return ret;
        }
        LogDebug("cache miss, query invert index");
        // 使用jieba对查询语句进行分词
        vector<string> queryWords;
        queryWords = _jieba(str);
        // 检查倒排索引，确保所有的词都在倒排索引中
        for (auto &item : queryWords) {
            auto it = _invertIndex.find(item);
            // 如果有个词不在倒排索引中，返回默认答案，并缓存结果
            if (it == _invertIndex.end()) {
                LogInfo("%s not found", item.c_str());
                ret = returnNoAnswer();
                current_thread::predis->set(str, ret);
                return ret;
            }
        }
        // 计算每个词的权重
        vector<double> weightList = getQueryWordsWeight(queryWords);
        SimilarityCompare similarityCom(weightList);
        // 获取每个查询词的相关文档id
        vector<int> pages = getPages(queryWords);
        if (pages.size() == 0) {
            LogInfo("Not found");
            ret = returnNoAnswer();
            current_thread::predis->set(str, ret);
            return ret;
        }

        // 构建结果列表并排序
        vector<pair<int, vector<double>>> resultList;
        for (auto &docid : pages) {     // 遍历每一个文档id，构建每个文档的词权重列表
            vector<double> wordWeight;
            for (auto &word : queryWords) {
                auto tmpSet = _invertIndex.find(word)->second;
                for (auto &item : tmpSet) {
                    if (item.first == docid) {
                        wordWeight.push_back(item.second);
                    }
                }
            }
            resultList.emplace_back(make_pair(docid, weightList));
        }
        // 对resultList进行稳定排序，排序依据是词权重与基权重的余弦相似度
        std::stable_sort(resultList.begin(), resultList.end(), similarityCom);
        // 清空pages，并将排序后的结果存入pages中
        pages.clear();
        for (auto &item : resultList) {
            pages.push_back(item.first);
        }
        // 生成json并缓存结果
        ret = createJson(pages, queryWords);
        current_thread::predis->set(str, ret);
        return ret;
    }

    // 获取所有查询词都出现的page
    vector<int> WordQuery::getPages(vector<string> queryWords) {
        vector<int> pages;
        for (auto &item : _invertIndex[queryWords[0]]) {
            pages.push_back(item.first);
        }
        std::sort(pages.begin(), pages.end());
        for (auto it = queryWords.begin() + 1; it != queryWords.end(); ++it) {
            vector<int> temp, rhs;
            for (auto &item : _invertIndex[*it])
            {
                rhs.push_back(item.first);
            }
            std::sort(rhs.begin(), rhs.end());
            std::set_intersection(pages.begin(), pages.end(), rhs.begin(),
                                  rhs.end(), std::back_inserter(temp));
            pages.swap(temp);
        }
        return pages;

        // 下面的代码是page出现了任意一个查询词，都作为返回结果
        /*
        unordered_set<int> pagesSet; // 使用unordered_set避免重复文档ID
        for (const auto &word : queryWords)
        {
            auto it = _invertIndex.find(word);
            if (it != _invertIndex.end())
            {
                for (const auto &item : it->second)
                {
                    pagesSet.insert(item.first); // 插入文档ID
                }
            }
        }

        vector<int> pages(pagesSet.begin(), pagesSet.end()); // 将unordered_set转换为vector
        return pages;
        */
    }

    // 使用TF-IDF算法计算查询词的权重
    vector<double> WordQuery::getQueryWordsWeight(vector<string> &queryWords){
        unordered_map<string, int> wordFreq;
        for (auto &item : queryWords){
            ++wordFreq[item];
        }
        // 计算权重
        vector<double> weightVec;
        double weightSum = 0;
        int totalPageNum = _pageLib.size();

        for (auto &word : queryWords){
            int tf = wordFreq[word];
            int df = _invertIndex[word].size();
            double idf = log2(static_cast<double>(totalPageNum / (df + 1)));
            double w = idf * tf;
            weightSum += pow(w, 2);
            weightVec.push_back(w);
        }
        // 归一化
        for (auto &item : weightVec){
            item /= sqrt(weightSum);
        }
        return weightVec;
    }

    string WordQuery::createJson(vector<int> &docIdVec,
                                 const vector<string> &queryWords) {
        Json::Value root;
        Json::Value arr;

        int cnt = 0;
        for (auto id : docIdVec) {
            string summary = _pageLib[id].summary(queryWords);
            string title = _pageLib[id].getTitle();
            string url = _pageLib[id].getUrl();

            Json::Value elem;
            elem["title"] = title;
            elem["summary"] = summary;
            elem["url"] = url;
            arr.append(elem);
        }

        root["files"] = arr;
        Json::StyledWriter writer;
        return writer.write(root);
    }

    string WordQuery::returnNoAnswer() {
        Json::Value root;
        Json::Value arr;

        Json::Value elem;
        elem["title"] = "404, not found";
        elem["summary"] = "未找到你搜索的内容";
        elem["url"] = "";
        arr.append(elem);
        root["files"] = arr;
        Json::StyledWriter writer;
        return writer.write(root);
    }
}
