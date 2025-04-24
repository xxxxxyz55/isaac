#include "ys_http_client.h"
#include "ys_url.h"

using namespace std;
using namespace isaac;

void test_url()
{
    vector<string> vturl = {
        "https://islam:aaadddg@islam3rd.top",
        "islam@islam3rd.top",
        "islam3rd.top",
        "https://islam:aaadddg@islam3rd.top/index.html",
        "https://islam:aaadddg@islam3rd.top/index.html?arg1=1",
        "https://islam:aaadddg@islam3rd.top/index.html?arg1=1&arg2=2",
        "https://islam:aaadddg@islam3rd.top:443/index.html?arg1=1&arg2=2#desc",
        "https://islam:aaadddg@islam3rd.top:443/中文.zip?arg1=1&arg2=2#desc",
        "ssh://root@islam3rd.top",
        "ssh://root@islam3rd.top:22",
    };
    
}

int32_t main(int32_t argc, char const *argv[])
{
    string url;
    if (argv[1] == nullptr)
    {
        url = YS_Util::stdGetString("url:");
    }
    else
    {
        url = argv[1];
    }
    CLI_DEBUG("url = {}", url);

    YS_Url   tUrl;

    if (tUrl.parse(url))
    {
        CLI_ERROR("invalid url {}.", url);
        return -1;
    }

    CLI_DEBUG("url = {}", tUrl.tail());

    auto vtip = YS_EndPoint::dns(tUrl.host);
    for (auto &&iter : vtip)
    {
        YS_HttpClient cl;
        if (tUrl.isSsl())
        {
            cl.useSsl(YS_Ssl::newCtx("", "", ""));
        }

        if (cl.connect(iter, tUrl.port))
        {
            continue;
        }

        CLI_DEBUG("download ip = {}", iter);

        YS_HttpRequest  req;
        YS_HttpResponse resp;
        req.setHeader("Range", "bytes=0-1023");
        req.setHeader("Host", "127.0.0.1");
        req.setRequest(http_method::HTTP_GET, tUrl.tail(), "");
        if (cl.doRequest(req, resp))
        {
            CLI_ERROR("do reqest fail.");
            continue;
        }
        resp.print();
    }

    return 0;
}