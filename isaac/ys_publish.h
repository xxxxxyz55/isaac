#ifndef __ys_publish_h__
#define __ys_publish_h__
#include "ys_pack_proto.h"

namespace isaac {

//发布订阅
class YS_Publish : public YS_TcpPackServer
{
public:
    // listen on
    // add channl
    void publish(std::string channl, std::string data)
    {
    }
};

class YS_Subscribe : YS_TcpPackClient
{
public:
};

} // namespace isaac

#endif