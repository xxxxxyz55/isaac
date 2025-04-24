#ifndef __YS_UDP_SERVER_H__
#define __YS_UDP_SERVER_H__

#include "ys_thread_asyn.h"
#include "tool/ys_util.hpp"
#include "ys_udp.h"

namespace isaac {

class YS_UdpListener : YS_Epoller
{
public:
    YS_UdpListener(const std::string &ip, uint32_t port)
        : YS_Epoller(1), _ed(ip, port)
    {
        disableEt();
    }

    ~YS_UdpListener()
    {
        stop();
        if (_data) delete _data;
        if (_sock) delSock(_sock);
    }

    void stop() { _run = false; }
    bool isRunning() { return _run; }

    int32_t start()
    {
        _sock = std::make_shared<YS_Udp>();
        if (_sock->bind(_ed.getIp(), _ed.getPort()))
        {
            CLI_ERROR("udp bind addr fail [{}:{}].", _ed.getIp(), _ed.getPort());
            return -1;
        }
        _data = new YS_EpData(_sock);
        addSock(_data);
        while (_run)
        {
            fireOnce();
        }
        return 0;
    }
    virtual void onAccept(YS_SocketPtr conn) = 0;

    void onEpollErr(YS_EpData *pData) { CLI_ERROR("accepter get ep error."); }

    void onEpollIn(YS_EpData *pData)
    {
        YS_EndPoint ed;
        YS_Buf      buf(YS_Buf::size32KB());
        uint32_t    len = buf.leftSize();
        if (_sock->recvfrom(buf.left(), &len, ed))
        {
            return;
        }
        else
        {
            buf.offset(len);
            CLI_DEBUG("recv: {}", buf.sData());
            CLI_DEBUG("recv addr {}:{}", ed.getIp(), ed.getPort());
        }
        clearClosed();

        auto pSock = forward(ed, buf);
    }

    void clearClosed()
    {
        std::vector<std::string> del;
        for (auto &&iter : _mp)
        {
            if (!iter.second->isConnected())
            {
                del.emplace_back(iter.first);
            }
        }
        for (auto &&iter : del)
        {
            CLI_DEBUG("delete connection {}", iter);
            _mp.erase(iter);
        }
    }

    YS_UdpPtr newConn(YS_EndPoint &ed)
    {
        std::string addr = ed.getIp() + ":" + std::to_string(ed.getPort());
        YS_UdpPtr   conn = std::make_shared<YS_Udp>();
        conn->bind("0.0.0.0", 0);
        _mp.insert({addr, conn});
        return conn;
    }

    YS_UdpPtr forward(YS_EndPoint &ed, YS_Buf &buf)
    {
        std::string addr = ed.getIp() + ":" + std::to_string(ed.getPort());
        auto        iter = _mp.find(addr);
        if (iter == _mp.end())
        {
            auto pSock = newConn(ed);
            if (pSock == nullptr) return nullptr;
            CLI_DEBUG("forward send to {}:{}", pSock->getLocEndPoint().getIp(), pSock->getLocEndPoint().getPort());
            _sock->sendTo(buf.sData(), buf.length(), pSock->getLocEndPoint());
            if (pSock->connect(ed))
            {
                CLI_ERROR("connect to {}:{} fail.", ed.getIp(), ed.getPort());
                return nullptr;
            }
            onAccept(pSock);
            return pSock;
        }
        else
        {
            _sock->sendTo(buf.sData(), buf.length(), iter->second->getLocEndPoint());
            return nullptr;
        }
    }

private:
    YS_EndPoint _ed;
    YS_UdpPtr   _sock = nullptr;
    YS_EpData  *_data = nullptr;
    bool        _run  = true;

    std::map<std::string, YS_UdpPtr> _mp;
};

class YS_UdpServer : private YS_UdpListener, private YS_BusiPool
{
public:
    YS_UdpServer(const std::string &ip, uint32_t port, YS_DriverPtr pHd) : YS_UdpListener(ip, port)
    {
        setDriver(pHd);
    }

    void onAccept(YS_SocketPtr conn)
    {
        addConn(conn);
    }
    virtual void onConn(YS_SocketPtr &p) {}
    virtual void onDisConn(YS_SocketPtr &p) {}
    using YS_BusiPool::getDriver;
    using YS_BusiPool::setDriver;
    using YS_BusiPool::setThreadPolicy;
    using YS_UdpListener::isRunning;
    void start()
    {
        YS_BusiPool::start();
        // YS_BusiPool::disableEt();
        YS_BusiPool::setConnTmout(30000);
        YS_UdpListener::start();
    }

    void stop()
    {
        YS_UdpListener::stop();
        YS_BusiPool::stop();
    }
};
} // namespace isaac

#endif