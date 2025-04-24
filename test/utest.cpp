#include "utest.h"

using namespace isaac;
using namespace std;

void utest_test();
void utest_util();
void utest_confver();
void utest_argh();
void utest_cpu();
void utest_file();
void utest_pack();
void utest_queue();
void utest_alg();
void utest_log();
void utest_tcp_server();
void utest_http_server();
void utest_redis();
void utest_validator();
void utest_regex();
void utest_handle_func();
void utest_guard();
void utest_http_bind();
void utest_download_upload();

void utest_perf_sm4();
void utest_dns();
void utest_url();
void utest_bind();
void utest_type();
void utest_template();

void utest_test()
{
    YS_Test ts({
        {utest_util,            "util test"       },
        {utest_confver,         "cond ver test"   },
        {utest_log,             "log test"        },
        {utest_perf_sm4,        "perf sm4"        },
        {utest_download_upload, "down upload file"},
        {utest_guard,           "test guard"      },
        {utest_file,            "test file"       },
        {utest_bind,            "test bind"       },
        {utest_type,            "test type"       },
        {utest_pack,            "test pack"       },
        {utest_template,        "test template"   },
    });
}

int main(int argc, char const *argv[])
{
    YS_Signal::saveSigSegv();
    YS_ArgParser parser(argv);
    if (parser.hasFlag("-l"))
    {
        utest_test();
        return 0;
    }
    // utest_util();
    // utest_confver();
    // utest_argh();
    // utest_cpu();
    utest_file();
    // utest_pack();
    // utest_queue();
    // utest_alg();
    // utest_log();
    utest_url();
    utest_tcp_server();
    utest_http_server();
    // utest_redis();
    // utest_validator();
    utest_regex();
    utest_handle_func();
    utest_guard();
    utest_http_bind();
    utest_dns();
    utest_template();
    CLI_DEBUG("all unit test passed.");
    return 0;
}

void utest_util()
{
    {
        YS_Timer tmer;
        tmer.start(2100);
        YS_Timer tmer1;
        tmer1.start(2000);
        YS_Util::mSleep(200);
        YS_Util::mSleep(800);
        YS_Util::mSleep(1000);
        assert(!tmer.isTimeOut());
        assert(tmer1.isTimeOut());
    }
    {
        string src = "1234567890!@#$%^&*()";
        string hex = YS_String::toHex(src);
        string bin = YS_String::toBin(hex);
        // CLI_DEBUG("src len = {} src :{}", src.length(), src);
        // CLI_DEBUG("bin len = {} bin :{}", bin.length(), bin);
        assert(bin == src);
    }
}

void utest_confver()
{
    YS_CondVar  cond;
    std::thread th1([&] { assert(cond.wait()); });
    th1.detach();
    std::thread th2([&] { assert(cond.waitFor(200) == false); });
    th2.detach();
    std::thread th3([&] { assert(cond.waitFor(500)); });
    th3.detach();
    YS_Util::mSleep(250);
    cond.notifyOne();
    cond.notifyAll();
}

void utest_argh()
{
    const char  *argv[] = {"utest", "-h", "-v", "-host=127.0.0.1", "-port", "443", "daemon", nullptr};
    YS_ArgParser parser(argv);
    assert(parser.hasFlag("-h"));
    assert(parser.hasFlag("-v"));
    assert(parser.hasOpt("daemon"));
    assert(parser.hasParam("-host"));
    assert(parser.hasParam("-port"));
    assert(parser("-host") == "127.0.0.1");
    assert(parser("-port") == "443");
}

void utest_cpu()
{
    assert(YS_Cpu::getCpuNum());
}

void utest_file()
{
    string path = "./tmp.txt";
    YS_File::remove(path);
    assert(YS_File::fmtPathEx(".//root\\../log./../conf/..//tmp.txt") == path);

    string data = {0x01, 0x02, 0x03, 0x04};
    data.append("1234");
    assert(!YS_File::exist(path));
    YS_File::write("./tmp.txt", data);
    assert(!YS_File::isFile("./"));
    assert(YS_File::isFile("./tmp.txt"));
    assert(YS_File::isDir("./"));
    assert(!YS_File::isDir("./tmp.txt"));
    assert(YS_File::read(path, 4, 4) == string("1234"));
    assert(YS_File::exist(path));
    assert(YS_File::size(path) == data.length());
    string rData = YS_File::read("./tmp.txt");
    assert(YS_File::exist(path) == true);
    YS_File::remove(path);
    assert(data == rData);
}

void utest_pack()
{
    struct objSt
    {
        uint32_t bits;
        uint8_t  x[4];
        uint8_t  y[4];
    };

    struct objSub
    {
        int32_t                   iNum;
        pair<uint32_t, uint8_t *> pStr;
        YS_PACK(iNum, pStr)

        void print()
        {
            CLI_DEBUG("iNum     [{}]", iNum);
            CLI_DEBUG("pStr     [{}]", YS_String::toHex(pStr.second, pStr.first));
        }
    };
    struct objCpack
    {
        int32_t  iNum;   // 有符号数字
        uint32_t uNum;   // 无符号数字
        char     sChar;  // 有符号字符
        uint8_t  uChar;  // 有符号字符
        float    fNum;   // 浮点
        double   dNum;   // 浮点
        bool     bFalse; // 布尔
        bool     bTrue;  // 布尔

        char                      sStr[32];   // 定长有符号字符串
        uint8_t                   uStr[32];   // 定长无符号字符串
        string                    cppStr;     // 深拷贝
        char                     *pSstr;      // 有符号字符串
        pair<uint32_t, uint8_t *> pUstr;      // 无符号字符串
        objSt                     tSub;       // 结构体
        objSt                    *pSub;       // 结构体指针
        objSub                    base;       // 嵌套
        objSt                     subArr[2];  // 结构体数组数组
        objSub                    baseArr[2]; // 嵌套数组
        YS_UstrRef                uStrRef;

        YS_PACK(iNum, uNum, sChar, uChar, fNum, dNum, bFalse, bTrue,
                sStr, uStr, cppStr, pSstr, pUstr, tSub, pSub, base,
                subArr, baseArr, uStrRef)
        void print()
        {
            printf("=================================================================\n");
            CLI_DEBUG("iNum     [{}]", iNum);
            CLI_DEBUG("uNum     [{}]", uNum);
            CLI_DEBUG("sChar    [{}]", sChar);
            CLI_DEBUG("uChar    [{}]", uChar);
            CLI_DEBUG("fNum     [{}]", fNum);
            CLI_DEBUG("dNum     [{}]", dNum);
            CLI_DEBUG("bFalse   [{}]", bFalse);
            CLI_DEBUG("bTrue    [{}]", bTrue);

            CLI_DEBUG("sStr     [{}]", sStr);
            CLI_DEBUG("uStr     [{}]", YS_String::toHex(uStr, sizeof(uStr)));
            CLI_DEBUG("cppStr   [{}]", cppStr);
            if (pSstr) CLI_DEBUG("pSstr    [{}]", pSstr);
            CLI_DEBUG("pUstr    [{}]", YS_String::toHex(pUstr.second, pUstr.first));
            CLI_DEBUG("tSub     [{}]", YS_String::toHex((uint8_t *)&tSub, sizeof(tSub)));
            if (pSub) CLI_DEBUG("pSub     [{}]", YS_String::toHex((uint8_t *)pSub, sizeof(*pSub)));
            base.print();
            CLI_DEBUG("tSubArr  [{}]", YS_String::toHex((uint8_t *)subArr, sizeof(subArr)));
            for (size_t i = 0; i < 2; i++)
            {
                baseArr[i].print();
            }
            CLI_DEBUG("uStrRef  [{}]", YS_String::toHex(uStrRef, uStrRef));
            printf("=================================================================\n");
        }
    };

    uint8_t uStr[32] = {0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0xbb, 0xcc, 0xdd,
                        0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0xbb, 0xcc, 0xdd,
                        0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0xbb, 0xcc, 0xdd,
                        0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0xbb, 0xcc, 0xdd};
    objSt   tSub     = {
        8,
        {'x', 'x', 'x', 'x'},
        {'y', 'y', 'y', 'y'}
    };
    objCpack obj = {
        -1,
        1,
        0x33,
        0xff,
        float(1.11),
        2.22,
        true,
        false,
        "sStr", //  0x73, 0x53, 74 72
        {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88},
        "cppstr",
        (char *)"sSstr",
        {8, uStr},
        {8, {'x', 'x', 'x', 'x'}, {'y', 'y', 'y', 'y'}},
        &tSub,
        {123, {8, uStr}},
        {{8, {'x', 'x', 'x', 'x'}, {'y', 'y', 'y', 'y'}}, {8, {'x', 'x', 'x', 'x'}, {'y', 'y', 'y', 'y'}}},
        {{123, {8, uStr}}, {123, {8, uStr}}},
        {sizeof(uStr), uStr}
    };

    {
        uint8_t  encBuf[8192];
        uint32_t encLen = 0;

        auto ret = obj._ys_pack(encBuf, &encLen);
        CLI_DEBUG("_ys_pack ret = {}", ret);
        CLI_DEBUG("pack:[{}]", YS_String::toHex(encBuf, encLen));
        objCpack objCp;
        objCp.pSub  = nullptr;
        objCp.pSstr = nullptr;
        ret         = objCp._ys_unpack(encBuf, encLen);
        CLI_DEBUG("_ys_unpack ret = {}", ret);
        obj.print();
        objCp.print();

        uint8_t  encBufCp[8192];
        uint32_t encLenCp = 0;
        objCp._ys_pack(encBufCp, &encLenCp);
        assert(string((char *)encBuf, encLen) == string((char *)encBufCp, encLenCp));
    }
}

void utest_queue()
{
    struct Obj
    {
        int32_t id = YS_Uid::genUid();
    };
    YS_ThQueue<Obj *> que;
    Obj              *pObj = nullptr;
    assert(!que.pop_front(pObj, 0, false));
    if (pObj) delete pObj;
    que.push_back(new Obj);
    thread th1([&] {
        Obj *tmp = nullptr;
        assert(que.pop_front(tmp, 1000));
        if (tmp) delete tmp;
    });
    que.push_back(new Obj, true);
    th1.join();
}

void utest_alg()
{
    {
        string src = YS_Sm3Drbg::random(32);
        assert(src.length() == 32);
        string b64 = YS_Base64::encode(src);
        string dec = YS_Base64::decode(b64);
        assert(dec == src);
        uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8'};
        uint8_t real[32];
        YS_String::toBin("0FFFFF81E971FA3F09107ABF77931463FC0710BFB8962EFEAE3D5654B073BB0C", real);
        uint8_t md[32];
        YS_Sm3::sm3(data, sizeof(data), md);
        assert(memcmp(md, real, sizeof(real)) == 0);
    }
    {
        uint8_t data[32];
        uint8_t enc[32];
        uint8_t dec[32];
        uint8_t key[16];
        YS_Sm3Drbg::randBytes(32, data);
        YS_Sm3Drbg::randBytes(16, key);
        YS_Sm4::sm4_ecb_encrypt(data, sizeof(data), key, enc);
        YS_Sm4::sm4_ecb_decrypt(enc, sizeof(data), key, dec);
        assert(memcmp(dec, data, sizeof(data)) == 0);
    }
}

void utest_log()
{
    YS_Log::useSingleFile();
    YS_Log::init("debug");

    CLI_DEBUG("log level debug");
    CLI_DEBUG("log {} {}", "level", YS_Log::tagToLev("debug"));
    CLI_INFO("log level info");
    CLI_INFO("log {} {}", "level", YS_Log::tagToLev("info"));
    CLI_WARN("log level warn");
    CLI_WARN("log {} {}", "level", YS_Log::tagToLev("warn"));
    CLI_ERROR("log level error");
    CLI_ERROR("log {} {}", "level", YS_Log::tagToLev("error"));
    CLI_FATAL("log level fatal");
    CLI_FATAL("log {} {}", "level", YS_Log::tagToLev("fatal"));
}

static uint16_t getTestPort()
{
    return 5443;
}

struct DataCbReq
{
    YS_UstrRef data;
    YS_PACK(data);
};

struct DataCbResp
{
    YS_UstrRef data;
    YS_PACK(data);
};

int32_t func_data_cb(DataCbReq *req, DataCbResp *resp)
{
    resp->data = req->data;
    CLI_DEBUG("get req : {}", (char *)req->data);
    return 0;
}

class memberTest
{
public:
    int32_t func_data_cb(DataCbReq *req, DataCbResp *resp)
    {
        resp->data = req->data;
        return 0;
    }
    int32_t func_data_cb_1(DataCbReq *req, DataCbResp *resp)
    {
        resp->data = req->data;
        return 0;
    }
};

template <class C, typename T, typename K>
class YS_EntryPoint
{
public:
    int32_t (C::*_ptr)(T *, K *);
    C *_obj;
    YS_EntryPoint(int32_t (C::*ptr)(T *, K *), C *obj) : _ptr(ptr), _obj(obj) {}

    int32_t invoke(void *arg1, void *arg2)
    {
        return _obj->*_ptr(arg1, arg2);
    }
};

void isaac_encode(YS_Buf &buf)
{
    YS_Packet *pkt     = YS_Packet::toPkt(buf);
    uint8_t    key[16] = {};
    YS_Islam::encrypt(pkt->data, pkt->dataLen, pkt->data, &pkt->dataLen, key);
    buf.setLength(pkt->totalLen());
}

int32_t isaac_decode(YS_Buf &buf)
{
    uint8_t    key[16] = {};
    YS_Packet *pkt     = YS_Packet::toPkt(buf);
    YS_Islam::decrypt(pkt->data, pkt->dataLen, pkt->data, &pkt->dataLen, key);
    buf.setLength(pkt->totalLen());
    return 0;
}

// server所需的pack协议的驱动
class YS_EncPackDriver : public YS_Driver, private YS_PackEngine
{
public:
    using YS_PackEngine::route;

    void    encode(YS_Buf &buf) { return isaac_encode(buf); }
    int32_t decode(YS_Buf &buf) { return isaac_decode(buf); }

    void handle(YS_NetBuf &buf)
    {
        CLI_DEBUG("YS_EncPackDriver handle.");
        done(buf);
    }

    int32_t isFullPkt(YS_Buf &buf, isaac::YS_PktCtx **) { return YS_Packet::isFullPkt(buf); }
};

// pack协议驱动的tcp服务
class YS_EncPackServer : public YS_TcpServer
{
    std::shared_ptr<YS_EncPackDriver> handle = std::make_shared<YS_EncPackDriver>();

public:
    YS_EncPackServer(std::string ip, uint32_t port) : YS_TcpServer(ip, port)
    {
        setDriver(handle);
    }

    void route(PackApiArr router) { handle->route(router); }
};
class YS_EncPackClient : public YS_TcpPackClient
{
    void    encode(YS_Buf &buf) { return isaac_encode(buf); }
    int32_t decode(YS_Buf &buf) { return isaac_decode(buf); }
};

void utest_tcp_server()
{
    // auto        p = &memberTest::func_data_cb;
    memberTest  mb;
    static auto func_data_cb_ptr = YS_PackApi::bind(&memberTest::func_data_cb, &mb);

    YS_EntryPoint<memberTest, DataCbReq, DataCbResp> t{&memberTest::func_data_cb, &mb};

    vector<pair<uint16_t, PackApi>> router({
        {1, YS_PackApiCustom(func_data_cb)    },
        {2, YS_PackApiCustom(func_data_cb_ptr)},
    });

    YS_EncPackServer serv("::", getTestPort());
    serv.route(router);
    std::thread th([&] { serv.start(); });
    YS_Util::mSleep(500);
#if __linux__
    vector<string> rows;
    assert(!YS_Shell::queryRows(fmt::format("lsof -t -i:{}", getTestPort()), rows));
    assert(rows.size());
    assert(atoi(rows[0].c_str()) == getpid());
#endif

    YS_EncPackClient cl;
    string           str = "hello utest";
    DataCbReq        req = {str};
    DataCbResp       resp;

    assert(!cl.connect("127.0.0.1", getTestPort()));
    assert(!cl.doRequest(req, resp, 1));
    CLI_DEBUG("req  :{}", (char *)req.data);
    CLI_DEBUG("resp :{}", (char *)resp.data);
    assert(resp.data == req.data);
    req.data.set(11, (uint8_t *)"hello isaac");
    assert(!cl.doRequest(req, resp, 2));
    CLI_DEBUG("req  :{}", (char *)req.data);
    CLI_DEBUG("resp :{}", string((char *)resp.data, resp.data.length()));
    assert(resp.data.length() == req.data.length());
    assert(memcmp((char *)resp.data, (char *)req.data, resp.data.length()) == 0);

    serv.stop();
    th.join();
}

void utest_http_server()
{
    YS_HttpServer serv("::", getTestPort());
    static string hello = "hello isaac!";
    serv.route({
        {"/data", [](YS_HttpRequest &req, YS_HttpResponse &resp) {
             CLI_DEBUG("get reqeust: {}", req.getUrl());
             resp.setResponse(http_status::HTTP_STATUS_OK, req.getBody());
         }},
        {"*",     [](YS_HttpRequest &req, YS_HttpResponse &resp) {
             CLI_DEBUG("get reqeust: {}", req.getUrl());
             resp.setResponse(http_status::HTTP_STATUS_OK, hello);
         }},
    });
    thread th([&]() {
        serv.start();
    });
    YS_Util::mSleep(500);
    YS_HttpClient cl;

    assert(!cl.connect("127.0.0.1", getTestPort()));
    YS_HttpRequest  req;
    YS_HttpResponse resp;
    req.setConnection("keep-alive");
    req.setRequest(http_method::HTTP_POST, "/", hello);
    assert(!cl.doRequest(req, resp));
    assert(resp.getBody() == hello);

    req.setConnection("close");
    CLI_DEBUG("req connection {}", req.getConnection());
    req.setRequest(http_method::HTTP_POST, "/data", "data test===");
    assert(!cl.doRequest(req, resp));
    CLI_DEBUG("req :{}", req.getBody());
    CLI_DEBUG("resp:{}", resp.getBody());
    assert(resp.getBody() == string("data test==="));
    YS_Util::mSleep(500);

    req.setRequest(http_method::HTTP_POST, "/data", "after close");
    assert(!cl.doRequest(req, resp));
    CLI_DEBUG("req :{}", req.getBody());
    CLI_DEBUG("resp:{}", resp.getBody());

    serv.stop();
    th.join();
}

void utest_redis()
{
#if _WIN32
    CLI_DEBUG("not support!");
#else
    YS_RdsPool                   Rds("127.0.0.1", 6379);
    vector<pair<string, string>> vtVal = {
        {"key1", "val1"         },
        {"key2", "val2.1 val2.2"},
        {"key3", ""             },
    };

    pair<string, vector<string>> vtList = {
        "keyList",
        {{"val1"}, {"val2"}, {"val3"}}
    };

    pair<string, map<string, string>> mpHash = {
        "keyList",
        {{"field1", "val1"},
          {"field2", "val2.1 val2.2"},
          {"field3", ""}}
    };

    for (auto &&i : vtVal)
    {
        assert(!Rds.exec({"set", i.first, i.second}));
    }

    for (auto &&i : vtVal)
    {
        assert(Rds.get(i.first) == i.second);
    }
    Rds.exec({"del", vtList.first});
    assert(!Rds.exec(fmt::format("LPUSH {} {}", vtList.first, vtList.second[0])));
    assert(!Rds.exec(fmt::format("LPUSH {} {} {}", vtList.first, vtList.second[1], vtList.second[2])));
    auto rpy1 = Rds.query(fmt::format("RPOP {}", vtList.first));
    assert(rpy1.getStr() == vtList.second[0]);
    auto rpy2 = Rds.query(fmt::format("RPOP {} 2", vtList.first));
    auto rows = rpy2.getRows();
    assert(rows[0] == vtList.second[1]);
    assert(rows[1] == vtList.second[2]);

    Rds.query({"HMSET",
               mpHash.first,
               mpHash.second.begin()->first,
               mpHash.second.begin()->second,
               (++mpHash.second.begin())->first,
               (++mpHash.second.begin())->second,
               (++ ++mpHash.second.begin())->first,
               (++ ++mpHash.second.begin())->second});

    vector<string> vtRes = Rds.hmget(mpHash.first, fmt::format("{} {} {}", mpHash.second.begin()->first,
                                                               (++mpHash.second.begin())->first, (++ ++mpHash.second.begin())->first));
    assert(vtRes[0] == mpHash.second.begin()->second);
    assert(vtRes[1] == (++mpHash.second.begin())->second);
    assert(vtRes[2] == (++ ++mpHash.second.begin())->second);
#endif
}

#include "tool/ys_validator.hpp"

void utest_validator()
{
    struct Req : public YS_Verify
    {
        int32_t id;
        string  name;
        string  ip;
        string  ePwd;
        string  hPwd;

        void validator(YS_Valid &r)
        {
            r.range(id, 8, 16)
                .pwdEasy(ePwd)
                .pwdHard(hPwd)
                .vchar(name)
                .except(name, "%?;")
                .ip(ip);
        }
    };
}

void utest_regex()
{
    assert(YS_Regex::length("1234", 8, 16) == false);
    assert(YS_Regex::length("12345678", 8, 16) == true);
    assert(YS_Regex::length("12345678", 8) == true);
    assert(YS_Regex::length("123456789", 8) == false);
    assert(YS_Regex::num("123456789") == true);
    assert(YS_Regex::num("123a%") == false);
    assert(YS_Regex::num("") == true);
    assert(YS_Regex::schar("123abcABC") == true);
    assert(YS_Regex::schar("123abcABC%?/") == false);
    assert(YS_Regex::schar("") == true);
    assert(YS_Regex::vchar("123abcABC%?/") == true);
    assert(YS_Regex::vchar("") == true);
    string raw = "123abcABC%?/";
    raw.push_back(0x01);
    assert(YS_Regex::vchar(raw) == false);
    assert(YS_Regex::except("123 abc ABC %?/", " ") == false);
    assert(YS_Regex::except("123 abc ABC %?/", "@#") == true);
    assert(YS_Regex::except("", "@#") == true);
    assert(YS_Regex::url("http://127.0.0.1") == true);
    assert(YS_Regex::url("http://127.0.0.1\";sleep 10") == false);
    assert(YS_Regex::url("") == true);
    assert(YS_Regex::user("isaac_1st@") == false);
    assert(YS_Regex::user("isaac_1st") == true);
    assert(YS_Regex::user("") == true);
    assert(YS_Regex::pwdEasy("12345") == true);
    assert(YS_Regex::pwdHard("12345") == false);
    assert(YS_Regex::pwdEasy("") == true);
    assert(YS_Regex::pwdHard("123ABCabc!@#") == true);
    assert(YS_Regex::pwdHard("123ABCabc!@#") == true);
    assert(YS_Regex::pwdHard("") == true);
    assert(YS_Regex::ip("127.0.0.1") == true);
    assert(YS_Regex::ip("fe80::1") == true);
    assert(YS_Regex::ip("::1") == true);
    assert(YS_Regex::ip("127.0.0.1 ") == false);
    assert(YS_Regex::ip("123 456") == false);
    assert(YS_Regex::ip("") == true);
    assert(YS_Regex::range(7, 8, 16) == false);
    assert(YS_Regex::range(10, 8, 16) == true);
    assert(YS_Regex::remarks(",123abcABC.@com!?") == true);
    CLI_DEBUG("{}", YS_String::toHex("备注"));
    assert(YS_Regex::remarks("") == true);
    assert(YS_Regex::remarks("备注,123abcABC") == true);
    assert(YS_Regex::remarks("备注,123abcABC.@com!?()") == false);

    assert(YS_Regex::inject("isaac;reboot") == false);
    assert(YS_Regex::inject("isaac;sleep(5)") == false);
    assert(YS_Regex::inject("isaac>sleep(5)") == false);
    assert(YS_Regex::inject("isaac sleep(5)") == false);
    assert(YS_Regex::inject("isaac|sleep(5)") == false);
    assert(YS_Regex::inject("isaac&sleep(5)") == false);
    assert(YS_Regex::inject("isaac--sleep(5)") == false);
    assert(YS_Regex::inject("") == true);
}

class Ts_Func
{
public:
    void func1() { CLI_DEBUG("func1"); };
    void func2() { CLI_DEBUG("func2"); };

    void done(string str)
    {
    }
};

void utest_handle_func()
{
    Ts_Func e;
    e.done("func1");
    e.done("func2");
}

class TS_ApiBase
{
public:
    virtual int32_t proc(string in, string out)
    {
        CLI_ERROR("empty proc.");
        assert(0);
        return 0;
    }

    template <typename T>
    static T *strToSt(string)
    {
        return nullptr;
    }
    virtual ~TS_ApiBase() {}
};

#define register_pack(ReqType, RespType)                          \
    int32_t proc(string in, string out)                           \
    {                                                             \
        return api(strToSt<ReqType>(in), strToSt<RespType>(out)); \
    }

class TS_Api_1 : public TS_ApiBase
{
public:
    struct Req_1
    {
    };
    struct Resp_1
    {
    };
    // register_pack(Req_1, Resp_1);
    int32_t proc(string in, string out) { return api(strToSt<Req_1>(in), strToSt<Resp_1>(out)); }

    int32_t api(Req_1 *, Resp_1 *)
    {
        CLI_DEBUG("api1===");
        return 0;
    }
};

class TS_Api_2 : public TS_ApiBase
{
public:
    struct Req_2
    {
    };
    struct Resp_2
    {
    };
    register_pack(Req_2, Resp_2);

    int32_t api(Req_2 *, Resp_2 *)
    {
        CLI_DEBUG("api2===");
        return 0;
    }
};

void utest_guard()
{
    CLI_PRINT("<html>");
    YS_Guard { CLI_PRINT("</html>\n"); };
    {
        CLI_PRINT("<header>");
        YS_Guard { CLI_PRINT("</header>"); };
    }
    {
        CLI_PRINT("<body>");
        YS_Guard { CLI_PRINT("</body>"); };
    }
}
class TS_HttpBind
{
public:
    void ts_http_1(YS_HttpRequest &tReq, YS_HttpResponse &tResp) { CLI_DEBUG("ts http api 1"); };
    void ts_http_2(YS_HttpRequest &tReq, YS_HttpResponse &tResp) { CLI_DEBUG("ts http api 2"); };

    template <class Obj, int32_t N = 0>
    static HttpApi bind(void (Obj::*pf)(YS_HttpRequest &tReq, YS_HttpResponse &tResp), Obj *obj)
    {
        static auto fn   = pf;
        static Obj *pObj = obj;
        return [](YS_HttpRequest &tReq, YS_HttpResponse &tResp) {
            (pObj->*fn)(tReq, tResp);
        };
    }
};

void utest_http_bind()
{
    TS_HttpBind ts;

    auto api1 = TS_HttpBind::bind(&TS_HttpBind::ts_http_1, &ts);
    auto api2 = TS_HttpBind::bind<TS_HttpBind, 1>(&TS_HttpBind::ts_http_2, &ts);

    YS_HttpRequest  req;
    YS_HttpResponse resp;

    api1(req, resp);
    api2(req, resp);
}

void utest_perf_sm4()
{
    class Perf : public YS_TestMul
    {
        uint8_t key[16];
        uint8_t data[8192];

    public:
        int32_t init(size_t id)
        {
            YS_Sm3Drbg::randBytes(sizeof(key), key);
            YS_Sm3Drbg::randBytes(sizeof(data), data);
            return 0;
        }

        int32_t run(size_t id)
        {
            uint8_t cipher[8192];
            YS_Sm4::sm4_ecb_encrypt(data, sizeof(data), key, cipher);
            return 0;
        }
    };

    Perf ts;
    ts.loopMulti(8192);
}

void utest_download_upload()
{
    YS_TcpPackServer serv{"::", getTestPort()};
    // serv.setRoute({
    //     {1, downloadFile   },
    //     {2, getFileRange   },
    //     {3, uploadFile     },
    //     {4, uploadFileRange},
    // });
    std::thread th([&] { serv.start(); });
    YS_Guard { serv.stop(); };
    YS_Util::mSleep(1000);
}

#include "tool/ys_ini.hpp"

void utest_ini()
{
    mINI::INIFile      file("./test.ini");
    mINI::INIStructure st;
    file.read(st);
    auto &opt = st["sec"]["opt"];
    assert(!opt.empty());
}

void utest_dns()
{
    YS_EndPoint::dns("www.baidu.com");
    // YS_EndPoint::dns("www.islam3rd.top");
    // YS_EndPoint::dns("islam3rd.top");
}

void url_parse(string url, bool withheader)
{
    YS_Url  tUrl;
    int32_t ret;
    if (withheader)
    {
        ret = tUrl.parse(url);
    }
    else
    {
        ret = tUrl.parseTail(url);
    }

    if (ret)
    {
        CLI_ERROR("url parse fail :{} ", url);
    }
    else
    {
        CLI_DEBUG("url parse ok : path = [{}] ", tUrl.path);
    }
}

void utest_url()
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

    for (auto &&i : vturl)
    {
        url_parse(i, true);
    }

    vector<string> vt = {
        "/s/ARs8X85LPPcNsHT/download?path=%2F&files=%E5%8D%B8%E8%BD%BD%E8%BD%AF%E4%BB%B6.rar",
        "",
        "/index.html",
        "/index.html?arg1=1",
        "/index.html?arg1=1&arg2=2",
        "/index.html?arg1=1&arg2=2#desc",
        "/中文.zip?arg1=1&arg2=2#desc",
    };
    for (auto &&i : vt)
    {
        url_parse(i, false);
    }
}

#include <tuple>

template <typename T>
class Binder
{
    T *_t                    = nullptr;
    void (T::*_func)(void *) = nullptr;

public:
    template <typename SUB>
    void bind(void (T::*func)(SUB *), T *t)
    {
        _t    = t;
        _func = reinterpret_cast<decltype(_func)>(func);
    }

    void call(void *k)
    {
        if (_t && _func) (_t->*_func)(k);
    }

    void clear()
    {
        _t    = nullptr;
        _func = nullptr;
    }
};

class Ts_FuncObj;

class Ts_Sub
{
public:
    string             data;
    Binder<Ts_FuncObj> cb;

    void run()
    {
        CLI_DEBUG("sub run.");
        cb.call(this);
    }
};

class Ts_FuncObj
{
public:
    void func(Ts_Sub *sub)
    {
        CLI_DEBUG("func run.");
        CLI_DEBUG("sub data:{}", sub->data);
    }
};

void utest_bind()
{
    Ts_FuncObj obj;
    Ts_Sub     sub;

    std::function<void(Ts_Sub *)> func = std::bind(&Ts_FuncObj::func, &obj, std::placeholders::_1);

    sub.cb.bind(&Ts_FuncObj::func, &obj);
    sub.run();
}

void utest_type()
{
#define LOG_TYPE(x) CLI_DEBUG("type : {} size: {}", #x, sizeof(x))

    LOG_TYPE(int32_t);
    LOG_TYPE(int64_t);
    LOG_TYPE(uint32_t);
    LOG_TYPE(uint64_t);
    LOG_TYPE(int);
    LOG_TYPE(long);
    LOG_TYPE(u_int);
    LOG_TYPE(u_long);
    LOG_TYPE(uint8_t);
    LOG_TYPE(int8_t);
    LOG_TYPE(u_char);
    LOG_TYPE(char);
}


struct Req1
{
};
struct Rsp1
{
};
struct Req2
{
};
struct Rsp2
{
};

class Service
{
public:
    int32_t api_1(Req1 *, Rsp1 *, YS_NetBuf &)
    {
        CLI_DEBUG("api_1.");
        return 0;
    }

    int32_t api_2(Req2 *, Rsp2 *, YS_NetBuf &)
    {
        CLI_DEBUG("api_2.");
        return 0;
    }
};

void utest_template()
{
    Service serv;
    auto    api1 = YS_PackApi::binder(&Service::api_1, &serv);
    auto    api2 = YS_PackApi::binder(&Service::api_2, &serv);
    YS_NetBuf  buf;
    api1(nullptr, nullptr, buf);
    api2(nullptr, nullptr, buf);
}