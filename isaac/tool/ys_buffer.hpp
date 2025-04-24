#pragma once
#ifndef _YS_BUF_H_
#define _YS_BUF_H_
#include <cstdint>
#include <string>
#include <cstring>

namespace isaac {

// 可扩展的缓冲区，仅append支持自动拓展,自己写入需提前申请足够大小
class YS_Buf
{
public:
    static constexpr size_t size32KB() { return 1024 * 32; }

    // 无符号字符串到结构
    YS_Buf(const uint8_t *data, size_t len) { append(data, len); }

    // 空结构
    YS_Buf(){};

    // 预申请空间的空结构
    YS_Buf(size_t len) { alloc(len); }

    // 拷贝构造
    YS_Buf(YS_Buf &buf);

    // 析构
    ~YS_Buf();

    // 块大小
    static constexpr size_t blockSize() { return 1024; }

    // 容量
    size_t capacity() { return _cap; }

    // 可写首地址
    uint8_t *left() { return _buf + _len; }

    // 可写大小
    size_t leftSize() { return _cap - _len; }

    // 指定长度偏移,不要超过capacity
    void offset(int32_t off) { _len += off; }

    // 直接设置长度
    void setLength(size_t length) { _len = length; }

    // 扩展buffer
    void alloc(size_t len = blockSize());

    // 清空buffer
    void clear() { _len = 0; }

    // 数据长度
    size_t length() { return _len; }
    bool   empty() { return _len == 0; }
    bool   full() { return _len == _cap; }

    // 数据地址
    uint8_t *uData() { return _buf; }

    // 数据地址 满的时候会扩展添加\0
    char *sData();

    // 写入数据
    void append(const std::string &str) { append((uint8_t *)str.c_str(), str.length()); }

    // 写入数据
    void append(const uint8_t *data, size_t len);
    void append(const char *data, size_t len) { append((const uint8_t *)data, strlen(data)); };

private:
    uint8_t *_buf = nullptr;
    size_t   _len = 0;
    size_t   _cap = 0;
};

inline YS_Buf::YS_Buf(YS_Buf &buf)
{
    alloc(buf._cap);
    append(buf._buf, buf._len);
}

inline YS_Buf::~YS_Buf()
{
    if (_buf)
    {
        delete[] _buf;
        _buf = nullptr;
    }
}

inline void YS_Buf::alloc(size_t len)
{
    if (len == 0) return;
    len = ((len - 1) / blockSize() + 1) * blockSize();

    uint8_t *pBuf = _buf;
    _cap += len;
    _buf = new uint8_t[_cap];
    if (pBuf)
    {
        memcpy(_buf, pBuf, _len);
        delete[] pBuf;
    }
}

inline char *YS_Buf::sData()
{
    if (_buf)
    {
        if (_len == _cap) alloc();
        _buf[_len] = 0;
    }
    return (char *)_buf;
}

inline void YS_Buf::append(const uint8_t *data, size_t len)
{
    while (leftSize() < len)
    {
        alloc(leftSize() - len);
    }
    memcpy(left(), data, len);
    _len += len;
}

} // namespace isaac

#endif