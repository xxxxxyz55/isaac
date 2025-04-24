#pragma once
#ifndef _YS_CLIENT_H_
#define _YS_CLIENT_H_

#include "ys_cli_log.h"
#include "ys_epoller.h"
#include "ys_tcp.h"

namespace isaac {

// 一个客户端一个连接
// 主动接收 发送
class YS_TcpClient : private YS_Epoller
{
private:
    virtual int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **) = 0;
    virtual void    onEpollErr(YS_EpData *pData) { YS_EXCEPT_RUNTIME_ERROR("rewrite this"); }
    virtual void    onEpollIn(YS_EpData *pData) { YS_EXCEPT_RUNTIME_ERROR("rewrite this"); }

public:
    // pf 接收数据完整条件
    YS_TcpClient(size_t evNum = 1);

    virtual ~YS_TcpClient();

    friend class YS_TcpClientAsyn;

    void useSsl(YS_SslCtxPtr ctx) { _ctx = ctx; }

    // 连接host
    int32_t connect(const std::string &ip, int32_t port);

    // 重连
    bool reconnected();

    // 连接成功后可用，获取本地ip
    std::string getLocAddr(int32_t *port = nullptr);

    // 可发送数据
    bool canWrite();

    // 发送数据
    int32_t send(const uint8_t *in, uint32_t inLen);

    int32_t send(YS_Buf *buffer) { return send(buffer->uData(), buffer->length()); }

    // 接收一个包或者超过大小
    int32_t recvPkt(YS_Buf &buf, YS_PktCtx **arg);

    // 等待ev然后接收
    int32_t recvSyn(YS_Buf &buf, YS_PktCtx **arg, uint32_t timeoutms = 3000);

    // 同步发送接收数据 超时为3秒 out 会清空
    int32_t sendRecv(const uint8_t *in, uint32_t inLen, YS_Buf &out, YS_PktCtx **arg = nullptr);

    // 同步发送接收
    int32_t sendRecv(YS_Buf &buffer);

    // 内置一个缓存区
    YS_Buf &getBuf() { return _buf; }

    // 关闭连接
    void closeSock();
    bool isGmssl() { return _sock ? _sock->isGmssl() : false; }

private:
    YS_TcpPtr    _sock = nullptr;
    YS_Buf       _buf;
    YS_EpData   *_data = nullptr;
    YS_SslCtxPtr _ctx  = nullptr;
};

class YS_TcpClientAsyn : public YS_TcpClient
{
private:
    bool _fRun = false;

    virtual void asynHandle(YS_EpData *) = 0;

    void onEpollErr(YS_EpData *pData);

    void onEpollIn(YS_EpData *pData) { return asynHandle(pData); }

public:
    YS_TcpClientAsyn(size_t evNum = 1)
        : YS_TcpClient(evNum) {}

    virtual ~YS_TcpClientAsyn() { stop(); };

    // 循环等待处理事件
    void start();

    // 正在阻塞循环
    bool isRunning() { return _fRun; }

    // 停止处理事件
    void stop() { _fRun = false; }
    int32_t recvPkt(uint8_t *buf, uint32_t *len, YS_FullPktCb pfIsComplete, YS_PktCtx **arg) = delete;
    // 同步发送接收数据 超时为3秒 outLen 输入out缓冲区大小
    int32_t sendRecv(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t *outLen, YS_PktCtx **arg = nullptr) = delete;
    // 同步发送接收
    int32_t sendRecv(YS_Buf *buffer) = delete;

private:
    int32_t isFullPkt(YS_Buf &, isaac::YS_PktCtx **) { return -1; }; //=delete
};

// 任意包通过
class YS_DemoClient : public YS_TcpClient
{
public:
    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **) { return buf.empty() ? buf.capacity() : 0; }
};

inline YS_TcpClient::YS_TcpClient(size_t evNum)
    : YS_Epoller(evNum)
{
    _buf.alloc(YS_NetBuf::capacity());
}

inline YS_TcpClient::~YS_TcpClient()
{
    closeSock();
    _ctx = nullptr;
};

inline int32_t YS_TcpClient::connect(const std::string &ip, int32_t port)
{
    _sock = YS_Tcp::connect(ip.c_str(), port, _ctx);
    if (_sock)
    {
        _data = new YS_EpData(_sock);
        addSock(_data);
        return 0;
    }
    else
    {
        return -1;
    }
}

inline bool YS_TcpClient::reconnected()
{
    if (_sock->getFd() > 0)
    {
        delSock(_sock);
        if (_data)
        {
            delete _data;
        }
    }
    if (_sock->reconnected(_ctx))
    {
        _data = new YS_EpData(_sock);
        addSock(_data);
        return true;
    }
    return false;
}

inline std::string YS_TcpClient::getLocAddr(int32_t *port)
{
    if (_sock == nullptr)
    {
        CLI_DEBUG("connct first.");
        return "";
    }
    return _sock->getLocEndPoint().getIp();
}

inline bool YS_TcpClient::canWrite()
{
    modSock(_data, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLOUT);
    int32_t num = wait(100);
    modSock(_data, EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP);
    if (num < 0)
    {
        return false;
    }

    int32_t ev = getEvent(0);
    if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        CLI_ERROR("peer closed , reconnected ev[{:x}].", ev);
        return reconnected();
    }
    else
    {
        return true;
    }
}

inline int32_t YS_TcpClient::send(const uint8_t *in, uint32_t inLen)
{
    if (_sock == nullptr)
    {
        return -1;
    }
    if (!_sock->isConnected() && !reconnected())
    {
        CLI_ERROR("reconnect fail.");
        return -1;
    }

    if (!canWrite())
    {
        return -1;
    }

    if (_sock->send(in, inLen) != int32_t(inLen))
    {
        CLI_ERROR("send data fail.");
        return -1;
    }

    return 0;
}

inline int32_t YS_TcpClient::recvPkt(YS_Buf &buf, YS_PktCtx **arg)
{
    int32_t  ret;
    YS_Timer tmer;
    tmer.start(2000);

    uint32_t recvLen;
    uint32_t recvTotal   = 0;
    buf.clear();
    int32_t needRecvLen = isFullPkt(buf, arg);

    while (!tmer.isTimeOut())
    {
        recvLen = needRecvLen;
        ret     = _sock->recv(buf.uData() + recvTotal, &recvLen);
        if (ret)
        {
            CLI_ERROR("recv err , need recv len = {}.", needRecvLen);
            return -1;
        }
        else
        {
            if (recvLen > 0)
            {
                recvTotal += recvLen;
                buf.offset(recvLen);
                needRecvLen = isFullPkt(buf, arg);
                if (needRecvLen == 0)
                {
                    break; // 收到完整包
                }
                else if (needRecvLen < 0)
                {
                    CLI_ERROR("recv err pkt.");
                    return -2;
                }
                else
                {
                    if (needRecvLen + recvTotal > buf.capacity())
                    {
                        CLI_ERROR("recv buffer not enough.");
                        return -1;
                    }
                }
            }
        }
    }
    return 0;
}

inline int32_t YS_TcpClient::recvSyn(YS_Buf &buf, YS_PktCtx **arg, uint32_t timeoutms)
{
    int32_t num = wait(timeoutms);
    if (num <= 0)
    {
        CLI_ERROR("recv timeout sock[{}] errno {}.", int32_t(_sock->getFd()), YS_ERRNO);
        return -1;
    }
    else
    {
        int32_t ev = getEvent(0);
        if (ev & EPOLLIN)
        {
            int32_t iRet = recvPkt(buf, arg);
            if (iRet)
            {
                CLI_ERROR("recv data fail ret = {}.", iRet);
                return iRet;
            }
        }
        else // if (ev & EPOLLERR)
        {
            closeSock();
            CLI_ERROR("get epollerr [{0:x}].", ev);
            return -1;
        }

        return 0;
    }
}

inline int32_t YS_TcpClient::sendRecv(const uint8_t *in, uint32_t inLen, YS_Buf &out, YS_PktCtx **arg)
{
    if (send(in, inLen))
    {
        CLI_ERROR("send data fail.");
        return -1;
    }

    return recvSyn(out, arg);
}

inline int32_t YS_TcpClient::sendRecv(YS_Buf &buffer)
{
    return sendRecv(buffer.uData(), buffer.length(), buffer);
}

inline void YS_TcpClient::closeSock()
{
    if (_sock)
    {
        delSock(_sock);
        delete _data;
        _data = nullptr;
        _sock = nullptr;
    }
}

inline void YS_TcpClientAsyn::onEpollErr(YS_EpData *pData)
{
    delSock(pData->sock);
    stop();
    _sock->getFd().reset(-1);
    if (_data)
    {
        delete _data;
        _data = nullptr;
    }
}

inline void YS_TcpClientAsyn::start()
{
    while (_fRun)
    {
        fireOnce();
    }
}
} // namespace isaac

#endif