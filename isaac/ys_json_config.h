#ifndef _YS_JSON_CONFIG_H_
#define _YS_JSON_CONFIG_H_

#include "xpack/json.h"
#include "tool/ys_file.hpp"
#include "ys_cli_log.h"

namespace isaac {
// json配置文件到结构体 依赖xpack
class YS_Conf
{
public:
    // 导入json文件配置
    // 一个类型一份
    // 空path指向程序路径下程序名.json
    template <class T>
    static T &load(std::string path = "");

    // 保存配置到文件
    template <class T>
    static int32_t save(T &tConf, std::string path = "./ys_conf.json", bool withSpace = true);
};

template <class T>
T &isaac::YS_Conf::load(std::string path)
{
    static T    tConf;
    static bool hasLoad = false;
    if (hasLoad)
    {
        return tConf;
    }
    if (path.empty())
    {
        path = YS_Proc::exeFile() + ".json";
    }
    std::string conf = YS_File::read(path);
    if (conf.length())
    {
        try
        {
            xpack::json::decode(conf, tConf);
        }
        catch (const std::exception &e)
        {
            CLI_ERROR("parse conf fail [{}].", e.what());
        }
        hasLoad = true;
    }
    return tConf;
}

template <class T>
int32_t YS_Conf::save(T &tConf, std::string path, bool withSpace)
{
    std::string             conf;
    rapidjson::StringBuffer sBuf;
    if (path.empty())
    {
        path = YS_Proc::exeFile() + ".json";
    }

    try
    {
        conf = xpack::json::encode(tConf);
        // 格式化为带缩进的json
        rapidjson::Document doc;
        doc.Parse(conf.c_str());
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sBuf);
        doc.Accept(writer);
    }
    catch (const std::exception &e)
    {
        CLI_ERROR("encode conf fail [{}]", e.what());
        return -1;
    }
    CLI_INFO("save config to {}", path);
    return YS_File::write(path, (uint8_t *)sBuf.GetString(), sBuf.GetLength());
}

}; // namespace isaac

#endif