#ifndef _YS_HTTP_PARSER_H_
#define _YS_HTTP_PARSER_H_

#include <cstring>
#include <map>
#include "ys_cli_log.h"
#include "ys_http_parser_ex.h"
#include "ys_socket.h"
#include "ys_url.h"
#include<algorithm>

namespace isaac {

class YS_HttpRequest;
class YS_HttpResponse;

class YS_HttpParserCtx : public YS_PktCtx
{
protected:
    http_parser          _parser;
    http_parser_settings _setting;
    uint32_t             _pos;
    bool                 _fRecv;

    std::string _url;
    std::string _status;

    std::string _header_field;
    std::string _header_val;
    bool        _last_is_val;
    std::string _body;

    std::multimap<std::string, std::string> _headers;

    int32_t on_message_begin(http_parser *)
    {
        return 0;
    }

    static int32_t pf_on_message_begin(http_parser *parser)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_message_begin(parser);
    }

    int32_t on_url(http_parser *, const char *at, size_t length)
    {
        _url.append(at, length);
        return 0;
    }

    static int32_t pf_on_url(http_parser *parser, const char *at, size_t length)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_url(parser, at, length);
    }

    int32_t on_status(http_parser *, const char *at, size_t length)
    {
        _status.append(at, length);
        return 0;
    }

    static int32_t pf_on_status(http_parser *parser, const char *at, size_t length)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_status(parser, at, length);
    }

    int32_t on_header_field(http_parser *, const char *at, size_t length)
    {
        if (_last_is_val)
        {
            if (_header_field.length())
            {
                _headers.insert({_header_field, _header_val});
                _header_field.clear();
                _header_val.clear();
            }
        }
        _header_field.append(at, length);
        _last_is_val = false;
        return 0;
    }

    static int32_t pf_on_header_field(http_parser *parser, const char *at, size_t length)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_header_field(parser, at, length);
    }

    int32_t on_header_value(http_parser *, const char *at, size_t length)
    {
        _header_val.append(at, length);
        _last_is_val = true;
        return 0;
    }

    static int32_t pf_on_header_value(http_parser *parser, const char *at, size_t length)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_header_value(parser, at, length);
    }

    int32_t on_headers_complete(http_parser *parser)
    {
        try
        {
            _headers.insert({_header_field, _header_val});
        }
        catch (const std::exception &e)
        {
            CLI_ERROR("err http header [{}]", _header_field);
            return -1;
        }

        return 0;
    }

    static int32_t pf_on_headers_complete(http_parser *parser)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_headers_complete(parser);
    }

    int32_t on_body(http_parser *parser, const char *at, size_t length)
    {
        _body.append(at, length);
        return 0;
    }

    static int32_t pf_on_body(http_parser *parser, const char *at, size_t length)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_body(parser, at, length);
    }

    int32_t on_message_complete(http_parser *parser)
    {
        _fRecv = false;
        // print();
        return 0;
    }

    static int32_t pf_on_message_complete(http_parser *parser)
    {
        return (reinterpret_cast<YS_HttpParserCtx *>(parser->data))->on_message_complete(parser);
    }

    static bool hasCrlf(const std::string &s)
    {
        for (auto &&c : s)
        {
            if (c == '\r' || c == '\n')
            {
                return true;
            }
        }

        return false;
    }

    void formatHeader(std::string &sHeader)
    {
        if (!hasHeader("Connection"))
        {
            setHeader("Connection", "close");
        }

        for (auto &&i : _headers)
        {
            sHeader.append(i.first);
            sHeader.append(": ");
            sHeader.append(i.second);
            sHeader.append("\r\n");
        }
        sHeader.append("\r\n");
    }

    // uint32_t formatHeader(char *buf, size_t size)
    // {
    //     if (!hasHeader("Connection"))
    //     {
    //         setHeader("Connection", "close");
    //     }
    //     uint32_t totalLen = 0;
    //     for (auto &&i : _headers)
    //     {
    //         totalLen += snprintf(buf + totalLen, size - totalLen, "%s: %s\r\n",
    //                              i.first.c_str(), i.second.c_str());
    //     }
    //     totalLen += snprintf(buf + totalLen, size - totalLen, "\r\n");
    //     return totalLen;
    // }

public:
    void setConnection(const std::string &sConn)
    {
        updateHeader("Connection", sConn);
    }
    std::string getConnection()
    {
        auto iter = _headers.find("Connection");
        if(iter!= _headers.end())
        {
            return iter->second;
        }
        setHeader("Connection", "keep-alive");
        return "keep-alive";
    }

    const char *getType()
    {
        switch (_parser.type)
        {
        case 0:
            return "REQUEST";
        case 1:
            return "RESPNOSE";
        case 2:
            return "BOTH";

        default:
            return nullptr;
        }
    }

    bool hasHeader(const std::string &sKey) { return _headers.find(sKey) != _headers.end(); }
    void setHeader(const std::string &sKey, const std::string sVal)
    {
        if (!hasCrlf(sKey) && !hasCrlf(sVal))
        {
            _headers.emplace(sKey, sVal);
        }
    }
    void delHeader(const std::string &sKey)
    {
        if (hasHeader(sKey)) _headers.erase(sKey);
    }

    std::string getHeader(const std::string &sKey)
    {
        auto iter = _headers.find(sKey);
        if (iter != _headers.end())
        {
            return iter->second;
        }
        return "";
    }

    void updateHeader(const std::string &sKey, const std::string sVal)
    {
        delHeader(sKey);
        setHeader(sKey, sVal);
    }

    bool hasConnectionClose()
    {
        auto ptr = _headers.find("Connection");
        if (ptr == _headers.end())
        {
            return false; // 默认长连接
        }

        std::string tmp = ptr->second;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

        if (tmp == "close")
        {
            return true;
        }
        return false;
    }

    void setStatusCode(http_status stat)
    {
        _parser.status_code = stat;
    }

    uint32_t getStatusCode()
    {
        return _parser.status_code;
    }

    std::string getUrl()
    {
        return YS_Url::getPath(_url);
    }
    uint32_t     getMethod() { return _parser.method; }
    std::string &getBody() { return _body; }
    int32_t      getContentLen()
    {
        auto iter = _headers.find("Content-Length");
        if (iter != _headers.end())
        {
            try
            {
                return std::stoi(iter->second);
            }
            catch (...)
            {
                return 0;
            }
        }
        return 0;
    }

    std::multimap<std::string, std::string> &getHeaders() { return _headers; }

    YS_HttpRequest  *toReq() { return (YS_HttpRequest *)this; }
    YS_HttpResponse *toResp() { return (YS_HttpResponse *)this; }

    bool isComplete() { return !_fRecv; }
    bool isMultipartData()
    {
        auto content = getHeader("Content-Type");
        if (content.empty()) return false;
        return content.find("multipart/form-data") != std::string::npos;
    }

    void print()
    {
        CLI_DEBUG("===============HTTP {}==============", getType());
        uint32_t umethod = _parser.method;
        CLI_DEBUG("METHOD [{}]", HttpParser::http_method_str((http_method)umethod));
        CLI_DEBUG("STATUS [{} {}]", getStatusCode(), _status);
        CLI_DEBUG("URL    [{}]", _url);
        for (auto &&i : _headers)
        {
            CLI_DEBUG("HEADER [{}][{}]", i.first, i.second);
        }
        CLI_DEBUG("BODY   [{}]", _body);
    }

    friend class YS_HttpParser;

    void init(enum http_parser_type type)
    {
        HttpParser::http_parser_init(&_parser, type);
        HttpParser::http_parser_settings_init(&_setting);
        clear();
        _pos                         = 0;
        _fRecv                       = true;
        _parser.data                 = this;
        _last_is_val                 = true;
        _setting.on_message_begin    = pf_on_message_begin;
        _setting.on_url              = pf_on_url;
        _setting.on_status           = pf_on_status;
        _setting.on_header_field     = pf_on_header_field;
        _setting.on_header_value     = pf_on_header_value;
        _setting.on_headers_complete = pf_on_headers_complete;
        _setting.on_body             = pf_on_body;
        _setting.on_message_complete = pf_on_message_complete;
    }

    void clear()
    {
        _url.clear();
        _status.clear();
        _header_val.clear();
        _body.clear();
        _headers.clear();
    }

    YS_HttpParserCtx(enum http_parser_type type)
    {
        init(type);
    }
};

class YS_HttpParser
{
public:
    static int32_t httpParseReq(YS_Buf &buf, /*YS_HttpParserCtx */ YS_PktCtx **arg)
    {
        return httpParse(buf, arg, HTTP_REQUEST);
    }

    static int32_t httpParseResp(YS_Buf &buf, /*YS_HttpParserCtx */ YS_PktCtx **arg)
    {
        return httpParse(buf, arg, HTTP_RESPONSE);
    }

    static int32_t parserAppend(YS_HttpParserCtx *ctx, const char *data, uint32_t len)
    {
        size_t parserLen = HttpParser::http_parser_execute(&ctx->_parser, &ctx->_setting, data, len);
        if (parserLen != len)
        {
            auto err = ctx->_parser.http_errno;
            CLI_ERROR("parseLen [{}] recvLen [{}] errno [{}] [{}].",
                      parserLen, len, err,
                      HttpParser::http_errno_description(http_errno(ctx->_parser.http_errno)));
            return -1;
        }
        else
        {
            ctx->_pos += parserLen;
            if (ctx->_fRecv)
            {
                return 8192;
            }
        }
        return 0;
    }

    static int32_t httpParse(YS_Buf &buf, /*YS_HttpParserCtx */ YS_PktCtx **arg,
                             http_parser_type type = HTTP_REQUEST)
    {
        if(arg == nullptr)
        {
            CLI_ERROR("arg == nullptr");
            return -1;
        }

        if (*arg == nullptr)
        {
            *arg                   = new YS_HttpParserCtx(type);
            YS_HttpParserCtx *pReq = static_cast<YS_HttpParserCtx *>(*arg);
            pReq->init(type);
        }

        if (buf.empty()) return buf.capacity();

        YS_HttpParserCtx *pReq = static_cast<YS_HttpParserCtx *>(*arg);

        size_t parseLen = HttpParser::http_parser_execute(&pReq->_parser, &pReq->_setting,
                                                          (char *)buf.uData() + pReq->_pos, buf.length() - pReq->_pos);
        if (parseLen != buf.length() - pReq->_pos)
        {
            auto err = pReq->_parser.http_errno;
            CLI_ERROR("parseLen [{}] recvLen [{}] errno [{}] [{}].",
                      parseLen, buf.length(), err,
                      HttpParser::http_errno_description(http_errno(pReq->_parser.http_errno)));
            return -1;
        }
        else
        {
            pReq->_pos += parseLen;
            if (buf.left() == 0)
            {
                return 0;
            }
            if (pReq->_fRecv)
            {
                return buf.leftSize();
            }
        }

        return 0;
    }
};
} // namespace isaac

#endif