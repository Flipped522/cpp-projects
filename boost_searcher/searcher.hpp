#include "index.hpp"
#include "util.hpp"
#include "algorithm"
#include <jsoncpp/json/json.h>

namespace ns_searcher
{
    class Searcher
    {
    public:
        Searcher() {}
        ~Searcher() {}

        void InitSearcher(const std::string &input)
        {
            // 1. 获取或者创建index对象
            index = ns_index::Index::GetInstance();
            std::cout << "获取index单例成功" << std::endl;
            // 2. 根据index对象建立索引
            index->BuildIndex(input);
            std::cout << "建立正排和倒排索引成功" << std::endl;
        }

        // query：搜索关键字
        // json_string：返回给用户浏览器的搜索结果
        void Search(const std::string &query, std::string *json_string)
        {
            // 1. [分词]:对query进行按照searcher的要求进行分词
            std::vector<std::string> words;
            ns_util::JiebaUtil::CutString(query, &words);
            // 2. [触发]:根据分词的各个“词”，进行index查找,建立索引的值忽略大小写
            ns_index::InvertedList inverted_list_all;
            for (std::string word : words)
            {
                boost::to_lower(word);
                ns_index::InvertedList *inverted_list = index->GetInvertedList(word);
                if (nullptr == inverted_list)
                {
                    continue;
                }
                inverted_list_all.insert(inverted_list_all.end(),
                                         inverted_list->begin(), inverted_list->end());
            }
            // 3. [合并排序]:汇总查找结果，按照相关性进行(weight)降序排序
            sort(inverted_list_all.begin(), inverted_list_all.end(), [](const ns_index::InvertedElem &e1, const ns_index::InvertedElem &e2)
                 { return e1.weight > e2.weight; });
            // 4. [构建]：根据查找出来的结果，构建json串 --jsoncpp
            Json::Value root;
            for (auto &item : inverted_list_all)
            {
                ns_index::DocInfo *doc = index->GetForwordIndex(item.doc_id);
                if (nullptr == doc)
                {
                    continue;
                }
                Json::Value elem;
                elem["title"] = doc->title;
                // elem["desc"] = doc->content;
                elem["desc"] = GetDesc(doc->content, item.word);
                elem["url"] = doc->url;
                elem["id"] = (int)item.doc_id;
                elem["weight"] = item.weight;

                root.append(elem);
            }
            Json::StyledWriter writer;
            *json_string = writer.write(root);
        }

        std::string GetDesc(const std::string &html_content, const std::string &word)
        {
            // 找到word在html_content中的首次出现，然后往后找50字节（如果没有，从begin开始），往后u找100字节（如果没有，到end即可）
            // 截取这部分内容
            const int prev_step = 50;
            const int next_step = 100;
            // 1. 找到首次出现
            auto iter = std::search(html_content.begin(), html_content.end(), word.begin(), word.end(), [](int x, int y)
                                    { return (std::tolower(x) == std::tolower(y)); });
            if(iter == html_content.end())
                return "None";
            
            int pos = std::distance(html_content.begin(), iter);

            // 2. 获取start,end
            int start = 0;
            int end = html_content.size() - 1;
            // 如果之前有50+字符，就更新开始位置
            if (pos > start + prev_step)
                start = pos - prev_step;
            if (pos < end - next_step)
                end = pos + next_step;

            // 3.截取子串，return
            if (start >= end)
                return "None";
            std::string desc = html_content.substr(start, end - start);
            desc += "...";
            return desc;
        }

    private:
        ns_index::Index *index;
    };
}