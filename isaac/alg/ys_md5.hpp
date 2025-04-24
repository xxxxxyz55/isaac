#ifndef __YS_MD5_H__
#define __YS_MD5_H__
#include <cstdint>
#include <memory>
#include <cstring>
#include <cstdio>

namespace isaac {

class YS_Md5
{
public:
    YS_Md5() { init(); }

    void update(const uint8_t *in, uint32_t len);
    void final(uint8_t out[16]);

    static void md5(const uint8_t *in, uint32_t len, uint8_t out[16]);

private:
    uint32_t _h[4]     = {};
    uint8_t  _data[64] = {};
    uint32_t _nl       = 0;
    uint32_t _nh       = 0;
    uint32_t _num      = 0;

    static void     compress(uint32_t state[4], const uint8_t *data, uint32_t blockCnt);
    void            init();
    static uint32_t CRYPTO_load_u32_le(const void *in);
    static uint32_t CRYPTO_rotl_u32(uint32_t value, int shift);
    static void     CRYPTO_store_u32_le(void *out, uint32_t v);
};

inline void YS_Md5::CRYPTO_store_u32_le(void *out, uint32_t v)
{
    memcpy(out, &v, sizeof(v));
}

inline uint32_t YS_Md5::CRYPTO_load_u32_le(const void *in)
{
    uint32_t v;
    memcpy(&v, in, sizeof(v));
    return v;
}

inline void YS_Md5::compress(uint32_t state[4], const uint8_t *data, uint32_t num)
{
    uint32_t A, B, C, D;
    uint32_t XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7, XX8, XX9, XX10, XX11, XX12,
        XX13, XX14, XX15;
#ifdef X
#undef X
#endif
#define X(i)       XX##i
#define F(b, c, d) ((((c) ^ (d)) & (b)) ^ (d))
#define G(b, c, d) ((((b) ^ (c)) & (d)) ^ (c))
#define H(b, c, d) ((b) ^ (c) ^ (d))
#define I(b, c, d) (((~(d)) | (b)) ^ (c))
#define R0(a, b, c, d, k, s, t)                \
    do                                         \
    {                                          \
        (a) += ((k) + (t) + F((b), (c), (d))); \
        (a) = CRYPTO_rotl_u32(a, s);           \
        (a) += (b);                            \
    } while (0)

#define R1(a, b, c, d, k, s, t)                \
    do                                         \
    {                                          \
        (a) += ((k) + (t) + G((b), (c), (d))); \
        (a) = CRYPTO_rotl_u32(a, s);           \
        (a) += (b);                            \
    } while (0)

#define R2(a, b, c, d, k, s, t)                \
    do                                         \
    {                                          \
        (a) += ((k) + (t) + H((b), (c), (d))); \
        (a) = CRYPTO_rotl_u32(a, s);           \
        (a) += (b);                            \
    } while (0)

#define R3(a, b, c, d, k, s, t)                \
    do                                         \
    {                                          \
        (a) += ((k) + (t) + I((b), (c), (d))); \
        (a) = CRYPTO_rotl_u32(a, s);           \
        (a) += (b);                            \
    } while (0)

    A = state[0];
    B = state[1];
    C = state[2];
    D = state[3];

    for (; num--;)
    {
        X(0) = CRYPTO_load_u32_le(data);
        data += 4;
        X(1) = CRYPTO_load_u32_le(data);
        data += 4;
        // Round 0
        R0(A, B, C, D, X(0), 7, 0xd76aa478L);
        X(2) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(D, A, B, C, X(1), 12, 0xe8c7b756L);
        X(3) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(C, D, A, B, X(2), 17, 0x242070dbL);
        X(4) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(B, C, D, A, X(3), 22, 0xc1bdceeeL);
        X(5) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(A, B, C, D, X(4), 7, 0xf57c0fafL);
        X(6) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(D, A, B, C, X(5), 12, 0x4787c62aL);
        X(7) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(C, D, A, B, X(6), 17, 0xa8304613L);
        X(8) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(B, C, D, A, X(7), 22, 0xfd469501L);
        X(9) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(A, B, C, D, X(8), 7, 0x698098d8L);
        X(10) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(D, A, B, C, X(9), 12, 0x8b44f7afL);
        X(11) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(C, D, A, B, X(10), 17, 0xffff5bb1L);
        X(12) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(B, C, D, A, X(11), 22, 0x895cd7beL);
        X(13) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(A, B, C, D, X(12), 7, 0x6b901122L);
        X(14) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(D, A, B, C, X(13), 12, 0xfd987193L);
        X(15) = CRYPTO_load_u32_le(data);
        data += 4;
        R0(C, D, A, B, X(14), 17, 0xa679438eL);
        R0(B, C, D, A, X(15), 22, 0x49b40821L);
        // Round 1
        R1(A, B, C, D, X(1), 5, 0xf61e2562L);
        R1(D, A, B, C, X(6), 9, 0xc040b340L);
        R1(C, D, A, B, X(11), 14, 0x265e5a51L);
        R1(B, C, D, A, X(0), 20, 0xe9b6c7aaL);
        R1(A, B, C, D, X(5), 5, 0xd62f105dL);
        R1(D, A, B, C, X(10), 9, 0x02441453L);
        R1(C, D, A, B, X(15), 14, 0xd8a1e681L);
        R1(B, C, D, A, X(4), 20, 0xe7d3fbc8L);
        R1(A, B, C, D, X(9), 5, 0x21e1cde6L);
        R1(D, A, B, C, X(14), 9, 0xc33707d6L);
        R1(C, D, A, B, X(3), 14, 0xf4d50d87L);
        R1(B, C, D, A, X(8), 20, 0x455a14edL);
        R1(A, B, C, D, X(13), 5, 0xa9e3e905L);
        R1(D, A, B, C, X(2), 9, 0xfcefa3f8L);
        R1(C, D, A, B, X(7), 14, 0x676f02d9L);
        R1(B, C, D, A, X(12), 20, 0x8d2a4c8aL);
        // Round 2
        R2(A, B, C, D, X(5), 4, 0xfffa3942L);
        R2(D, A, B, C, X(8), 11, 0x8771f681L);
        R2(C, D, A, B, X(11), 16, 0x6d9d6122L);
        R2(B, C, D, A, X(14), 23, 0xfde5380cL);
        R2(A, B, C, D, X(1), 4, 0xa4beea44L);
        R2(D, A, B, C, X(4), 11, 0x4bdecfa9L);
        R2(C, D, A, B, X(7), 16, 0xf6bb4b60L);
        R2(B, C, D, A, X(10), 23, 0xbebfbc70L);
        R2(A, B, C, D, X(13), 4, 0x289b7ec6L);
        R2(D, A, B, C, X(0), 11, 0xeaa127faL);
        R2(C, D, A, B, X(3), 16, 0xd4ef3085L);
        R2(B, C, D, A, X(6), 23, 0x04881d05L);
        R2(A, B, C, D, X(9), 4, 0xd9d4d039L);
        R2(D, A, B, C, X(12), 11, 0xe6db99e5L);
        R2(C, D, A, B, X(15), 16, 0x1fa27cf8L);
        R2(B, C, D, A, X(2), 23, 0xc4ac5665L);
        // Round 3
        R3(A, B, C, D, X(0), 6, 0xf4292244L);
        R3(D, A, B, C, X(7), 10, 0x432aff97L);
        R3(C, D, A, B, X(14), 15, 0xab9423a7L);
        R3(B, C, D, A, X(5), 21, 0xfc93a039L);
        R3(A, B, C, D, X(12), 6, 0x655b59c3L);
        R3(D, A, B, C, X(3), 10, 0x8f0ccc92L);
        R3(C, D, A, B, X(10), 15, 0xffeff47dL);
        R3(B, C, D, A, X(1), 21, 0x85845dd1L);
        R3(A, B, C, D, X(8), 6, 0x6fa87e4fL);
        R3(D, A, B, C, X(15), 10, 0xfe2ce6e0L);
        R3(C, D, A, B, X(6), 15, 0xa3014314L);
        R3(B, C, D, A, X(13), 21, 0x4e0811a1L);
        R3(A, B, C, D, X(4), 6, 0xf7537e82L);
        R3(D, A, B, C, X(11), 10, 0xbd3af235L);
        R3(C, D, A, B, X(2), 15, 0x2ad7d2bbL);
        R3(B, C, D, A, X(9), 21, 0xeb86d391L);

        A = state[0] += A;
        B = state[1] += B;
        C = state[2] += C;
        D = state[3] += D;
    }
#undef X
#undef F
#undef G
#undef H
#undef I
#undef R0
#undef R1
#undef R2
#undef R3
}

inline uint32_t YS_Md5::CRYPTO_rotl_u32(uint32_t value, int shift)
{
#if defined(_MSC_VER)
    return _rotl(value, shift);
#else
    return (value << shift) | (value >> ((-shift) & 31));
#endif
}

inline void YS_Md5::init()
{
    _h[0] = 0x67452301UL;
    _h[1] = 0xefcdab89UL;
    _h[2] = 0x98badcfeUL;
    _h[3] = 0x10325476UL;
}
inline void YS_Md5::update(const uint8_t *in, uint32_t len)
{
    if (len == 0) return;

    uint32_t l = _nl + (((uint32_t)len) << 3);
    if (l < _nl)
    {
        _nh++;
    }
    _nh += (uint32_t)(len >> 29);
    _nl = l;

    size_t n = _num;
    if (n != 0)
    {
        if (len >= 64 || len + n >= 64)
        {
            memcpy(_data + n, in, 64 - n);
            compress(_h, _data, 1);
            n = 64 - n;
            in += n;
            len -= n;
            _num = 0;
            memset(_data, 0, 64);
        }
        else
        {
            memcpy(_data + n, in, len);
            _num += (unsigned)len;
            return;
        }
    }

    n = len / 64;
    if (n > 0)
    {
        compress(_h, in, n);
        n *= 64;
        in += n;
        len -= n;
    }

    if (len != 0)
    {
        _num = (unsigned)len;
        memcpy(_data, in, len);
    }
}

inline void YS_Md5::final(uint8_t out[16])
{
    size_t n = _num;
    _data[n] = 0x80;
    n++;

    if (n > 64 - 8)
    {
        memset(_data + n, 0, 64 - n);
        n = 0;
        compress(_h, _data, 1);
    }
    memset(_data + n, 0, 64 - 8 - n);

    CRYPTO_store_u32_le(_data + 64 - 8, _nl);
    CRYPTO_store_u32_le(_data + 64 - 4, _nh);
    compress(_h, _data, 1);
    _num = 0;
    memset(_data, 0, 64);
    CRYPTO_store_u32_le(out, _h[0]);
    CRYPTO_store_u32_le(out + 4, _h[1]);
    CRYPTO_store_u32_le(out + 8, _h[2]);
    CRYPTO_store_u32_le(out + 12, _h[3]);
}

inline void YS_Md5::md5(const uint8_t *in, uint32_t len, uint8_t out[16])
{
    YS_Md5 ctx;
    ctx.update(in, len);
    ctx.final(out);
}
} // namespace isaac

#endif