#pragma once

#include <string>
#include <vector>

namespace Infra
{
    std::vector<std::string> splitStr(std::string const &str, std::string const &delim)
    {
        std::vector<std::string> strs;

        std::size_t begin = 0, loc = std::string::npos;

        do
        {
            loc = str.find(delim, begin);

            if (loc != begin)
            {
                strs.push_back(str.substr(begin, loc - begin));
            }

            begin = loc + 1;
        } while (loc != std::string::npos && begin < str.size());

        return strs;
    }
}
