#ifndef _YS_CPU_H_
#define _YS_CPU_H_

#include <cstdint>
#if __linux__
#include <pthread.h>
#include <unistd.h>
#elif _WIN32
#include <Windows.h>
#endif
#include <thread>

namespace isaac {
// 获取CPU核数量
class YS_Cpu
{
public:
    // 获取cpu数量
    static uint32_t getCpuNum();

    // 线程绑定cpu 仅linux
    static void threadBindCpu(uint32_t off);

    // 线程绑定cpu 仅linux
    static void threadBindCpu(std::thread *ptd, uint32_t off);

    // 线程绑定指定cpu 仅linux
    static void threadBindCpuId(std::thread *ptd, uint32_t id);

    // 线程设置名称 仅linux
    static void threadSetName(std::thread *ptd, const std::string &name);
};

inline uint32_t YS_Cpu::getCpuNum()
{
#if _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#else
    return sysconf(/*_SC_NPROCESSORS_CONF*/ 83);
#endif
}

inline void YS_Cpu::threadBindCpu(uint32_t off)
{
#if _WIN32
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(off % getCpuNum(), &mask);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &mask);
#endif
}

inline void YS_Cpu::threadBindCpu(std::thread *ptd, uint32_t off)
{
#if _WIN32
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(off % getCpuNum(), &mask);
    pthread_setaffinity_np(ptd->native_handle(), sizeof(cpu_set_t), &mask);
#endif
}

inline void threadBindCpuId(std::thread *ptd, uint32_t id)
{
#if _WIN32
#else
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(id, &mask);
    pthread_setaffinity_np(ptd->native_handle(), sizeof(cpu_set_t), &mask);
#endif
}

inline void YS_Cpu::threadSetName(std::thread *ptd, const std::string &name)
{
#if _WIN32
#else
    pthread_setname_np(ptd->native_handle(), name.c_str());
#endif
}

} // namespace isaac

#endif