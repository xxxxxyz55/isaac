#include "tool/ys_test.hpp"
#include "ys_http_client.h"
#include "tool/ys_argh.hpp"

using namespace isaac;
using namespace std;

static int32_t port = 8080;
static string  addr = "127.0.0.1";
static uint32_t threadNum = YS_Cpu::getCpuNum();
static uint32_t connection = 1;

void callonce()
{
    YS_HttpClient cl;
    if (cl.connect(addr, port))
    {
        return;
    }
    YS_HttpRequest  req;
    YS_HttpResponse resp;
    req.setConnection("keep-alive");
    req.setRequest(http_method::HTTP_POST, "/", "hello");
    cl.doRequest(req, resp);
}

int32_t main(int32_t argc, char const *argv[])
{
    YS_ArgParser parser(argv);
    if (argc == 1)
    {
        printf(R"(
        -p      port
        -h      host
        -g      call once
        -t      thread num
        -c      connection: close
        )");
        return 1;
    }

    if (parser.hasParam("-p"))
    {
        port = stoi(parser("-p"));
    }

    if (parser.hasParam("-h"))
    {
        addr = parser("-h");
    }

    if (parser.hasFlag("-c"))
    {
        CLI_DEBUG("connection :close");
        connection = 0;
    }

    if (parser.hasFlag("-g"))
    {
        callonce();
        return 0;
    }
    if (parser.hasParam("-t"))
    {
        threadNum = stoi(parser("-t"));
    }

    class TsPerf : public YS_TestMul
    {
    public:
        YS_HttpClient& getClient()
        {
            thread_local YS_HttpClient cl;
            return cl;
        }
        int32_t init(size_t i)
        {
            if (connection)
            {
                return getClient().connect(addr, port);
            }
            return 0;
        }

        int32_t run(size_t i)
        {
            YS_HttpRequest  req;
            YS_HttpResponse resp;
            req.setConnection("keep-alive");
            req.setRequest(http_method::HTTP_POST, "/", "hello");
            if (connection)
            {
                return getClient().doRequest(req, resp);
            }
            else
            {
                YS_HttpClient cl;
                if (cl.connect(addr, port))
                {
                    return -1;
                }
                return cl.doRequest(req, resp);
            }
        }
    };

    TsPerf ts;
    if (!connection)
    {
        ts.breakOnErr(false);
    }
    ts.loopMulti(0,threadNum);
    return 0;
}