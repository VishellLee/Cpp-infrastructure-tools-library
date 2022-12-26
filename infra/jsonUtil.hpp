#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include "json/json.h"

namespace Infra
{
    inline Json::Value strToJson(std::string const &str, bool collectComments = true)
    {
        Json::Value ret;
        Json::Reader().parse(str, ret, collectComments);

        return ret;
    }

    inline void strToJson(std::string const &str, Json::Value &value, bool collectComments = true)
    {
        Json::Reader().parse(str, value, collectComments);
    }

    inline void jsonToStrFormat(Json::Value const &value, std::string &str)
    {
        std::ostringstream oss;
        std::unique_ptr<Json::StreamWriter> { Json::StreamWriterBuilder().newStreamWriter() } -> write(value, &oss);
        str = oss.str();
    }

    inline std::string jsonToStrFormat(Json::Value const &value)
    {
        std::ostringstream oss;
        std::unique_ptr<Json::StreamWriter> { Json::StreamWriterBuilder().newStreamWriter() } -> write(value, &oss);
        return oss.str();
    }

    inline void jsonToStr(Json::Value const &value, std::string &str)
    {
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = "";
        std::ostringstream oss;
        std::unique_ptr<Json::StreamWriter> { builder.newStreamWriter() } -> write(value, &oss);
        str = oss.str();
    }

    inline std::string jsonToStr(Json::Value const &value)
    {
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = "";
        std::ostringstream oss;
        std::unique_ptr<Json::StreamWriter> { builder.newStreamWriter() } -> write(value, &oss);
        return oss.str();
    }

    inline Json::Value readJson(std::string const &fileName)
    {
        Json::Value ret;
        std::ifstream ifs(fileName, std::ios_base::in);

        if (!ifs.is_open())
        {
            return ret;
        }

        Json::CharReaderBuilder reader;
        std::string errs;
        Json::parseFromStream(reader, ifs, &ret, &errs);

        return ret;
    }

    inline Json::Value &removeJsonEmptyMember(Json::Value &value)
    {
        Json::Value::Members members = value.getMemberNames();

        for (auto &member : members)
        {
            if (value[member].empty())
            {
                value.removeMember(member);
            }
        }

        return value;
    }
}
