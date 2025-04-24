#include "ys_test.hpp"
#include "ys_udp.h"
#include "ys_udp_client.h"
#include "tool/ys_argh.hpp"
#include "ys_xpack.h"

using namespace std;
using namespace isaac;

// channl
void create();
void join();
void del();

// user
void lists();
void config();
void exit();

YS_UdpClient &getClient()
{
    YS_UdpClient cl;
    return cl;
};

template <typename T>
void encodeToBuf(T &obj, YS_Buf &buf)
{
    string str = xpack::json::encode(obj);
    buf.clear();
    buf.append(str);
}

template <typename T>
int32_t decodeFromBuf(YS_Buf &buf, T &obj)
{
    try
    {
        xpack::json::decode(buf.sData(), obj);
    }
    catch (const std::exception &e)
    {
        CLI_ERROR("json decode fail, {}", e.what());
        return -1;
    }
    return 0;
}

int32_t usage()
{
    CLI_PRINT(R"(
        heart_cli -host xx.xx.xx.xx -port xx
            -v,--veraion        version
            -h,-help            usage
            -host               host
            -port               port
)");
    return 0;
}


int32_t main(int32_t argc, char const *argv[])
{
    YS_ArgParser parser(argv);
    if (parser.hasFlag({"-v", "--version"}))
    {
        CLI_PRINT("1.0.0");
        return 0;
    }

    if (parser.hasFlag({"-h", "-help"}) ||
        !parser.hasParam("-host") ||
        !parser.hasParam("-port"))
    {
        return usage();
    }

    string  host = parser("-host");
    int32_t port = std::stoi(parser("port"));

    auto &cl = getClient();

    if (cl.bind("0.0.0.0", 0))
    {
        CLI_ERROR("bind fail, {}", YS_ERRNO);
        return -1;
    }

    CLI_DEBUG("bind addr {}:{}",
              cl.getSock()->getLocEndPoint().getIp(),
              cl.getSock()->getLocEndPoint().getPort());

    YS_Test ts({
        {create, "create channl"        },
        {join,   "join channl"          },
        {exit,   "exit channl"          },
        {del,    "del channl"           },
        {lists,  "list member in channl"},
        {config, "config channl"        },
    });
    return 0;
}

struct ReqChannlCreate
{
    string   channl;
    string   pin;
    uint32_t intvl;
    string   user;
    XPACK(M(channl, pin, intvl), O(user));
};

struct RspChannlCreate
{
    int32_t code;
    string  msg;
    XPACK(O(code, msg));
};

struct ReqChannlDelete
{
    string channl;
    string pin;
    XPACK(M(channl, pin));
};

struct RspChannlDelete
{
    int32_t code;
    string  msg;
    XPACK(O(code, msg));
};

struct ReqChannlJoin
{
    string channl;
    string pin;
    string user;
    XPACK(M(channl, pin, user));
};

struct RspChannlJoin
{
    int32_t code;
    string  msg;
    XPACK(O(code, msg));
};

struct ReqChannlExit
{
    string channl;
    string pin;
    string user;
    XPACK(M(channl, pin));
};

struct RspChannlExit
{
    int32_t code;
    string  msg;
    XPACK(O(code, msg));
};

struct ReqChannlList
{
    string channl;
    string pin;
    XPACK(M(channl, pin));
};

struct Member
{
    string name;
    string ip;
    string port;
    XPACK(M(name, ip, port))
};

struct RspChannlList
{
    int32_t code;
    string  msg;

    vector<Member> members;
    XPACK(O(code, msg, members));
};

struct ReqChannlConfig
{
    string channl;
    string pin;
    string intvl;
    XPACK(M(channl, pin, intvl));
};

struct RspChannlConfig
{
    int32_t code;
    string  msg;
    XPACK(O(code, msg));
};

class HeartClient : public YS_UdpClient
{
public:
    void onEpollIn(YS_EpData *pData)
    {
    }
};

// channl
void create()
{
    ReqChannlCreate req;
    req.channl = YS_Util::stdGetString("频道名:");
    req.pin    = YS_Util::stdGetString("口令:");
    req.intvl  = 10;
    req.user   = YS_Util::stdGetString("用户名:");
}

void join()
{
}

void del()
{
}

// user
void lists()
{
}

void config()
{
}

void exit()
{
}