#pragma once
#ifndef _YS_UDP_EX_H_
#define _YS_UDP_EX_H_

#include "ys_socket.h"

namespace isaac {

// UDP连接
class YS_Udp;
using YS_UdpPtr = std::shared_ptr<YS_Udp>;

class YS_Udp : public YS_Socket
{
public:
    // 创建一个udp套接字 //window仅ipv4
    int32_t createSock();

    // 绑定地址
    int32_t bind();
    int32_t bind(const std::string &ip, int32_t port) { return bind(ip.c_str(), port); }
    int32_t bind(const char *ip, int32_t port);

    // 发送
    int32_t sendTo(const char *data, uint32_t dataLen, YS_EndPoint &ed);
    // 连接到地址
    int32_t connect(YS_EndPoint &ed);

    static int32_t recvAddr(YS_SocketPtr sock, YS_EndPoint &ed);

    std::string toIpv6(const std::string &ip);
};

inline int32_t YS_Udp::createSock()
{
    if (getFd() < 0)
    {
#if _WIN32
        getFd().reset(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
#else
        getFd().reset(::socket(AF_INET, SOCK_DGRAM, 0));
        // getFd().reset(::socket(AF_INET6, SOCK_DGRAM, 0));
#endif
        if (getFd() < 0)
        {
            CLI_ERROR("create socket fail {}.", YS_ERRNO);
            return -1;
        }
    }

    return 0;
}

inline int32_t YS_Udp::bind()
{
    return bind(getLocSock().getIp(), getLocSock().getPort());
}

inline std::string YS_Udp::toIpv6(const std::string &ip)
{
    auto pos = ip.find(":");
    if (pos != std::string::npos)
    {
        return ip;
    }
    else
    {
        return "::" + ip;
    }
}

inline int32_t YS_Udp::bind(const char *ip, int32_t port)
{
    if (createSock())
    {
        return -1;
    }

    setBlock(false);
    setCloseOnExec();
    setReuseAddr();
    // setIpv6Only();
    setRecvBufSize();
    setSendBufSize();
    std::string sIp = ip;
#if _WIN32
#else
    // sIp = toIpv6(sIp);
#endif
    YS_EndPoint ed(sIp, port);
    if (::bind(getFd(), &ed.addr, ed.sockSize()) < 0)
    {
        CLI_ERROR("bind fail [error:{}][{}][{}][socksize:{}].", YS_ERRNO, ip, port, ed.sockSize());
        perror("bind fail ");
        getFd().reset(-1);
        return -1;
    }
    else
    {
        CLI_DEBUG("bind OK [{}][{}].", ip, port);
        return 0;
    }
}

inline int32_t YS_Udp::sendTo(const char *data, uint32_t dataLen, YS_EndPoint &ed)
{
    if (createSock())
    {
        return -1;
    }
    int32_t sendLen = 0;

    sendLen = ::sendto(getFd(), data, dataLen, 0, &ed.addr, ed.sockSize());
    if (sendLen < int32_t(dataLen))
    {
        CLI_ERROR("sock sendto data fail [{}] [{}] error {}.", sendLen, dataLen, YS_ERRNO);
        return -1;
    }
    return 0;
}

inline int32_t YS_Udp::connect(YS_EndPoint &ed)
{
    setBlock(false);
    setCloseOnExec();
    setReuseAddr();
    setIpv6Only();
    setRecvBufSize();
    setSendBufSize();

    if (::connect(getFd(), &ed.addr, ed.sockSize()))
    {
        CLI_ERROR("connect fail, {}", YS_ERRNO);
        perror("connect fail.");
        return -1;
    }
    return 0;
}

inline int32_t YS_Udp::recvAddr(YS_SocketPtr sock, YS_EndPoint &ed)
{
    char    buf[1];

#if _WIN32
    int32_t sz;
#else
    socklen_t sz;
#endif
    sz = ed.sockSize();
    if (::recvfrom(sock->getFd(), buf, sizeof(buf), MSG_PEEK, &ed.addr, &sz) < 0)
    {
        return -1;
    }
    return 0;
}

} // namespace isaac

#endif