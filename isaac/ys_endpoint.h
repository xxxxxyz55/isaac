#ifndef _YS_ENDPOINT_H_
#define _YS_ENDPOINT_H_

#include "ys_cli_log.h"
#include <cstring>

#if _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Winsock2.h>
#include <ws2tcpip.h>

#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netdb.h>
#endif

namespace isaac {
// ipv4 ipv6兼容 地址
class YS_EndPoint
{
public:
    union
    {
        struct sockaddr     addr;
        struct sockaddr_in  addr4;
        struct sockaddr_in6 addr6;
    };

    // 构造
    YS_EndPoint(const char *ip, int32_t port) { setAddr(ip, port); }
    YS_EndPoint(const std::string &ip, int32_t port) { setAddr(ip, port); }
    YS_EndPoint(const YS_EndPoint &ep) { memcpy(&addr, &ep, sizeof(addr6)); }
    YS_EndPoint(struct sockaddr *addr) { setAddr(addr); }
    // 空构造
    YS_EndPoint(){};

    // 设置地址
    void setAddr(const std::string &ip, int32_t port) { setAddr(ip.c_str(), port); }
    void setAddr(YS_EndPoint &ed) { memcpy(&addr, &ed.addr, sizeof(addr6)); }
    void setAddr(const char *ip, int32_t port);
    void setAddr(struct sockaddr *pAddr);

    // 比较
    int32_t cmpare(YS_EndPoint &ed);

    // socket大小默认ipv6
    size_t sockSize() const;

    // 是不是ipv6
    bool isIpv6() const;

    static std::vector<std::string> dns(std::string host)
    {
        std::vector<std::string> vt;
        struct addrinfo  hints = {};
        struct addrinfo *res   = nullptr;
        hints.ai_family        = AF_UNSPEC;
        hints.ai_socktype      = SOCK_STREAM;

        int32_t ret = getaddrinfo(host.c_str(), nullptr, &hints, &res);
        if (ret)
        {
            CLI_ERROR("getaddrinfo fail , host {}, {}.", host, gai_strerror(ret));
            return {};
        }
        for (auto p = res; p != nullptr; p = p->ai_next)
        {
            if (p->ai_family == AF_INET || p->ai_family == AF_INET6)
            {
                YS_EndPoint ed(p->ai_addr);
                CLI_DEBUG("{}==>{}", host, ed.getIp());
                vt.emplace_back(ed.getIp());
            }
        }

        freeaddrinfo(res);
        return vt;
    }

    // 获取地址
    std::string getIp() const;
    // 获取端口
    uint32_t getPort() const;
    std::string getAddr() { return getIp() + ":" + std::to_string(getPort()); };
};

inline void YS_EndPoint::setAddr(const char *ip, int32_t port)
{
    memset(&addr6, 0x00, sizeof(addr6));
    if (strchr(ip, ':'))
    {
        if (inet_pton(AF_INET6, ip, &addr6.sin6_addr) != 1)
        {
            CLI_ERROR("invalid addr [{}]", ip);
            return;
        }
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port   = htons(port);
    }
    else
    {
        if (inet_pton(AF_INET, ip, &addr4.sin_addr) != 1)
        {
            CLI_ERROR("invalid addr [{}]", ip);
            return;
        }
        addr4.sin_family = AF_INET;
        addr4.sin_port   = htons(port);
    }
}

inline int32_t YS_EndPoint::cmpare(YS_EndPoint &ed)
{
    if (ed.sockSize() == sockSize())
    {
        return memcmp(this, &ed, sockSize());
    }
    else
    {
        return 1;
    }
}

inline void YS_EndPoint::setAddr(struct sockaddr *pAddr)
{
    if (pAddr->sa_family == AF_INET)
    {
        memcpy(&addr4, pAddr, sizeof(addr4));
    }
    else if (pAddr->sa_family == AF_INET6)
    {
        memcpy(&addr6, pAddr, sizeof(addr6));
    }
    else
    {
        CLI_ERROR("unsupport family type.");
    }
}

inline size_t YS_EndPoint::sockSize() const
{
    if (isIpv6())
    {
        return sizeof(addr6);
    }
    else
    {
        return sizeof(addr4);
    }
}

inline bool YS_EndPoint::isIpv6() const
{
    if (addr.sa_family == AF_INET6)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline std::string YS_EndPoint::getIp() const
{
    char ip[INET6_ADDRSTRLEN] = {};
    if (isIpv6())
    {
        inet_ntop(addr.sa_family, (void *)&addr6.sin6_addr, ip, INET6_ADDRSTRLEN);
    }
    else
    {
        inet_ntop(addr.sa_family, (void *)&addr4.sin_addr, ip, INET6_ADDRSTRLEN);
    }
    return std::string(ip);
}

inline uint32_t YS_EndPoint::getPort() const
{
    if (isIpv6())
    {
        return ntohs(addr6.sin6_port);
    }
    else
    {
        return ntohs(addr4.sin_port);
    }
}
} // namespace isaac

#endif