#pragma once
#ifndef _YS_SERVER_ASYN_H_
#define _YS_SERVER_ASYN_H_

#include "ys_accepter.h"
#include "ys_thread_asyn.h"

// 异步tcp线程
namespace isaac {

class YS_TcpServer : private YS_BusiPool, private YS_Accepter
{
private:
    uint32_t _maxConn = 1024;
    
public:
    YS_TcpServer(const std::string &ip, uint32_t port, YS_DriverPtr pHd)
        : YS_Accepter(ip, port)
    {
        setDriver(pHd);
    }
    YS_TcpServer(const std::string &ip, uint32_t port) : YS_Accepter(ip, port) {}

    void onAccept(YS_SocketPtr conn)
    {
        if (_maxConn > connNum())
        {
            addConn(conn);
        }
        else
        {
            CLI_ERROR("reach max connection num {} {}", _maxConn, connNum());
        }
    }
    
    virtual void onConn(YS_SocketPtr &p) {}
    virtual void onDisConn(YS_SocketPtr &p) {}

    void setMaxConnNum(uint32_t num) { _maxConn = num; }
    using YS_Accepter::isRunning;
    using YS_Accepter::setSslCtx;
    using YS_BusiPool::getDriver;
    using YS_BusiPool::setDriver;
    using YS_BusiPool::setThreadPolicy;

    void start()
    {
        YS_BusiPool::start();
        YS_Accepter::start();
    }

    void stop()
    {
        YS_Accepter::stop();
        YS_BusiPool::stop();
    }
};

} // namespace isaac

#endif