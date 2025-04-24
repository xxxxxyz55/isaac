#pragma once
#ifndef _YS_UTIL_H_
#define _YS_UTIL_H_

#include <string>
#include <thread>
#include <cstdint>
#include <iostream>
#include "ys_exception.hpp"
#if _WIN32
#include <WinSock2.h>
#endif

namespace isaac {

// 工具函数
class YS_Util
{
public:
    // 微秒sleep
    static void mSleep(uint32_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    // 标准输入获取一个整数
    static int32_t stdGetInt(const std::string &tag) { return atoi(stdGetString(tag, "", true).c_str()); }

    // 标准输入获取一个字符串
    static std::string stdGetString(const std::string &tag, std::string def = "", bool skipFirstEnter = false);

    static bool canRead(int32_t fd, long ms);
    static bool canWrite(int32_t fd, long ms);
#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 8)
#define __GNUC_4_8__ 1
#endif
};

inline std::string YS_Util::stdGetString(const std::string &tag, std::string def, bool skipFirstEnter)
{
    std::string buf;
    std::cout << tag;
    fflush(stdout);
#ifdef _MSC_VER
    std::fflush(stdin);
#else
    std::setvbuf(stdin, nullptr, _IOLBF, 0);
#endif
    uint32_t len = 0;
    while (true)
    {
        auto c = getchar();
        if (skipFirstEnter)
        {
            if (!len && c == '\n')
            {
                continue;
            }
        }
        if (c == EOF || c == '\n')
        {
            break;
        }
        else if (!len && c == ' ')
        {
            break;
        }
        else
        {
            buf += c;
            len++;
        }
    }

    return buf.length() ? buf : def;
}

inline bool YS_Util::canRead(int32_t fd, long ms)
{
    timeval tmout = {ms / 1000, (ms % 1000) * 1000};
    fd_set  rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    auto ret = ::select(fd + 1, &rset, nullptr, nullptr, &tmout);
    if (ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline bool YS_Util::canWrite(int32_t fd, long ms)
{
    timeval tmout = {ms / 1000, (ms % 1000) * 1000};
    fd_set  wset;
    FD_ZERO(&wset);
    FD_SET(fd, &wset);
    auto ret = ::select(fd + 1, nullptr, &wset, nullptr, &tmout);
    if (ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace isaac
#endif
