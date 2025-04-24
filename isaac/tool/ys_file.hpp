#ifndef _YS_FILE_H_
#define _YS_FILE_H_
#include <string>
#include <cstdint>
#if __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#else
#include <WinSock2.h>
#include <windows.h>
#include <direct.h>
#endif
#include "ys_string.hpp"
#include "ys_exception.hpp"
#include <algorithm>
#include <vector>
#include "ys_scope_guard.hpp"
#include <fstream>
#include <sstream>

namespace isaac {
class YS_File
{
public:
    // 整理路径 双斜杠变单斜杠
    static std::string fmtPath(const std::string &path);

    /**
     * 整理路径 默认分割符为/
     * 去掉..
     * 去掉中间的./
     * 去掉多余的/
     */
    static std::string fmtPathEx(const std::string &path);

    // 读取整个文件到string
    static std::string read(const std::string &sPath);
    static std::string read(const std::string &sPath, size_t len, size_t off);

    // 覆盖写入文件,不存在则创建
    static int32_t write(const std::string &sPath, const uint8_t *uData, uint32_t uLen, bool append = false);
    static int32_t write(const std::string &sPath, const std::string &data, bool append = false);

    // 路径是否存在
    static bool exist(const std::string &path);

    // 目录不存在则创建
    static bool mkdir(const std::string &path);

    // 路径是一个普通文件
    static bool isFile(const std::string &path);

    // 路径是一个目录
    static bool isDir(const std::string &path);

    // 文件大小
    static size_t size(const std::string &path);

    struct DirInfo
    {
        std::string name;
        std::string modTm;
        uint64_t    size;
        bool        isDir;
    };

    // 遍历目录
    static int32_t     scanDir(const std::string &path, std::vector<DirInfo> &vtDir);
    static std::string name(std::string path);

private:
    static int32_t scanDirWin32(std::string path, std::vector<DirInfo> &vtDir);

    static int32_t scanDirLinux(const std::string &path, std::vector<DirInfo> &vtDir);

public:
    // 删除路径
    static void remove(std::string path);
};

inline std::string YS_File::fmtPath(const std::string &path)
{
    std::string sPath = path;
// win
#if _WIN32
    std::replace(sPath.begin(), sPath.end(), '/', '\\');
    sPath = YS_String::replace(sPath, "\\\\", "\\");
#else
    // linux
    std::replace(sPath.begin(), sPath.end(), '\\', '/');
    sPath     = YS_String::replace(sPath, "//", "/");
#endif
    return sPath;
}

inline std::string YS_File::fmtPathEx(const std::string &path)
{
    if (path.empty()) return path;

    std::string tmp = path;
    std::replace(tmp.begin(), tmp.end(), '\\', '/');
    tmp = YS_String::replace(tmp, "//", "/");

    std::vector<std::string> vtSub;

    size_t op  = 0;
    size_t pos = 0;

    while ((pos = tmp.find_first_of('/', pos)) != std::string::npos)
    {
        if (pos == 0)
        {
            vtSub.emplace_back("/");
        }
        else
        {
            vtSub.emplace_back(tmp.substr(op, pos - op));
        }
        op  = pos + 1;
        pos = op;
    }
    if (op != tmp.length())
    {
        vtSub.emplace_back(tmp.substr(op++));
    }

    bool                     beginOnLoc = false;
    std::vector<std::string> stk;
    // 去除带.
    for (size_t i = 0; i < vtSub.size(); i++)
    {
        if (vtSub[i] == ".")
        {
            if (i == 0)
            {
                beginOnLoc = true;
                continue;
            }
            else
            {
                continue;
            }
        }
        stk.emplace_back(vtSub[i]);
    }

    std::vector<std::string> stk1;
    for (auto &&iter : stk)
    {
        if (iter == "..")
        {
            if (stk1.size() && stk1[stk1.size() - 1] != ".." && stk[stk1.size() - 1] != "/")
            {
                stk1.pop_back();
                continue;
            }
        }
        stk1.emplace_back(iter);
    }
    std::string relPath;
    relPath.reserve(path.size());
    if (beginOnLoc)
    {
        relPath.append("./");
    }

    for (size_t i = 0; i < stk1.size(); i++)
    {
        if (i != 0)
        {
            relPath.append("/");
        }
        relPath.append(stk1[i]);
    }
    if (tmp[tmp.size() - 1] == '/')
    {
        relPath.append("/");
    }
    return YS_String::replace(relPath, "//", "/");
}

inline std::string YS_File::read(const std::string &sPath, size_t len, size_t off)
{
    std::ifstream file(sPath, std::ios::binary);
    if (file.bad()) return "";
    file.seekg(off, std::ios::beg);
    if (file.bad()) return "";
    std::vector<char> buf(len + 1);
    file.read(buf.data(), len);
    return std::string(buf.data(), file.gcount());
}

inline std::string YS_File::read(const std::string &sPath)
{
    std::ifstream file(sPath, std::ios::binary);
    if (!file) return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline int32_t YS_File::write(const std::string &sPath, const uint8_t *uData, uint32_t uLen, bool append)
{
    std::ofstream file(sPath, std::ios::binary);
    if (file.fail()) return -1;

    if (append) file.seekp(std::ios::end);

    file.write((char *)uData, uLen);
    if (file.fail()) return -1;

    return 0;
}

inline int32_t YS_File::write(const std::string &sPath, const std::string &data, bool append)
{
    std::ofstream file(sPath, std::ios::binary);
    if (file.fail()) return -1;

    if (append) file.seekp(std::ios::end);

    file << data;
    if (file.fail()) return -1;

    return 0;
}

inline bool YS_File::exist(const std::string &path)
{
#if __linux__
    struct stat info;
    if (::stat(path.c_str(), &info))
#else
    auto type = GetFileAttributesA(path.c_str());
    if (type == INVALID_FILE_ATTRIBUTES)
#endif
    {
        return false;
    }

    return true;
}

inline bool YS_File::mkdir(const std::string &path)
{
#if __linux__
    if (::mkdir(path.c_str(), 0777) == 0)
#else
    if (::_mkdir(path.c_str()) == 0)
#endif
    {
        return true;
    }
    else
    {
        if (errno == EEXIST) return true;
        return false;
    }
}

inline bool YS_File::isFile(const std::string &path)
{
#if _WIN32
    auto type = GetFileAttributesA(path.c_str());
    if (type == INVALID_FILE_ATTRIBUTES)
#else
    struct stat info;
    if (::stat(path.c_str(), &info))
#endif
    {
        return false;
    }

#if _WIN32
    if (type == FILE_ATTRIBUTE_NORMAL || type == FILE_ATTRIBUTE_ARCHIVE)
#else
    if (info.st_mode & S_IFREG)
#endif
    {
        return true;
    }

    return false;
}

inline bool YS_File::isDir(const std::string &path)
{
#if __linux__
    struct stat info;
    if (::stat(path.c_str(), &info))
#else
    auto type = GetFileAttributesA(path.c_str());
    if (type == INVALID_FILE_ATTRIBUTES)
#endif
    {
        return false;
    }

#if __linux__
    if (info.st_mode & S_IFDIR)
#else
    if (type == FILE_ATTRIBUTE_DIRECTORY)
#endif
    {
        return true;
    }

    return false;
}

inline size_t YS_File::size(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.bad()) return 0;
    return file.tellg();
}

inline int32_t YS_File::scanDir(const std::string &path, std::vector<DirInfo> &vtDir)
{
    if (!exist(path))
    {
        return -1;
    }
#if _WIN32
    return scanDirWin32(path, vtDir);
#else
    return scanDirLinux(path, vtDir);
#endif
}

inline int32_t YS_File::scanDirWin32(std::string path, std::vector<DirInfo> &vtDir)
{
#if _WIN32
    WIN32_FIND_DATAA fd{0};
    HANDLE           handle{INVALID_HANDLE_VALUE};
    path.append("*");

    handle = FindFirstFileA(path.c_str(), &fd);
    if (handle == INVALID_HANDLE_VALUE)
    {
        YS_EXCEPT_LOG("find first file fail, %s %d", path.c_str(), GetLastError());
        return -1;
    }

    do
    {
        DirInfo info;
        info.name = fd.cFileName;
        ULARGE_INTEGER ul;
        ul.LowPart  = fd.nFileSizeLow;
        ul.HighPart = fd.nFileSizeHigh;
        info.size   = ul.QuadPart;
        SYSTEMTIME sTm;
        FileTimeToSystemTime(&fd.ftLastWriteTime, &sTm);
        char buf[32] = {};
        ::snprintf(buf, sizeof(buf), "%d-%d-%d %d:%d:%d",
                   sTm.wYear, sTm.wMonth, sTm.wDay,
                   sTm.wHour, sTm.wMinute, sTm.wSecond);
        info.modTm = buf;
        info.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
        vtDir.emplace_back(info);
        ::memset(&fd, 0, sizeof(fd));
    } while (FindNextFileA(handle, &fd));

    if (handle != INVALID_HANDLE_VALUE) FindClose(handle);
    return 0;
#else
    return -1;
#endif
}

inline int32_t YS_File::scanDirLinux(const std::string &path, std::vector<DirInfo> &vtDir)
{
#if _WIN32
#else
    std::string sDir = path;
    if (!isDir(path))
    {
        sDir = sDir.substr(0, sDir.find_last_of('/'));
    }

    struct dirent *pSub = nullptr;

    DIR *pDir = opendir(sDir.c_str());
    if (pDir == nullptr)
    {
        YS_EXCEPT_LOG("opendir fail , %s.", sDir.c_str());
        return -1;
    }

    while ((pSub = readdir(pDir)))
    {
        DirInfo info;
        if (pSub->d_type == DT_DIR)
        {
            info.isDir = true;
        }
        else if (pSub->d_type == DT_REG)
        {
            info.isDir = false;
        }
        else
        {
            YS_EXCEPT_LOG("ignore file %s.", pSub->d_name);
            continue;
        }
        info.name = pSub->d_name;

        struct stat tStat = {};
        std::string fPath = sDir + "/" + info.name;
        if (!::stat(fPath.c_str(), &tStat))
        {
            info.modTm   = tStat.st_mtime;
            info.size    = tStat.st_size;
            char buf[32] = {};
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %z", localtime(&tStat.st_mtime));
            info.modTm = buf;
        }
        vtDir.emplace_back(info);
    }
    closedir(pDir);
    return 0;
#endif
}

inline void YS_File::remove(std::string path)
{
    if (path == "/")
    {
        return;
    }
    ::remove(path.c_str());
}

inline std::string YS_File::name(std::string path)
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos)
    {
#if _WIN32
        pos = path.find_last_of('\\');
        if (pos == std::string::npos)
#endif
        {
            return path;
        }
    }
    return path.substr(pos + 1);
}

}; // namespace isaac

#endif