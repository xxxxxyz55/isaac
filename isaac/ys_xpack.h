#ifndef _YS_XPACK_H_
#define _YS_XPACK_H_

#include "xpack/json.h"
#include "ys_cli_log.h"
#include "ys_handle.h"

namespace isaac {

class YS_Xpack
{
public:
    template <typename T>
    static std::string objToJsonStr(T &obj)
    {
        try
        {
            return xpack::json::encode(obj);
        }
        catch (const std::exception &e)
        {
            CLI_ERROR("xpack encode fail.");
            return "";
        }
    }

    template <typename T>
    static void jsonStrToObj(std::string &sStr, T &obj)
    {
        try
        {
            xpack::json::decode(sStr, obj);
        }
        catch (const std::exception &e)
        {
            CLI_ERROR("xpack decode fail.");
        }
    }

    // 简单判断json字符串结束
    static int32_t isComplete(YS_Buf &buf, isaac::YS_PktCtx *arg)
    {
        uint32_t left  = 0;
        uint32_t right = 0;
        for (size_t i = 0; i < buf.length(); i++)
        {
            if (buf.sData()[i] == '{')
            {
                left++;
            }
            else if (buf.sData()[i] == '}')
            {
                right++;
            }
        }
        if (left == 0 || left < right)
        {
            return -1;
        }
        else if (left == right)
        {
            return 0;
        }
        else
        {
            return 2048;
        }
    }
};
} // namespace isaac
#endif