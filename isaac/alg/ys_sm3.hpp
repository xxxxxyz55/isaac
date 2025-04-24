#ifndef _YS_SM3_AVX2_H_
#define _YS_SM3_AVX2_H_

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "ys_rol32.hpp"

#if YS_MAVX2
#if __linux__
#include <x86intrin.h>
#endif
#include <immintrin.h>

#define _mm_rotl_epi32(X, i) \
    _mm_xor_si128(_mm_slli_epi32((X), (i)), _mm_srli_epi32((X), 32 - (i)))

#endif

namespace isaac {

class YS_Sm3
{
#define SM3_MD_LEN 32
public:
    // 计算sm3
    static int32_t sm3(const uint8_t *data, uint32_t dataLen, uint8_t hash[SM3_MD_LEN]);

    void init();
    void update(const uint8_t *data, uint32_t data_len);
    void final(uint8_t digest[SM3_MD_LEN]);

private:
    uint32_t _digest[8];
    uint64_t _nblocks;
    uint8_t  _block[64];
    int32_t  _num;

    static void sm3_compress_blocks(uint32_t             digest[8],
                                    const unsigned char *data, size_t blocks);

    void sm3_compress(uint32_t digest[8], const unsigned char block[64])
    {
        return sm3_compress_blocks(digest, block, 1);
    }
};

#define SM3_BLOCK_SIZE 64
#define ROTL(x, n)     (((x) << (n)) | ((x) >> (32 - (n))))
#define P0(x)          ((x) ^ ROL32((x), 9) ^ ROL32((x), 17))
#define P1(x)          ((x) ^ ROL32((x), 15) ^ ROL32((x), 23))

#define FF00(x, y, z) ((x) ^ (y) ^ (z))
#define FF16(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG00(x, y, z) ((x) ^ (y) ^ (z))
#define GG16(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))

#define R(A, B, C, D, E, F, G, H, xx)                    \
    SS1 = ROL32((ROL32(A, 12) + E + K[j]), 7);           \
    SS2 = SS1 ^ ROL32(A, 12);                            \
    TT1 = FF##xx(A, B, C) + D + SS2 + (W[j] ^ W[j + 4]); \
    TT2 = GG##xx(E, F, G) + H + SS1 + W[j];              \
    B   = ROL32(B, 9);                                   \
    H   = TT1;                                           \
    F   = ROL32(F, 19);                                  \
    D   = P0(TT2);                                       \
    j++

#define R8(A, B, C, D, E, F, G, H, xx) \
    R(A, B, C, D, E, F, G, H, xx);     \
    R(H, A, B, C, D, E, F, G, xx);     \
    R(G, H, A, B, C, D, E, F, xx);     \
    R(F, G, H, A, B, C, D, E, xx);     \
    R(E, F, G, H, A, B, C, D, xx);     \
    R(D, E, F, G, H, A, B, C, xx);     \
    R(C, D, E, F, G, H, A, B, xx);     \
    R(B, C, D, E, F, G, H, A, xx)

inline int32_t YS_Sm3::sm3(const uint8_t *data, uint32_t dataLen, uint8_t hash[SM3_MD_LEN])
{
    YS_Sm3 ctx;
    ctx.init();
    ctx.update(data, dataLen);
    ctx.final(hash);
    return 0;
}

inline void YS_Sm3::init()
{
    memset(this, 0, sizeof(*this));
    _digest[0] = 0x7380166F;
    _digest[1] = 0x4914B2B9;
    _digest[2] = 0x172442D7;
    _digest[3] = 0xDA8A0600;
    _digest[4] = 0xA96F30BC;
    _digest[5] = 0x163138AA;
    _digest[6] = 0xE38DEE4D;
    _digest[7] = 0xB0FB0E4E;
}

inline void YS_Sm3::update(const uint8_t *data, uint32_t data_len)
{
    size_t blocks;

    if (_num)
    {
        unsigned int left = SM3_BLOCK_SIZE - _num;
        if (data_len < left)
        {
            memcpy(_block + _num, data, data_len);
            _num += data_len;
            return;
        }
        else
        {
            memcpy(_block + _num, data, left);
            sm3_compress_blocks(_digest, _block, 1);
            _nblocks++;
            data += left;
            data_len -= left;
        }
    }

    blocks = data_len / SM3_BLOCK_SIZE;
    sm3_compress_blocks(_digest, data, blocks);
    _nblocks += blocks;
    data += SM3_BLOCK_SIZE * blocks;
    data_len -= SM3_BLOCK_SIZE * blocks;

    _num = data_len;
    if (data_len)
    {
        memcpy(_block, data, data_len);
    }
}

inline void YS_Sm3::final(uint8_t digest[SM3_MD_LEN])
{
    int i;

    _block[_num] = 0x80;

    if (_num + 9 <= SM3_BLOCK_SIZE)
    {
        memset(_block + _num + 1, 0, SM3_BLOCK_SIZE - _num - 9);
    }
    else
    {
        memset(_block + _num + 1, 0, SM3_BLOCK_SIZE - _num - 1);
        sm3_compress(_digest, _block);
        memset(_block, 0, SM3_BLOCK_SIZE - 8);
    }
    PUTU32(_block + 56, _nblocks >> 23);
    PUTU32(_block + 60, (_nblocks << 9) + (_num << 3));

    sm3_compress(_digest, _block);
    for (i = 0; i < 8; i++)
    {
        PUTU32(digest + i * 4, _digest[i]);
    }
}

inline void YS_Sm3::sm3_compress_blocks(uint32_t             digest[8],
                                 const unsigned char *data, size_t blocks)
{
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
    uint32_t E;
    uint32_t F;
    uint32_t G;
    uint32_t H;
    uint32_t W[68];
    uint32_t SS1, SS2, TT1, TT2;
    int      j;

    static const uint32_t K[64] = {
        0x79cc4519U, 0xf3988a32U, 0xe7311465U, 0xce6228cbU,
        0x9cc45197U, 0x3988a32fU, 0x7311465eU, 0xe6228cbcU,
        0xcc451979U, 0x988a32f3U, 0x311465e7U, 0x6228cbceU,
        0xc451979cU, 0x88a32f39U, 0x11465e73U, 0x228cbce6U,
        0x9d8a7a87U, 0x3b14f50fU, 0x7629ea1eU, 0xec53d43cU,
        0xd8a7a879U, 0xb14f50f3U, 0x629ea1e7U, 0xc53d43ceU,
        0x8a7a879dU, 0x14f50f3bU, 0x29ea1e76U, 0x53d43cecU,
        0xa7a879d8U, 0x4f50f3b1U, 0x9ea1e762U, 0x3d43cec5U,
        0x7a879d8aU, 0xf50f3b14U, 0xea1e7629U, 0xd43cec53U,
        0xa879d8a7U, 0x50f3b14fU, 0xa1e7629eU, 0x43cec53dU,
        0x879d8a7aU, 0x0f3b14f5U, 0x1e7629eaU, 0x3cec53d4U,
        0x79d8a7a8U, 0xf3b14f50U, 0xe7629ea1U, 0xcec53d43U,
        0x9d8a7a87U, 0x3b14f50fU, 0x7629ea1eU, 0xec53d43cU,
        0xd8a7a879U, 0xb14f50f3U, 0x629ea1e7U, 0xc53d43ceU,
        0x8a7a879dU, 0x14f50f3bU, 0x29ea1e76U, 0x53d43cecU,
        0xa7a879d8U, 0x4f50f3b1U, 0x9ea1e762U, 0x3d43cec5U};
#if YS_AVX2
    __m128i X, T, R;
    __m128i M = _mm_setr_epi32(0, 0, 0, 0xffffffff);
    __m128i V = _mm_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);
#endif

    while (blocks--)
    {
        A = digest[0];
        B = digest[1];
        C = digest[2];
        D = digest[3];
        E = digest[4];
        F = digest[5];
        G = digest[6];
        H = digest[7];
#if YS_AVX2
        for (j = 0; j < 16; j += 4)
        {
            X = _mm_loadu_si128((__m128i *)(data + j * 4));
            X = _mm_shuffle_epi8(X, V);
            _mm_storeu_si128((__m128i *)(W + j), X);
        }

        for (j = 16; j < 68; j += 4)
        {
            /* X = (W[j - 3], W[j - 2], W[j - 1], 0) */
            X = _mm_loadu_si128((__m128i *)(W + j - 3));
            X = _mm_andnot_si128(M, X);

            X = _mm_rotl_epi32(X, 15);
            T = _mm_loadu_si128((__m128i *)(W + j - 9));
            X = _mm_xor_si128(X, T);
            T = _mm_loadu_si128((__m128i *)(W + j - 16));
            X = _mm_xor_si128(X, T);

            /* P1() */
            T = _mm_rotl_epi32(X, (23 - 15));
            T = _mm_xor_si128(T, X);
            T = _mm_rotl_epi32(T, 15);
            X = _mm_xor_si128(X, T);

            T = _mm_loadu_si128((__m128i *)(W + j - 13));
            T = _mm_rotl_epi32(T, 7);
            X = _mm_xor_si128(X, T);
            T = _mm_loadu_si128((__m128i *)(W + j - 6));
            X = _mm_xor_si128(X, T);

            /* W[j + 3] ^= P1(ROL32(W[j + 1], 15)) */
            R = _mm_shuffle_epi32(X, 0);
            R = _mm_and_si128(R, M);
            T = _mm_rotl_epi32(R, 15);
            T = _mm_xor_si128(T, R);
            T = _mm_rotl_epi32(T, 9);
            R = _mm_xor_si128(R, T);
            R = _mm_rotl_epi32(R, 6);
            X = _mm_xor_si128(X, R);

            _mm_storeu_si128((__m128i *)(W + j), X);
        }
#else
        for (j = 0; j < 16; j++)
            W[j] = GETU32(data + j * 4);

        for (; j < 68; j++)
            W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROL32(W[j - 3], 15)) ^ ROL32(W[j - 13], 7) ^ W[j - 6];
#endif
        j = 0;
        R8(A, B, C, D, E, F, G, H, 00);
        R8(A, B, C, D, E, F, G, H, 00);
        R8(A, B, C, D, E, F, G, H, 16);
        R8(A, B, C, D, E, F, G, H, 16);
        R8(A, B, C, D, E, F, G, H, 16);
        R8(A, B, C, D, E, F, G, H, 16);
        R8(A, B, C, D, E, F, G, H, 16);
        R8(A, B, C, D, E, F, G, H, 16);

        digest[0] ^= A;
        digest[1] ^= B;
        digest[2] ^= C;
        digest[3] ^= D;
        digest[4] ^= E;
        digest[5] ^= F;
        digest[6] ^= G;
        digest[7] ^= H;

        data += 64;
    }
}

} // namespace isaac

#undef _mm_rotl_epi32
#undef ROTL
#undef P0
#undef P1
#undef FF00
#undef FF16
#undef GG00
#undef GG16
#undef R
#undef R8
#undef SM3_BLOCK_SIZE

#endif