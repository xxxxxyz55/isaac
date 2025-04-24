#ifndef __YS_URL_H__
#define __YS_URL_H__

#include <string>
#include <regex>
#include "ys_cli_log.h"

namespace isaac {

class YS_Url
{
public:
    std::string protocol;
    std::string user;
    std::string pass;
    std::string host;
    uint16_t    port = 0;
    std::string path;
    std::string param;
    std::string mark;

    std::string decode(const std::string &encoded)
    {
        std::string decoded;
        for (size_t i = 0; i < encoded.length(); ++i)
        {
            if (encoded[i] == '%')
            {
                if (i + 2 < encoded.length())
                {
                    std::string hex_str = encoded.substr(i + 1, 2);
                    int         hex_value;
                    std::istringstream(hex_str) >> std::hex >> hex_value;
                    decoded += static_cast<char>(hex_value);
                    i += 2; // 跳过已处理的两个字符
                }
                else
                {
                    decoded += encoded[i];
                }
            }
            else if (encoded[i] == '+')
            {
                decoded += ' ';
            }
            else
            {
                decoded += encoded[i];
            }
        }
        return decoded;
    }

    std::string tail()
    {
        std::string url;
        if (host.empty()) return "";
        if (!path.empty())
        {
            if (path[0] == '/')
            {
                url.append(path);
            }
            else
            {
                url.append("/").append(path);
            }
        }

        if (!param.empty())
        {
            url.append("?").append(param);
        }
        if (!mark.empty())
        {
            url.append("#").append(mark);
        }
        return url;
    }

    std::string format()
    {
        if (host.empty()) return "";

        std::string url;
        if (!protocol.empty())
        {
            url.append(protocol).append("://");
        }
        if (!user.empty())
        {
            user.append(user);
            if (!pass.empty())
            {
                user.append(":").append(pass);
            }
            user.append("@");
        }
        url.append(host);
        if (port != 0)
        {
            url.append(":").append(std::to_string(port));
        }
        if (!path.empty())
        {
            if (path[0] == '/')
            {
                url.append(path);
            }
            else
            {
                url.append("/").append(path);
            }
        }

        if (!param.empty())
        {
            url.append("?").append(param);
        }
        if (!mark.empty())
        {
            url.append("#").append(mark);
        }
        return url;
    }

    static uint16_t defaultPort(std::string protocol)
    {
        if (protocol.empty()) return 80;

        static std::map<std::string, uint16_t> _mp = {
            {"https", 443},
            {"http",  80 },
            {"ssh",   22 },
            {"ftp",   21 },
            {"sftp",  22 },
        };
        auto iter = _mp.find(YS_String::tolower(protocol));
        if (iter != _mp.end())
        {
            return iter->second;
        }
        else
        {
            return 80;
        }
    }

    bool isSsl()
    {
        static std::map<std::string, std::nullptr_t> _mp = {
            {"https", nullptr},
            {"ssh",   nullptr},
            {"sftp",  nullptr},
        };
        auto iter = _mp.find(YS_String::tolower(protocol));
        if (iter != _mp.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    /*
    (^
    (?:(\w+):\/\/)?         //协议
    (?:(\w+)(?::(\w+))?@)?  //用户名:密码@
    ([\w.-]+)               //主机
    (?::(\d{1,5}))?         //:端口
    (\/(?:[^\?#:]*))?       ///路径
    (?:\?([^#]*))?          //?参数
    (?:#([\w\-]*))?         //标签
    $)
    */
    int32_t parse(const std::string &url)
    {
        std::string sUrl = decode(url);
        if (sUrl != url)
        {
            CLI_DEBUG("decode url = [{}].", sUrl);
        }
        static std::regex reg(R"(^(?:(\w+):\/\/)?(?:(\w+)(?::(\w+))?@)?([\w.-]+)(?::(\d{1,5}))?(\/(?:[^\?#:]*))?(?:\?([^#]*))?(?:#([\w\-]*))?$)");

        std::smatch match;
        if (!std::regex_match(sUrl, match, reg)) return -1;

        protocol = match[1];
        user     = match[2];
        pass     = match[3];
        host     = match[4];

        std::string sport = match[5];
        if (!sport.empty())
        {
            port = std::stoi(sport);
        }
        else
        {
            port = defaultPort(protocol);
        }

        path  = match[6];
        if (path.empty()) path = "/";
        param = match[7];
        mark  = match[8];
        return 0;
    }

    int32_t parseTail(const std::string &url)
    {
        std::string sUrl = decode(url);
        if (sUrl != url)
        {
            CLI_DEBUG("decode url = [{}].", sUrl);
        }
        static std::regex reg(R"(^(\/(?:[^\?#:]*))?(?:\?([^#]*))?(?:#([\w\-]*))?$)");
        std::smatch       match;
        if (!std::regex_match(sUrl, match, reg)) return -1;
        path = match[1];
        if (path.empty()) path = "/";
        param = match[2];
        mark  = match[3];

        return 0;
    }

    static std::string getPath(const std::string &url)
    {
#ifdef __GNUC_4_8__
        auto pos = url.find("?");
        if (pos != std::string::npos)
        {
            return url.substr(0, pos);
        }
        pos = url.find("&");
        if (pos != std::string::npos)
        {
            return url.substr(0, pos);
        }
        pos = url.find("#");
        if (pos != std::string::npos)
        {
            return url.substr(0, pos);
        }
        return url;
#else
        YS_Url tUrl;
        tUrl.parseTail(url);
        return tUrl.path;
#endif
    }
};

} // namespace isaac

#endif