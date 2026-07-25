#include "searcher.hpp"
#include "cpphttplib/httplib.h"

const std::string root_path = "./wwwroot";

int main()
{
    httplib::Server svr;

    svr.set_base_dir(root_path.c_str());
    svr.Get("/hi", [](const httplib::Request &req, httplib::Response &rsp){
        rsp.set_content("必须要有搜索关键字!", "text/plain; charset=utf-8");
    });
    svr.listen("0.0.0.0", 8081);
    return 0;
}