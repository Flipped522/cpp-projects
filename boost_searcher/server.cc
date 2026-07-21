#include "searcher.hpp"

const std::string input = "./data/raw_html/raw.txt";

int main()
{
    // for test
    ns_searcher::Searcher *seracher = new ns_searcher::Searcher();
    seracher->InitSearcher(input);

    std::string query;
    std::string json_string;
    while(true)
    {
        std::cout << "Please Enter Your Searcher Query # ";
        std::cin >> query;
        seracher->Search(query, &json_string);

        std::cout << json_string << std::endl;
    }

    return 0;
}