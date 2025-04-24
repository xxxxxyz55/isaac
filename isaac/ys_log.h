#ifndef _YS_LOG_H_
#define _YS_LOG_H_

#include "tool/ys_exception.hpp"
#include "tool/ys_file.hpp"
#include "tool/ys_timer.hpp"
#include "tool/ys_thread_queue.hpp"
#include "tool/ys_condvar.hpp"
#include "tool/ys_util.hpp"
#include "tool/ys_proc.hpp"
#include <thread>
#include <cstdarg>
#include "fmt/chrono.h"

#if _WIN32
#define YS_ERRNO WSAGetLastError()
#else
#define YS_ERRNO errno
#endif

namespace isaac {
// 简单的文件日志
class YS_Log
{
public:
    enum LEV : uint32_t
    {
        LEV_CLOSE,
        LEV_FATAL,
        LEV_ERROR,
        LEV_WARN,
        LEV_INFO,
        LEV_DEBUG,
        LEV_END,
    };
    static LEV         tagToLev(const std::string &lev);
    static const char *levToTag(LEV lev);

private:
    // 一条日志
    class LogNode
    {
    public:
        LEV         _lev;
        std::string _log;
        tm          _tm;

        template <typename... Args>
        LogNode(LEV lev, const char *sfmt, const char *Lev, Args... args)
        {
            _lev     = lev;
            time_t t = time(nullptr);
            _tm      = fmt::localtime(t);
            _log     = fmt::format(sfmt, _tm, Lev, args...);
        }
    };

public:
    // 初始化 空path指向程序路径下logs
    // 全局文件日志初始化
    static bool init(const std::string &lev, std::string path = "");
    static bool init(LEV lev = LEV_INFO, std::string path = "") { return global().initConf(lev, path); }
    static void useSingleFile() { global()._singleFile = true; }
    static void setfileSize(uint32_t MB) { global()._fileSize = (MB - 1) * 1024 * 1024; }
    // 获取全局对象
    static YS_Log &global();

    // 空path指向程序路径下logs
    bool initConf(LEV lev = LEV_INFO, std::string path = "");

    // 析构
    ~YS_Log();

    // 是否启用
    static bool isEnabled() { return global()._fInit; }

#define YS_LOG_EXPAND(sfmt, lev, ...) \
    if (isaac::YS_Log::isEnabled()) isaac::YS_Log::global().lev("{:%Y-%m-%d %H:%M:%S} [{}] [{}:{}:{}] " sfmt "\n", __YS_FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOG_DEBUG(sfmt, ...) YS_LOG_EXPAND(sfmt, debug, ##__VA_ARGS__)
#define LOG_INFO(sfmt, ...)  YS_LOG_EXPAND(sfmt, info, ##__VA_ARGS__)
#define LOG_WARN(sfmt, ...)  YS_LOG_EXPAND(sfmt, warn, ##__VA_ARGS__)
#define LOG_ERROR(sfmt, ...) YS_LOG_EXPAND(sfmt, error, ##__VA_ARGS__)
#define LOG_FATAL(sfmt, ...) YS_LOG_EXPAND(sfmt, fatal, ##__VA_ARGS__)

    template <typename... Args>
    void debug(const char *sfmt, Args... args)
    {
        if (!_fInit) return;

        if (_level < LEV_DEBUG) return;

        pushLog(new LogNode(LEV_DEBUG, sfmt, "DEBG", args...));
    }

    template <typename... Args>
    void info(const char *sfmt, Args... args)
    {
        if (!_fInit) return;

        if (_level < LEV_INFO) return;

        pushLog(new LogNode(LEV_INFO, sfmt, "INFO", args...));
    }

    template <typename... Args>
    void warn(const char *sfmt, Args... args)
    {
        if (!_fInit) return;

        if (_level < LEV_WARN) return;

        pushLog(new LogNode(LEV_WARN, sfmt, "WARN", args...));
    }

    template <typename... Args>
    void error(const char *sfmt, Args... args)
    {
        if (!_fInit) return;

        if (_level < LEV_ERROR) return;

        pushLog(new LogNode(LEV_ERROR, sfmt, "EROR", args...));
    }

    template <typename... Args>
    void fatal(const char *sfmt, Args... args)
    {
        if (!_fInit) return;

        if (_level < LEV_FATAL) return;

        pushLog(new LogNode(LEV_FATAL, sfmt, "FATAL", args...));
    }

private:
    // 关闭对应等级的日志文件
    void closeLogFile(LEV lev, bool chgName);
    // 日期是否改变
    bool dayChanged(LEV lev, tm *ttm);

    std::string getLogFilePath(LEV lev, tm *ttm, uint32_t pos);

    FILE *getLogFile(LEV lev, tm *ttm, size_t nextLogSize);

    void saveLog();

    void pushLog(LogNode *node);

    LogNode *popLog();

private:
    bool        _fInit = false;                 // 是否初始化
    LEV         _level = LEV_INFO;              // 日志等级
    std::string _logPath;                       // 路径目录
    size_t      _fileSize   = 99 * 1024 * 1024; // 单个文件大小
    bool        _singleFile = false;            // 所有等级记录到一个文件

    FILE       *_fd[LEV_END];       // 打开的fd
    tm          _fileTm[LEV_END];   // 打开的文件时间 用于每天切换文件
    std::string _filePath[LEV_END]; // 打开的文件路径
    size_t      _nowSz[LEV_END];    // 打开的文件大小

    uint32_t              _pos[LEV_END]; // 一天内多个文件
    bool                  _fRun = true;  // 运行标志
    YS_ThQueue<LogNode *> _queue;        // 保存队列
    std::thread          *_th;
};

inline bool YS_Log::init(const std::string &lev, std::string path)
{
    if (global().initConf(tagToLev(lev), path))
    {
        return true;
    }
    return false;
}

inline YS_Log &YS_Log::global()
{
    static YS_Log obj;
    return obj;
}

inline const char *YS_Log::levToTag(LEV lev)
{
    static const char *tag[] = {"", "fatal", "error", "warn", "info", "debug"};
    return tag[lev];
}

inline YS_Log::LEV YS_Log::tagToLev(const std::string &lev)
{
    if (lev == "close" || lev == "")
        return LEV::LEV_CLOSE;
    else if (lev == "debug")
        return LEV::LEV_DEBUG;
    else if (lev == "info")
        return LEV::LEV_INFO;
    else if (lev == "warn")
        return LEV::LEV_WARN;
    else if (lev == "error")
        return LEV::LEV_ERROR;
    else if (lev == "fatal")
        return LEV::LEV_FATAL;
    else
    {
        return LEV::LEV_DEBUG;
    }
}

inline bool YS_Log::initConf(LEV lev, std::string path)
{
    if (_fInit) return true;

    _level = lev;
    if (path.empty())
    {
        path = YS_Proc::exePath() + "/logs";
    }
    _logPath = path;

    if (!YS_File::mkdir(_logPath))
    {
        YS_EXCEPT_LOG("mkdir fail , %s.", _logPath.c_str())
        return false;
    }

    if (!YS_File::isDir(_logPath))
    {
        YS_EXCEPT_LOG("%s is`nt a dir.", _logPath.c_str())
        return false;
    }

    _th = new std::thread(&YS_Log::saveLog, this);

    _fInit = true;
    return true;
}

inline YS_Log::~YS_Log()
{
    if (!_fInit) return;

    _fRun = false;
    _queue.notifyT();
    if (_th && _th->joinable())
    {
        _th->join();
    }
    LogNode *log = nullptr;
    while (_queue.pop_front(log))
    {
        delete log;
    }

    for (auto &&i : {LEV_CLOSE, LEV_DEBUG, LEV_INFO, LEV_ERROR})
    {
        closeLogFile(i, false);
    }
    YS_NOTE("log exit.");
}

inline void YS_Log::closeLogFile(LEV lev, bool chgName)
{
    if (_fd[lev])
    {
        fclose(_fd[lev]);
        _fd[lev] = nullptr;
        if (chgName)
        {
            for (uint32_t i = 1; i < 1000; i++)
            {
                std::string path = getLogFilePath(lev, &_fileTm[lev], i);
                if (YS_File::exist(path))
                {
                    continue;
                }
                rename(_filePath[lev].c_str(), path.c_str());
            }
        }
    }
}

inline bool YS_Log::dayChanged(LEV lev, tm *ttm)
{
    if (_fileTm[lev].tm_mday == ttm->tm_mday &&
        _fileTm[lev].tm_mon == ttm->tm_mon &&
        _fileTm[lev].tm_year == ttm->tm_year)
    {
        return false;
    }
    else
    {
        YS_NOTE("log date changed.");
        return true;
    }
}

inline std::string YS_Log::getLogFilePath(LEV lev, tm *ttm, uint32_t pos)
{
    if (_singleFile)
    {
        if (pos)
        {
            return YS_File::fmtPath(fmt::format("{}/{}-{:%Y-%m-%d}.{:03d}.log", _logPath, YS_Proc::exeName(), *ttm, pos));
        }
        else
        {
            return YS_File::fmtPath(fmt::format("{}/{}-{:%Y-%m-%d}.log", _logPath, YS_Proc::exeName(), *ttm));
        }
    }
    else
    {
        if (pos)
        {
            return YS_File::fmtPath(fmt::format("{}/{}-{}-{:%Y-%m-%d}.{:03d}.log", _logPath, YS_Proc::exeName(), levToTag(lev), *ttm, pos));
        }
        else
        {
            return YS_File::fmtPath(fmt::format("{}/{}-{}-{:%Y-%m-%d}.log", _logPath, YS_Proc::exeName(), levToTag(lev), *ttm));
        }
    }
}

inline FILE *YS_Log::getLogFile(LEV lev, tm *ttm, size_t nextLogSize)
{
    if (_fd[lev])
    {
        if (_nowSz[lev] + nextLogSize > _fileSize)
        {
            YS_NOTE("max log file size.");
            closeLogFile(lev, true);
        }
        if (dayChanged(lev, ttm))
        {
            closeLogFile(lev, false);
        }

        if (!YS_File::exist(_filePath[lev]))
        {
            YS_NOTE("log file not exist.");
            closeLogFile(lev, false);
        }
    }

    if (_fd[lev] == nullptr)
    {
        _filePath[lev] = getLogFilePath(lev, ttm, 0);
        memcpy(&_fileTm[lev], ttm, sizeof(tm));
        _nowSz[lev] = 0;

        _fd[lev] = fopen(_filePath[lev].c_str(), "a");
        if (_fd[lev] == nullptr)
        {
            YS_EXCEPT_LOG("open file fail [%s].", _filePath[lev].c_str());
        }
        else
        {
            YS_NOTE("open log file , %s", _filePath[lev].c_str());
            _nowSz[lev] = ftell(_fd[lev]);
        }
    }

    return _fd[lev];
}

inline void YS_Log::saveLog()
{
    while (_fRun)
    {
        auto log = popLog();
        if (log)
        {
            LEV tLev = log->_lev;
            if (_singleFile) tLev = LEV::LEV_CLOSE;

            auto fp = getLogFile(tLev, &log->_tm, log->_log.length());
            if (fwrite(log->_log.c_str(), log->_log.length(), 1, fp) != 0)
            {
                _nowSz[tLev] += log->_log.length();
                if (fflush(fp))
                {
                    YS_EXCEPT_LOG("fflush failed.");
                    closeLogFile(tLev, false);
                }
            }
            else
            {
                YS_EXCEPT_LOG("log write failed %d.", YS_ERRNO);
                closeLogFile(tLev, false);
            }
            delete log;
        }
    }
}

inline void YS_Log::pushLog(LogNode *node)
{
    if (node->_lev > LEV_ERROR)
    {
        _queue.push_back(node, 4096, true);
    }
    else
    {
        _queue.push_back(node, true);
    }
}

inline YS_Log::LogNode *YS_Log::popLog()
{
    LogNode *log = nullptr;

    if (!_queue.pop_front(log, 500))
    {
        return nullptr;
    }
    return log;
}

} // namespace isaac

#endif