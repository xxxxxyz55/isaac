#ifndef _YS_ACCEPTER_H_
#define _YS_ACCEPTER_H_

#include "ys_endpoint.h"
#include "ys_epoller.h"
#include "ys_tcp.h"

namespace isaac {

class YS_Accepter : private YS_Epoller
{
public:
    // 1 代表同时就一个ev
    YS_Accepter(const std::string &ip, uint32_t port)
        : YS_Epoller(1), _ed(ip, port) {}

    // 析构，关闭端口
    ~YS_Accepter();

    // 收到连接请求后处理
    virtual void onAccept(YS_SocketPtr) = 0;

    // 外部控制ssl
    void setSslCtx(YS_SslCtxPtr pCtx) { _ctx = pCtx; }

    // 重写，接收到事件后调用onAccept
    void onEpollIn(YS_EpData *pData);

    // 异常
    void onEpollErr(YS_EpData *pData) { CLI_ERROR("accepter get ep error."); }

    // 开始监听端口
    int32_t start();

    void stop() { _run = false; }
    bool isRunning() { return _run; }

private:
    YS_EndPoint  _ed;
    YS_SslCtxPtr _ctx  = nullptr;
    bool         _run  = true;
    YS_SocketPtr _sock = nullptr;
    YS_EpData   *_data = nullptr;
};

inline YS_Accepter::~YS_Accepter()
{
    stop();
    if (_data) delete _data;
    if (_sock) delSock(_sock);
    _ctx = nullptr;
}

inline void YS_Accepter::onEpollIn(YS_EpData *pData)
{
    while (true)
    {
        YS_SocketPtr pConn = YS_Tcp::accept(pData->sock, _ctx);
        if (pConn)
        {
            onAccept(pConn);
        }
        else
        {
            break;
        }
    }
}

inline int32_t YS_Accepter::start()
{
    _sock = YS_Tcp::listen(_ed, _ctx);
    if (!_sock)
    {
        CLI_ERROR("start listen fail.");
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

} // namespace isaac

#endif