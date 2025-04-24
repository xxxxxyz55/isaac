#ifndef _YS_SOCKET_H_
#define _YS_SOCKET_H_

#include "ys_endpoint.h"
#include "ys_cli_log.h"
#include "tool/ys_buffer.hpp"
#include "ys_ssl.h"
#include "tool/ys_timer.hpp"
#include <memory>

#if _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#endif

namespace isaac {
// tcp操作 socket

// fd 描述符
class YS_FdGuard
{
private:
    int32_t _fd;

public:
    YS_FdGuard(int32_t fd) : _fd(fd) {}
    YS_FdGuard() : _fd(-1) {}
    ~YS_FdGuard() { reset(-1); }

    // 赋值
    void reset(int32_t fd);

    operator int32_t() const { return _fd; }
};

class YS_Socket;
using YS_SocketPtr = std::shared_ptr<YS_Socket>;
using YS_SockCb    = void (*)(YS_SocketPtr &);

// 套接字
class YS_Socket
{
public:
    YS_FdGuard  &getFd() { return _fd; }
    YS_EndPoint &getLocSock() { return _edLoc; }
    YS_EndPoint &getPeerSock() { return _edPeer; }
    SSL         *getSsl() { return _pSsl; }
    bool         isGmssl();

    void close();

    ~YS_Socket();

    bool isConnected() { return getFd() == -1 ? false : true; }

    void setBlock(bool bBlock);
    void setCloseOnExec();
    void setCloseWait(bool bOnOff, int32_t delay);
    void setReuseAddr();

    // 仅windows
    void setIpv6Only(uint32_t opt = 0);

    void setSendBufSize(int32_t sz = 512 * 1024) { setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, (char *)&sz, sizeof(sz)); }
    void setRecvBufSize(int32_t sz = 512 * 1024) { setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, (char *)&sz, sizeof(sz)); }

    YS_EndPoint &getLocEndPoint();

    // 先bind或者connect
    // 0 全部发送
    // <0 发送错误
    // >0 发送成功部分
    int32_t send(const uint8_t *data, uint32_t dataLen);
    int32_t send(std::string &buf);
    int32_t send(YS_Buf &buf);

    // 接收len长或一次engine
    int32_t recv(uint8_t *buf, uint32_t *len) { return _recvfrom(buf, len, nullptr, nullptr); }

    int32_t recvfrom(uint8_t *buf, uint32_t *len);
    int32_t recvfrom(uint8_t *buf, uint32_t *len, YS_EndPoint &ed);

    int32_t recv(YS_Buf &buf);
    void    closeWait() { _fclosewait = true; }
    bool    isCloseWait() { return _fclosewait; }

protected:
    SSL *_pSsl = nullptr;

private:
    YS_EndPoint _edLoc;
    YS_EndPoint _edPeer;
    YS_FdGuard  _fd;
    bool        _fclosewait = false; // 等待关闭，短链接使用

#if _WIN32
    using SockLen_t = int32_t;
#else
    using SockLen_t = uint32_t;
#endif

    int32_t _recvfrom(uint8_t *buf, uint32_t *len, sockaddr *from, SockLen_t *fromlen);
};

inline void YS_FdGuard::reset(int32_t fd)
{
    if (_fd >= 0)
    {
#if _WIN32
        // CLI_DEBUG("close fd == {}", _fd);
        ::closesocket(_fd);
#else
        ::close(_fd);
#endif
    }
    _fd = fd;
}

inline void YS_Socket::close()
{
    if (_pSsl)
    {
        YS_Ssl::freeSSL(_pSsl);
        _pSsl = nullptr;
    }
    _fd.reset(-1);
}

inline void YS_Socket::setBlock(bool bBlock)
{
#if _WIN32
    uint64_t ul = bBlock ? 0 : 1;

    int32_t ret;
    ret = ioctlsocket(_fd, FIONBIO, (unsigned long *)&ul);
    if (ret == SOCKET_ERROR)
    {
        CLI_ERROR("ioctlsocket FIONBIO error.");
        return;
    }
#else

    int32_t val = 0;
    if ((val = fcntl(_fd, F_GETFL, 0)) == -1)
    {
        CLI_ERROR("fcntl F_GETFL error.");
        return;
    }

    if (!bBlock)
    {
        val |= O_NONBLOCK;
    }
    else
    {
        val &= ~O_NONBLOCK;
    }

    if (fcntl(_fd, F_SETFL, val) == -1)
    {
        CLI_ERROR("fcntl F_SETFL error.");
        return;
    }
#endif
}

inline YS_Socket::~YS_Socket()
{
    if (_pSsl) YS_Ssl::freeSSL(_pSsl);
}

inline void YS_Socket::setCloseOnExec()
{
#if _WIN32
#else
    fcntl(_fd, F_SETFD, 1 /*FD_CLOSEXEC*/);
#endif
}

inline void YS_Socket::setCloseWait(bool bOnOff, int32_t delay)
{
    linger opt;
    opt.l_onoff  = bOnOff ? 1 : 0; // close socket 后运行逗留的时间
    opt.l_linger = delay;

    setsockopt(_fd, SOL_SOCKET, SO_LINGER, (const char *)&opt, sizeof(opt));
}

inline void YS_Socket::setReuseAddr()
{
    const int32_t on = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR,
                   (const char *)&on, sizeof(on)) != 0)
    {
        CLI_ERROR("setsockopt SO_REUSEADDR fail.");
    }

#if __linux__
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT,
                   (const char *)&on, sizeof(on)) != 0)
    {
        CLI_ERROR("setsockopt SO_REUSEPORT fail.");
    }
#endif
}

inline void YS_Socket::setIpv6Only(uint32_t opt)
{
#if _WIN32
    if (setsockopt(_fd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&opt, sizeof(opt)) < 0)
    {
        CLI_ERROR("setsockopt IPV6_V6ONLY fail [{}].", YS_ERRNO);
    }
#endif
}

inline YS_EndPoint &YS_Socket::getLocEndPoint()
{
    SockLen_t sockLen = sizeof(_edLoc.addr6);
    getsockname(getFd(), &_edLoc.addr, &sockLen);
    return _edLoc;
}

inline int32_t YS_Socket::send(const uint8_t *data, uint32_t dataLen)
{
    uint32_t sendLen = 0;
    do
    {
        int32_t tLen;
        if (getSsl())
        {
            tLen = YS_Ssl::write(getSsl(), (const char *)data + sendLen, dataLen - sendLen);
        }
        else
        {
            tLen = ::send(getFd(), (const char *)data, dataLen - sendLen, 0);
        }
        if (tLen < 0)
        {
            if (getSsl())
            {
                if (YS_Ssl::isSslEngine(getSsl(), tLen))
                {
                    if (YS_Util::canWrite(getFd(), 3000))
                    {
                        continue;
                    }
                    else
                    {
                        CLI_DEBUG("tLen = {} senLen = {}", tLen, sendLen);
                        return sendLen;
                    }
                }
                else
                {
                    CLI_ERROR("ssl error:{}", YS_Ssl::opensslErr());
                }
            }
            else
            {
#if _WIN32
                if (YS_ERRNO == WSAEWOULDBLOCK)
#else
                if (YS_ERRNO == EAGAIN || YS_ERRNO == EWOULDBLOCK)
#endif
                {
                    if (YS_Util::canWrite(getFd(), 3000))
                    {
                        continue;
                    }
                    else
                    {
                        CLI_DEBUG("tLen = {} senLen = {}", tLen, sendLen);
                        return sendLen;
                    }
                }
            }
            CLI_DEBUG("send tlen = {} want send len = {} error {}", tLen, dataLen - sendLen, YS_ERRNO);
            if (sendLen == 0)
            {
                return tLen;
            }
            else
            {
                return sendLen;
            }
        }
        else
        {
            sendLen += tLen;
        }

    } while (sendLen < dataLen);
    return sendLen;
}

inline int32_t YS_Socket::recvfrom(uint8_t *buf, uint32_t *len)
{
    SockLen_t sockSize = sizeof(_edPeer.addr6);
    return _recvfrom(buf, len, &_edPeer.addr, &sockSize);
}

inline int32_t YS_Socket::recvfrom(uint8_t *buf, uint32_t *len, YS_EndPoint &ed)
{
    SockLen_t sockSize = sizeof(ed.addr6);
    return _recvfrom(buf, len, &ed.addr, &sockSize);
}

inline int32_t YS_Socket::recv(YS_Buf &buf)
{
    uint32_t len = buf.leftSize();
    if (recv(buf.left(), &len))
    {
        CLI_ERROR("recv fail ,{}.", YS_ERRNO);
        return -1;
    }
    buf.offset(len);
    return 0;
}

inline int32_t YS_Socket::send(std::string &buf)
{
    if (buf.length() == 0) return 0;
    return send((const uint8_t *)buf.c_str(), buf.length());
}

inline int32_t YS_Socket::send(YS_Buf &buf)
{
    if (buf.length() == 0) return 0;
    return send(buf.uData(), buf.length());
}

inline int32_t YS_Socket::_recvfrom(uint8_t *buf, uint32_t *len, sockaddr *from, SockLen_t *fromlen)
{
    int32_t  recvLen;
    uint32_t recvTotal = 0;

    do
    {
        if (getSsl())
        {
            recvLen = YS_Ssl::read(getSsl(), (char *)buf + recvTotal, *len - recvTotal);
        }
        else
        {
            recvLen = ::recvfrom(getFd(), (char *)buf + recvTotal, *len - recvTotal, 0, from, fromlen);
        }

        if (recvLen > 0)
        {
            recvTotal += recvLen;
            if (recvTotal == *len) return 0;
        }
        else if (recvLen == 0)
        {
            if (recvTotal != 0)
            {
                *len = recvTotal;
                return 0;
            }
            CLI_ERROR("recv len = 0,close socket ssl = {}.", recvTotal, getSsl() ? true : false);
            return -1;
        }
        else
        {
            if (getSsl())
            {
                if (YS_Ssl::isSslEngine(getSsl(), recvLen)) break;
            }
            else
            {
#if _WIN32
                if (YS_ERRNO == WSAEWOULDBLOCK) break;
#else
                if (YS_ERRNO == EWOULDBLOCK || YS_ERRNO == EAGAIN) break;
#endif
            }

            CLI_ERROR("recv err pkt errno {} recvlen[{}]", YS_ERRNO, recvTotal);
            break;
        }
    } while (recvLen >= 0);

    *len = recvTotal;
    return 0;
}

inline bool YS_Socket::isGmssl()
{
    return YS_Ssl::isGmssl(_pSsl);
}

} // namespace isaac

#endif