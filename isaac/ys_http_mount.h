#ifndef _YS_HTTP_MOUNT_H_
#define _YS_HTTP_MOUNT_H_

#include "ys_http_server.h"
#include "ys_mount_html.h"

namespace isaac {

// web文件索引
class YS_HttpMountDriver : public YS_Driver
{
    std::string                     _root = "/www";
    YS_Engine<std::string, HttpApi> _engine;

    using Ranges = std::vector<std::pair<uint64_t, uint64_t>>;

public:
    void route(HttpApiArr vt) { _engine.route(vt); }

    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **arg)
    {
        return YS_HttpParser::httpParseReq(buf, arg);
    }

    void getUrlFile(std::string sUrl, YS_HttpRequest *pReq, YS_HttpResponse *pResp)
    {
        if (sUrl.find("..") != std::string::npos)
        {
            pResp->setResponse(HTTP_STATUS_BAD_REQUEST, "");
            return;
        }

        if (sUrl == "" || sUrl == "/")
        {
            sUrl = "/index.html";
        }

        std::string realPath = YS_File::fmtPath(_root + sUrl);
        // CLI_DEBUG("read path = {}", realPath);
        if (YS_File::exist(realPath))
        {
            if (YS_File::isFile(realPath))
            {
                sendFile(realPath, pReq, pResp);
            }
            else
            {
                getAutoIndex(sUrl, realPath, pResp);
            }
        }
        else
        {
            if (sUrl == "/index.html")
            {
                getAutoIndex("/", _root, pResp);
            }
            else if (sUrl == "/favicon.ico")
            {
                getDefIcon(pResp);
            }
            else
            {
                CLI_ERROR("url: {} not found.", sUrl);
                pResp->setResponse(HTTP_STATUS_NOT_FOUND, "");
            }
        }
    }

    Ranges parseRange(YS_HttpRequest *pReq)
    {
        auto &hd   = pReq->getHeaders();
        auto  iter = hd.find("Range");
        if (iter != hd.end())
        {
            return {};
        }

        return {};
    }

    void sendFile(std::string realPath, YS_HttpRequest *pReq, YS_HttpResponse *pResp)
    {
        Ranges rg = parseRange(pReq);
        if (YS_File::size(realPath) < 5 * 1024 * 1024)
        {
            pResp->setResponse(HTTP_STATUS_OK, YS_File::read(realPath));
            pResp->setHeader("Content-Type", fileContentType(realPath));
            // pResp->setHeader("Content-Disposition",
            //                  fmt::format("attachment; filename=\"{}\"", YS_File::name(realPath)));
        }
        else
        {
            pResp->sendFile(realPath);
        }
    }

    void getDefIcon(YS_HttpResponse *pResp)
    {
        static std::string data = YS_String::toBin("89504E470D0A1A0A0000000D49484452000000100000001008060000001FF3FF61000000097048597300000B1300000B1301009A9C180000004E49444154388D6360A0149C591C78F4CCE2A0FF68F8300906606826051F061BF0F9D58DFFA482CF2FAF810D61F87C38E9FFFF336964E1CF8712FF3390AB1986470D1835806A061C26DB80D369870029D7DBF37FCE83A90000000049454E44AE426082");
        pResp->setHeader("Content-Type", "image/png");
        pResp->setResponse(HTTP_STATUS_OK, data);
    }

    void getAutoIndex(const std::string &url, const std::string &path, YS_HttpResponse *pResp)
    {
        std::vector<YS_File::DirInfo> vtInfo;

        std::string sPath = YS_File::fmtPath(path);
        if (YS_File::scanDir(sPath, vtInfo))
        {
            CLI_DEBUG("scanDir fail, {}", sPath);
            pResp->setResponse(HTTP_STATUS_NOT_FOUND, "");
        }
        else
        {
            std::string sResp = YS_MountHtml::format(url, vtInfo);
            pResp->setResponse(HTTP_STATUS_OK, sResp);
            pResp->setHeader("Content-Type", "text/html");
        }
    }

    enum FileType : uint32_t
    {
        UNKNOW,
        CSS_JS,
        X_ICO,
        PNG,
        HTML,
    };

    std::string fileExt(const std::string &path)
    {
#ifdef __GNUC_4_8__
        auto pos = path.find_last_of('.');
        if (pos == std::string::npos)
        {
            return std::string();
        }
        else
        {
            return path.substr(pos + 1);
        }
#else
        std::smatch m;
        static auto re = std::regex("\\.([a-zA-Z0-9]+)$");
        if (std::regex_search(path, m, re))
        {
            return m[1].str();
        }
        return std::string();
#endif
    }

    std::string fileContentType(const std::string &path)
    {
        auto ext = fileExt(path);

        static std::map<std::string, std::string> mp = {
            {"css",   "text/css"                   },
            {"csv",   "text/csv"                   },
            {"htm",   "text/html"                  },
            {"html",  "text/html"                  },
            {"js",    "text/javascript"            },
            {"mjs",   "text/javascript"            },
            {"txt",   "text/plain"                 },
            {"vtt",   "text/vtt"                   },

            {"apng",  "image/apng"                 },
            {"avif",  "image/avif"                 },
            {"bmp",   "image/bmp"                  },
            {"gif",   "image/gif"                  },
            {"png",   "image/png"                  },
            {"svg",   "image/svg+xml"              },
            {"webp",  "image/webp"                 },
            {"ico",   "image/x-icon"               },
            {"tif",   "image/tiff"                 },
            {"tiff",  "image/tiff"                 },
            {"jpg",   "image/jpeg"                 },
            {"jpeg",  "image/jpeg"                 },

            {"mp4",   "video/mp4"                  },
            {"mpeg",  "video/mpeg"                 },
            {"webm",  "video/webm"                 },

            {"mp3",   "audio/mp3"                  },
            {"mpga",  "audio/mpeg"                 },
            {"weba",  "audio/webm"                 },
            {"wav",   "audio/wave"                 },

            {"otf",   "font/otf"                   },
            {"ttf",   "font/ttf"                   },
            {"woff",  "font/woff"                  },
            {"woff2", "font/woff2"                 },

            {"7z",    "application/x-7z-compressed"},
            {"atom",  "application/atom+xml"       },
            {"pdf",   "application/pdf"            },
            {"json",  "application/json"           },
            {"rss",   "application/rss+xml"        },
            {"tar",   "application/x-tar"          },
            {"xht",   "application/xhtml+xml"      },

            {"xslt",  "application/xslt+xml"       },
            {"xml",   "application/xml"            },
            {"gz",    "application/gzip"           },
            {"zip",   "application/zip"            },
            {"wasm",  "application/wasm"           }
        };

        auto iter = mp.find(ext);
        if(iter == mp.end())
        {
            return "applcation/octet-stream";
        }
        else
        {
            return iter->second;
        }
    }

    void handle(YS_NetBuf &buf)
    {
        YS_HttpRequest *pReq = static_cast<YS_HttpRequest *>(buf.ctx);
        YS_HttpResponse tResp(buf.refSock);
        tResp.setConnection(pReq->getConnection());

        std::string sUrl   = pReq->getUrl();
        CLI_DEBUG("mount handle url = {}", sUrl);
        auto        method = pReq->getMethod();
        if (method != http_method::HTTP_GET &&
            method != http_method::HTTP_POST &&
            method != http_method::HTTP_OPTIONS)
        {
            tResp.setResponse(HTTP_STATUS_METHOD_NOT_ALLOWED, "");
            return;
        }

        HttpApi pf = _engine.get(sUrl);
        if (pf)
        {
            (*pf)(*pReq, tResp);
            if (!tResp.hasResped())
            {
                std::string sResp = tResp.format();
                buf.refSock->send(sResp);
            }
        }
        else
        {
            std::string sRespData;
            getUrlFile(sUrl, pReq, &tResp);
            std::string sResp = tResp.format();
            buf.refSock->send(sResp);
        }

        if (pReq->hasConnectionClose())
        {
            buf.refSock->closeWait();
        }
    }

    void setRoot(const std::string &root)
    {
        _root = root;
    }
};

class YS_HttpMount : public YS_TcpServer
{
    std::shared_ptr<YS_HttpMountDriver> _hd;

public:
    YS_HttpMount(const std::string &ip = "::", uint32_t port = 80)
        : YS_TcpServer(ip, port, nullptr)
    {
    }

    void mount(const std::string &root)
    {
        _hd = std::make_shared<YS_HttpMountDriver>();
        _hd->setRoot(root);
        setDriver(_hd);
    }

    // 不要*方法
    void route(HttpApiArr vt) { _hd->route(vt); }
};

} // namespace isaac

#endif