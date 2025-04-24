#ifndef _YS_HTTP_CLIENT_H_
#define _YS_HTTP_CLIENT_H_

#include "ys_tcp_client.h"
#include "ys_http_pkt.h"

namespace isaac {
class YS_HttpClient : public YS_TcpClient
{
public:
    YS_HttpClient() {}

    // 是一个完整的http包
    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **ctx)
    {
        return YS_HttpParser::httpParseResp(buf, ctx);
    }

    // 执行http请求
    int32_t doRequest(YS_HttpRequest &tReq, YS_HttpResponse &tResp)
    {
        std::string sReq  = tReq.format();
        YS_PktCtx  *pResp = &tResp;
        tResp.init(http_parser_type::HTTP_RESPONSE);
        int32_t  iRet   = sendRecv((const uint8_t *)sReq.c_str(), sReq.length(),
                                   getBuf(), &pResp);
        if (iRet)
        {
            return iRet;
        }

        return 0;
    }
};
} // namespace isaac

#endif