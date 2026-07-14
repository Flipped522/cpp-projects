#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "util.hpp"

// 一个目录，存放所有的html
const std::string src_path = "data/input";
const std::string out_put = "data/raw_html/raw.txt";

typedef struct DocInfo
{
    std::string title;   // 文档标题
    std::string content; // 文档内容
    std::string url;     // 该文档在官网中的url
} DocInfo_t;
// const &:输入
// *: 输出
// &:输入输出
bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list);
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results);
bool SavaHtml(const std::vector<DocInfo_t> &results, const std::string &out_put);

int main()
{
    std::vector<std::string> file_list;
    // 1.递归式的将每个html文件名带路径，保存到files_list中，方便后期进行一个一个对文件进行读取
    if (!EnumFile(src_path, &file_list))
    {
        std::cerr << "enum file name error!" << std::endl;
        return 1;
    }

    // 2. 按照files_list 读取每个文件的内容，并进行解析
    std::vector<DocInfo_t> results;
    if (!ParseHtml(file_list, &results))
    {
        std::cerr << "parse html erroe" << std::endl;
        return 2;
    }

    // 3. 将解析完毕的各个文件内容，写入到output中，按照\3作为分隔符（\3不会显示）
    if (!SavaHtml(results, out_put))
    {
        std::cerr << "save html error" << std::endl;
        return 3;
    }

    return 0;
}

bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list)
{
    namespace fs = boost::filesystem;
    fs::path root_path(src_path);

    // 判断路径是否存在，不存在，就没必要往后执行
    if (!fs::exists(root_path))
    {
        std::cerr << src_path << " not exists" << std::endl;
        return false;
    }

    // 定义一个空的迭代器，用来进行判断递归的结束
    fs::recursive_directory_iterator end;
    for (fs::recursive_directory_iterator iter(root_path); iter != end; iter++)
    {
        // 判断文件是否是普通文件，.html 都是普通文件
        if (!fs::is_regular_file(*iter))
        {
            continue;
        }

        // 判断文件名的后缀是否符合要求
        if (iter->path().extension() != ".html")
        {
            continue;
        }
        // std::cout << "debug " << iter->path().string() << std::endl;
        // 当前路径是以.html结束的普通网页文件，将所有.html文件保存到filelist中，方便后续进行文本分析
        files_list->push_back(iter->path().string());
    }
    return true;
}

static bool ParseTitle(const std::string &file, std::string *title)
{
    size_t begin = file.find("<title>");
    if(std::string::npos == begin)
    {
        return false;
    }
    size_t end = file.find("</title>");
    if(std::string::npos == end)
    {
        return false;
    }
    begin += std::string("<title>").size();

    if(begin > end)
    {
        return false;
    }
    *title = file.substr(begin, end - begin);

    return true;
}

static bool ParseContent(const std::string &file, std::string *content)
{
    // 状态机
    enum status{
        LABEL,
        CONTENT
    }; 

    enum status s = LABEL;
    for(char c : file)
    {
        switch(s)
        {
            // 只要到了'>',说明当前的标签处理完毕
            case LABEL:
            if(c == '>')
                s = CONTENT;
            break;
            case CONTENT:
            if(c == '<')    // 只要碰到了'<',说明新的标签开始了
                s = LABEL;
            else
            {
                // 不保留原始文件的\n,\n作为html解析后的文本的分隔符
                if(c == '\n')
                    c = ' ';
                content->push_back(c);
            }
            break;
            default:
            break;
        }
    }
    return true;
}

static bool ParseUrl(const std::string& file_path, std::string *url)
{
    std::string url_head = "https://www.boost.org/doc/libs/1_78_0/doc/html";
    std::string url_tail = file_path.substr(src_path.size());
    *url = url_head + url_tail;

    return true;
}

void showDoc(DocInfo_t &doc)
{
    std::cout << "title: "<< doc.title << std::endl;
    std::cout << "content: " << doc.content << std::endl;
    std::cout << "url: " << doc.url << std::endl;
}

bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results)
{
    for (const std::string &file : files_list)
    {
        // 读取文件
        std::string result;
        if (!ns_util::FileUtil::ReadFile(file, &result))
        {
            continue;
        }
        DocInfo_t doc;
        // 解析指定的文件，提取title
        if (!ParseTitle(result, &doc.title))
        {
            continue;
        }
        // 解析指定的文件，提取content,本质是去标签
        if (!ParseContent(result, &doc.content))
        {
            continue;
        }
        // 解析指定的文件路径，构建url
        if (!ParseUrl(file, &(doc.url)))
        {
            continue;
        }

        results->push_back(doc);

        // fordebug
        showDoc(doc);
        break;
    }
    return true;
}

bool SavaHtml(const std::vector<DocInfo_t> &results, const std::string &out_put)
{
    return true;
}