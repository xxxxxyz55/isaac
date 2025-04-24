#ifndef __YS_UDP_CLIENT_H__
#define __YS_UDP_CLIENT_H__
#include "ys_udp.h"
#include "ys_epoller.h"

namespace isaac {

class YS_UdpClient : public YS_Epoller
{
private:
    YS_UdpPtr _sock;

public:
    YS_UdpClient() : YS_Epoller(1), _sock(nullptr) {}

    YS_UdpPtr getSock() { return _sock; }

    int32_t create()
    {
        if (_sock == nullptr || !_sock->isConnected())
        {
            _sock = std::make_shared<YS_Udp>();
            return _sock->createSock();
        }
        return 0;
    }

    int32_t bind(const std::string &ip, uint32_t port)
    {
        int32_t ret;
        if ((ret = create())) return ret;
        return _sock->bind(ip, port);
    }

    void close()
    {
        if (_sock)
        {
            _sock->close();
            _sock = nullptr;
        }
    }

    int32_t sendTo(YS_Buf &buf, YS_EndPoint &ep)
    {
        return _sock->sendTo(buf.sData(), buf.length(), ep);
    }

    int32_t send(YS_Buf &buf)
    {
        return _sock->send(buf);
    }

    int32_t recvFrom(YS_Buf &buf, YS_EndPoint &ep)
    {
        uint32_t len = buf.leftSize();
        int32_t  ret = _sock->recvfrom(buf.left(), &len, ep);
        if (ret)
        {
            CLI_ERROR("udp recv fail, {}.", YS_ERRNO);
            return ret;
        }
        buf.offset(len);
        return 0;
    }

    void wait() { fireOnce(); }

    void onEpollIn(YS_EpData *pData){}
    void onEpollErr(YS_EpData *pData)
    {
        CLI_ERROR("udp client get epoll error.");
    };
};

} // namespace isaac

#endif