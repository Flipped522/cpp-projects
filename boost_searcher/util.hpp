#include <iostream>
#include <string>
#include <fstream>

namespace ns_util{
    class FileUtil{
        public:
            static bool ReadFile(const std::string& file_path, std::string* out)
            {
                std::ifstream in(file_path, std::ios::in);
                if(in.is_open())
                {
                    std::cerr << "open file " << file_path << "error" << std::endl;
                }

                std::string line;
                while(std::getline(in, line))
                {
                    *out += line;
                }

                in.close();
                return true;
            } 
    };
};