#ifndef __ys_thread_h__
#define __ys_thread_h__

#include <cstdint>
#include <thread>
#include "tool/ys_cpu.hpp"
#include "ys_cli_log.h"

namespace isaac {

class YS_Thread
{
private:
    bool         _flag = false; // 线程正在运行
    std::thread *_th   = nullptr;

    void run();

protected:
    virtual int32_t init() { return 0; } // 线程启动执行一次
    virtual void    handle() = 0;        // 线程内循环执行
    virtual void    final() {}           // 线程结束执行一次

    int32_t start();

    void bindCpu(uint32_t off) { YS_Cpu::threadBindCpu(_th, off); }

    void name(const std::string &str) { YS_Cpu::threadSetName(_th, str); }

    void stop() { _flag = false; }

public:
    bool isRunning() { return _flag; }

    virtual ~YS_Thread();
};

inline void YS_Thread::run()
{
    if (init())
    {
        _flag = false;
        return;
    }

    while (_flag)
    {
        handle();
    }

    final();
    _flag = false;
}

inline int32_t YS_Thread::start()
{
    if (_flag)
    {
        CLI_ERROR("thread is running.");
        return -1;
    }
    _flag = true;
    _th   = new std::thread(&YS_Thread::run, this);
    if (_th == nullptr)
    {
        _flag = false;
        return -1;
    }
    return 0;
}

inline YS_Thread::~YS_Thread()
{
    if (_th && _th->joinable())
    {
        _th->join();
        delete _th;
    }
}

} // namespace isaac

#endif