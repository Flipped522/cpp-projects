#include "index.hpp"

namespace ns_searcher{
    class Searcher
    {
        public:
            Searcher(){}
            ~Searcher(){}

            void InitSearcher(const std::string& input)
            {
                // 1. 获取或者创建index对象

                // 2. 根据index对象建立索引
            }

            // query：搜索关键字
            // json_string：返回给用户浏览器的搜索结果
            void Search(const std::string& query, std::string *json_string)
            {
                // 1. [分词]:对query进行按照searcher的要求进行分词

                // 2. [触发]:根据分词的各个“词”，进行index查找

                // 3. [合并排序]:汇总查找结果，按照相关性进行(weight)降序排序

                // 4. [构建]：根据查找出来的结果，构建json串
            }
        private:
            ns_index::Index *index;
    };
}