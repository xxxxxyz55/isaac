#include "ys_http_mount.h"
#include "tool/ys_argh.hpp"
#include "tool/ys_signal.hpp"

using namespace std;
using namespace isaac;

int32_t help()
{
    CLI_PRINT(R"(
        islam: web/fs mount 
            option:
                -h,--help     用法
                -D,--daemon   后台运行
                -d  [dir]      挂载目录,默认./www 或./,
                -p  [port]     端口,默认80
                -sp [port]     SSL端口,默认443
                -gp [port]     GMSSL端口,默认8080
                -v,--version   version
)");
    return 0;
}
int32_t version()
{
    string ver = "1.0.0";
    CLI_PRINT("{}\n", ver);
    return 0;
}

void hello(YS_HttpRequest &tReq, YS_HttpResponse &tResp)
{
    tResp.setResponse(HTTP_STATUS_OK, "hello islam!");
}

YS_SslCtxPtr getSslCtx()
{
    static string crt = R"(-----BEGIN CERTIFICATE-----
MIIDKTCCAhGgAwIBAgIUH4C2NRDMjopvuM6CcZ5sXhBu7i4wDQYJKoZIhvcNAQEL
BQAwMzELMAkGA1UEBhMCSFgxDTALBgNVBAgMBE1BUlMxFTATBgNVBAMMDGlzbGFt
LWZhbWlseTAeFw0yNTAzMTQwOTQyNDNaFw0zNTAzMTIwOTQyNDNaMCwxCzAJBgNV
BAYTAkhYMQ0wCwYDVQQIDARNQVJTMQ4wDAYDVQQDDAVpc2FhYzCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBAMsI+2vZPNG1KFBVXQSvoKNjNjT1Bkpstpnu
S8OqVf/F8QCEht5TUdB1TrNLLOhtHrSVNwmhK4xwF9fSke2rnHVQIyBYy4GENhYm
BPvGFVu8cjde+z8kKr7Rp7A0IyeMKjIbNkzCd7qZKLvut6V/v/eNFHAL6f/Mopws
8I8kUUV2nxybPxATKIrwwLX/xUGAEOepgoKJ2aRdEfr7/DprNJ1kTwaODizsJOSm
nlJ9emQmcZjF73OKk6icM6iv5G4+wFFTPCHJItJIKT7js8jE4GmTj/Juxp7FtULV
UTjVF8qhtsiAsSCBQ4sVIIwwvpxyV+eGqhXuGEhKlCYXzw50V7UCAwEAAaM8MDow
CQYDVR0TBAIwADAgBgNVHSUBAf8EFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwCwYD
VR0PBAQDAgXgMA0GCSqGSIb3DQEBCwUAA4IBAQC1Xuw6RaokpmRsx8KDujX1+ZOW
1ZvacKO/oDyqubA+V786sQIgW2+cy/IkFU9RLuiRu4DGmAzhJ5s890/YCWrVGudN
rFqBgYwQUlt9btU3TsHwOB/kc64H1FXeO+JUP8+M9Pe7SyYkfCKefnCEtogtJOhZ
aseBTs91Y/pIrGEaOcwRKmPFAujoTaNUa3LOVtU7Xh4VUct4afn9hMmpmdJX1jTf
MlBYTbZFm257m+v8GkCIywQdG1gJrcebg3vXjCy0gaNYB1lehqLdcbdXwti8ZMy1
AyLvxXqruRDJz5cs0FoPUW6y1a1TO72dYjgaGrYVg+nE6YuaPOFDhWsH6Pnt
-----END CERTIFICATE-----
)";

    static string key = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEAywj7a9k80bUoUFVdBK+go2M2NPUGSmy2me5Lw6pV/8XxAISG
3lNR0HVOs0ss6G0etJU3CaErjHAX19KR7aucdVAjIFjLgYQ2FiYE+8YVW7xyN177
PyQqvtGnsDQjJ4wqMhs2TMJ3upkou+63pX+/940UcAvp/8yinCzwjyRRRXafHJs/
EBMoivDAtf/FQYAQ56mCgonZpF0R+vv8Oms0nWRPBo4OLOwk5KaeUn16ZCZxmMXv
c4qTqJwzqK/kbj7AUVM8Icki0kgpPuOzyMTgaZOP8m7GnsW1QtVRONUXyqG2yICx
IIFDixUgjDC+nHJX54aqFe4YSEqUJhfPDnRXtQIDAQABAoIBACHRaS9Zi8vSRNi8
nzW8myEb0VeYyGd1HCUfj/cTVCe6LJ5NdLlrX2E9aNJsXxqHsmmZwQUVl09ZDF+i
bzQLNZBEvNUSdTMpdmjSBFp7b+tLxCp845RszcHi4e9zf7eCnTSjW7QFYDPwdtx2
spgaYzs0gIb5NQyqZRtmUNtGzVKoV1rRptfMxQ2qb7BwiwPxQK8ZOwbYF0iEbCTG
9DV/pggwwPLx+L0bsiXlTA4c51jCH2KuCiDhYn2ySCwBl8DGuxG3ziEcumcVmd0e
oh96KHAHXFtUrxsAMDGyYfJT7h+y2gf0mz1qK6ZcOnjT7fvL2HV3iF3WBUHieGBc
obzDNLECgYEA8gIlIuHXVrVLHIK7ImdpFNxvzrx5iVP6Wcu0+mBpJAzaLA00U3Bn
FiSgmzsVBGjxP1HCQFcfat8Q9O4LqsMfmY04b1SQAJy468TbwHaoo43mXS3e+fzt
4OZ7HIuk6TMvGjK6Nl/9DfSuTxDpcNT9Du4TxbScmkXHdq2zLXAZzqMCgYEA1sYC
4DQnEXj6OUlOr7Eu/Akd4qzxrsN+eI4jPli+uNvNLpz5xaO+lDNTW3jB6zONLym2
jjXCCiJEeHXu5sf0wRGFpmm4cQL/UUzN5st9hJHC29bUaLwgLH7QIT3jh+ehSX/z
PpL8ogQossgk54kgkjUEQBdvoChqrBWvKKsW3ccCgYEAl2lmKKPZ4YL94nGceVLv
uMCOJ29eLuBess2zhi7JuWddHNCKG2Yo6P6ziUt2w9KQlYgmm7CmlGvJpSRGImmA
HMbxNx5WFjv3qnL49jqHGgUOyt+HMSrg/aPWBK8N12VRo/d278wwfAnJAOjY1OI9
DZlAv6JXlGHyALt+bgBD9TcCgYEAksXhIz0g0dT+E3YgmvqYJE9KqCUOSSN6Y4Kn
XS0isYwsjUVjgAhl7juhJKrCwyyIuIlmI4tjXq+KYVQe/oH5ppOF5XLsQ/cLapD1
SVsZHSbLppcKs8SUrDPBomX7/11xIJybK9Ehassy0PLsHXrd0mz+yZn6g9X7CD0N
Zdp+FvMCgYAgpwAyxRb+eeAtGAa5hELzbA4rw2BUiZHLAV6AzRY3fxkNSBMY0d9G
aChBfFP5ypLdL7h9eCD8SjG+SIrj8MUTxfNfZSMcm13POMT7b0+ypH+t9C6QmWNI
d7xaklB6Vn9E5l++WQ3N51A10N7x19Zd/+Haa6mmPkh6RRE7jcTVYA==
-----END RSA PRIVATE KEY-----
)";

    string crtPath = YS_Proc::exePath() + "/cert/sign_rsa.crt";
    string keyPath = YS_Proc::exePath() + "/cert/sign_rsa.key";
    if (!YS_File::isFile(crtPath))
    {
        string dir = YS_Proc::exePath() + "/cert";
        if (!YS_File::isDir(dir))
        {
            YS_File::mkdir(dir);
        }
        YS_File::write(crtPath, crt);
        YS_File::write(keyPath, key);
    }
    return YS_Ssl::newCtx("", crtPath, keyPath);
}

YS_SslCtxPtr getGmsslCtx()
{
    static string signcrt = R"(-----BEGIN CERTIFICATE-----
MIIBkjCCATigAwIBAgIJALvcH45C37dsMAoGCCqBHM9VAYN1MDMxCzAJBgNVBAYT
AkhYMQ0wCwYDVQQIDARNQVJTMRUwEwYDVQQDDAxpc2xhbS1mYW1pbHkwHhcNMjUw
MzE3MDIyMjE1WhcNMzUwMzE1MDIyMjE1WjAsMQswCQYDVQQGEwJIWDENMAsGA1UE
CAwETUFSUzEOMAwGA1UEAwwFaXNhYWMwWTATBgcqhkjOPQIBBggqgRzPVQGCLQNC
AASYbaxzl4F03zp4/LDeoPJH+6PWc2aqcNfO/Ur6Q7MNbYcnj/mx9INKq5Sk8EHa
m4PRxMVMIwN8pYKYhxAimL4fozwwOjAJBgNVHRMEAjAAMCAGA1UdJQEB/wQWMBQG
CCsGAQUFBwMBBggrBgEFBQcDAjALBgNVHQ8EBAMCBeAwCgYIKoEcz1UBg3UDSAAw
RQIhAMth4SqZqYsjR7OX6rHY9OetC8aMlrm+7e5M4pp29ytBAiBIeyuI7cniSg7D
TL7hWktABnKIb3J/i9avZdQaSW7zIw==
-----END CERTIFICATE-----
)";

    static string signkey = R"(-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIFIM6Kf0R8Fh3YXyEjUFhpak/7kCP+jgJu8wZXkycELJoAoGCCqBHM9V
AYItoUQDQgAEmG2sc5eBdN86ePyw3qDyR/uj1nNmqnDXzv1K+kOzDW2HJ4/5sfSD
SquUpPBB2puD0cTFTCMDfKWCmIcQIpi+Hw==
-----END EC PRIVATE KEY-----
)";
    static string enccrt  = R"(-----BEGIN CERTIFICATE-----
MIIBjzCCATWgAwIBAgIJALvcH45C37dtMAoGCCqBHM9VAYN1MDMxCzAJBgNVBAYT
AkhYMQ0wCwYDVQQIDARNQVJTMRUwEwYDVQQDDAxpc2xhbS1mYW1pbHkwHhcNMjUw
MzE3MDIyMjE1WhcNMzUwMzE1MDIyMjE1WjAsMQswCQYDVQQGEwJIWDENMAsGA1UE
CAwETUFSUzEOMAwGA1UEAwwFaXNhYWMwWTATBgcqhkjOPQIBBggqgRzPVQGCLQNC
AASLw6MfhTZXjglrcmj3cnMttw4j62RwOshITrqdZEL5hw1ZJAAnBVXKchdVHtbh
K954Q/+d8fUbQXaukDQL6E65ozkwNzAJBgNVHRMEAjAAMB0GA1UdJQQWMBQGCCsG
AQUFBwMBBggrBgEFBQcDAjALBgNVHQ8EBAMCBSAwCgYIKoEcz1UBg3UDSAAwRQIh
APDNRpFfX5WHXIhz4E5wah+wrM2jw3CV52/MbJuG2pL6AiBrxfXQfF0rj4ktG8P1
nArv6XInP8hL6dny/1FhJRqbkg==
-----END CERTIFICATE-----
)";

    static string enckey = R"(-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIFbR9kPNdwOIkLZHijTfDhIBB6OzFCNIxSLI0u6aOgGHoAoGCCqBHM9V
AYItoUQDQgAEi8OjH4U2V44Ja3Jo93JzLbcOI+tkcDrISE66nWRC+YcNWSQAJwVV
ynIXVR7W4SveeEP/nfH1G0F2rpA0C+hOuQ==
-----END EC PRIVATE KEY-----
)";

    string signcrtPath = YS_Proc::exePath() + "/cert/sign_sm2.crt";
    string signkeyPath = YS_Proc::exePath() + "/cert/sign_sm2.key";
    string enccrtPath  = YS_Proc::exePath() + "/cert/enc_sm2.crt";
    string enckeyPath  = YS_Proc::exePath() + "/cert/enc_sm2.key";
    if (!YS_File::isFile(signkeyPath))
    {
        string dir = YS_Proc::exePath() + "/cert";
        if (!YS_File::isDir(dir))
        {
            YS_File::mkdir(dir);
        }
        YS_File::write(signcrtPath, signcrt);
        YS_File::write(signkeyPath, signkey);
        YS_File::write(enccrtPath, enccrt);
        YS_File::write(enckeyPath, enckey);
    }
    return YS_Ssl::newGmCtx("", signcrtPath, signkeyPath, enccrtPath, enckeyPath);
}

void upload(YS_HttpRequest &req, YS_HttpResponse &resp)
{
    CLI_DEBUG("get upload file.");
    string path = "./upload.txt";
    YS_File::write(path, req.getBody());
    if (!req.isComplete())
    {
        CLI_DEBUG("detached recv.");
        auto ctxLen = req.getContentLen();
        if (ctxLen <= 0)
        {
            return resp.setResponse(HTTP_STATUS_BAD_REQUEST, "");
        }
        resp.recvFile(path, ctxLen);
    }
    else
    {
        req.print();
    }

    return resp.setResponse(HTTP_STATUS_OK, "");
}

int32_t main(int32_t argc, char const *argv[])
{
    YS_ArgParser parser(argv);

    if (parser.hasFlag({"-h", "--help"})) return help();
    if (parser.hasFlag({"-v", "--version"})) return version();
    if (parser.hasFlag({"-D,--daemon"}))
    {
        YS_Signal::ignoreHup();
        YS_Proc::deamonForkRun();
    }

    string path;
    if (parser.hasParam("-d"))
    {
        path = parser("-d");
    }
    else
    {
        path = "./www";
        if (!YS_File::isDir(path))
        {
            path = "./";
        }
        CLI_DEBUG("mount {}", path);
    }

    uint16_t port   = 80;
    uint16_t sport  = 443;
    uint16_t gmport = 8080;
    if (parser.hasParam("-p"))
    {
        port = std::stoi(parser("-p"));
    }
    if (parser.hasParam("-sp"))
    {
        sport = std::stoi(parser("-sp"));
    }
    if (parser.hasParam("-gp"))
    {
        gmport = std::stoi(parser("-gp"));
    }

    YS_Signal::ignorePipe();
    YS_Signal::saveSigSegv();

    HttpApiArr router = {
        {"/v1/api/upload", upload}
    };

    YS_HttpMount serv("::", port);
    YS_HttpMount servSsl("::", sport);
    YS_HttpMount servGmssl("::", gmport);

    serv.mount(path);
    serv.route(router);

    servSsl.mount(path);
    servSsl.route(router);
    servSsl.setSslCtx(getSslCtx());

    servGmssl.mount(path);
    servGmssl.route(router);
    servGmssl.setSslCtx(getGmsslCtx());

    thread th1([&]() { serv.start(); });
    thread th2([&]() { servSsl.start(); });
    thread th3([&]() { servGmssl.start(); });

    while (YS_ProcCond::run(3000))
    {
    }

    return 0;
}