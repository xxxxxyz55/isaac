#include "ys_http_server.h"
#include "tool/ys_argh.hpp"

using namespace isaac;
using namespace std;


int main(int argc, char const *argv[])
{
    YS_ArgParser parser(argv);
    if (parser.hasFlag("--version"))
    {
        printf("1.0\n");
        return 0;
    }

    if (parser.hasFlag("-h"))
    {
        printf("-p  port\n");
    }
    uint16_t port = 8080;

    if(parser.hasParam("-p"))
    {
        port = stoi(parser("-p"));
    }

    YS_HttpServer serv("::", port);
    serv.route({
        {"*", [](YS_HttpRequest &req, YS_HttpResponse &resp) {
             resp.setResponse(http_status::HTTP_STATUS_OK, req.getBody());
         }}
    });
    serv.start();
    return 0;
}
