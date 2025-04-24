#pragma once
#ifndef _YS_PACK_TO_STR_H_
#define _YS_PACK_TO_STR_H_

#include <cstdint>
#include <vector>
#include <string>
#include "ys_ustr_ref.hpp"
#include "ys_buffer.hpp"
#include "ys_exception.hpp"

#if 0
#define PACK_NOTE YS_NOTE
#else
#define PACK_NOTE(...)
#endif

namespace isaac {

class YS_PackArgs
{
    static constexpr int32_t YPACK_ERR_DATA_LEN = 1;

public:
    template <typename arg1, typename... args>
    static void pack(uint8_t *_buf, uint32_t *_bufSz, size_t &off, arg1 &var1, args &...vars)
    {
        encode(var1, _buf, off);
        pack(_buf, _bufSz, off, vars...);
    }

    static void pack(uint8_t *_buf, uint32_t *_bufSz, size_t &off)
    {
        return;
    }

    template <typename arg1, typename... args>
    static void unpack(uint8_t *_data, uint32_t _dataLen, size_t &off, arg1 &var1, args &...vars)
    {
        decode(var1, _data, _dataLen, &off);
        unpack(_data, _dataLen, off, vars...);
    }

    static void unpack(uint8_t *_data, uint32_t _dataLen, size_t &off)
    {
        return;
    }

    static void encode(int32_t field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode int32_t");
        *(int32_t *)(buf + off) = field;
        off += sizeof(int32_t);
    }
    // 无符号数字
    static void encode(uint32_t field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode uint32_t");
        *(int32_t *)(buf + off) = field;
        *(uint32_t *)(buf + off) = field;
        off += sizeof(int32_t);
    }

    static void encode(uint64_t field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode uint64_t");
        *(uint64_t *)(buf + off) = field;
        off += sizeof(int64_t);
    }
    // char
    static void encode(char field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode char");
        *(buf + off) = field;
        off += sizeof(char);
    }
    // uint8_t
    static void encode(uint8_t field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode uint8_t");
        *(buf + off) = field;
        off += sizeof(char);
    }
    // float
    static void encode(float field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode float");
        *(float *)(buf + off) = field;
        off += sizeof(float);
    }
    // double
    static void encode(double field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode double");
        *(double *)(buf + off) = field;
        off += sizeof(double);
    }

    // bool
    static void encode(bool field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode bool");
        *(bool *)(buf + off) = field;
        off += sizeof(bool);
    }

    // 结构体数组(深拷贝)
    template <typename T, typename... K>
    static void encode_arr(T *field, size_t N, uint8_t *buf, size_t &off, K... arg)
    {
        memcpy(buf + off, field, sizeof(T) * N);
        off += sizeof(T) * N;
    }

    static void encode_arr(char *field, size_t N, uint8_t *buf, size_t &off)
    {
        memcpy(buf + off, field, N);
        off += N;
    }

    static void encode_arr(uint8_t *field, size_t N, uint8_t *buf, size_t &off)
    {
        memcpy(buf + off, field, N);
        off += N;
    }

    // 嵌套数组
    template <typename T, int32_t (T::*)(uint8_t *, uint32_t *) = &T::_ys_pack>
    static void encode_arr(T *field, size_t N, uint8_t *buf, size_t &off)
    {
        for (size_t i = 0; i < N; i++)
        {
            uint32_t uoff = 0;
            auto     ret  = field[i]._ys_pack(buf + off, &uoff);
            if (ret) throw ret;
            off += uoff;
        }
    }

    template <size_t N, typename T>
    static void encode(T (&field)[N], uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode array off = %d", off);
        T *pVal = field;
        encode_arr(pVal, N, buf, off);
        PACK_NOTE("encode array end off = %d", off);
    }

    // std::string (深拷贝)
    static void encode(std::string &field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode string");
        uint32_t len = field.length();
        encode(len, buf, off);
        memcpy(buf + off, field.c_str(), field.length());
        off += field.length();
    }

    // 长度必须已知
    static void pointer(std::pair<uint32_t, uint8_t *> &field, uint8_t *buf, size_t &off)
    {
        encode(field.first, buf, off);
        field.second = buf + off;
        off += field.first;
    }

    // 无符号字符串
    static void encode(std::pair<uint32_t, uint8_t *> &field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode pair<uint32_t, uint8_t *>");
        encode(field.first, buf, off);
        memcpy(buf + off, field.second, field.first);
        off += field.first;
    }

    // 长度必须已知
    static void pointer(YS_UstrRef &field, uint8_t *buf, size_t &off)
    {
        encode(size_t(field), buf, off);
        field.setVal(buf + off);
        off += size_t(field);
    }
    // 无符号字符串
    static void encode(YS_UstrRef &field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode YS_UstrRef");
        encode(size_t(field), buf, off);
        memcpy(buf + off, field, field);
        off += size_t(field);
    }

    template <typename T>
    static void encode_st(T &field, uint8_t *buf, size_t &off)
    {
        memcpy(buf + off, &field, sizeof(T));
        off += sizeof(T);
    }

    // 结构体指针
    template <typename T>
    static void encode(T *&field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode st*");
        memcpy(buf + off, field, sizeof(T));
        off += sizeof(T);
    }

    static void encode(char *&field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode char*");
        size_t len = strlen(field);
        memcpy(buf + off, field, len);
        off += len;
        buf[off] = 0;
        off += 1;
    }

    // 结构体(深拷贝)
    template <typename T, typename... K>
    static void encode(T &field, uint8_t *buf, size_t &off, K... arg)
    {
        PACK_NOTE("encode st");
        encode_st(field, buf, off, arg...);
    }

    // 嵌套
    template <typename T, int32_t (T::*)(uint8_t *, uint32_t *) = &T::_ys_pack>
    static void encode(T &field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode sub off = %d", off);
        uint32_t uoff = 0;
        auto     ret  = field._ys_pack(buf+off, &uoff);
        if (ret) throw ret;
        off += uoff;
        PACK_NOTE("encode sub end off = %d", off);
    }

    // 结构体(深拷贝)
    template <typename T, typename... K>
    static void encode(std::vector<T> &field, uint8_t *buf, size_t &off, K... arg)
    {
        PACK_NOTE("encode vector<T>");
        uint32_t size = field.size();
        encode(size, buf, off);
        for (auto &&i : field)
        {
            encode_st(i, buf, off, arg...);
        }
    }

    // 嵌套
    template <typename T, int32_t (T::*)(uint8_t *, uint32_t *) = &T::_ys_pack>
    static void encode(std::vector<T> &field, uint8_t *buf, size_t &off)
    {
        PACK_NOTE("encode vector<sub> off = %d", off);
        uint32_t size = field.size();
        encode(size, buf, off);
        for (auto &&i : field)
        {
            uint32_t uoff = 0;
            auto     ret  = i._ys_pack(buf + off, &uoff);
            if (ret) throw ret;
            off += uoff;
        }
        PACK_NOTE("encode vector<sub> end off = %d", off);
    }
    // int32_t
    static void decode(int32_t &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode int32_t");
        if (dataLen < *off + sizeof(int32_t))
            throw YPACK_ERR_DATA_LEN;
        val = *(int32_t *)(data + *off);
        *off += sizeof(int32_t);
    }
    // uint32_t
    static void decode(uint32_t &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode uint32_t");
        if (dataLen < *off + sizeof(uint32_t))
            throw YPACK_ERR_DATA_LEN;
        val = *(uint32_t *)(data + *off);
        *off += sizeof(uint32_t);
    }

    // uint64_t
    static void decode(uint64_t &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode uint64_t");
        if (dataLen < *off + sizeof(uint64_t))
            throw YPACK_ERR_DATA_LEN;
        val = *(uint64_t *)(data + *off);
        *off += sizeof(uint64_t);
    }
    // char
    static void decode(char &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode char");
        if (dataLen < *off + sizeof(char))
            throw YPACK_ERR_DATA_LEN;
        val = *(char *)(data + *off);
        *off += sizeof(char);
    }
    // uint8_t
    static void decode(uint8_t &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode uint8_t");
        if (dataLen < *off + sizeof(uint8_t))
            throw YPACK_ERR_DATA_LEN;
        val = *(uint8_t *)(data + *off);
        *off += sizeof(uint8_t);
    }
    // float
    static void decode(float &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode float");
        if (dataLen < *off + sizeof(float))
            throw YPACK_ERR_DATA_LEN;
        val = *(float *)(data + *off);
        *off += sizeof(float);
    }
    // double
    static void decode(double &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode double");
        if (dataLen < *off + sizeof(double))
            throw YPACK_ERR_DATA_LEN;
        val = *(double *)(data + *off);
        *off += sizeof(double);
    }
    // bool
    static void decode(bool &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode bool");
        if (dataLen < *off + sizeof(bool))
            throw YPACK_ERR_DATA_LEN;
        val = *(bool *)(data + *off);
        *off += sizeof(bool);
    }

    // 结构体数组 (深拷贝)
    template <typename K, typename... T>
    static void decode_arr(K *val, size_t N, uint8_t *data, uint32_t dataLen, size_t *off, T... arg)
    {
        PACK_NOTE("decode_arr T*");
        if (dataLen < *off + sizeof(K) * N)
            throw YPACK_ERR_DATA_LEN;
        memcpy((void *)val, data + *off, sizeof(K) * N);
        *off += sizeof(K) * N;
    }

    // 有符号定长字符串
    static void decode_arr(char *val, size_t N, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode_arr char*");
        if (dataLen < *off + N)
            throw YPACK_ERR_DATA_LEN;
        memcpy(val, data + *off, N);
        *off += N;
    }
    // 无符号定长字符串
    static void decode_arr(uint8_t *val, size_t N, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode_arr uint8_t*");
        if (dataLen < *off + N)
            throw YPACK_ERR_DATA_LEN;
        memcpy(val, data + *off, N);
        *off += N;
    }

    // 嵌套
    template <typename T, int32_t (T::*)(uint8_t *, uint32_t, uint32_t *) = &T::_ys_unpack>
    static void decode_arr(T *val, size_t N, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode_arr sub*");
        int32_t iRet;
        for (size_t i = 0; i < N; i++)
        {
            uint32_t uoff = 0;
            iRet          = val[i]._ys_unpack(data + *off, dataLen - *off, &uoff);
            if (iRet) throw iRet;
            *off += uoff;
        }
    }

    // 定长有符号字符串(深拷贝)
    template <size_t N, typename T>
    static void decode(T (&val)[N], uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode T&[N]");
        T *pVal = val;
        return decode_arr(pVal, N, data, dataLen, off);
    }

    // string(深拷贝)
    static void decode(std::string &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode string");
        uint32_t len;
        decode(len, data, dataLen, off);
        if (dataLen < *off + len)
            throw YPACK_ERR_DATA_LEN;
        val = std::string((char *)(data + *off), len);
        *off += len;
    }

    // 无符号字符串(浅拷贝)
    static void decode(std::pair<uint32_t, uint8_t *> &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode pair<uint32_t, uint8_t *>");
        decode(val.first, data, dataLen, off);
        if (dataLen < *off + val.first)
            throw YPACK_ERR_DATA_LEN;
        val.second = data + *off;
        *off += val.first;
    }

    // 无符号字符串(浅拷贝)
    static void decode(YS_UstrRef &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode YS_UstrRef");
        decode((size_t &)val, data, dataLen, off);
        if (dataLen < *off + size_t(val))
            throw YPACK_ERR_DATA_LEN;
        val.setVal(data + *off);
        *off += size_t(val);
    }

    // 结构体指针(浅拷贝)
    template <typename T>
    static void decode(T *&val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode T*");
        val = (T *)(data + *off);
        *off += sizeof(T);
        if (dataLen < *off)
            throw YPACK_ERR_DATA_LEN;
    }
    // 有符号字符串(浅拷贝)
    static void decode(char *&val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode char*");
        val = (char *)(data + *off);
        *off += strlen(val) + 1;
        if (dataLen < *off)
            throw YPACK_ERR_DATA_LEN;
    }

    template <typename T>
    static void decode_st(T &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        if (dataLen < *off + sizeof(T))
            throw YPACK_ERR_DATA_LEN;
        memcpy((void *)&val, data + *off, sizeof(T));
        *off += sizeof(T);
    }

    // 结构体 (深拷贝)
    template <typename K, typename... T>
    static void decode(K &val, uint8_t *data, uint32_t dataLen, size_t *off, T... arg)
    {
        PACK_NOTE("decode st");
        decode_st(val, data, dataLen, off, arg...);
    }

    // 嵌套
    template <typename T, int32_t (T::*)(uint8_t *, uint32_t, uint32_t *) = &T::_ys_unpack>
    static void decode(T &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode sub");
        uint32_t _uoff = 0;
        int32_t  iRet  = val._ys_unpack(data + *off, dataLen - *off, &_uoff);
        if (iRet) throw iRet;
        *off  += _uoff;
    }

    template <typename K, typename... T>
    static void decode(std::vector<K> &val, uint8_t *data, uint32_t dataLen, size_t *off, T... arg)
    {
        PACK_NOTE("decode vector<st>");
        uint32_t size;
        decode(size, data, dataLen, off);
        for (size_t i = 0; i < size; i++)
        {
            K tVal;
            decode_st(tVal, data, dataLen, off, arg...);
            val.emplace_back(tVal);
        }
    }

    // 嵌套
    template <typename T, int32_t (T::*)(uint8_t *, uint32_t, uint32_t *) = &T::_ys_unpack>
    static void decode(std::vector<T> &val, uint8_t *data, uint32_t dataLen, size_t *off)
    {
        PACK_NOTE("decode vector<sub>");
        uint32_t size;
        decode(size, data, dataLen, off);
        for (size_t i = 0; i < size; i++)
        {
            uint32_t _uoff = 0;
            T        tVal;
            int32_t  iRet = tVal._ys_unpack(data, dataLen, &_uoff);
            if (iRet) throw iRet;
            val.emplace_back(tVal);
            *off += _uoff;
        }
    }
};

// 注入一个打包解包函数
#define YS_PACK(...)                                                               \
    int32_t _ys_pack(uint8_t *_buf, uint32_t *_bufSz)                              \
    {                                                                              \
        size_t _off = 0;                                                           \
        try                                                                        \
        {                                                                          \
            isaac::YS_PackArgs::pack(_buf, _bufSz, _off, __VA_ARGS__);             \
        }                                                                          \
        catch (int32_t err)                                                        \
        {                                                                          \
            return err;                                                            \
        }                                                                          \
        *_bufSz = _off;                                                            \
        return 0;                                                                  \
    }                                                                              \
    int32_t _ys_unpack(uint8_t *_data, uint32_t _dataLen, uint32_t *off = nullptr) \
    {                                                                              \
        size_t _off = 0;                                                           \
        try                                                                        \
        {                                                                          \
            isaac::YS_PackArgs::unpack(_data, _dataLen, _off, __VA_ARGS__);        \
        }                                                                          \
        catch (int32_t err)                                                        \
        {                                                                          \
            return err;                                                            \
        }                                                                          \
        if (off) *off += _off;                                                     \
        return 0;                                                                  \
    }
#define YS_PACK_NULL()                                                           \
    int32_t _ys_pack(uint8_t *_buf, uint32_t *_bufSz)                            \
    {                                                                            \
        return 0;                                                                \
    }                                                                            \
    int32_t _ys_unpack(uint8_t *_data, uint32_t _dataLen, size_t *off = nullptr) \
    {                                                                            \
        return 0;                                                                \
    }

#define YS_PACK_HEADER 0xFFFF
#pragma pack(1)
struct YS_Packet
{
    uint16_t header;
    uint32_t dataLen;
    uint16_t cmd;
    int32_t  err;
    uint8_t  data[1];

    size_t headerLen() { return sizeof(YS_Packet) - 1; }
    size_t totalLen() { return sizeof(YS_Packet) - 1 + dataLen; }

    static int32_t isFullPkt(YS_Buf &buf)
    {
        if (buf.empty()) return buf.capacity();

        // 最小长度
        constexpr static size_t miniPkt = sizeof(YS_Packet) - 1;
        // 小于最小长度
        if (buf.length() < miniPkt) return -1;

        YS_Packet *pPkt = (YS_Packet *)buf.uData();
        // 包头对不上
        if (pPkt->header != YS_PACK_HEADER) return -1;

        // 不完整
        if (buf.length() < pPkt->totalLen()) return pPkt->totalLen() - buf.length();

        return 0;
    }
    static YS_Packet *toPkt(YS_Buf &buf) { return (YS_Packet *)buf.uData(); }

    // 结构体 序列化成 YS_Packet格式buf
    template <typename T>
    static void encode(T &obj, uint16_t cmd, YS_Buf &out)
    {
        YS_Packet *pPkt = (YS_Packet *)out.left();
        pPkt->header    = YS_PACK_HEADER;
        pPkt->cmd       = cmd;
        pPkt->err       = 0;
        pPkt->dataLen   = 0;
        obj._ys_pack(pPkt->data, &pPkt->dataLen);
        out.offset(pPkt->totalLen());
    }

    // YS_Packet buf 解包成结构体
    template <typename T>
    static int32_t decode(T &obj, YS_Buf &in)
    {
        YS_Packet *pPkt = (YS_Packet *)in.uData();
        return obj._ys_unpack(pPkt->data, pPkt->dataLen);
    }
};

#pragma pack()
} // namespace isaac

#endif