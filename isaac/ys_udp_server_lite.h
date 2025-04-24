#ifndef __YS_UDP_SERVER_H__
#define __YS_UDP_SERVER_H__

#include "tool/ys_thread_queue.hpp"
#include "tool/ys_util.hpp"
#include "ys_epoller.h"
#include "ys_udp.h"
#include "tool/ys_cpu.hpp"
#include <map>

namespace isaac {

class YS_UdpServerLite : public YS_Epoller
{
public:
    YS_UdpServerLite(const std::string &ip, uint32_t port, YS_DriverPtr pHd)
        : _hd(pHd), _ed(ip, port)
    {
    }

    struct Node
    {
        YS_NetBuf   buf;
        YS_EndPoint peer;
    };

    ~YS_UdpServerLite()
    {
        stop();
        if (_data) delete _data;
        if (_sock) delSock(_sock);
        Node *pNode = nullptr;
        while (_queIdle.pop_front(pNode))
        {
            delete pNode;
        }
    }

    void stop() { _run = false; }
    bool isRunning() { return _run; }

    int32_t start()
    {
        if (_hd == nullptr)
        {
            CLI_ERROR("driver not set.");
            return -1;
        }
        _sock = std::make_shared<YS_Udp>();
        if (_sock->bind(_ed.getIp(), _ed.getPort()))
        {
            CLI_ERROR("udp bind addr fail [{}:{}].", _ed.getIp(), _ed.getPort());
            return -1;
        }
        _data = new YS_EpData(_sock);
        addSock(_data);
        for (size_t i = 0; i < YS_Cpu::getCpuNum(); i++)
        {
            _vtBusi.emplace_back(std::thread(&YS_UdpServerLite::busiRun, this));
        }

        _hd->init();
        while (_run)
        {
            fireOnce();
        }
        _hd->final();
        return 0;
    }

    void onEpollErr(YS_EpData *pData) { CLI_ERROR("accepter get ep error."); }

    void onEpollIn(YS_EpData *pData)
    {
        YS_EndPoint ed;
        if (YS_Udp::recvAddr(_sock, ed)) return;
        Node    *pNode = getNode(ed);
        bool     block = pNode->buf.recvData.empty();
        uint32_t len   = pNode->buf.recvData.leftSize();
        if (_sock->recvfrom(pNode->buf.recvData.uData(), &len, pNode->peer))
        {
            if (block) delRecv(ed);
            pushIdle(pNode);
            return;
        }
        pNode->buf.recvData.offset(len);

        auto ret = _hd->isFullPkt(pNode->buf.recvData, &pNode->buf.ctx);
        if (ret < 0)
        {
            pushIdle(pNode);
            if (block) delRecv(ed);
            return;
        }
        else if (ret > 0)
        {
            pushRecv(pNode, block);
        }
        else
        {
            if (block) delRecv(ed);
            pushBusi(pNode);
        }
    }

    void busiRun()
    {
        while (_run)
        {
            Node *pNode = getBusi();
            if (!pNode) continue;

            _hd->handle(pNode->buf);

            if (!pNode->buf.sendData.empty())
            {
                _sock->sendTo(pNode->buf.sendData.sData(), pNode->buf.sendData.length(), pNode->peer);
            }
        }
    }

    Node *getNode(YS_EndPoint &ep)
    {
        auto addr = ep.getIp() + ":" + std::to_string(ep.getPort());
        auto iter = _queRecv.find(addr);
        if (iter != _queRecv.end())
        {
            return iter->second;
        }
        return getIdle();
    }

    void delRecv(YS_EndPoint &ep)
    {
        auto addr = ep.getIp() + ":" + std::to_string(ep.getPort());
        _queRecv.erase(addr);
    }

    void pushRecv(Node *pNode, bool isupdata)
    {
        auto addr = pNode->peer.getIp() + ":" + std::to_string(pNode->peer.getPort());
        if (isupdata)
        {
            _queRecv[addr] = pNode;
        }
        else
        {
            _queRecv.insert({addr, pNode});
        }
    }

    Node *getIdle()
    {
        Node *pNode = nullptr;

        if (!_queIdle.pop_front(pNode, 0, false))
        {
            pNode = new Node;
        }
        else
        {
            pNode->buf.clear();
        }
        return pNode;
    }

    Node *getBusi()
    {
        Node *pNode = nullptr;
        if (!_queBusi.pop_front(pNode, 0, false))
        {
            return nullptr;
        }
        return pNode;
    }

    void pushBusi(Node *node) { _queBusi.push_back(node); }
    void pushIdle(Node *node) { _queIdle.push_back(node); }

private:
    YS_DriverPtr                  _hd;
    YS_EndPoint                   _ed;
    YS_UdpPtr                     _sock = nullptr;
    YS_EpData                    *_data = nullptr;
    bool                          _run  = true;
    std::vector<std::thread>      _vtBusi;
    YS_ThQueue<Node *>            _queBusi;
    YS_ThQueue<Node *>            _queIdle;
    std::map<std::string, Node *> _queRecv;
};

} // namespace isaac

#endif