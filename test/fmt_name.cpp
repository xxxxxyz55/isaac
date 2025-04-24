#include "tool/ys_file.hpp"
#include "ys_cli_log.h"

using namespace isaac;
using namespace std;

string convertName(string);

int32_t main(int32_t argc, char const *argv[])
{
    std::string path = "./";
    if (argc == 2)
    {
        path = argv[1];
    }

    if (!YS_File::isDir(path))
    {
        CLI_ERROR("{} isn`t a dir.", path);
        return -1;
    }

    vector<YS_File::DirInfo> vtDir;

    int32_t ret = YS_File::scanDir(path, vtDir);
    if (ret)
    {
        CLI_ERROR("scan {} fail.", path);
        return -1;
    }
    vector<pair<string,string>> vtFile;

    for (auto &&iter : vtDir)
    {
        if (iter.name == "." || iter.name == "..") continue;

        string name = convertName(iter.name);
        if(name != iter.name)
        {
            string src = path + "/" + iter.name;
            string dst = path + "/" + name;
            if (rename(src.c_str(), dst.c_str()) == 0)
            {
                CLI_PRINT("{:32s} ==> {:32s}\n", src,dst);
            }
            else
            {
                CLI_ERROR("rename fail, {} => {}", src, dst);
                continue;
            }
        }
        vtFile.push_back({name, iter.name});
    }

    string arr;
    for (auto &&iter : vtFile)
    {
        arr.append(fmt::format("    {{name:\"{}\",desc:\"{}\"}},\n", iter.first, iter.second));
    }
    if (!arr.empty())
    {
        arr.pop_back();
        arr.pop_back();
    }
    CLI_PRINT("\n{{\n{}\n}}\n", arr);

    return 0;
}

string convertName(string src)
{
    string dst;
    dst.reserve(src.length());
    for (auto &&iter : src)
    {
        if (iter == ' ' || iter == '\"' || iter == '\'' || iter == '`')
        {
            dst.push_back('-');
        }
        else
        {
            dst.push_back(iter);
        }
    }
    return dst;
}