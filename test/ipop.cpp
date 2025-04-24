#include "tool/ys_argh.hpp"
#include "ys_udp.h"

using namespace isaac;
using namespace std;

int32_t main(int32_t argc, char const *argv[])
{
    YS_ArgParser parser(argv);

    YS_Udp udp;

    YS_EndPoint ed(parser({"-host"}), std::stoi(parser("-port")));
    string      data = parser("-in");
    if (udp.sendTo(data.c_str(), data.length(), ed))
    {
        CLI_ERROR("send fail.");
    }
    else
    {
        CLI_DEBUG("send : {}", data);
    }

    YS_Buf buf(YS_Buf::size32KB());
    udp.setBlock(false);
    if (YS_Util::canRead(udp.getFd(), 3000))
    {
        if (udp.recv(buf))
        {
            CLI_ERROR("recv fail.");
        }
        else
        {
            CLI_DEBUG("recv : {}", buf.sData());
        }
    }
    else
    {
        CLI_DEBUG("recv timeout.");
    }
    return 0;
}
