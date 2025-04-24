#ifndef _YS_SIGNAL_H_
#define _YS_SIGNAL_H_
#include <csignal>
#include <iostream>
#include <cstdint>
#include <cctype>
#if _WIN32
#include <Windows.h>
#include <DbgHelp.h>
#else
#include <execinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include "ys_util.hpp"
#include "ys_file.hpp"
#include "ys_proc.hpp"
#include "ys_shell.hpp"
#include <mutex>

namespace isaac {
// 信号处理
class YS_Signal
{
public:
    // 忽略管道信号 仅linux
    static void ignorePipe();

    // 忽略终端信号
    static void ignoreHup();

    // 保存段错误堆栈
    static void saveSigSegv();

private:
    // 地址转函数名
    static std::string addr2line(std::string addr);

    // 保存段错误
    static void saveBacktrace(int32_t sig);
};

inline void YS_Signal::ignorePipe()
{
#if _WIN32
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

inline void YS_Signal::ignoreHup()
{
#if _WIN32
#else
    std::signal(SIGHUP, SIG_IGN);
#endif
}

inline void YS_Signal::saveSigSegv()
{
    std::signal(SIGSEGV, saveBacktrace);
    std::signal(SIGABRT, saveBacktrace);
    std::signal(SIGFPE, saveBacktrace);
}

inline std::string YS_Signal::addr2line(std::string addr)
{
    std::string cmd = "addr2line -e " + YS_Proc::exeFile() + " " + addr;
    std::string row;
    YS_Shell::queryRow(cmd, row);
    auto pos = row.find_last_of('/');
    if (pos != std::string::npos)
    {
        row = row.substr(pos + 1);
    }
    return row;
}

inline void YS_Signal::saveBacktrace(int32_t sig)
{
    YS_NOTE("program get signal [%d].", sig);
    static std::mutex           _lock;
    std::lock_guard<std::mutex> lck(_lock);
    std::string                 sTrace;

    void  *stack[2048];
    size_t frames;

#if _WIN32
#if 0 // 依赖 Dbghelp
        std::string sPath = YS_File::fmtPath(YS_Proc::exePath() + "/backtrace-" + std::to_string(time(nullptr)) + ".txt");
        YS_NOTE("save backtrace to [%s]", sPath.c_str());
        SYMBOL_INFO    symbol[4];
        HANDLE         process;

        process = GetCurrentProcess();
        SymInitialize(process, nullptr, true);
        frames                 = CaptureStackBackTrace(0, 2048, stack, nullptr);
        symbol[0].MaxNameLen   = sizeof(symbol) / 4 * 3;
        symbol[0].SizeOfStruct = sizeof(SYMBOL_INFO);

        for (size_t i = 0; i < frames; i++)
        {
            if (SymFromAddr(process, (DWORD64)stack[i], 0, &symbol[0]))
            {
                sTrace.append(symbol[0].Name).append(" - ");
                sTrace.append(std::to_string(symbol[0].Address));
                sTrace.append("\n");
            }
            else
            {
                YS_EXCEPT_LOG("SymFromAddr error %d", GetLastError());
            }
        }
        YS_File::write(sPath, sTrace);
        exit(1);
#endif
#else
    std::string sPath = YS_File::fmtPath(YS_Proc::exePath() + "/backtrace.txt");
    YS_NOTE("save backtrace to [%s]", sPath.c_str());
    std::string cmd = "date \"+%Y-%m-%d %H:%M:%S\" >> " + sPath;
    if(system(cmd.c_str()))
    {
        YS_NOTE("system fail, %s.", cmd.c_str());
    }
    sTrace.append("CATCHED SIG ").append(std::to_string(sig)).append("\n");
    frames     = backtrace(stack, 2048);
    char **str = backtrace_symbols(stack, frames);
    if (str)
    {
        for (size_t i = 0; i < frames; i++)
        {
            sTrace.append(str[i]);

            sTrace.append(" ").append(addr2line(YS_String::findSubStr(str[i], "[", "]")));
            sTrace.append("\n");
        }
        free(str);
    }
    sTrace.append("\n");
    YS_File::write(sPath, sTrace, true);
    exit(1);
#endif
}

} // namespace isaac

#endif