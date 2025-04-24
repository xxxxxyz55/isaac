#ifndef _YS_TEST_H_
#define _YS_TEST_H_

#include <iostream>
#include <map>
#include <thread>
#include <atomic>
#include <vector>
#include "ys_condvar.hpp"
#include "ys_util.hpp"
#include "ys_cpu.hpp"

namespace isaac {
// 测试程序面板,编写测试程序

// 测试面板
class YS_Test
{
    using ys_test_functor = void (*)();

public:
    // 添加一项测试
    void push(ys_test_functor func, std::string desc);

    // 构造空测试
    YS_Test(/* args */){};

    // 构造
    YS_Test(std::vector<std::pair<ys_test_functor, std::string>> vtCases);

    // 在此运行
    ~YS_Test();

private:
    std::map<int32_t, std::pair<std::string, ys_test_functor>> mpTests;

    int32_t _key = 1;

    // 显示测试菜单
    void printMenu();

    // 运行测试
    void run();
};

// 多线程测试模板,继承该类,重写虚函数
class YS_TestMul
{
public:
    // 重写这三个
    virtual int32_t init(size_t i = 0) { return 0; }
    virtual int32_t run(size_t i = 0) = 0;
    virtual void    uninit(size_t i = 0) {}

    // 设置线程数量
    void setThreadNum(size_t threadNum) { _threadNum = threadNum; }

    // 获取线程数量
    size_t getThreadNum() { return _threadNum; }

    // 设置数据长度
    void setDataLength(size_t bytes) { _loopBytes = bytes; }

    // 出错时线程退出
    void breakOnErr(bool flag) { _breakOnErr = flag; }

    // 循环一段时间 0 为死循环
    int64_t loopFor(uint32_t sec = 0);

    // 多线程循环测试
    void loopMulti(uint32_t dataLen = 0, uint32_t threadNum = YS_Cpu::getCpuNum());

    YS_TestMul(size_t threadNum = 1) : _threadNum(threadNum) {}
    virtual ~YS_TestMul() {}

private:
    bool                  _fRun = true;
    std::atomic<uint64_t> _count;
    size_t                _threadNum;
    YS_CondVar            _start;
    YS_CondVar            _init;             // 所有线程初始化完成
    std::atomic<uint32_t> _initNum    = {0}; // 已初始化完成的数量
    size_t                _loopBytes  = 0;
    bool                  _breakOnErr = true;

    // 开始
    void start();
    // 停止
    void stop() { _fRun = false; }

    // 任务循环
    void task_route(size_t i);

    void show_route();
};

inline void YS_Test::printMenu()
{
#if _WIN32
    static bool encodeFlag = false;
    if (!encodeFlag)
    {
        encodeFlag = true;
        system("chcp 65001");
    }
#endif
    std::printf("\n 0 exit\n");

    for (auto iter = mpTests.begin(); iter != mpTests.end(); iter++)
    {
        std::printf("%2d %s\n", iter->first, iter->second.first.c_str());
    }
}

inline void YS_Test::run()
{
    int32_t         val   = 0;
    ys_test_functor pTest = nullptr;
    do
    {
        printMenu();
        val = YS_Util::stdGetInt("");
        if (val)
        {
            try
            {
                pTest = mpTests.at(val).second;
            }
            catch (...)
            {
                std::printf("key %d test not found.\n", val);
                val = -1;
            }

            if (pTest)
            {
                std::printf("\nkey %d  run %s.\n", val, mpTests.at(val).first.c_str());
                pTest();
            }
        }
    } while (val != 0);
}

inline void YS_Test::push(ys_test_functor func, std::string desc)
{
    mpTests.insert({
        _key++,
        {desc, func}
    });
}

inline YS_Test::YS_Test(std::vector<std::pair<ys_test_functor, std::string>> vtCases)
{
    for (auto &&i : vtCases)
    {
        push(i.first, i.second);
    }
};

inline YS_Test::~YS_Test()
{
    run();
    mpTests.clear();
}

inline void YS_TestMul::start()
{
    _count = 0;
    _fRun  = true;
    _start.notifyAll();
}

inline void YS_TestMul::task_route(size_t i)
{
    YS_Cpu::threadBindCpu(i);
    // std::printf("test thread bind cpu %zu\n", i);

    if (init(i)) return;
    _initNum++;
    _init.notifyAll();
    _start.wait();
    std::printf("test thread run %zu\n", i);
    while (_fRun)
    {
        if (run(i))
        {
            if (_breakOnErr)
            {
                std::printf("thread exit!\n");
                break;
            }
            continue;
        }
        _count++;
    }
    uninit(i);
}

inline void YS_TestMul::show_route()
{
    _start.wait();
    while (_fRun)
    {
        YS_Util::mSleep(1000);
        uint64_t tCount = _count.exchange(0);
        std::printf("\nSEPPD %8lu Tps\n", tCount);
        if (_loopBytes)
        {
            std::printf("SPEED %8lu Mbps\n", tCount * _loopBytes * 8 / 1024 / 1024);
        }
    }
}

inline int64_t YS_TestMul::loopFor(uint32_t sec)
{
    std::vector<std::thread> vtThread;
    std::printf("thread num : %zu\n", _threadNum);

    for (size_t i = 0; i < _threadNum; i++)
    {
        vtThread.push_back(std::thread(&YS_TestMul::task_route, this, i));
    }

    if (!sec)
    {
        _count = 0;
        vtThread.push_back(std::thread(&YS_TestMul::show_route, this));
    }

    while (_initNum < _threadNum)
    {
        _init.wait(true);
        continue;
    }

    start();
    if (sec)
    {
        YS_Util::mSleep(sec * 1000);
        stop();
    }
    else
    {
        int32_t val = 1;
        std::printf("input 0 to exit:");
        fflush(stdout);
        while (val)
        {
            std::cin >> val;
        }
        stop();
    }

    for (auto &&i : vtThread)
    {
        i.join();
    }

    if (sec)
    {
        std::printf("\nSEPPD %010ld Tps\n", _count / sec);
        if (_loopBytes)
        {
            std::printf("SPEED %010ld Mbps\n", _count * _loopBytes * 8 / sec / 1024 / 1024);
        }
    }

    return _count;
}

inline void YS_TestMul::loopMulti(uint32_t dataLen, uint32_t threadNum)
{
    setThreadNum(threadNum);
    setDataLength(dataLen);
    loopFor();
}

} // namespace isaac

#endif