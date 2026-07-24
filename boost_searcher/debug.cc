#include "searcher.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>

const std::string input = "./data/raw_html/raw.txt";

int main()
{
    // for test
    ns_searcher::Searcher *searcher = new ns_searcher::Searcher();
    searcher->InitSearcher(input);

    std::string query;
    std::string json_string;
    char buffer[1024];
    while(true)
    {
        std::cout << "Please Enter Your Searcher Query # ";
        fgets(buffer, sizeof(buffer) - 1, stdin);
        buffer[strlen(buffer) - 1] = 0;
        query = buffer;
        // std::cin >> query;
        searcher->Search(query, &json_string);

        std::cout << json_string << std::endl;
    }

    return 0;
}