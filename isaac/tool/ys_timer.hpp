#ifndef _YS_TIMER_H_
#define _YS_TIMER_H_

#include <cstdint>
#if _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
#include <ctime>
#include <iomanip>

namespace isaac {

// 计算时间
class YS_Timer
{
public:
    // 获取当前时间，毫秒级
    static uint64_t getTimeMs();
    // 获取当前时间，秒级
    static uint64_t getTime() { return time(nullptr); }

    // 开始计时
    uint64_t start(uint64_t ms);
    // 停止
    void stop() { _run = false; }

    // 距开始过了多少毫秒
    uint64_t count() { return _run ? (getTimeMs() - _startMs) : 0; }
    // 剩余时间
    uint64_t left() { return isTimeOut() ? 0 : _timeOut - (getTimeMs() - _startMs); }
    // 是否超时
    bool isTimeOut() { return _run ? (getTimeMs() - _startMs) > _timeOut : false; }
    // 重置
    uint64_t reset() { return _startMs = getTimeMs(); }

private:
    uint64_t _startMs;
    uint64_t _timeOut;
    bool     _run = false;

};

inline uint64_t YS_Timer::getTimeMs()
{
#if _WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

inline uint64_t YS_Timer::start(uint64_t ms)
{
    _run     = true;
    _timeOut = ms;
    _startMs = getTimeMs();
    return _startMs;
}

} // namespace isaac

#endif