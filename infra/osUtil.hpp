#pragma once

#include <string>
#include <cstdio>

namespace Infra
{
    inline std::string bash(std::string const &bashScript)
    {
        constexpr int BUF_SIZE = 1024;

        char outBuffer[BUF_SIZE];
        FILE *pipePtr = popen(bashScript.c_str(), "r");

        std::string ret;

        if (!pipePtr)
        {
            perror("Fail to popen\n");
            return ret;
        }

        while (std::fgets(outBuffer, BUF_SIZE, pipePtr) != nullptr)
        {
            ret += outBuffer;
        }

        pclose(pipePtr);

        return ret;
    }
}
