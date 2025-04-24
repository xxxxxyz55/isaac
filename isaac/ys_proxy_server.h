#ifndef __YS_PROXY_SERVER_H__
#define __YS_PROXY_SERVER_H__

#include "ys_tcp_server.h"
#include "ys_http_server.h"
#include "ys_http_client.h"
#include "ys_pack_proto.h"
#include <functional>

namespace isaac {

class YS_ProxyDriver : public YS_Driver
{
    YS_PackDriver pDev;
    YS_HttpDriver hDev;

    std::string _httpServer;
    uint32_t    _httpPort;

public:
    void route(PackApiArr router) { pDev.route(router); }
    void route(HttpApiArr vtRouter) { hDev._engine.route(vtRouter); }

    void handle(YS_NetBuf &buf)
    {
        if (buf.ctx)
        {
            hDev.handle(buf);
        }
        else
        {
            pDev.handle(buf);
        }
    }

    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **ctx)
    {
        int32_t ret = 0;
        if (*ctx || (ret = YS_Packet::isFullPkt(buf)) < 0)
        {
            return YS_HttpParser::httpParseReq(buf, ctx);
        }
        return ret;
    }
};

class YS_ProxyMidClient : public YS_TcpClientAsyn
{
public:
    void asynHandle(YS_EpData *ep)
    {
        // recv till engine and send
    }
};

class YS_ProxySession
{
    uint32_t          sid;
    YS_ProxyMidClient _cl;
    YS_SocketPtr      _sock;

public:
};

class YS_ProxyServ : public YS_TcpServer
{
private:
    std::shared_ptr<YS_ProxyDriver>    handle = std::make_shared<YS_ProxyDriver>();
    std::string                        _httpHost;
    uint32_t                           _httpPort;
    std::map<int32_t, YS_ProxySession> _mpSess; // fd->session

    // 伪装
    void httpCamouflage(YS_HttpRequest &tReq, YS_HttpResponse &tResp)
    {
        if (_httpHost.empty())
        {
            return tResp.setResponse(HTTP_STATUS_NOT_FOUND, "");
        }
        else
        {
            YS_HttpClient cl;
            if (cl.connect(_httpHost, _httpPort))
            {
                return tResp.setResponse(HTTP_STATUS_BAD_GATEWAY, "");
            }

            if (cl.doRequest(tReq, tResp))
            {
                return tResp.setResponse(HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED, "");
            }
        }
    }

    struct ReqData
    {
        uint32_t    sId; // 防重放
        std::string ip;
        std::string domainName;
        uint16_t    port;
        std::string data;
        YS_PACK(sId, ip, domainName, port, data);
    };

    struct RespData
    {
        uint32_t    sId;
        std::string data;
        YS_PACK(data);
    };

    int32_t dataProxy(ReqData *req, uint8_t *out, uint32_t *outLen)
    {
        std::vector<std::string> vtIp;
        if (!req->domainName.empty())
        {
            vtIp = YS_EndPoint::dns(req->domainName);
        }
        else if (!req->ip.empty())
        {
            vtIp.emplace_back(req->ip);
        }
    }

public:
    void onConn(YS_SocketPtr &pSock)
    {
    }
    void onDisconn(YS_SocketPtr &pSock)
    {
    }

    YS_ProxyServ(std::string ip, uint32_t port) : YS_TcpServer(ip, port)
    {
        static auto httpCamouflagePtr = YS_HttpDriver::bind(&YS_ProxyServ::httpCamouflage, this);
        static auto dataProxyPtr      = YS_PackApi::bind(dataProxy, this);
        handle->route({
            {"*", YS_HttpApiCustom(httpCamouflagePtr)}
        });
        handle->route({
            {1, YS_PackApiOnlyReq(dataProxyPtr)}
        });
        setDriver(handle);
    }

    void setCamouflage(std::string host, uint32_t port)
    {
        _httpHost = host;
        _httpPort = port;
    }
};

} // namespace isaac

#endif