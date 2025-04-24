#pragma once
#ifndef _YS_TCP_H_
#define _YS_TCP_H_

#include "ys_epoller.h"
#if _WIN32
#include <mstcpip.h>
#endif

namespace isaac {

class YS_Tcp;
using YS_TcpPtr = std::shared_ptr<YS_Tcp>;
// tcp连接
class YS_Tcp : public YS_Socket
{
public:
    // tcp参数
    void setTcpNoDelay();

    // tcp保活
    void setKeepAlive();
    void setKeepAliveParam();

    // 接受连接
    static YS_TcpPtr accept(YS_SocketPtr sock, YS_SslCtxPtr pCtx = nullptr);
    // 监听端口
    static YS_TcpPtr listen(const std::string &ip, int32_t port, YS_SslCtxPtr pCtx = nullptr);
    static YS_TcpPtr listen(const YS_EndPoint &ep, YS_SslCtxPtr pCtx = nullptr);

    YS_FdGuard &connect();
    // 连接至
    static YS_TcpPtr connect(const char *ip, int32_t port, YS_SslCtxPtr pCtx = nullptr);
    // 重连
    bool reconnected(YS_SslCtxPtr pCtx = nullptr);
};

inline void YS_Tcp::setTcpNoDelay()
{
    int32_t flag = 1;
    if (setsockopt(getFd(), IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(flag)) == -1)
    {
        CLI_ERROR("set tcp no delay error.");
    }
}

inline void YS_Tcp::setKeepAlive()
{
    int32_t flag = 1;
    if (setsockopt(getFd(), SOL_SOCKET, SO_KEEPALIVE, (const char *)&flag, sizeof(flag)) == -1)
    {
        CLI_ERROR("set keep alive error.");
    }
}

inline void YS_Tcp::setKeepAliveParam()
{
#if _WIN32
    DWORD         len  = 0;
    tcp_keepalive conf = {0}, sRet = {0};
    conf.onoff             = 1;
    conf.keepalivetime     = 6000;
    conf.keepaliveinterval = 3000;
    if (WSAIoctl(getFd(), SIO_KEEPALIVE_VALS, &conf, sizeof(conf),
                 &sRet, sizeof(sRet), &len, nullptr, nullptr))
    {
        CLI_ERROR("set keepalive val fail.");
    }
#else
    int32_t keepidle     = 60;
    int32_t keepinterval = 5;
    int32_t keepcnt      = 3;

    if (setsockopt(getFd(), SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) < 0)
    {
        CLI_ERROR("Unable to set socket TCP_KEEPIDLE option.");
    }

    if (setsockopt(getFd(), SOL_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval)) < 0)
    {
        CLI_ERROR("Unable to set socket TCP_KEEPINTVL option.");
    }

    // TCP_KEEPCNT
    if (setsockopt(getFd(), SOL_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) < 0)
    {
        CLI_ERROR("Unable to set socket TCP_KEEPCNT option.");
    }
#endif
}

inline YS_TcpPtr YS_Tcp::accept(YS_SocketPtr sock, YS_SslCtxPtr pCtx)
{
    YS_TcpPtr pSock(new YS_Tcp());
#if __linux__
    uint32_t sockSize;
#elif _WIN32
    int32_t  sockSize;
#endif

    sockSize = sock->getLocSock().sockSize();
    pSock->getFd().reset(::accept(sock->getFd(),
                                  &pSock->getPeerSock().addr, &sockSize));
    if (pSock->getFd() < 0)
    {
        return nullptr;
    }

    pSock->setBlock(false);
    pSock->setCloseOnExec();
    pSock->setKeepAlive();
    pSock->setKeepAliveParam();
    pSock->setTcpNoDelay();
    pSock->setCloseWait(0, 0);
    pSock->setSendBufSize();
    pSock->setRecvBufSize();

    if (pCtx)
    {
        if (!(pSock->_pSsl = YS_Ssl::accept(pCtx, pSock->getFd())))
        {
            CLI_ERROR("ssl accept fail.");
            return nullptr;
        }
    }
    return pSock;
}

inline YS_TcpPtr YS_Tcp::listen(const std::string &ip, int32_t port, YS_SslCtxPtr pCtx)
{
    YS_TcpPtr pSock(new YS_Tcp());

    pSock->getLocSock().setAddr(ip, port);

    pSock->getFd().reset(::socket(pSock->getLocSock().addr.sa_family, SOCK_STREAM, 0));
    if (pSock->getFd() < 0)
    {
        CLI_DEBUG("create socket fail {}.", YS_ERRNO);
        return nullptr;
    }
    else
    {
        pSock->setBlock(false);
        pSock->setReuseAddr();
        pSock->setIpv6Only(0);
        pSock->setCloseOnExec();
        pSock->setKeepAlive();
        pSock->setKeepAliveParam();
        pSock->setTcpNoDelay();
        pSock->setCloseWait(0, 0);
        pSock->setSendBufSize();
        pSock->setRecvBufSize();

        if (::bind(pSock->getFd(), &pSock->getLocSock().addr,
                   pSock->getLocSock().sockSize()))
        {
            CLI_ERROR("bind addr fail port [{}].", port);
            return nullptr;
        }

        if (::listen(pSock->getFd(), 1024) < 0)
        {
            CLI_ERROR("listen fail.");
            return nullptr;
        }
        else
        {
            CLI_DEBUG("listen on [{}:{}].", ip, port);
        }
    }

    if (pCtx)
    {
        if (!(pSock->_pSsl = YS_Ssl::newSSl(pCtx, pSock->getFd())))
        {
            return nullptr;
        }
    }

    return pSock;
}

inline YS_TcpPtr YS_Tcp::listen(const YS_EndPoint &ep, YS_SslCtxPtr pCtx)
{
    return listen(ep.getIp(), ep.getPort(), pCtx);
}

inline YS_FdGuard &YS_Tcp::connect()
{
    YS_FdGuard &rFd = getFd();
    rFd.reset(::socket(getPeerSock().addr.sa_family, SOCK_STREAM, 0));
    if (rFd < 0)
    {
        CLI_ERROR("create socket fail [{}].", YS_ERRNO);
        return rFd;
    }

    setReuseAddr();
    setBlock(false);

#if _WIN32
    int32_t sockLen = getPeerSock().sockSize();
#else
    uint32_t sockLen = getPeerSock().sockSize();
#endif

    if (::connect(rFd, &getPeerSock().addr, getPeerSock().sockSize()) < 0)
    {
        if (YS_Util::canWrite(rFd, 3000))
        {
            // int32_t err = 0;
            // if (getsockopt(rFd, SOL_SOCKET, SO_ERROR, (char *)&err, &sockLen) < 0 || err)
            // {
            //     CLI_ERROR("connect fail [{}] [{}].", err, YS_ERRNO);
            //     rFd.reset(-1);
            //     return rFd;
            // }
        }
        else
        {
            CLI_ERROR("connect timeout {}.", YS_ERRNO);
            rFd.reset(-1);
            return rFd;
        }
    }

    getsockname(rFd, &getLocSock().addr, &sockLen);

    setCloseOnExec();
    setKeepAlive();
    setKeepAliveParam();
    setSendBufSize();
    setRecvBufSize();
    setCloseWait(0, 0);
    setTcpNoDelay();
    return rFd;
}

inline YS_TcpPtr YS_Tcp::connect(const char *ip, int32_t port, YS_SslCtxPtr pCtx)
{
    YS_TcpPtr pSock(new YS_Tcp());

    pSock->getPeerSock().setAddr(ip, port);

    if (pSock->connect() < 0)
    {
        CLI_ERROR("connect fail [{}][{}].", ip, port);
        return nullptr;
    }

    if (pCtx)
    {
        if (!(pSock->_pSsl = YS_Ssl::connect(pCtx, pSock->getFd())))
        {
            return nullptr;
        }
    }

    return pSock;
}

inline bool YS_Tcp::reconnected(YS_SslCtxPtr pCtx)
{
    if (connect() < 0) return false;

    if (pCtx)
    {
        if (_pSsl)
        {
            YS_Ssl::freeSSL(_pSsl);
        }

        if (!(_pSsl = YS_Ssl::connect(pCtx, getFd())))
        {
            return false;
        }
    }
    return true;
}

} // namespace isaac

#endif