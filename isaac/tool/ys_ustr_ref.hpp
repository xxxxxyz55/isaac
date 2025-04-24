#ifndef _YS_USTR_REF_H_
#define _YS_USTR_REF_H_

#include <cstdint>
#include <cstring>
#include <string>

namespace isaac {

// 无符号字符串引用
class YS_UstrRef
{
private:
    size_t   _len;
    uint8_t *_str;

public:
    YS_UstrRef(size_t len, uint8_t *str) : _len(len), _str(str) {}
    YS_UstrRef() : _len(0), _str(nullptr) {}
    // read only
    YS_UstrRef(std::string &str) : _len(str.length()), _str((uint8_t *)str.data()) {}

    operator uint8_t *() { return _str; }
    operator char *() { return (char *)_str; }
    operator void *() { return _str; }
    operator size_t &() { return _len; }
    size_t   length() { return _len; }
    uint8_t &operator[](size_t pos) { return _str[pos]; }

    // 比较
    bool operator==(YS_UstrRef &dst)
    {
        if (dst._len != _len) return false;
        if (memcmp(dst, *this, dst)) return false;

        return true;
    }

    bool operator!=(YS_UstrRef &dst) { return !operator==(dst); }

    // 设置长度
    void setLen(size_t len) { _len = len; }
    // 设置值
    void setVal(uint8_t *str) { _str = str; }

    // 设置字符串
    void set(size_t len, uint8_t *str)
    {
        _len = len;
        _str = str;
    }
};
} // namespace isaac

#endif