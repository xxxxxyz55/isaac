#ifndef _YS_THREAD_ASYN_H_
#define _YS_THREAD_ASYN_H_

#include <thread>
#include <vector>
#include "ys_epoller.h"
#include "ys_handle.h"
#include "tool/ys_cpu.hpp"
#include "tool/ys_thread_queue.hpp"
#include "ys_conn_list.h"
#include "ys_thread.h"

namespace isaac {

//
class YS_NetBufQueue
{
    YS_ThQueue<YS_NetBuf *> _pQuBusi; // 已用的
    YS_ThQueue<YS_NetBuf *> _pQuIdle; // 空闲的

public:
    // 可根据客户端并发数量调整
    // 空闲队列扩容至
    void resize(uint32_t size)
    {
        if (size > _pQuIdle.size())
        {
            auto ext = size - _pQuIdle.size();
            for (size_t i = 0; i < ext; i++)
            {
                _pQuIdle.push_back(new YS_NetBuf);
            }
        }
    }

    ~YS_NetBufQueue()
    {
        YS_NetBuf *pBuf = nullptr;
        while (_pQuIdle.pop_front(pBuf, 0, false))
        {
            delete pBuf;
        }
    }
    // 空闲数量
    uint32_t size() { return _pQuIdle.size(); }

    YS_NetBuf *popIdle()
    {
        YS_NetBuf *pBuf = nullptr;
        _pQuIdle.pop_front(pBuf, 200, true);
        return pBuf;
    }

    void pushIdle(YS_NetBuf *pBuf) { _pQuIdle.push_back(pBuf); }

    YS_NetBuf *popTask()
    {
        YS_NetBuf *pBuf = nullptr;
        _pQuBusi.pop_front(pBuf, 200);
        return pBuf;
    }

    void pushTask(YS_NetBuf *pBuf) { _pQuBusi.push_back(pBuf, true); }
};

class YS_ProcThread : public YS_Thread
{
private:
    YS_DriverPtr    _pHd;
    YS_NetBufQueue *_pQueue;
    uint32_t        _tid;

    int32_t init()
    {
        _pHd->init();
        return 0;
    }

    void handle()
    {
        YS_NetBuf *pBuf = _pQueue->popTask();
        if (!pBuf) return;

        pBuf->tid = _tid;
        _pHd->handle(*pBuf);

        if (size_t(pBuf->refSock->send(pBuf->sendData)) < pBuf->sendData.length())
        {
            CLI_ERROR("send data fail busiId [{}]", pBuf->tid);
        }

        if (pBuf->cbBusiFinal)
        {
            pBuf->cbBusiFinal(pBuf);
        }
        pBuf->clear();
        _pQueue->pushIdle(pBuf);
    }

    void final() { _pHd->final(); }

public:
    YS_ProcThread(YS_DriverPtr pHd, YS_NetBufQueue *pQueue)
        : _pHd(pHd), _pQueue(pQueue)
    {
    }

    // 业务线程启动
    void start(uint32_t tidoff, uint32_t cpu)
    {
        if (YS_Thread::start() == 0)
        {
            bindCpu(cpu);
            name("busit_" + std::to_string(cpu));
        }
    }
    void stop()
    {
        YS_Thread::stop();
    }
};

// 业务线程池
class YS_ProcGruop : public YS_NetBufQueue
{
private:
    uint32_t _thPerCpu = 2;

    std::vector<YS_ProcThread *> _vtProc;

public:
    YS_ProcGruop() {}
    ~YS_ProcGruop()
    {
        for (auto &&i : _vtProc)
        {
            if (i) delete i;
        }

        _vtProc.clear();
    }

    void setThreadNum(uint32_t num) { _thPerCpu = num; }
    void setBufQueueSize(uint32_t sz) { resize(sz); }

    // 启动业务线程池
    void start(YS_DriverPtr pHd, uint32_t off)
    {
        if (_vtProc.size())
        {
            return;
        }

        for (size_t i = 0; i < _thPerCpu; i++)
        {
            YS_ProcThread *pTh = new YS_ProcThread(pHd, this);
            _vtProc.emplace_back(pTh);
            pTh->start(i + (off * _thPerCpu), off);
        }
    }

    // 停止业务线程池
    void stop()
    {
        for (auto &&i : _vtProc)
        {
            i->stop();
        }
    }
};

class YS_ConnCb
{
public:
    virtual void onConn(YS_SocketPtr &)    = 0;
    virtual void onDisConn(YS_SocketPtr &) = 0;
};

// 网络线程
class YS_NetTh : public YS_Epoller, public YS_ConnCb
{
private:
    std::thread *_pThNet = nullptr;
    YS_DriverPtr _pHd;
    bool         _run = true;
    YS_ProcGruop _busi;
    YS_ConnList  _conns;
    YS_ConnCb   *_connCb;

    // 重新注册到epoll
    void regSock(YS_NetBuf *buf)
    {
        if (buf->refSock->isConnected())
        {
            addSock(_conns.getData(buf->refSock));
        }
    }

public:
    void onConn(YS_SocketPtr &p)
    {
        if (_connCb) _connCb->onConn(p);
    }

    void onDisConn(YS_SocketPtr &p)
    {
        if (_connCb) _connCb->onDisConn(p);
    }
    void setConnTmout(size_t ms) { _conns.setConnTmout(ms); }

    void onEpollIn(YS_EpData *pData)
    {
        YS_NetBuf *pBuf;

        if (pData->buf)
        {
            pBuf = pData->buf;
        }
        else
        {
            pData->tmer.start(3000);
            pBuf = _busi.popIdle();
            if (!pBuf)
            {
                CLI_ERROR("exception: no idle buffer.");
                return;
            }
        }

        int32_t err = pData->sock->recv(pBuf->recvData);
        if (err)
        {
            CLI_DEBUG("recv fail, has recv {}", pBuf->recvData.length());
            pData->buf = nullptr;
            pBuf->recvData.clear();
            _busi.pushIdle(pBuf);
            return;
        }

        if (!pBuf->recvData.length())
        {
            pData->buf = nullptr;
            _busi.pushIdle(pBuf);
            return;
        }

        err = _pHd->isFullPkt(pBuf->recvData, &pBuf->ctx);
        if (err < 0) // 错误包丢弃
        {
            CLI_DEBUG("recv invalid pkt.");
            pData->buf = nullptr;
            pBuf->recvData.clear();
            _busi.pushIdle(pBuf);
            return;
        }
        else if (pBuf->recvData.full())
        {
            if (_pHd->supportBlock)
            {
                delSock(pData->sock->getFd()); // 先移除
                pBuf->cbBusiFinal = std::bind(&YS_NetTh::regSock, this, std::placeholders::_1);
                pData->buf        = nullptr;
                pBuf->refSock     = pData->sock;
                _busi.pushTask(pBuf);
            }
            else
            {
                CLI_DEBUG("unsupport big pkg.");
                pData->buf = nullptr;
                pBuf->recvData.clear();
                _busi.pushIdle(pBuf);
            }
        }
        else if (err > 0) // 等待内核再次通知,继续接收
        {
            CLI_DEBUG("contine to recv.");
            pData->buf = pBuf;
            return;
        }
        else // 接收完整
        {
            pData->buf    = nullptr;
            pBuf->refSock = pData->sock;
            _busi.pushTask(pBuf);
            return;
        }
    }

    void epollRun() noexcept
    {
        while (_run)
        {
            if (fireOnce() == 0)
            {
                clearTimeOut();
                clearClosed();
            }
        }
    }

    void onEpollErr(YS_EpData *pData)
    {
        CLI_DEBUG("socket get epoll err,fd = {}", pData->sock->getFd())
        if (pData->buf)
        {
            pData->buf->clear();
            _busi.pushIdle(pData->buf);
            pData->buf = nullptr;
        }
        if (pData->sock->isConnected())
        {
            delSock(pData->sock);
            _conns.del(pData->sock);
        }
        onDisConn(pData->sock);
        return;
    }

    //
    void clearTimeOut()
    {
        auto vt = _conns.getAllTimeOutBuf();
        for (auto &&iter : vt)
        {
            _busi.pushIdle(iter);
        }
    }

    void clearClosed()
    {
        {
            auto vt = _conns.getAllClosedSock();
            for (auto &&iter : vt)
            {
                delSock(iter);
            }
        }
        {
            auto vt = _conns.getAllTimeOutSock();
            for (auto &&iter : vt)
            {
                delSock(iter);
                iter->close();
            }
        }
    }

public:
    YS_NetTh(YS_DriverPtr pHd, YS_ConnCb *pCb) : _pHd(pHd), _connCb(pCb) {}
    ~YS_NetTh()
    {
        stop();
        if (_pThNet)
        {
            if (_pThNet->joinable())
            {
                _pThNet->join();
                delete _pThNet;
                _pThNet = nullptr;
            }
        }

        auto vt = _conns.getAllSock();
        for (auto &&iter : vt)
        {
            delSock(iter);
            onDisConn(iter);
        }
    }
    void stop()
    {
        _run = false;
        _busi.stop();
    }
    void setThreadNum(uint32_t num) { _busi.setThreadNum(num); }
    void setBufQueueSize(uint32_t sz) { _busi.setBufQueueSize(sz); }

    // 网络线程启动
    void start(uint32_t off)
    {
        _busi.start(_pHd, off);

        _pThNet = new std::thread(&isaac::YS_NetTh::epollRun, this);
        YS_Cpu::threadBindCpu(_pThNet, off);
        // CLI_DEBUG("net thread bind cpu {}", off % YS_Cpu::getCpuNum());
        YS_Cpu::threadSetName(_pThNet, "net_" + std::to_string(off));
    }

    uint32_t getSockNum() { return _conns.size(); }

    void addConn(YS_SocketPtr pSock)
    {
        if (_run)
        {
            YS_EpData *pData = _conns.add(pSock, nullptr);
            addSock(pData);
            onConn(pSock);
        }
    }
};

class YS_BusiPool : public YS_ConnCb
{
private:
    std::vector<YS_NetTh *> _vtPool;
    YS_DriverPtr            _pHd;
    std::vector<uint32_t>   _cpus;
    uint32_t                _thPerCpu  = 2;
    uint32_t                _queueSize = 32;

public:
    YS_BusiPool() {}
    ~YS_BusiPool() { stop(); }

    void addConn(YS_SocketPtr pSock)
    {
        uint32_t totalConn = connNum();

        for (auto &&i : _vtPool)
        {
            if (i->getSockNum() <= totalConn / _vtPool.size())
            {
                i->addConn(pSock);
                break;
            }
        }
    }
    void disableEt()
    {
        for (auto &&iter : _vtPool)
        {
            iter->disableEt();
        }
    }

    void setConnTmout(size_t ms)
    {
        for (auto &&iter : _vtPool)
        {
            iter->setConnTmout(ms);
        }
    }

    uint32_t connNum()
    {
        uint32_t totalConn = 0;
        for (auto &&i : _vtPool)
        {
            totalConn += i->getSockNum();
        }
        return totalConn;
    }
    // 网络线程池启动
    void start()
    {
        if (_cpus.size() == 0)
        {
            uint32_t cpuNum = YS_Cpu::getCpuNum();
            for (size_t i = 0; i < cpuNum; i++)
            {
                _cpus.push_back(i);
            }
        }

        CLI_DEBUG("thread num [{}]", _cpus.size() * _thPerCpu);
        for (size_t i = 0; i < _cpus.size(); i++)
        {
            YS_NetTh *pTh = new YS_NetTh(_pHd, this);
            pTh->setThreadNum(_thPerCpu);
            pTh->setBufQueueSize(_queueSize);
            pTh->start(i);
            _vtPool.emplace_back(pTh);
        }
    }

    void stop()
    {
        for (auto &&i : _vtPool)
        {
            i->stop();
        }
        for (auto &&i : _vtPool)
        {
            delete i;
        }
        _vtPool.clear();
    }

    // 在哪些cpu上运行， 每个cpu多少业务线程,缓存区总数量=cpus.size()*queueSize=并发支持
    void setThreadPolicy(std::vector<uint32_t> cpus, uint32_t thPerCpu, uint32_t queueSize = 32)
    {
        _cpus.clear();
        for (auto &&i : cpus)
        {
            _cpus.push_back(i);
        }
        _thPerCpu  = thPerCpu;
        _queueSize = queueSize;
    }

    void         setDriver(YS_DriverPtr pHd) { _pHd = pHd; }
    YS_DriverPtr getDriver() { return _pHd; }
};
} // namespace isaac

#endif