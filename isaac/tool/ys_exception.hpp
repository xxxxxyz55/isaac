#ifndef _YS_EXCEPT_LOG_H_
#define _YS_EXCEPT_LOG_H_

#include <cstdio>
#include <cstring>
#include <exception>

namespace isaac {

#ifdef LOG_FULL_PATH
#define __YS_FILE__ __FILE__
#else
#if _WIN32
#define __YS_FILE__ (strrchr("\\" __FILE__, '\\') + 1)
#else
#define __YS_FILE__ (strrchr("/" __FILE__, '/') + 1)
#endif
#endif

#define YS_NOTE(sfmt, ...)                       \
    fprintf(stdout, "note:" sfmt "[%s:%d:%s]\n", \
            ##__VA_ARGS__, __YS_FILE__, __LINE__, __FUNCTION__);

// debug版本使用
#define YS_EXCEPT_LOG(sfmt, ...)                                             \
    fprintf(stderr, "\033[0m\033[1;33mexcetion:" sfmt "[%s:%d:%s]\033[0m\n", \
            ##__VA_ARGS__, __YS_FILE__, __LINE__, __FUNCTION__);

#define YS_EXCEPT_LOGIC_ERROR(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);  \
    throw std::logic_error("");

// 参数错误
#define YS_EXCEPT_INVALID_ARGUMENT(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);       \
    throw std::invalid_argument("");
// 长度错误
#define YS_EXCEPT_LENGTH_ERROR(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);   \
    throw std::length_error("");
// 超出范围
#define YS_EXCEPT_OUT_OF_RANGE(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);   \
    throw std::out_of_range("");
// 运行时错误
#define YS_EXCEPT_RUNTIME_ERROR(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);    \
    throw std::runtime_error("");
// 上溢出
#define YS_EXCEPT_OVERFLOW_ERROR(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);     \
    throw std::owerflow_error("");
// 下溢出
#define YS_EXCEPT_UNDERFLOW_ERROR(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);      \
    throw std::underflow_error("");
// 范围错误
#define YS_EXCEPT_RANGE_ERROR(sfmt, ...) \
    YS_EXCEPT_LOG(sfmt, ##__VA_ARGS__);  \
    throw std::range_error("");

} // namespace isaac
#endif