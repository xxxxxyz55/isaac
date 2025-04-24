#ifndef _YS_HTTP_SERVER_H_
#define _YS_HTTP_SERVER_H_

#include "ys_handle.h"
#include "ys_engine.h"
#include "ys_http_pkt.h"
#include "ys_tcp.h"
#include "ys_tcp_server.h"
#include "ys_xpack.h"

namespace isaac {

using HttpApi    = void (*)(YS_HttpRequest &tReq, YS_HttpResponse &tResp);
using HttpApiArr = std::vector<std::pair<std::string, HttpApi>>;

class YS_HttpDriver : public YS_Driver
{
public:
    YS_Engine<std::string, HttpApi> _engine;

    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **arg)
    {
        return YS_HttpParser::httpParseReq(buf, arg);
    }

    void handle(YS_NetBuf &buf)
    {
        YS_HttpRequest *pReq = static_cast<YS_HttpRequest *>(buf.ctx);
        YS_HttpResponse tResp(buf.refSock);
        tResp.setConnection(pReq->getConnection());

        std::string    sUrl = pReq->getUrl();
        HttpApi        pf   = _engine.get(sUrl);
        static HttpApi def  = _engine.get("*");
        if (pf == nullptr)
        {
            if (def == nullptr)
            {
                CLI_ERROR("url [{}] 404 not found.", sUrl);
                tResp.setResponse(HTTP_STATUS_NOT_FOUND, "");
            }
            else
            {
                (*def)(*pReq, tResp);
            }
        }
        else
        {
            (*pf)(*pReq, tResp);
        }
        if (!tResp.hasResped())
        {
            std::string sResp = tResp.format();
            buf.refSock->send(sResp);
        }

        if (pReq->hasConnectionClose())
        {
            buf.refSock->closeWait();
        }
    }

    // 成员函数用这个转换下
    template <class Obj>
    static std::function<void(YS_HttpRequest &, YS_HttpResponse &)> bind(void (Obj::*pf)(YS_HttpRequest &, YS_HttpResponse &), Obj *obj)
    {
        return std::bind(pf, &obj, std::placeholders::_1, std::placeholders::_2);
    }
#define YS_HttpApiCustom(funcPtr) [](YS_HttpRequest &req, YS_HttpResponse &resp) { funcPtr(req, resp); }
};

class YS_HttpServer : public YS_TcpServer
{
private:
public:
    YS_HttpServer(const std::string &ip = "::", uint32_t port = 80)
        : YS_TcpServer(ip, port, nullptr)
    {
    }

    void route(HttpApiArr vtRoute)
    {
        auto pHd = std::make_shared<YS_HttpDriver>();
        pHd->_engine.route(vtRoute);
        setDriver(pHd);
    }

    void supportBlock()
    {
        getDriver()->supportBlock = true;
    }
};

class YS_HttpJsonApi
{
    static void *globalFunPtr(void *ptr)
    {
        static int32_t set = 0;
        if (set != 0 && set != 1)
        {
            YS_EXCEPT_RUNTIME_ERROR("do not init mutli server at the same time.");
        }

        static std::vector<void *> vtFunc;
        if (ptr)
        {
            set++;
            vtFunc.emplace_back(ptr);
            return nullptr;
        }
        else
        {
            set--;
            return vtFunc[vtFunc.size() - 1];
        }
    }

    template <typename T, typename K>
    static void http_proc(YS_HttpRequest &req, YS_HttpResponse &resp, int32_t api(T &, K &))
    {
        T tReq;
        K tResp;
        YS_Xpack::jsonStrToObj(req.getBody(), tReq);
        tResp.code = api(tReq, tResp);
        resp.setResponse(HTTP_STATUS_OK, YS_Xpack::objToJsonStr(tResp));
    }

public:
    template <typename ReqType, typename RespType>
    static HttpApi custom(int32_t (*pf)(ReqType *, RespType *))
    {
        globalFunPtr((void *)pf);
        return [](YS_HttpRequest &req, YS_HttpResponse &resp) {
            return http_proc(req, resp, (decltype(pf))globalFunPtr(nullptr));
        };
    }
};

} // namespace isaac

#endif