#ifndef _YS_BASE64_H_
#define _YS_BASE64_H_

#include <string>

namespace isaac {

class YS_Base64
{
public:
    // base64编码
    static std::string encode(const std::string &in);
    // base64解码
    static std::string decode(const std::string &in);
};

inline std::string YS_Base64::encode(const std::string &in)
{
    std::string out;
    out.reserve(in.length());

    int           s;
    unsigned int  i;
    unsigned int  j;
    unsigned char c;
    unsigned char l;

    static const char base64en[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'};

    s = 0;
    l = 0;
    for (i = j = 0; i < in.length(); i++)
    {
        c = in[i];

        switch (s)
        {
        case 0:
            s = 1;
            out.push_back(base64en[(c >> 2) & 0x3F]);
            j++;
            break;
        case 1:
            s = 2;
            out.push_back(base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)]);
            j++;
            break;
        case 2:
            s = 0;
            out.push_back(base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)]);
            j++;
            out.push_back(base64en[c & 0x3F]);
            j++;
            break;
        }
        l = c;
    }

    switch (s)
    {
    case 1:
        out.push_back(base64en[(l & 0x3) << 4]);
        j++;
        out.push_back('=');
        j++;
        out.push_back('=');
        j++;
        break;
    case 2:
        out.push_back(base64en[(l & 0xF) << 2]);
        j++;
        out.push_back('=');
        j++;
        break;
    }

    return out;
}

inline std::string YS_Base64::decode(const std::string &in)
{
    std::string out;
    out.reserve(in.length() * 2);
    unsigned int  i;
    unsigned int  j;
    unsigned char c;

    static const unsigned char base64de[] = {
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 62, 255, 255, 255, 63,
        52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 255, 255, 255, 255, 255, 255,
        255, 0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22,
        23, 24, 25, 255, 255, 255, 255, 255,
        255, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, 255, 255, 255, 255, 255};

    if (in.length() & 0x3)
    {
        return out;
    }

    for (i = j = 0; i < in.length(); i++)
    {
        if (in[i] == '=')
        {
            break;
        }
        if (in[i] < '+' || in[i] > 'z')
        {
            return 0;
        }

        c = base64de[(unsigned char)in[i]];
        if (c == 255)
        {
            return 0;
        }

        switch (i & 0x3)
        {
        case 0:
            out.push_back((c << 2) & 0xFF);
            break;
        case 1:
            out[j++] |= (c >> 4) & 0x3;
            out.push_back((c & 0xF) << 4);
            break;
        case 2:
            out[j++] |= (c >> 2) & 0xF;
            out.push_back((c & 0x3) << 6);
            break;
        case 3:
            out[j++] |= c;
            break;
        }
    }
    for (size_t i = 0; i < out.length() - j; i++)
    {
        out.pop_back();
    }

    return out;
}

} // namespace isaac

#endif