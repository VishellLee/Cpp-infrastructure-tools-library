#pragma once

#include <string>
#include <vector>

namespace Infra
{
    inline std::vector<std::string> splitStr(std::string const& str, std::string const& delim) {
        std::vector<std::string> result;

        if (delim.empty()) {
            result.push_back(str);
            return result;
        }

        std::size_t start = 0;
        std::size_t end = str.find(delim);

        while (end != std::string::npos) {
            std::string token = str.substr(start, end - start);
            if (!token.empty()) {
                result.push_back(token);
            }
            start = end + delim.size();
            end = str.find(delim, start);
        }

        std::string lastToken = str.substr(start);
        if (!lastToken.empty()) {
            result.push_back(lastToken);
        }

        return result;
    }
}
