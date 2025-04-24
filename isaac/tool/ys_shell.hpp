#pragma once
#ifndef _YS_SHELL_H_
#define _YS_SHELL_H_

#include <vector>
#include <cstdint>
#include "ys_file.hpp"

namespace isaac {

// shell命令
class YS_Shell
{
public:
    // 执行命令获取多行结果
    // 一行最大8192,超过会被分割
    // 结尾\n删除
    // window使用注意路径和参数包含空格的情况
    static int32_t queryRows(std::string cmd, std::vector<std::string> &rows);

    // 执行命令获取结果
    // window使用注意路径和参数包含空格的情况
    static int32_t queryRow(std::string cmd, std::string &row);

    // 执行命令
    static int32_t exec(std::string cmd) { return ::system(cmd.c_str()); }

    // 当前环境是否包含工具
    // linux 使用where
    // windows 使用 command -v
    static bool supportCmd(std::string cmd);
};

inline int32_t YS_Shell::queryRows(std::string cmd, std::vector<std::string> &rows)
{
    FILE *pFile;
    char  sLine[8192] = {0x00};

#if _WIN32
    pFile = _popen(cmd.c_str(), "r");
#else
    pFile = popen(cmd.c_str(), "r");
#endif
    if (!pFile)
    {
        return -1;
    }
    else
    {
        while (fgets(sLine, sizeof(sLine), pFile))
        {
            int32_t iLen = strlen(sLine);
            if (iLen == 0)
            {
                continue;
            }
            if (sLine[iLen - 1] == '\n')
            {
                sLine[iLen - 1] = '\0';
            }
            rows.push_back(sLine);
        }
#if _WIN32
        _pclose(pFile);
#else
        pclose(pFile);
#endif
    }

    return 0;
}

inline int32_t YS_Shell::queryRow(std::string cmd, std::string &row)
{
    FILE *pFile;
    char  sLine[8192] = {0x00};

#if _WIN32
    pFile = _popen(cmd.c_str(), "r");
#else
    pFile = popen(cmd.c_str(), "r");
#endif
    if (!pFile)
    {
        return -1;
    }
    else
    {
        while (fgets(sLine, sizeof(sLine), pFile))
        {
            int32_t iLen = strlen(sLine);
            if (iLen == 0)
            {
                continue;
            }
            row.append(sLine);
        }
#if _WIN32
        _pclose(pFile);
#else
        pclose(pFile);
#endif
    }

    return 0;
}

inline bool YS_Shell::supportCmd(std::string cmd)
{
    std::vector<std::string> rows;
#if _WIN32
    queryRows("where " + cmd, rows);
    for (auto &&iter : rows)
    {
        if (YS_File::exist(iter)) return true;
    }
    return false;
#else
    queryRows("command -v " + cmd, rows);
    return rows.size() > 0;
#endif
}
} // namespace isaac

#endif