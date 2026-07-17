#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include "util.hpp"

namespace ns_index{
    struct DocInfo
    {
        std::string title; // 文档标题
        std::string content; // 文档对应的去标签之后的内容
        std::string url; // 官网文档的url
        uint64_t doc_id;      // 文档的id
    };

    struct InvertedElem{
        uint64_t doc_id;
        std::string word;
        int weight;
    };
    typedef std::vector<InvertedElem> InvertedList;
    class Index{
        private:
            // 正排索引的数据结构用数组，数组下表当作文档的id
            std::vector<DocInfo> forword_index; // 正排索引
            // 倒排索引：一个关键字和一组InvertedElem对应
            std::unordered_map<std::string, InvertedList> inverted_index; 

            DocInfo* BuildForwordIndex(const std::string &line)
            {
                // 解析line，字符串切分
                // line-> string title content url
                const std::string sep = "\3";
                std::vector<std::string> results;
                ns_util::StringUtil::CutString(line, &results,sep);
                if(3 != results.size())
                {
                    return nullptr;
                }
                // 字符串进行填充，填充到DocInfo中
                DocInfo doc;
                doc.title = results[0];
                doc.content = results[1];
                doc.url = results[2];
                doc.doc_id = forword_index.size(); // 先保存id,再插入
                // 插入到正排索引的vector
                forword_index.push_back(std::move(doc));
                return &forword_index.back();
            }

            bool BuildInvertedIndex(const DocInfo &doc)
            {
                // DocInfo(title, content, url, doc_id)
                // word -> 倒排拉链
                
                return true;
            }
        public:
            Index()
            {

            }

            ~Index()
            {

            }

            // 根据doc_id找到文档内容
            DocInfo* GetForwordIndex(const uint64_t doc_id)
            {
                if(doc_id >= forword_index.size())
                {
                    std::cerr << "doc_id" << std::endl;
                    return nullptr;
                }
                return &forword_index[doc_id];
            }

            // 根据关键词string,获得倒排拉链
            InvertedList* GetInvertedList(const std::string &word)
            {
                auto iter = inverted_index.find(word);
                if(inverted_index.end() == iter)
                {
                    std::cerr << word << " have no InvertedList" << std::endl;
                    return nullptr;
                }
                return &(iter->second);
            }

            // 构建索引 parse处理完毕的数据交给这个
            bool BuildIndex(const std::string &input) 
            {
                std::ifstream in(input, std::ios::in | std::ios::binary);
                if(!in.is_open())
                {
                    std::cerr << "sorry, " << input << "open erroe" << std::endl;
                    return false;
                }
                std::string line;
                while(std::getline(in, line))
                {
                    DocInfo* doc = BuildForwordIndex(line);
                    if(nullptr == doc)
                    {
                        std::cerr << "build " << line << "error" << std::endl;// for debug
                        continue;              
                    }

                    BuildInvertedIndex(*doc);
                }
                return true;
            }
    };
}