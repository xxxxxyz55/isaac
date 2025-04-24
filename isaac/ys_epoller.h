#ifndef _YS_EPOLLER_EX_H_
#define _YS_EPOLLER_EX_H_

#include "ys_socket.h"
#include "ys_wepoll.h"
#include "ys_handle.h"

namespace isaac {

// epoll 数据,从套接字构造
class YS_EpData
{
public:
    YS_SocketPtr sock; // 对应的套接字
    YS_NetBuf   *buf;  // 接收发送缓冲区
    YS_Timer     tmer; // 计时
    void        *ext;  // 协议需要的用这个

    YS_EpData(YS_SocketPtr pSock, void *ext = nullptr)
        : sock(pSock), buf(nullptr), ext(nullptr)
    {
    }
    // 析构，删除缓冲区，sock指向nullptr
    ~YS_EpData();
};

// epoll封装, 继承该类重写三个虚函数
class YS_Epoller
{
private:
    virtual void onEpollIn(YS_EpData *pData) = 0;
    virtual void onEpollOut(YS_EpData *pData) {}
    virtual void onEpollErr(YS_EpData *pData) = 0;

public:
    // 最大连接数
    static constexpr size_t maxConnNum() { return 10240; }
    static constexpr size_t maxEventNum() { return 32; }

    // 添加一个监听sockert
    void addSock(YS_EpData *data, uint32_t event);

    // 添加一个socket，根据处理函数监听事件
    void addSock(YS_EpData *data, bool evOut = false);

    // 修改
    void modSock(YS_EpData *data, uint32_t event);

    // 删除一个监听socket
    void delSock(YS_SocketPtr sock);
    void delSock(int32_t fd);

    YS_Epoller(size_t evSize = maxEventNum());

    virtual ~YS_Epoller();

    // 等待一段时间，返回数量
    int32_t wait(int32_t ms) noexcept;

    // 获取数据
    YS_EpData *getEventData(int32_t i) { return (YS_EpData *)_pevs[i].data.ptr; }
    // 获取事件类型
    int32_t getEvent(int32_t i) { return _pevs[i].events; }

    // wait一次 ,处理事件， 返回事件数量
    int32_t fireOnce();

    // 使用回调处理事件
    void dealEvent(YS_EpData *pData, int32_t ev);
    void disableEt() { _et = false; }

private:
#if __linux__
    int32_t _epfd;
#else
    HANDLE _epfd;
#endif
    bool         _et;
    epoll_event *_pevs;
    size_t       _evSize;
};

inline YS_EpData::~YS_EpData()
{
    if (buf)
    {
        delete buf;
        buf = nullptr;
    }
    sock = nullptr;
};

inline void YS_Epoller::addSock(YS_EpData *data, uint32_t event)
{
    epoll_event ev;
    ev.data.ptr = data;
    ev.events   = event;

#if __linux__
    if (_et) ev.events |= EPOLLET;
#endif

    if (epoll_ctl(_epfd, EPOLL_CTL_ADD, data->sock->getFd(), &ev))
    {
        auto uEv = ev.events;
        CLI_ERROR("epoll add fail ev[{0:x}].", uEv);
    }
}

inline void YS_Epoller::addSock(YS_EpData *data, bool evOut)
{
    uint32_t events = EPOLLHUP | EPOLLERR | EPOLLRDHUP | EPOLLIN;
    if (evOut) events |= EPOLLOUT;
    // CLI_DEBUG("epoll {} addSock {} addr {}:{}", _epfd, data->sock->getFd(),
    //           data->sock->getLocEndPoint().getIp(),
    //           data->sock->getLocEndPoint().getPort());
    addSock(data, events);
}

inline void YS_Epoller::modSock(YS_EpData *data, uint32_t event)
{
    epoll_event ev;
    ev.data.ptr = data;
    ev.events   = event;

#if __linux__
    if (_et) ev.events |= EPOLLET;
#endif

    if (epoll_ctl(_epfd, EPOLL_CTL_MOD, data->sock->getFd(), &ev))
    {
        auto uEv = ev.events;
        CLI_ERROR("epoll mod fail ev[{0:x}].", uEv);
    }
}

inline void YS_Epoller::delSock(YS_SocketPtr sock)
{
    epoll_ctl(_epfd, EPOLL_CTL_DEL, sock->getFd(), 0);
}

inline void YS_Epoller::delSock(int32_t fd)
{
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, 0);
}

inline YS_Epoller::YS_Epoller(size_t evSize)
{
    _epfd   = epoll_create(maxConnNum());
    _pevs   = new epoll_event[evSize];
    _evSize = evSize;
    _et     = true;
}

inline YS_Epoller::~YS_Epoller()
{
#if _WIN32
    epoll_close(_epfd);
#else
    close(_epfd);
#endif
    delete[] _pevs;
}

inline int32_t YS_Epoller::wait(int32_t ms) noexcept
{
    int32_t ret = epoll_wait(_epfd, _pevs, _evSize, ms);
#if __linux__
    if (ret < 0 && YS_ERRNO == EINTR)
    {
        return wait(ms);
    }
#endif
    return ret;
}

inline int32_t YS_Epoller::fireOnce()
{
    int32_t num = wait(200);
    for (int32_t i = 0; i < num; i++)
    {
        dealEvent(getEventData(i), getEvent(i));
    }
    return num;
}

inline void YS_Epoller::dealEvent(YS_EpData *pData, int32_t ev)
{
    if (!pData) return;

    if (ev & EPOLLERR || ev & EPOLLHUP || ev & EPOLLRDHUP)
    {
        onEpollErr(pData);
    }
    else
    {
        if (ev & EPOLLOUT) onEpollOut(pData);

        if (ev & EPOLLIN)
        {
            onEpollIn(pData);
        }
    }
}

} // namespace isaac

#endif