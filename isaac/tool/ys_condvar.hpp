#ifndef _YS_CONDVAR_H_
#define _YS_CONDVAR_H_

#if _WIN32
#include <condition_variable>
#include <mutex>
#else
#include <pthread.h>
#include "ys_exception.hpp"
#endif

namespace isaac {
// 超时等待
#if _WIN32
class YS_CondVar
{
public:
    // 超时等待
    bool waitFor(int32_t ms);

    // 超时等待,不重置,唤醒一次后,后续所有都无需等待
    bool waitForEx(int32_t ms);

    // 等待唤醒，已唤醒过则直接返回
    bool wait(bool resetNotify = false);

    // 唤醒所有,确保再唤醒前其他线程已经开始等待
    void notifyAll();

    // 唤醒一个
    void notifyOne();

private:
    std::condition_variable _cond;
    std::mutex              _mutex;
    bool                    _hasNotified = false;
};
#else

// 超时等待
class YS_CondVar
{
public:
    // 构造
    YS_CondVar();
    // 析构
    ~YS_CondVar();

    // 超时等待
    bool waitFor(uint32_t ms);

    // 超时等待,不重置,唤醒一次后,后续所有都无需等待
    bool waitForEx(uint32_t ms);

    // 等待唤醒，已唤醒过则直接返回
    bool wait(bool resetNotify = false);

    // 唤醒所有,确保再唤醒前其他线程已经开始等待
    void notifyAll();

    // 唤醒一个
    void notifyOne();

private:
    pthread_mutex_t    _mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t     _cond;
    pthread_condattr_t attr;

    bool _hasNotified = false;

    // c锁
    class YS_UniqueLock
    {
        pthread_mutex_t *_mutex;

    public:
        // 构造加锁
        YS_UniqueLock(pthread_mutex_t *mut);

        // 析构解锁
        ~YS_UniqueLock();
    };

    // 毫秒转具体时间
    void clockGetTm(uint32_t ms, struct timespec &spec);
};
#endif

#if _WIN32
inline bool YS_CondVar::waitFor(int32_t ms)
{
    std::unique_lock<std::mutex> lck(_mutex);
    if (!_hasNotified)
    {
        if (_cond.wait_for(lck, std::chrono::milliseconds(ms)) == std::cv_status::timeout)
        {
            return false;
        }
    }
    _hasNotified = false;
    return true;
}

inline bool YS_CondVar::waitForEx(int32_t ms)
{
    std::unique_lock<std::mutex> lck(_mutex);
    return _cond.wait_for(lck, std::chrono::milliseconds(ms), [&]() { return _hasNotified; });
}

inline bool YS_CondVar::wait(bool resetNotify)
{
    std::unique_lock<std::mutex> lck(_mutex);
    if (_hasNotified)
    {
        return true;
    }
    _cond.wait(lck);
    if (resetNotify) _hasNotified = false;
    return true;
}

inline void YS_CondVar::notifyAll()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _hasNotified = true;
    _cond.notify_all();
}

inline void YS_CondVar::notifyOne()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _hasNotified = true;
    _cond.notify_one();
}
#else

inline YS_CondVar::YS_CondVar()
{
    if (pthread_condattr_init(&attr))
    {
        YS_EXCEPT_LOG("pthread_condattr_init fail, %d", errno);
    }
    if (pthread_condattr_setclock(&attr, CLOCK_MONOTONIC))
    {
        YS_EXCEPT_LOG("pthread_condattr_setclock fail, %d", errno);
    }
    if (pthread_cond_init(&_cond, &attr))
    {
        YS_EXCEPT_LOG("pthread_cond_init fail, %d", errno);
    }
}

inline YS_CondVar::YS_UniqueLock::YS_UniqueLock(pthread_mutex_t *mut) : _mutex(mut)
{
    if (pthread_mutex_lock(mut))
    {
        YS_EXCEPT_LOG("pthread_mutex_lock fail, %d", errno);
    }
}

inline YS_CondVar::YS_UniqueLock::~YS_UniqueLock()
{
    if (pthread_mutex_unlock(_mutex))
    {
        YS_EXCEPT_LOG("pthread_mutex_unlock fail, %d", errno);
    }
}

inline YS_CondVar::~YS_CondVar()
{
    {
        YS_UniqueLock lck(&_mutex);
    }
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_cond);
    pthread_condattr_destroy(&attr);
}

inline void YS_CondVar::clockGetTm(uint32_t ms, struct timespec &spec)
{
    clock_gettime(CLOCK_MONOTONIC, &spec);
    uint64_t tms  = ms;
    uint64_t nsec = spec.tv_nsec + (tms * 1000 * 1000);
    spec.tv_sec += nsec / 1000000000;
    spec.tv_nsec = nsec % 1000000000;
}

inline bool YS_CondVar::waitFor(uint32_t ms)
{
    YS_UniqueLock lck(&_mutex);
    if (!_hasNotified)
    {
        struct timespec spec;
        clockGetTm(ms, spec);
        if (pthread_cond_timedwait(&_cond, &_mutex, &spec) == ETIMEDOUT)
        {
            return false;
        }
    }
    _hasNotified = false;
    return true;
}

inline bool YS_CondVar::waitForEx(uint32_t ms)
{
    YS_UniqueLock lck(&_mutex);
    if (!_hasNotified)
    {
        struct timespec spec;
        clockGetTm(ms, spec);
        if (pthread_cond_timedwait(&_cond, &_mutex, &spec) == ETIMEDOUT)
        {
            return false;
        }
    }
    return true;
}

inline bool YS_CondVar::wait(bool resetNotify)
{
    YS_UniqueLock lck(&_mutex);
    if (_hasNotified)
    {
        return true;
    }
    pthread_cond_wait(&_cond, &_mutex);
    if (resetNotify) _hasNotified = false;
    return true;
}

inline void YS_CondVar::notifyAll()
{
    YS_UniqueLock lck(&_mutex);
    _hasNotified = true;
    pthread_cond_broadcast(&_cond);
}

inline void YS_CondVar::notifyOne()
{
    YS_UniqueLock lck(&_mutex);
    _hasNotified = true;
    pthread_cond_signal(&_cond);
}

#endif

} // namespace isaac
#endif