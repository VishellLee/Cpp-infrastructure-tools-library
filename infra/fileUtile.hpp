#pragma once

#include <string>
#include <fstream>
#include <sstream>

namespace Infra
{
    inline std::string readFile(std::string const &fileName)
    {
        std::ifstream ifs(fileName, std::ios_base::in);

        if (!ifs.is_open())
        {
            return std::string();
        }

        std::stringstream ss;
        ss << ifs.rdbuf();

        return ss.str();
    }
}
