#include "ys_udp_server_lite.h"
#include "ys_pack_proto.h"
#include "ys_xpack.h"
#include <type_traits>

using namespace isaac;
using namespace std;

using JsonApi = int32_t (*)(YS_NetBuf &buf);

struct YS_JsonPkt
{
    string cmd;
    XPACK(M(cmd))
};

string jpktGetCmd(YS_Buf &buf)
{
    YS_JsonPkt pkt;
    try
    {
        xpack::json::decode(buf.sData(), pkt);
    }
    catch (const std::exception &e)
    {
        CLI_ERROR("json decode fail, {}.", e.what());
        return string();
    }
    return pkt.cmd;
}

class YS_HeartDriver : public YS_Driver
{
public:
    YS_Engine<string, JsonApi> e;
    void                       handle(YS_NetBuf &buf)
    {
        CLI_DEBUG("heart handle");
        auto cb = e.get(jpktGetCmd(buf.recvData));
        if (cb)
        {
            cb(buf);
        }
        else
        {
            buf.sendData.append(R"({"code":-1,"msg":"bad request"})");
        }
    }

    void route(vector<pair<string, JsonApi>> vtRouter) { e.route(vtRouter); }

    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **)
    {
        return YS_Xpack::isComplete(buf, nullptr);
    }
};

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

class Channl
{
    string              channl;
    string              pin;
    uint32_t            intvl;
    map<string, Member> _mp;

public:
    int32_t join() { return 0; }
    int32_t exit() { return 0; }
    int32_t list() { return 0; }
    int32_t config() { return 0; }
};

class ChannlGroup
{
    map<string, Channl> _mp;

public:
    int32_t create(ReqChannlCreate *req, RspChannlCreate *resp, YS_NetBuf &buf)
    {
        CLI_POINT();
        return 0;
    }
    int32_t del(ReqChannlDelete *req, RspChannlDelete *resp, YS_NetBuf &buf)
    {
        CLI_POINT();
        return 0;
    }
    int32_t join(ReqChannlJoin *req, RspChannlJoin *resp, YS_NetBuf &buf)
    {
        CLI_POINT();
        return 0;
    }
    int32_t exit(ReqChannlExit *req, RspChannlExit *resp, YS_NetBuf &buf)
    {
        CLI_POINT();
        return 0;
    }
    int32_t list(ReqChannlList *req, RspChannlList *resp, YS_NetBuf &buf)
    {
        CLI_POINT();
        return 0;
    }
    int32_t config(ReqChannlConfig *req, RspChannlConfig *resp, YS_NetBuf &buf)
    {
        CLI_POINT();
        return 0;
    }
};
template <typename T>
struct HeartReq
{
    string cmd;
    T      req;
    XPACK(M(cmd, req))
};

template <typename T>
struct HeartRsp
{
    string cmd;
    T      rsp;
    XPACK(M(cmd, rsp))
};

template <typename F,
          typename ReqPtr = typename YS_ArgType<F>::type1,
          typename RspPtr = typename YS_ArgType<F>::type2,
          typename Req    = typename std::remove_pointer<ReqPtr>::type,
          typename Rsp    = typename std::remove_pointer<RspPtr>::type>
int32_t heartPackApi(YS_NetBuf &buf, F pf)
{
    HeartReq<Req> tIn  = {};
    HeartRsp<Rsp> tOut = {};
    int32_t err;
    try
    {
        xpack::json::decode(string(buf.recvData.sData(), buf.recvData.length()), tIn);
    }
    catch (const std::exception &e)
    {
        CLI_ERROR("json decode fail, {}", e.what());
        return 100;
    }

    err = pf(&tIn.req, &tOut.rsp, buf);
    if (err) return err;
    tOut.cmd    = tIn.cmd;
    string resp = xpack::json::encode(tOut);

    buf.sendData.append(resp);

    return 0;
}

#define HeartApi(func) [](YS_NetBuf &buf) { return heartPackApi(buf, func); }

int32_t main(int32_t argc, char const *argv[])
{
    static ChannlGroup group;
    auto               hd = std::make_shared<YS_HeartDriver>();

    YS_UdpServerLite serv("0.0.0.0", 3740, hd);
    hd->route({
  // {"/channl/create", HeartApi(apiChannlCreate)},
        {"/channl/create", HeartApi(YS_PackApi::binder(&ChannlGroup::create, &group))},
        {"/channl/delete", HeartApi(YS_PackApi::binder(&ChannlGroup::del,    &group))},
        {"/channl/join",   HeartApi(YS_PackApi::binder(&ChannlGroup::join,   &group))},
        {"/channl/exit",   HeartApi(YS_PackApi::binder(&ChannlGroup::exit,   &group))},
        {"/channl/list",   HeartApi(YS_PackApi::binder(&ChannlGroup::list,   &group))},
        {"/channl/config", HeartApi(YS_PackApi::binder(&ChannlGroup::config, &group))},
    });
    serv.start();
    return 0;
}
