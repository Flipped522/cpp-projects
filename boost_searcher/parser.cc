#include <iostream>
#include <string>
#include <vector>

// 一个目录，存放所有的html
const std::string src_path = "data/input/";
const std::string out_put = "data/raw_html/raw.txt";

typedef struct DocInfo
{
    std::string title;   // 文档标题
    std::string content; // 文档内容
    std::string url;     // 该文档在官网中的url
}DocInfo_t;
// const &:输入
// *: 输出
// &:输入输出
bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list);
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results);
bool SavaHtml(const std::vector<DocInfo_t>& results, const std::string &out_put);

int main()
{ 
    std::vector<std::string> file_list;
    // 1.递归式的将每个html文件名带路径，保存到files_list中，方便后期进行一个一个对文件进行读取
    if(!EnumFile(src_path, &file_list))
    {
        std::cerr << "enum file name error!" << std::endl;
        return 1;
    }

    // 2. 按照files_list 读取每个文件的内容，并进行解析
    std::vector<DocInfo_t> results;
    if(!ParseHtml(file_list, &results))
    {
        std::cerr << "parse html erroe" << std::endl;
        return 2;
    }

    // 3. 将解析完毕的各个文件内容，写入到output中，按照\3作为分隔符（\3不会显示）
    if(!SavaHtml(results, out_put))
    {
        std::cerr << "save html error" << std::endl;
        return 3;
    }

    return 0;
}

bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list)
{

}

bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results)
{

}

bool SavaHtml(const std::vector<DocInfo_t>& results, const std::string &out_put)
{

}