#pragma once
#ifndef _YS_PROC_H_
#define _YS_PROC_H_

#include <string>
#include <cstdint>
#if __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/prctl.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include <fcntl.h>
#include "ys_exception.hpp"
#include <signal.h>
#if __linux__
extern char **environ;
#endif
#include "ys_condvar.hpp"

namespace isaac {

// 进程相关
class YS_Proc
{
public:
    // 当前程序目录
    static std::string exePath();

    // 当前程序名称
    static std::string exeName();

    // 当前程序路径
    static std::string exeFile();

    // 进程锁防重复启动
    // linux pid 文件
    // windows 唯一名称
    // true表示退出
    static bool pidLock(const std::string &pidFile);

    // 守护进程运行
    // 仅posix
    static bool deamonForkRun();

    // 设置进程名称
    static void setProcTitle(char const *argv[], const char *append = "-worker");
};

// 进程阻塞条件变量
class YS_ProcCond
{
private:
    // 静态全局变量
    static YS_CondVar &getCond()
    {
        static YS_CondVar cond;
        return cond;
    }

public:
    // 继续运行一段时间，停止信号唤醒则返回false
    static bool run(uint32_t ms) { return !getCond().waitForEx(ms); }
    // 停止进程
    static void stop() { return getCond().notifyAll(); }
};

inline std::string YS_Proc::exePath()
{
    std::string file = exeFile();
    if (file.empty())
    {
        return file;
    }
#if _WIN32
    return file.substr(0, file.find_last_of('\\'));
#else
    return file.substr(0, file.find_last_of('/'));
#endif
}

inline std::string YS_Proc::exeName()
{
    std::string file = exeFile();
    if (file.empty()) return file;

    size_t subLen;
#if _WIN32
    auto pos = file.find_last_of('\\');
    subLen   = file.length() - pos - 1;
    if (subLen > 4 && file.compare(file.length() - 4, 4, ".exe") == 0)
    {
        subLen -= 4;
    }
#else
    auto pos = file.find_last_of('/');
    subLen   = file.length() - pos - 1;
#endif
    return file.substr(pos + 1, subLen);
}

inline std::string YS_Proc::exeFile()
{
    char path[1024] = {};
#if _WIN32
    if (GetModuleFileNameA(nullptr, path, sizeof(path)))
#else
    if (readlink("/proc/self/exe", path, sizeof(path)) > 0)
#endif
    {
        return std::string(path);
    }
    else
    {
        return "";
    }
}

inline bool YS_Proc::pidLock(const std::string &pidFile)
{
    if (pidFile.empty())
    {
        YS_EXCEPT_LOG("string empty.");
        return false;
    }
#if _WIN32
    static HANDLE pMutex = nullptr;
    if (pMutex != nullptr)
    {
        return true;
    }

    pMutex = ::CreateMutexA(nullptr, false, pidFile.c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(pMutex);
        pMutex = nullptr;
        return true;
    }

    atexit([]() { CloseHandle(pMutex); });

#else
    static int32_t ifd = -1;
    if (ifd >= 0)
    {
        return true;
    }

    ifd = open(pidFile.c_str(), O_CREAT | O_RDWR, 0666);
    if (ifd < 0)
    {
        YS_EXCEPT_LOG("create file [%s] failed.", pidFile.c_str());
        return false;
    }

    int32_t iRet = flock(ifd, LOCK_EX | LOCK_NB);
    if (iRet)
    {
        if (EWOULDBLOCK == errno)
        {
            return true;
        }
        YS_EXCEPT_LOG("flock failed , errno %d.", errno);
        return false;
    }
    static std::string locPidFile = pidFile;
    atexit([]() { close(ifd); 
        remove(locPidFile.c_str()); });

    char   sPid[16] = {0};
    size_t uLen     = snprintf(sPid, sizeof(sPid), "%d\n", getpid());
    if(::write(ifd, sPid, uLen) == -1)
    {
        YS_EXCEPT_LOG("write fail , errno %d.", errno);
    }
    ::fsync(ifd);
#endif

    return false;
}

inline bool YS_Proc::deamonForkRun()
{
#if _WIN32
#else
    if (daemon(0, 0))
    {
        perror("daemon error\n");
        return false;
    }
    while (1)
    {
        int32_t pid = fork();
        int32_t status;
        if (pid < 0)
        {
            perror("fork error\n");
            exit(errno);
        }
        else if (pid > 0) // main
        {
            waitpid(pid, &status, 0);
            auto sig = WTERMSIG(status);
            YS_NOTE("worker exit with signal %d ,reload", sig);
        }
        else
        {
            prctl(PR_SET_PDEATHSIG, SIGTERM); // 跟随父进程退出
            return false;
        }
    }
#endif
    return true;
}

inline void YS_Proc::setProcTitle(char const *argv[], const char *append)
{
    if (!argv) return;
#if __linux__
    const char *argvLast;

    // 程序名称后面的参数重新申请了空间
    // procName
    {
        u_char  *p;
        size_t   size;
        uint32_t i;

        size = 0;

        for (i = 0; environ[i]; i++)
        {
            size += strlen(environ[i]) + 1;
        }

        p = (u_char *)calloc(1, size);
        if (p == NULL)
        {
            return;
        }

        argvLast = argv[0];

        for (i = 0; argv[i]; i++)
        {
            if (argvLast == argv[i])
            {
                argvLast = argv[i] + strlen(argv[i]) + 1;
            }
        }

        for (i = 0; environ[i]; i++)
        {
            if (argvLast == environ[i])
            {
                size     = strlen(environ[i]) + 1;
                argvLast = environ[i] + size;

                memcpy(p, (u_char *)environ[i], size);
                environ[i] = (char *)p;
                p += size;
            }
        }

        argvLast--;
    }

    // set
    {
        argv[1]     = NULL;
        auto    len = strlen(argv[0]);
        u_char *p   = (u_char *)argv[0] + len;
        memcpy(p, append, strlen(append));
        p += strlen(append);

        if (argvLast - (const char *)p)
        {
            memset(p, '\0', argvLast - (char *)p);
        }
        prctl(PR_SET_NAME, argv[0]);
    }
#endif
};

} // namespace isaac

#endif