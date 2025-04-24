#ifndef _YS_HTTP_EX_H_
#define _YS_HTTP_EX_H_
#include <string>
#include <map>
#include <cstdint>
#include "ys_http_parser.h"

namespace isaac {

using Headers = std::multimap<std::string, std::string>;
// http请求
class YS_HttpRequest : public YS_HttpParserCtx
{
public:
    YS_HttpRequest() : YS_HttpParserCtx(HTTP_REQUEST) {}

    std::string format()
    {
        std::string req;
        req.reserve(_body.length() + 1024);

        req.append(HttpParser::http_method_str(http_method(_parser.method)));
        req.append(" ");
        req.append(_url);
        req.append(" HTTP/1.1\r\n");

        // header
        if (_parser.method == http_method::HTTP_POST && !hasHeader("Content-Type")) setHeader("Content-Type", "text/plain");

        if (!hasHeader("Host")) setHeader("Host", "");

        if (_parser.method == HTTP_POST || _body.length())
        {
            updateHeader("Content-Length", std::to_string(_body.length()));
        }

        formatHeader(req);
        // body
        req.append(_body);

        return req;
    };
    // 设置请求内容
    void setRequest(http_method method, const std::string &sUrl, const std::string &req)
    {
        _parser.method = method;
        _url           = sUrl;
        _body          = req;
    }
};

class YS_HttpResponse : public YS_HttpParserCtx
{
private:
    YS_SocketPtr _Sock;
    bool         _fDirect = false; // 内部发送

public:
    YS_HttpResponse(YS_SocketPtr pSock = nullptr) : YS_HttpParserCtx(HTTP_RESPONSE), _Sock(pSock) {}
    bool hasResped()
    {
        return _fDirect;
    }

    int32_t sendFile(const std::string path)
    {
        _fDirect  = true;
        size_t sz = YS_File::size(path);
        setHeader("Content-Length", std::to_string(sz));
        setHeader("Content-Disposition",
                  fmt::format("attachment; filename=\"{}\"", YS_File::name(path)));
        setResponse(HTTP_STATUS_OK, "");

        std::string data = format();
        int32_t     ret  = _Sock->send(data);
        size_t      off  = 0;
        while (off < sz)
        {
            std::string data = YS_File::read(path, 8 * 8192, off);
            if (data.empty())
            {
                return 0;
            }
            ret = _Sock->send(data);
            if (ret != int32_t(data.length()))
            {
                CLI_ERROR("file send fail off = {} datalen = {} send len = {}.", off, data.length(), ret);
                return -1;
            }
            off += data.length();
        }

        return 0;
    }

    int32_t recvFile(const std::string &path, size_t sz)
    {
        YS_Buf buf(YS_Buf::size32KB());
        size_t off = 0;
        do
        {
            if (_Sock->recv(buf))
            {
                CLI_ERROR("recv file fail.");
                return -1;
            }
            off += buf.length();
            YS_Util::canRead(_Sock->getFd(), 3000);
            YS_File::write(path, buf.uData(), buf.length(), true);
            buf.clear();
        } while (off < sz);

        return 0;
    }

    // 序列化为string
    std::string format()
    {
        std::string resp;
        resp.reserve(_body.length() + 1024);

        resp.append("HTTP/1.1 ");
        resp.append(std::to_string(getStatusCode()));
        resp.append(" ");
        resp.append(HttpParser::http_status_str(http_status(getStatusCode())));
        resp.append("\r\n");

        // header
        // if (!hasHeader("Content-Type"))
        // {
        //     setHeader("Content-Type", "text/plain");
        // }

        if (!hasHeader("Content-Length"))
        {
            std::string sLen = std::to_string(_body.length());
            setHeader("Content-Length", sLen);
        }

        formatHeader(resp);
        // body
        resp.append(_body);
        return resp;
    };

    // uint32_t format(char *buf, size_t size)
    // {
    //     uint32_t totalLen = snprintf(buf, size, "HTTP/1.1 %d %s\r\n",
    //                                  getStatusCode(),
    //                                  HttpParser::http_status_str(http_status(getStatusCode())));

    //     if (!hasHeader("Content-Type"))
    //     {
    //         setHeader("Content-Type", "text/plain");
    //     }

    //     if (!hasHeader("Content-Length"))
    //     {
    //         std::string sLen = std::to_string(_body.length());
    //         setHeader("Content-Length", sLen);
    //     }

    //     totalLen += formatHeader(buf + totalLen, size - totalLen);
    //     totalLen += snprintf(buf + totalLen, size - totalLen, "%s", _body.c_str());
    //     return totalLen;
    // }

    void setResponse(http_status stat, const std::string &content)
    {
        setStatusCode(stat);
        _body = content;
    }

    uint32_t getStatus()
    {
        return getStatusCode();
    }

    std::string getStatusStr()
    {
        return HttpParser::http_status_str((http_status)getStatusCode());
    }

    std::string &getContent()
    {
        return _body;
    }
};

} // namespace isaac

#endif