#include "ys_http_server.h"

using namespace std;
using namespace isaac;

void print(YS_HttpRequest &tReq, YS_HttpResponse &tResp)
{
    tReq.print();
    tResp.setResponse(http_status::HTTP_STATUS_NOT_FOUND, "");
}

int32_t main(int32_t argc, char const *argv[])
{
    YS_HttpServer serv("::", 911);
    serv.route({
        {"*", print}
    });
    serv.start();
    return 0;
}