#ifndef _YS_ID_H_
#define _YS_ID_H_
#include <cstdint>
#include <atomic>
#include <string>
namespace isaac {

class YS_Uid
{
public:
    static uint64_t genUid()
    {
        static std::atomic<uint64_t> _uid = {100000};
        return _uid++;
    }

    static std::string getUidStr()
    {
        return std::to_string(YS_Uid::genUid());
    }
};
} // namespace isaac

#endif