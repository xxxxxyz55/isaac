#pragma once
#ifndef _YS_PACK_PROTO_H_
#define _YS_PACK_PRORO_H_

#include "tool/ys_pack.hpp"
#include "tool/ys_template.hpp"
#include "ys_handle.h"
#include "ys_tcp_client.h"
#include "ys_tcp_server.h"
#include "ys_engine.h"
#include <memory>

namespace isaac {

using PackApi    = int32_t (*)(YS_NetBuf &buf);
using PackApiArr = std::vector<std::pair<uint16_t, PackApi>>;

class YS_PackEngine : public YS_Engine<uint16_t, PackApi>
{
public:
    using YS_Engine<uint16_t, PackApi>::YS_Engine;

    // 打包错误
    void            sendErr(int32_t err, YS_NetBuf &buf);
    virtual void    encode(YS_Buf &pkt) {}
    virtual int32_t decode(YS_Buf &pkt) { return 0; }

    // 处理请求包
    void done(YS_NetBuf &buf);
};

// server所需的pack协议的驱动
class YS_PackDriver : public YS_Driver, private YS_PackEngine
{
    // YS_PackEngine engine;

public:
    using YS_PackEngine::route;

    void handle(YS_NetBuf &buf) { done(buf); }

    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **) { return YS_Packet::isFullPkt(buf); }
};

class YS_PackApi
{
public:
    template <typename ReqType, typename RespType>
    static int32_t pack_proc(YS_NetBuf &buf, int32_t pf(ReqType *, RespType *));

    template <typename ReqType, typename RespType>
    static int32_t pack_proc(YS_NetBuf &buf, std::function<int32_t(ReqType *, RespType *)> &pf);

    template <typename ReqType>
    static int32_t pack_proc_only_req(YS_NetBuf &buf, int32_t pf(ReqType *, uint8_t *, uint32_t *));

    template <typename ReqType>
    static int32_t pack_proc_only_req(YS_NetBuf &buf, std::function<int32_t(ReqType *, uint8_t *, uint32_t *)> &pf);

    template <typename ObjType>
    static void unpack_pkt(YS_Buf &in, ObjType &obj, int32_t &err, uint16_t &cmd);

    template <typename ObjType>
    static void pack_pkt(ObjType &obj, uint16_t cmd, YS_Buf &out);

    // 成员函数用这个转换下
    template <typename T, typename K, class Obj>
    static std::function<int32_t(T *, K *)> bind(int32_t (Obj::*pf)(T *, K *), Obj *obj)
    {
        return std::bind(pf, obj, std::placeholders::_1, std::placeholders::_2);
    }

    template <typename T, class Obj>
    static std::function<int32_t(T *, uint8_t *, uint32_t *)> bind(int32_t (Obj::*pf)(T *, uint8_t *, uint32_t *), Obj *obj)
    {
        return std::bind(pf, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    // 绑定类和方法，返回函数指针，注意类的生命周期结束，返回的函数指针也不应该使用，否则会崩溃
    template <typename FUNC, typename OBJ,
              typename Req = typename YS_ArgType<FUNC>::type1,
              typename Rsp = typename YS_ArgType<FUNC>::type2>
    static int32_t (*binder(FUNC func, OBJ *obj))(Req, Rsp, YS_NetBuf &)
    {
        static auto _func = func;
        static auto _obj  = obj;
        return [](Req req, Rsp rsp, YS_NetBuf &buf) {
            return (_obj->*_func)(req, rsp, buf);
        };
    }

    // 解包请求 pf=处理 打包响应
    // 请求响应都自动处理
#define YS_PackApiCustom(func)  [](YS_NetBuf &buf) { return YS_PackApi::pack_proc(buf, func); }
    // 解包请求 pf=处理+打包响应
    // 响应手动处理
#define YS_PackApiOnlyReq(func) [](YS_NetBuf &buf) { return YS_PackApi::pack_proc_only_req(buf, func); }
};

// pack协议驱动的tcp服务
class YS_TcpPackServer : public YS_TcpServer
{
    std::shared_ptr<YS_PackDriver> handle = std::make_shared<YS_PackDriver>();

public:
    YS_TcpPackServer(std::string ip, uint32_t port) : YS_TcpServer(ip, port)
    {
        setDriver(handle);
    }

    void route(PackApiArr router)
    {
        handle->route(router);
    }
};

// pack协议驱动的tcp客户端
class YS_TcpPackClient : public YS_TcpClient
{
    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **)
    {
        return YS_Packet::isFullPkt(buf);
    }
    virtual void encode(YS_Buf &buf) {}

    virtual int32_t decode(YS_Buf &buf) { return 0; }

public:
    template <typename ReqType, typename RespType>
    int32_t doRequest(ReqType &req, RespType &resp, uint16_t cmd);
    template <typename ReqType>
    int32_t sendReq(ReqType &req, uint16_t cmd);
    template <typename RespType>
    int32_t recvResp(RespType &req, uint32_t timeoutms = 3000);
};

inline void YS_PackEngine::sendErr(int32_t err, YS_NetBuf &pBuf)
{
    pBuf.sendData.clear();
    YS_Packet *pPkt = (YS_Packet *)pBuf.sendData.uData();
    pPkt->header    = YS_PACK_HEADER;
    pPkt->dataLen   = 0;
    pPkt->cmd       = ((YS_Packet *)pBuf.recvData.uData())->cmd;
    pPkt->err       = err;
}

inline void YS_PackEngine::done(YS_NetBuf &buf)
{
    YS_Packet *pPkt = YS_Packet::toPkt(buf.recvData);

    PackApi pf = get(pPkt->cmd);
    if (pf)
    {
        if (decode(buf.recvData))
        {
            return sendErr(400, buf);
        }
        int32_t err = pf(buf);
        if (err)
        {
            return sendErr(err, buf);
        }
        encode(buf.sendData);
    }
    else
    {
        CLI_ERROR("cmd = {} pf = {:p}", pPkt->cmd, (void *)pf);
        return sendErr(404, buf);
    }
}

template <typename ReqType, typename RespType>
int32_t YS_PackApi::pack_proc(YS_NetBuf &buf, int32_t pf(ReqType *, RespType *))
{
    ReqType  tIn;
    RespType tOut;
    int32_t  err = 0;
    uint16_t cmd = 0;

    unpack_pkt(buf.recvData, tIn, err, cmd);
    if (err) return 100;

    err = pf(&tIn, &tOut);
    if (err) return err;

    pack_pkt(tOut, cmd, buf.sendData);
    return 0;
}

template <typename ReqType, typename RespType>
int32_t YS_PackApi::pack_proc(YS_NetBuf &buf, std::function<int32_t(ReqType *, RespType *)> &pf)
{
    ReqType  tIn  = {};
    RespType tOut = {};
    int32_t  err;
    uint16_t cmd;
    unpack_pkt(buf.recvData, tIn, err, cmd);
    if (err) return 100;

    err = pf(&tIn, &tOut);
    if (err) return err;

    pack_pkt(tOut, cmd, buf.sendData);
    return 0;
}

template <typename ObjType>
void YS_PackApi::unpack_pkt(YS_Buf &in, ObjType &obj, int32_t &err, uint16_t &cmd)
{
    YS_Packet *pPkt = YS_Packet::toPkt(in);

    cmd = pPkt->cmd;
    err = pPkt->err;
    if (err) return;

    err = obj._ys_unpack(pPkt->data, pPkt->dataLen);
    if (err)
    {
        CLI_ERROR("pack decode fail [{}]length[{}][{}].",
                  err, in.length(), YS_String::toHex(in.uData(), in.length()));
    }
    return;
}

template <typename ObjType>
void YS_PackApi::pack_pkt(ObjType &obj, uint16_t cmd, YS_Buf &out)
{
    YS_Packet *pPkt = (YS_Packet *)out.left();
    pPkt->header    = YS_PACK_HEADER;
    pPkt->cmd       = cmd;
    pPkt->err       = 0;
    pPkt->dataLen   = 0;
    obj._ys_pack(pPkt->data, &pPkt->dataLen);
    out.offset(pPkt->totalLen());
}

template <typename ReqType>
int32_t YS_PackApi::pack_proc_only_req(YS_NetBuf &buf, int32_t pf(ReqType *, uint8_t *, uint32_t *))
{
    ReqType  tIn = {};
    int32_t  ret = 0;
    uint16_t cmd = 0;

    unpack_pkt(buf.recvData, tIn, ret, cmd);
    if (ret) return 100;

    YS_Packet *pktOut = YS_Packet::toPkt(buf.sendData);
    pktOut->header    = YS_PACK_HEADER;
    pktOut->cmd       = YS_Packet::toPkt(buf.recvData)->cmd;
    pktOut->err       = 0;
    pktOut->dataLen   = 0;

    ret = pf(&tIn, pktOut->data, &pktOut->dataLen);
    if (ret) return ret;

    buf.sendData.offset(pktOut->totalLen());
    return 0;
}

template <typename ReqType>
int32_t pack_proc_only_req(YS_NetBuf &buf, std::function<int32_t(ReqType *, uint8_t *, uint32_t *)> &pf)
{
    ReqType  tIn = {};
    int32_t  ret = 0;
    uint16_t cmd = 0;

    unpack_pkt(buf.recvData, tIn, ret, cmd);
    if (ret) return 100;

    YS_Packet *pktOut = YS_Packet::toPkt(buf.sendData);
    pktOut->header    = YS_PACK_HEADER;
    pktOut->cmd       = YS_Packet::toPkt(buf.recvData)->cmd;
    pktOut->err       = 0;
    pktOut->dataLen   = 0;

    ret = pf(&tIn, pktOut->data, &pktOut->dataLen);
    if (ret) return ret;

    buf.sendData.offset(pktOut->totalLen());
    return 0;
}

template <typename ReqType>
int32_t YS_TcpPackClient::sendReq(ReqType &req, uint16_t cmd)
{
    auto &buf = getBuf();
    buf.clear();
    YS_PackApi::pack_pkt(req, cmd, buf);
    encode(buf);
    if (send(buf.uData(), buf.length()))
    {
        CLI_ERROR("send data fail.");
        return -1;
    }
    return 0;
}

template <typename RespType>
int32_t YS_TcpPackClient::recvResp(RespType &resp, uint32_t timeoutms)
{
    int32_t err = 0;
    auto   &buf = getBuf();
    buf.clear();
    err = recvSyn(buf, nullptr, timeoutms);
    if (err)
    {
        CLI_ERROR("send recv fail.");
        return err;
    }

    YS_Packet *pPkt = (YS_Packet *)buf.uData();
    if (buf.length() != pPkt->totalLen())
    {
        CLI_ERROR("err pack pkt len recv [{}] need [{}].", buf.length(), pPkt->totalLen());
        return -1;
    }

    if (pPkt->err)
    {
        CLI_DEBUG("recv err pkt {}", YS_String::toHex(buf.uData(), buf.length()));
        return pPkt->err;
    }

    if (decode(buf))
    {
        CLI_ERROR("recv resp decode fail.");
        return 400;
    }

    err = resp._ys_unpack(pPkt->data, pPkt->dataLen);
    if (err)
    {
        CLI_DEBUG("pack decode err{}", YS_String::toHex(buf.uData(), buf.length()));
    }
    return err;
}

template <typename ReqType, typename RespType>
int32_t YS_TcpPackClient::doRequest(ReqType &req, RespType &resp, uint16_t cmd)
{
    int32_t ret = sendReq(req, cmd);
    if (ret) return ret;
    return recvResp(resp);
}

} // namespace isaac

#endif