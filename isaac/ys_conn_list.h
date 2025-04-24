#ifndef _YS_CONN_LIST_H_
#define _YS_CONN_LIST_H_

#include <map>
#include "ys_epoller.h"

namespace isaac {

// 连接表
// 包括fd和对应的epolldata
class YS_ConnList
{
public:
    // 析构 删除所有data
    ~YS_ConnList();

    // 获取所有socket
    std::vector<YS_SocketPtr> getAllSock();

    // 获取接收超时的buf
    std::vector<YS_NetBuf *>  getAllTimeOutBuf();
    std::vector<YS_SocketPtr> getAllClosedSock();
    std::vector<YS_SocketPtr> getAllTimeOutSock();

    // 添加一个sock
    YS_EpData *add(YS_SocketPtr pSock, void *ext);

    // 调用前先从epoll删除
    // 删除一个连接
    void del(YS_SocketPtr pSock);

    // 调用前先从epoll删除
    // 删除所有连接
    void clear();

    // 获取epoll data
    YS_EpData *getData(YS_SocketPtr pSock);

    // 总连接数
    size_t size();

    void setConnTmout(size_t ms) { _connTmout = ms; }

private:
    // sock ==> epoll data
    std::map<int32_t, YS_EpData *> _mp;
    std::mutex                     _lock;
    size_t                         _connTmout = 0;
};

inline YS_ConnList::~YS_ConnList()
{
    for (auto &&iter : _mp)
    {
        delete iter.second;
    }
    _mp.clear();
}

inline std::vector<YS_SocketPtr> YS_ConnList::getAllSock()
{
    std::lock_guard<std::mutex> lck(_lock);
    std::vector<YS_SocketPtr>   vt;
    for (auto &&i : _mp)
    {
        vt.emplace_back(i.second->sock);
    }
    return vt;
}

inline std::vector<YS_NetBuf *> YS_ConnList::getAllTimeOutBuf()
{
    std::lock_guard<std::mutex> lck(_lock);
    std::vector<YS_NetBuf *>    vt;
    for (auto &&iter : _mp)
    {
        if (iter.second->buf && iter.second->tmer.isTimeOut())
        {
            CLI_ERROR("recv timeout, {}", iter.second->sock->getPeerSock().getIp());
            vt.emplace_back(iter.second->buf);
            iter.second->buf->recvData.clear();
            iter.second->buf = nullptr;
            iter.second->tmer.stop();
        }
    }
    return vt;
}

inline std::vector<YS_SocketPtr> YS_ConnList::getAllClosedSock()
{
    std::vector<YS_SocketPtr>   vt;
    std::lock_guard<std::mutex> lck(_lock);
    for (auto &&iter : _mp)
    {
        if (iter.second->sock->isCloseWait())
        {
            CLI_DEBUG("sock closed fd = {}", iter.first);
            vt.emplace_back(iter.second->sock);
            delete iter.second;
        }
    }

    for (auto &&iter : vt)
    {
        _mp.erase(iter->getFd());
    }

    return vt;
}

inline std::vector<YS_SocketPtr> YS_ConnList::getAllTimeOutSock()
{
    if (_connTmout == 0) return {};
    std::vector<YS_SocketPtr> vt;
    std::lock_guard<std::mutex> lck(_lock);
    for (auto &&iter : _mp)
    {
        if (iter.second->tmer.count() > _connTmout)
        {
            vt.emplace_back(iter.second->sock);
            delete iter.second;
        }
    }

    for (auto &&iter : vt)
    {
        _mp.erase(iter->getFd());
    }
    return vt;
}

inline YS_EpData *YS_ConnList::add(YS_SocketPtr pSock, void *ext)
{
    std::lock_guard<std::mutex> lck(_lock);

    auto iter = _mp.find(pSock->getFd());
    if (iter != _mp.end())
    {
        CLI_ERROR("add connection fail, duplicate fd.");
        return nullptr;
    }
    YS_EpData *pData = new YS_EpData(pSock, ext);
    _mp.insert({pSock->getFd(), pData});
    return pData;
}

inline void YS_ConnList::del(YS_SocketPtr pSock)
{
    std::lock_guard<std::mutex> lck(_lock);

    auto iter = _mp.find(pSock->getFd());
    if (iter != _mp.end())
    {
        _mp.erase(iter->first);
        delete iter->second;
    }
}

inline void YS_ConnList::clear()
{
    std::lock_guard<std::mutex> lck(_lock);
    for (auto &&iter : _mp)
    {
        delete iter.second;
    }
    _mp.clear();
}

inline YS_EpData *YS_ConnList::getData(YS_SocketPtr pSock)
{
    std::lock_guard<std::mutex> lck(_lock);

    auto iter = _mp.find(pSock->getFd());
    if (iter != _mp.end())
    {
        return iter->second;
    }
    else
    {
        return nullptr;
    }
}

inline size_t YS_ConnList::size()
{
    std::lock_guard<std::mutex> lck(_lock);
    return _mp.size();
}

} // namespace isaac

#endif