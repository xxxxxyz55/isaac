#ifndef _YS_SHARED_LOCK_H_
#define _YS_SHARED_LOCK_H_

#include <mutex>
#include <functional>
#include <condition_variable>
namespace isaac {

// 读写锁
class YS_SharedMutex
{
public:
    // 获取读锁
    void readLock();

    // 尝试读锁
    bool tryReadLock();

    // 解读锁
    void unReadLock();

    // 写锁
    void writeLock();

    // 尝试写锁
    bool tryWriteLock();

    // 解写锁
    void unWriteLock();

private:
    // 锁
    std::mutex _mutex;
    // 条件变量
    std::condition_variable _cond;
    // 是否在写
    bool _isWrite = false;
    // 读者数量
    size_t _readCount = 0;
    // 读条件
    bool isReadCond() const { return false == _isWrite; }
    // 写条件
    bool isWriteCond() const { return false == _isWrite && 0 == _readCount; }
};

// 读锁
class YS_RLock
{
private:
    YS_SharedMutex &_lck;

public:
    YS_RLock(YS_SharedMutex &lck) : _lck(lck) { _lck.readLock(); }
    ~YS_RLock() { _lck.unReadLock(); }
};

// 写锁
class YS_WLock
{
private:
    YS_SharedMutex &_lck;

public:
    YS_WLock(YS_SharedMutex &lck) : _lck(lck) { _lck.writeLock(); }
    ~YS_WLock() { _lck.writeLock(); }
};

inline void YS_SharedMutex::readLock()
{
    // 加锁, 直到可读
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, std::bind(&YS_SharedMutex::isReadCond, this));
    _readCount++;
}

inline bool YS_SharedMutex::tryReadLock()
{
    std::unique_lock<std::mutex> lck(_mutex);
    if (_cond.wait_for(lck, std::chrono::seconds(0)) == std::cv_status::timeout)
    {
        return false;
    }

    if (isReadCond())
    {
        _readCount++;
        return true;
    }

    return false;
}

inline void YS_SharedMutex::unReadLock()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _readCount--;
    _cond.notify_all();
}

inline void YS_SharedMutex::writeLock()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, std::bind([](const bool *is_w, const size_t *read_c) -> bool {
                   return false == *is_w && 0 == *read_c;
               },
                              &_isWrite, &_readCount));

    _isWrite = true;
}

inline bool YS_SharedMutex::tryWriteLock()
{
    std::unique_lock<std::mutex> lck(_mutex);
    if (_cond.wait_for(lck, std::chrono::seconds(0)) == std::cv_status::timeout)
    {
        return false;
    }

    if (isWriteCond())
    {
        _isWrite = true;
        return true;
    }

    return false;
}

inline void YS_SharedMutex::unWriteLock()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _isWrite = false;
    _cond.notify_all();
}

} // namespace isaac
#endif