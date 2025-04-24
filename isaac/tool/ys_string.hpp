#ifndef _YS_STRING_H_
#define _YS_STRING_H_

#include <string>
#include <cstring>
#include <vector>

namespace isaac {

class YS_String
{
public:
    // sString 中的sSrc 用sDest替代
    static std::string replace(const std::string &sString, const std::string &sSrc, const std::string &sDest);

    // 获取dst中找到的第一段frm之后to之前的字符串
    static std::string findSubStr(const std::string &dst, const std::string &frm, const std::string &to, uint32_t dstOff = 0);

    // strstr usigned version
    static const uint8_t *ustrstr(const uint8_t *data, uint32_t len, const uint8_t *dst, uint32_t dstLen);

    // 分割字符串
    static std::vector<std::string> splitBy(const std::string &str, char split = ' ');

    // 拼字符串
    static std::string fmtVector(const std::vector<std::string> &vt, char split = ' ');
    static std::string tolower(const std::string &src);
    static std::string toupper(const std::string &src);

    // 字符串转换为hex格式 123 => 313233
    static std::string toHex(const uint8_t *data, size_t len);

    // 字符串转换为hex格式 123 => 313233
    static std::string toHex(const std::string &str) { return toHex((uint8_t *)str.c_str(), str.length()); }

    // 字符串转换为bin格式 313233 => 123 如果原始hex不对，转换结果不正确
    static void toBin(const std::string str, uint8_t *buf);

    // 字符串转换为bin格式 313233 => 123 如果原始hex不对，转换结果不正确
    static std::string toBin(const std::string data);
};

inline std::string YS_String::replace(const std::string &sString, const std::string &sSrc, const std::string &sDest)
{
    if (sSrc.empty()) return sString;

    std::string            sBuf = sString;
    std::string::size_type pos  = 0;

    while ((pos = sBuf.find(sSrc)) != std::string::npos)
    {
        sBuf.replace(pos, sSrc.length(), sDest);
        pos += sDest.length();
    }

    return sBuf;
}

inline std::string YS_String::findSubStr(const std::string &dst, const std::string &frm, const std::string &to, uint32_t dstOff)
{
    std::string sub;

    auto pos1 = dst.find(frm, dstOff);

    if (pos1 == std::string::npos) return sub;

    auto pos2 = dst.find(to, dstOff + pos1 + frm.length());
    if (pos2 == std::string::npos) return sub;

    sub = dst.substr(dstOff + pos1 + frm.length(), pos2 - pos1 - frm.length());
    return sub;
}

inline const uint8_t *YS_String::ustrstr(const uint8_t *data, uint32_t len, const uint8_t *dst, uint32_t dstLen)
{
    if (dstLen > len) return nullptr;

    for (size_t i = 0; i <= len - dstLen; i++)
    {
        if (data[i] == dst[0] && memcmp(data + i, dst, dstLen))
        {
            return data + i;
        }
    }

    return nullptr;
}

inline std::vector<std::string> YS_String::splitBy(const std::string &str, char c)
{
    std::vector<std::string> vtStr;
    size_t                   fm = 0;
    size_t                   to = 0;

    while (fm <= str.length())
    {
        to = str.find(c, fm);
        vtStr.emplace_back(str.substr(fm, to - fm));
        fm = to;
        if (fm != std::string::npos)
        {
            do
            {
                fm++;
            } while (fm < str.length() && str[fm] == c);
        }
    }

    return vtStr;
}

inline std::string YS_String::fmtVector(const std::vector<std::string> &vt, char split)
{
    std::string str;
    for (auto &&i : vt)
    {
        str.append(i).push_back(split);
    }
    if (!str.empty()) str.pop_back();
    return str;
}

inline std::string YS_String::tolower(const std::string &src)
{
    std::string dst;
    dst.reserve(src.length());
    for (auto &&c : src)
    {
        dst.push_back(::tolower(c));
    }
    return dst;
}

inline std::string YS_String::toupper(const std::string &src)
{
    std::string dst;
    dst.reserve(src.length());
    for (auto &&c : src)
    {
        dst.push_back(::toupper(c));
    }
    return dst;
}

inline std::string YS_String::toHex(const uint8_t *data, size_t len)
{
    std::string hex;
    hex.reserve(len * 2);
    for (size_t i = 0; i < len; i++)
    {
        char c1 = data[i] / 16;
        if (c1 <= 9)
        {
            hex += '0' + c1;
        }
        else
        {
            hex += 'A' - 10 + c1;
        }

        char c2 = data[i] % 16;
        if (c2 <= 9)
        {
            hex += '0' + c2;
        }
        else
        {
            hex += 'A' - 10 + c2;
        }
    }
    return hex;
}

inline void YS_String::toBin(const std::string str, uint8_t *buf)
{
    uint8_t highByte, lowByte;
    if (str.length() < 2)
    {
        return;
    }

    for (size_t i = 0; i < str.length(); i += 2)
    {
        highByte = ::toupper(str[i]);
        lowByte  = ::toupper(str[i + 1]);

        if (highByte > 0x39)
        {
            highByte -= 0x37;
        }
        else
        {
            highByte -= 0x30;
        }

        if (lowByte > 0x39)
        {
            lowByte -= 0x37;
        }
        else
        {
            lowByte -= 0x30;
        }

        buf[i / 2] = (highByte << 4) | lowByte;
    }
}

inline std::string YS_String::toBin(const std::string data)
{
    uint8_t *buf = new uint8_t[data.length()]();
    toBin(data, buf);
    std::string bin((const char *)buf, data.length() / 2);
    delete[] buf;
    return bin;
}

} // namespace isaac

#endif