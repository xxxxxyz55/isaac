#ifndef _YS_CLI_LOG_H_
#define _YS_CLI_LOG_H_

#include <cstdint>
#include <string>

#define FMT_HEADER_ONLY

#include "fmt/core.h"
#include "fmt/color.h"

#include "ys_log.h"

// 命令行调试日志
namespace isaac {
// cli log

// stdout 日志控制
class YS_CliLog
{
public:
    // 设置日志级别 debug info error close
    static void setLev(const std::string &lev) { setLev(YS_Log::tagToLev(lev)); }

    static void setLev(YS_Log::LEV lev) { getLev() = lev; }

    // 日志等级全局变量
    static uint32_t &getLev()
    {
        static uint32_t lev = YS_Log::LEV::LEV_DEBUG;
        return lev;
    }
};
// 调试日志
#ifndef CLI_DEBUG
#define CLI_DEBUG(sfmt, ...)                                                \
    do                                                                      \
    {                                                                       \
        if (isaac::YS_CliLog::getLev() >= isaac::YS_Log::LEV::LEV_DEBUG)    \
            fmt::print(stdout, "[{}] [{}:{}:{}] " sfmt "\n",                \
                       fmt::format(fg(fmt::terminal_color::green), "DEBG"), \
                       __YS_FILE__, __LINE__, __FUNCTION__,                 \
                       ##__VA_ARGS__);                                      \
        LOG_DEBUG(sfmt, ##__VA_ARGS__);                                     \
    } while (0);
#endif

// 信息日志
#ifndef CLI_INFO
#define CLI_INFO(sfmt, ...)                                                \
    do                                                                     \
    {                                                                      \
        if (isaac::YS_CliLog::getLev() >= isaac::YS_Log::LEV::LEV_INFO)    \
            fmt::print(stdout, "[{}] [{}:{}:{}] " sfmt "\n",               \
                       fmt::format(fg(fmt::terminal_color::blue), "INFO"), \
                       __YS_FILE__, __LINE__, __FUNCTION__,                \
                       ##__VA_ARGS__);                                     \
        LOG_INFO(sfmt, ##__VA_ARGS__);                                     \
    } while (0);
#endif

// 信息日志
#ifndef CLI_WARN
#define CLI_WARN(sfmt, ...)                                                         \
    do                                                                              \
    {                                                                               \
        if (isaac::YS_CliLog::getLev() >= isaac::YS_Log::LEV::LEV_WARN)             \
            fmt::print(stdout, "[{}] [{}:{}:{}] " sfmt "\n",                        \
                       fmt::format(fg(fmt::terminal_color::bright_yellow), "WARN"), \
                       __YS_FILE__, __LINE__, __FUNCTION__,                         \
                       ##__VA_ARGS__);                                              \
        LOG_WARN(sfmt, ##__VA_ARGS__);                                              \
    } while (0);
#endif

// 错误日志
#ifndef CLI_ERROR
#define CLI_ERROR(sfmt, ...)                                              \
    do                                                                    \
    {                                                                     \
        if (isaac::YS_CliLog::getLev() >= isaac::YS_Log::LEV::LEV_ERROR)  \
            fmt::print(stdout, "[{}] [{}:{}:{}] " sfmt "\n",              \
                       fmt::format(fg(fmt::terminal_color::red), "EROR"), \
                       __YS_FILE__, __LINE__, __FUNCTION__,               \
                       ##__VA_ARGS__);                                    \
        LOG_ERROR(sfmt, ##__VA_ARGS__);                                   \
    } while (0);
#endif

// 致命错误日志
#ifndef CLI_FATAL
#define CLI_FATAL(sfmt, ...)                                                      \
    do                                                                            \
    {                                                                             \
        if (isaac::YS_CliLog::getLev() >= isaac::YS_Log::LEV::LEV_FATAL)          \
            fmt::print(stdout, "[{}] [{}:{}:{}] " sfmt "\n",                      \
                       fmt::format(fg(fmt::terminal_color::bright_red), "FATAL"), \
                       __YS_FILE__, __LINE__, __FUNCTION__,                       \
                       ##__VA_ARGS__);                                            \
        LOG_FATAL(sfmt, ##__VA_ARGS__);                                           \
    } while (0);
#endif

// fmt::print
#define CLI_PRINT(sfmt, ...) fmt::print(stdout, sfmt, ##__VA_ARGS__);
// 运行到当前位置，则输出
#define CLI_POINT()          fmt::print(stdout, "[{}][{}:{}:{}]\n", fmt::format(fg(fmt::terminal_color::cyan), "BREAK"), __YS_FILE__, __LINE__, __FUNCTION__)

#if 0
// 用于拓展错误码
#ifndef YS_ERROR_MAP_EX
#define YS_ERROR_MAP_EX(XX)
#endif

#define YS_ERROR_MAP(XX) \
    /*code,name, decs*/  \
    XX(0, OK, "ok")      \
    YS_ERROR_MAP_EX(XX)

enum YS_Error
{
#define XX(code, name, desc) YS_ERROR_##name = code,
    YS_ERROR_MAP(XX)
#undef XX
};

inline const char *getErrorDesc(int32_t code)
{
    switch (code)
    {
#define XX(code, name, desc) \
    case YS_ERROR_##name:    \
        return desc;
        YS_ERROR_MAP(XX)
#undef XX
    default:
        return "unknow";
    }
}
#endif

} // namespace isaac
#endif