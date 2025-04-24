#ifndef _YS_ISLAM_H_
#define _YS_ISLAM_H_

#include "ys_base64.hpp"
#include <cstdint>
#include "ys_rol32.hpp"
#include <memory>

#if YS_MAVX2

#if __linux__
#include <x86intrin.h>
#endif
#include <immintrin.h>
#endif

// copy from gmssl
namespace isaac {
// sm4 变种
class YS_Islam
{
public:
    static int32_t encrypt(std::string &data, std::string &out, const std::string &pin = "0000");
    static int32_t decrypt(std::string &enc, std::string &out, const std::string &pin = "0000");
    static void    encrypt(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t *outLen, const uint8_t key[16]);
    static void    decrypt(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t *outLen, const uint8_t key[16]);

private:
    typedef struct
    {
        uint32_t rk[32];
    } sms4_key_t;

    static uint32_t getFK(int32_t i);

    static uint8_t         getSbox(int32_t i);
    static const uint32_t *getCK();
    static const uint32_t *getTbox();

    static uint32_t getTboxVal(uint8_t pos);

    static void sms4_avx2_ecb_encrypt_blocks(const unsigned char *in, unsigned char *out,
                                             size_t blocks, const sms4_key_t *key);

    static void sms4_encrypt(const unsigned char in[16], unsigned char out[16], const sms4_key_t *key);

    static void sms4_set_encrypt_key(sms4_key_t *key, const unsigned char user_key[16]);

    static void sms4_set_decrypt_key(sms4_key_t *key, const unsigned char user_key[16]);

    static void islam_ecb_encrypt(const uint8_t *data, uint32_t dataLen, const uint8_t key[16], uint8_t *cipher);

    static void islam_ecb_decrypt(const uint8_t *cipher, uint32_t cipherLen, const uint8_t key[16], uint8_t *data);

    static uint8_t *getDefKey();

    static void islamAddPad(std::string &data);
    static void islamAddPad(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t *outLen);

    static void islamDelPad(std::string &data);
    static void islamDelPad(uint8_t *data, uint32_t *dataLen);
};

#if YS_MAVX2
#define YS_GET_BLKS(x0, x1, x2, x3, in)                             \
    t0 = _mm256_i32gather_epi32((int *)(in + 4 * 0), vindex_4i, 4); \
    t1 = _mm256_i32gather_epi32((int *)(in + 4 * 1), vindex_4i, 4); \
    t2 = _mm256_i32gather_epi32((int *)(in + 4 * 2), vindex_4i, 4); \
    t3 = _mm256_i32gather_epi32((int *)(in + 4 * 3), vindex_4i, 4); \
    x0 = _mm256_shuffle_epi8(t0, vindex_swap);                      \
    x1 = _mm256_shuffle_epi8(t1, vindex_swap);                      \
    x2 = _mm256_shuffle_epi8(t2, vindex_swap);                      \
    x3 = _mm256_shuffle_epi8(t3, vindex_swap)

#define YS_PUT_BLKS(out, x0, x1, x2, x3)                               \
    t0 = _mm256_shuffle_epi8(x0, vindex_swap);                         \
    t1 = _mm256_shuffle_epi8(x1, vindex_swap);                         \
    t2 = _mm256_shuffle_epi8(x2, vindex_swap);                         \
    t3 = _mm256_shuffle_epi8(x3, vindex_swap);                         \
    _mm256_storeu_si256((__m256i *)(out + 32 * 0), t0);                \
    _mm256_storeu_si256((__m256i *)(out + 32 * 1), t1);                \
    _mm256_storeu_si256((__m256i *)(out + 32 * 2), t2);                \
    _mm256_storeu_si256((__m256i *)(out + 32 * 3), t3);                \
    x0 = _mm256_i32gather_epi32((int *)(out + 8 * 0), vindex_read, 4); \
    x1 = _mm256_i32gather_epi32((int *)(out + 8 * 1), vindex_read, 4); \
    x2 = _mm256_i32gather_epi32((int *)(out + 8 * 2), vindex_read, 4); \
    x3 = _mm256_i32gather_epi32((int *)(out + 8 * 3), vindex_read, 4); \
    _mm256_storeu_si256((__m256i *)(out + 32 * 0), x0);                \
    _mm256_storeu_si256((__m256i *)(out + 32 * 1), x1);                \
    _mm256_storeu_si256((__m256i *)(out + 32 * 2), x2);                \
    _mm256_storeu_si256((__m256i *)(out + 32 * 3), x3)

#define _mm256_rotl_epi32(a, i) _mm256_xor_si256( \
    _mm256_slli_epi32(a, i), _mm256_srli_epi32(a, 32 - i))

#define SIMD_ROUND_TBOX(x0, x1, x2, x3, x4, i)            \
    t0 = _mm256_set1_epi32(*(rk + i));                    \
    t1 = _mm256_xor_si256(x1, x2);                        \
    t2 = _mm256_xor_si256(x3, t0);                        \
    x4 = _mm256_xor_si256(t1, t2);                        \
    t0 = _mm256_and_si256(x4, vindex_mask);               \
    t0 = _mm256_i32gather_epi32((int *)getTbox(), t0, 4); \
    t0 = _mm256_rotl_epi32(t0, 8);                        \
    x4 = _mm256_srli_epi32(x4, 8);                        \
    x0 = _mm256_xor_si256(x0, t0);                        \
    t0 = _mm256_and_si256(x4, vindex_mask);               \
    t0 = _mm256_i32gather_epi32((int *)getTbox(), t0, 4); \
    t0 = _mm256_rotl_epi32(t0, 16);                       \
    x4 = _mm256_srli_epi32(x4, 8);                        \
    x0 = _mm256_xor_si256(x0, t0);                        \
    t0 = _mm256_and_si256(x4, vindex_mask);               \
    t0 = _mm256_i32gather_epi32((int *)getTbox(), t0, 4); \
    t0 = _mm256_rotl_epi32(t0, 24);                       \
    x4 = _mm256_srli_epi32(x4, 8);                        \
    x0 = _mm256_xor_si256(x0, t0);                        \
    t1 = _mm256_i32gather_epi32((int *)getTbox(), x4, 4); \
    x4 = _mm256_xor_si256(x0, t1)
#endif

#define YS_ROUNDS(x0, x1, x2, x3, x4) \
    YS_ROUND(x0, x1, x2, x3, x4, 0);  \
    YS_ROUND(x1, x2, x3, x4, x0, 1);  \
    YS_ROUND(x2, x3, x4, x0, x1, 2);  \
    YS_ROUND(x3, x4, x0, x1, x2, 3);  \
    YS_ROUND(x4, x0, x1, x2, x3, 4);  \
    YS_ROUND(x0, x1, x2, x3, x4, 5);  \
    YS_ROUND(x1, x2, x3, x4, x0, 6);  \
    YS_ROUND(x2, x3, x4, x0, x1, 7);  \
    YS_ROUND(x3, x4, x0, x1, x2, 8);  \
    YS_ROUND(x4, x0, x1, x2, x3, 9);  \
    YS_ROUND(x0, x1, x2, x3, x4, 10); \
    YS_ROUND(x1, x2, x3, x4, x0, 11); \
    YS_ROUND(x2, x3, x4, x0, x1, 12); \
    YS_ROUND(x3, x4, x0, x1, x2, 13); \
    YS_ROUND(x4, x0, x1, x2, x3, 14); \
    YS_ROUND(x0, x1, x2, x3, x4, 15); \
    YS_ROUND(x1, x2, x3, x4, x0, 16); \
    YS_ROUND(x2, x3, x4, x0, x1, 17); \
    YS_ROUND(x3, x4, x0, x1, x2, 18); \
    YS_ROUND(x4, x0, x1, x2, x3, 19); \
    YS_ROUND(x0, x1, x2, x3, x4, 20); \
    YS_ROUND(x1, x2, x3, x4, x0, 21); \
    YS_ROUND(x2, x3, x4, x0, x1, 22); \
    YS_ROUND(x3, x4, x0, x1, x2, 23); \
    YS_ROUND(x4, x0, x1, x2, x3, 24); \
    YS_ROUND(x0, x1, x2, x3, x4, 25); \
    YS_ROUND(x1, x2, x3, x4, x0, 26); \
    YS_ROUND(x2, x3, x4, x0, x1, 27); \
    YS_ROUND(x3, x4, x0, x1, x2, 28); \
    YS_ROUND(x4, x0, x1, x2, x3, 29); \
    YS_ROUND(x0, x1, x2, x3, x4, 30); \
    YS_ROUND(x1, x2, x3, x4, x0, 31);

#define L32_(x)       \
    ((x) ^            \
     ROL32((x), 13) ^ \
     ROL32((x), 23))

#define S32(A)                             \
    ((getSbox(((A) >> 24)) << 24) ^        \
     (getSbox(((A) >> 16) & 0xff) << 16) ^ \
     (getSbox(((A) >> 8) & 0xff) << 8) ^   \
     (getSbox(((A)) & 0xff)))

#define ENC_ROUND(x0, x1, x2, x3, x4, i)       \
    x4        = x1 ^ x2 ^ x3 ^ *(getCK() + i); \
    x4        = S32(x4);                       \
    x4        = x0 ^ L32_(x4);                 \
    *(rk + i) = x4

#define DEC_ROUND(x0, x1, x2, x3, x4, i)            \
    x4             = x1 ^ x2 ^ x3 ^ *(getCK() + i); \
    x4             = S32(x4);                       \
    x4             = x0 ^ L32_(x4);                 \
    *(rk + 31 - i) = x4

#define ROUND_TBOX(x0, x1, x2, x3, x4, i)    \
    x4 = x1 ^ x2 ^ x3 ^ *(rk + i);           \
    t0 = ROL32(getTboxVal((uint8_t)x4), 8);  \
    x4 >>= 8;                                \
    x0 ^= t0;                                \
    t0 = ROL32(getTboxVal((uint8_t)x4), 16); \
    x4 >>= 8;                                \
    x0 ^= t0;                                \
    t0 = ROL32(getTboxVal((uint8_t)x4), 24); \
    x4 >>= 8;                                \
    x0 ^= t0;                                \
    t1 = getTboxVal(x4);                     \
    x4 = x0 ^ t1

inline uint32_t YS_Islam::getFK(int32_t i)
{
    static const uint32_t FK[4] = {
        0xa3b1bac6,
        0x56aa3350,
        0x677d9197,
        0xb27022dc,
    };
    return FK[i];
}
inline uint8_t YS_Islam::getSbox(int32_t i)
{
    static const uint8_t islam_s[256] = {
        0x69, 0x73, 0x6c, 0x61, 0x6d, 0x78, 0xf9, 0xf0,
        0x86, 0x7b, 0xfe, 0x40, 0xb4, 0x48, 0x29, 0x88,
        0x0a, 0xca, 0xde, 0x66, 0xd6, 0x8e, 0xe6, 0xbe,
        0xf3, 0xbf, 0x21, 0x8a, 0xff, 0x96, 0x5a, 0xb3,
        0xc0, 0x1b, 0xfb, 0xe9, 0x0e, 0x42, 0x64, 0x1d,
        0xf7, 0xa1, 0x8b, 0xc6, 0x2b, 0x9f, 0xa3, 0x7a,
        0x0d, 0xab, 0x7f, 0x80, 0xd8, 0x00, 0xd2, 0xa9,
        0xf2, 0x3d, 0x77, 0x04, 0x3c, 0xa2, 0xb9, 0x10,
        0xdc, 0xf6, 0xc2, 0x5b, 0x32, 0x44, 0x6e, 0x74,
        0xf4, 0x98, 0x76, 0xec, 0x25, 0x2a, 0x11, 0x28,
        0x2e, 0x3a, 0xda, 0x49, 0x6b, 0xd0, 0xd4, 0x75,
        0xe5, 0xc3, 0xe3, 0x68, 0x55, 0x8d, 0xb6, 0x4b,
        0x90, 0x50, 0x30, 0x62, 0xa0, 0xc4, 0x81, 0x7c,
        0xdf, 0x1a, 0x8c, 0x4e, 0x2c, 0x5f, 0xb8, 0xed,
        0x33, 0xd5, 0xe4, 0x20, 0x3b, 0x57, 0x58, 0x17,
        0xcd, 0x06, 0x52, 0xe2, 0x46, 0x63, 0xcf, 0xe8,
        0x34, 0xa7, 0x82, 0x1e, 0x27, 0x45, 0x24, 0xdd,
        0xb7, 0xb0, 0x07, 0xf1, 0xd9, 0x4d, 0x1f, 0xb2,
        0xe7, 0x15, 0xf8, 0xaa, 0xba, 0xea, 0x7e, 0xaf,
        0x2f, 0x38, 0xad, 0x89, 0x72, 0x5c, 0xc7, 0x4a,
        0x01, 0x2d, 0x97, 0x84, 0x8f, 0xa6, 0x35, 0x9c,
        0xfd, 0xbc, 0x3e, 0xa5, 0xc5, 0x22, 0x91, 0x59,
        0xb1, 0x92, 0x6f, 0x18, 0xf5, 0x83, 0xdb, 0x12,
        0xac, 0xef, 0x09, 0xe0, 0x23, 0xa4, 0x60, 0x9a,
        0x1c, 0xbd, 0x43, 0x9b, 0x02, 0x3f, 0x05, 0x08,
        0xfc, 0x4c, 0x85, 0x36, 0x13, 0x0b, 0x0f, 0x95,
        0xc9, 0x9e, 0x0c, 0x5d, 0x5e, 0xae, 0x41, 0x70,
        0x03, 0xcc, 0xd1, 0x51, 0xbb, 0xcb, 0xfa, 0xc1,
        0x19, 0x9d, 0x65, 0x26, 0x14, 0x47, 0x39, 0xce,
        0x6a, 0xee, 0x71, 0xb5, 0x31, 0xa8, 0x99, 0xeb,
        0xd3, 0x67, 0x94, 0x16, 0xe1, 0xd7, 0x37, 0x7d,
        0x87, 0xc8, 0x53, 0x56, 0x93, 0x4f, 0x54, 0x79};
    return islam_s[i];
}

inline const uint32_t *YS_Islam::getCK()
{
    static uint32_t CK[32] = {
        0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
        0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
        0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
        0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
        0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
        0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
        0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
        0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279};
    return CK;
}

inline const uint32_t *YS_Islam::getTbox()
{
    static const uint32_t SMS4_T[256] = {
        0x8ed55b5bU, 0xd0924242U, 0x4deaa7a7U, 0x06fdfbfbU,
        0xfccf3333U, 0x65e28787U, 0xc93df4f4U, 0x6bb5dedeU,
        0x4e165858U, 0x6eb4dadaU, 0x44145050U, 0xcac10b0bU,
        0x8828a0a0U, 0x17f8efefU, 0x9c2cb0b0U, 0x11051414U,
        0x872bacacU, 0xfb669d9dU, 0xf2986a6aU, 0xae77d9d9U,
        0x822aa8a8U, 0x46bcfafaU, 0x14041010U, 0xcfc00f0fU,
        0x02a8aaaaU, 0x54451111U, 0x5f134c4cU, 0xbe269898U,
        0x6d482525U, 0x9e841a1aU, 0x1e061818U, 0xfd9b6666U,
        0xec9e7272U, 0x4a430909U, 0x10514141U, 0x24f7d3d3U,
        0xd5934646U, 0x53ecbfbfU, 0xf89a6262U, 0x927be9e9U,
        0xff33ccccU, 0x04555151U, 0x270b2c2cU, 0x4f420d0dU,
        0x59eeb7b7U, 0xf3cc3f3fU, 0x1caeb2b2U, 0xea638989U,
        0x74e79393U, 0x7fb1ceceU, 0x6c1c7070U, 0x0daba6a6U,
        0xedca2727U, 0x28082020U, 0x48eba3a3U, 0xc1975656U,
        0x80820202U, 0xa3dc7f7fU, 0xc4965252U, 0x12f9ebebU,
        0xa174d5d5U, 0xb38d3e3eU, 0xc33ffcfcU, 0x3ea49a9aU,
        0x5b461d1dU, 0x1b071c1cU, 0x3ba59e9eU, 0x0cfff3f3U,
        0x3ff0cfcfU, 0xbf72cdcdU, 0x4b175c5cU, 0x52b8eaeaU,
        0x8f810e0eU, 0x3d586565U, 0xcc3cf0f0U, 0x7d196464U,
        0x7ee59b9bU, 0x91871616U, 0x734e3d3dU, 0x08aaa2a2U,
        0xc869a1a1U, 0xc76aadadU, 0x85830606U, 0x7ab0cacaU,
        0xb570c5c5U, 0xf4659191U, 0xb2d96b6bU, 0xa7892e2eU,
        0x18fbe3e3U, 0x47e8afafU, 0x330f3c3cU, 0x674a2d2dU,
        0xb071c1c1U, 0x0e575959U, 0xe99f7676U, 0xe135d4d4U,
        0x661e7878U, 0xb4249090U, 0x360e3838U, 0x265f7979U,
        0xef628d8dU, 0x38596161U, 0x95d24747U, 0x2aa08a8aU,
        0xb1259494U, 0xaa228888U, 0x8c7df1f1U, 0xd73bececU,
        0x05010404U, 0xa5218484U, 0x9879e1e1U, 0x9b851e1eU,
        0x84d75353U, 0x00000000U, 0x5e471919U, 0x0b565d5dU,
        0xe39d7e7eU, 0x9fd04f4fU, 0xbb279c9cU, 0x1a534949U,
        0x7c4d3131U, 0xee36d8d8U, 0x0a020808U, 0x7be49f9fU,
        0x20a28282U, 0xd4c71313U, 0xe8cb2323U, 0xe69c7a7aU,
        0x42e9ababU, 0x43bdfefeU, 0xa2882a2aU, 0x9ad14b4bU,
        0x40410101U, 0xdbc41f1fU, 0xd838e0e0U, 0x61b7d6d6U,
        0x2fa18e8eU, 0x2bf4dfdfU, 0x3af1cbcbU, 0xf6cd3b3bU,
        0x1dfae7e7U, 0xe5608585U, 0x41155454U, 0x25a38686U,
        0x60e38383U, 0x16acbabaU, 0x295c7575U, 0x34a69292U,
        0xf7996e6eU, 0xe434d0d0U, 0x721a6868U, 0x01545555U,
        0x19afb6b6U, 0xdf914e4eU, 0xfa32c8c8U, 0xf030c0c0U,
        0x21f6d7d7U, 0xbc8e3232U, 0x75b3c6c6U, 0x6fe08f8fU,
        0x691d7474U, 0x2ef5dbdbU, 0x6ae18b8bU, 0x962eb8b8U,
        0x8a800a0aU, 0xfe679999U, 0xe2c92b2bU, 0xe0618181U,
        0xc0c30303U, 0x8d29a4a4U, 0xaf238c8cU, 0x07a9aeaeU,
        0x390d3434U, 0x1f524d4dU, 0x764f3939U, 0xd36ebdbdU,
        0x81d65757U, 0xb7d86f6fU, 0xeb37dcdcU, 0x51441515U,
        0xa6dd7b7bU, 0x09fef7f7U, 0xb68c3a3aU, 0x932fbcbcU,
        0x0f030c0cU, 0x03fcffffU, 0xc26ba9a9U, 0xba73c9c9U,
        0xd96cb5b5U, 0xdc6db1b1U, 0x375a6d6dU, 0x15504545U,
        0xb98f3636U, 0x771b6c6cU, 0x13adbebeU, 0xda904a4aU,
        0x57b9eeeeU, 0xa9de7777U, 0x4cbef2f2U, 0x837efdfdU,
        0x55114444U, 0xbdda6767U, 0x2c5d7171U, 0x45400505U,
        0x631f7c7cU, 0x50104040U, 0x325b6969U, 0xb8db6363U,
        0x220a2828U, 0xc5c20707U, 0xf531c4c4U, 0xa88a2222U,
        0x31a79696U, 0xf9ce3737U, 0x977aededU, 0x49bff6f6U,
        0x992db4b4U, 0xa475d1d1U, 0x90d34343U, 0x5a124848U,
        0x58bae2e2U, 0x71e69797U, 0x64b6d2d2U, 0x70b2c2c2U,
        0xad8b2626U, 0xcd68a5a5U, 0xcb955e5eU, 0x624b2929U,
        0x3c0c3030U, 0xce945a5aU, 0xab76ddddU, 0x867ff9f9U,
        0xf1649595U, 0x5dbbe6e6U, 0x35f2c7c7U, 0x2d092424U,
        0xd1c61717U, 0xd66fb9b9U, 0xdec51b1bU, 0x94861212U,
        0x78186060U, 0x30f3c3c3U, 0x897cf5f5U, 0x5cefb3b3U,
        0xd23ae8e8U, 0xacdf7373U, 0x794c3535U, 0xa0208080U,
        0x9d78e5e5U, 0x56edbbbbU, 0x235e7d7dU, 0xc63ef8f8U,
        0x8bd45f5fU, 0xe7c82f2fU, 0xdd39e4e4U, 0x68492121U};
    return SMS4_T;
}

inline uint32_t YS_Islam::getTboxVal(uint8_t pos)
{
    return *(getTbox() + pos);
}

inline void YS_Islam::sms4_avx2_ecb_encrypt_blocks(const unsigned char *in, unsigned char *out,
                                                   size_t blocks, const sms4_key_t *key)
{
#if YS_AVX2
    const int *rk = (int *)key->rk;
    __m256i    x0, x1, x2, x3, x4;
    __m256i    t0, t1, t2, t3;
    __m256i    vindex_4i   = _mm256_setr_epi32(0, 4, 8, 12, 16, 20, 24, 28);
    __m256i    vindex_mask = _mm256_set1_epi32(0xff);
    __m256i    vindex_read = _mm256_setr_epi32(0, 8, 16, 24, 1, 9, 17, 25);
    __m256i    vindex_swap = _mm256_setr_epi8(
        3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12,
        3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);

    while (blocks >= 8)
    {
        YS_GET_BLKS(x0, x1, x2, x3, in);
#define YS_ROUND SIMD_ROUND_TBOX
        YS_ROUNDS(x0, x1, x2, x3, x4);
#undef YS_ROUND
        YS_PUT_BLKS(out, x0, x4, x3, x2);
        in += 128;
        out += 128;
        blocks -= 8;
    }
#endif

    while (blocks--)
    {
        sms4_encrypt(in, out, key);
        in += 16;
        out += 16;
    }
}

inline void YS_Islam::sms4_encrypt(const unsigned char in[16], unsigned char out[16], const sms4_key_t *key)
{
    const uint32_t *rk = key->rk;
    uint32_t        x0, x1, x2, x3, x4;
    uint32_t        t0, t1;

    x0 = GETU32(in);
    x1 = GETU32(in + 4);
    x2 = GETU32(in + 8);
    x3 = GETU32(in + 12);
#define YS_ROUND ROUND_TBOX
    YS_ROUNDS(x0, x1, x2, x3, x4);
#undef YS_ROUND
    PUTU32(out, x0);
    PUTU32(out + 4, x4);
    PUTU32(out + 8, x3);
    PUTU32(out + 12, x2);
}

inline void YS_Islam::sms4_set_encrypt_key(sms4_key_t *key, const unsigned char user_key[16])
{
    uint32_t *rk = key->rk;
    uint32_t  x0, x1, x2, x3, x4;

    x0 = GETU32(user_key) ^ getFK(0);
    x1 = GETU32(user_key + 4) ^ getFK(1);
    x2 = GETU32(user_key + 8) ^ getFK(2);
    x3 = GETU32(user_key + 12) ^ getFK(3);

#define YS_ROUND ENC_ROUND
    YS_ROUNDS(x0, x1, x2, x3, x4);
#undef YS_ROUND

    x0 = x1 = x2 = x3 = x4 = 0;
}

inline void YS_Islam::sms4_set_decrypt_key(sms4_key_t *key, const unsigned char user_key[16])
{
    uint32_t *rk = key->rk;
    uint32_t  x0, x1, x2, x3, x4;

    x0 = GETU32(user_key) ^ getFK(0);
    x1 = GETU32(user_key + 4) ^ getFK(1);
    x2 = GETU32(user_key + 8) ^ getFK(2);
    x3 = GETU32(user_key + 12) ^ getFK(3);

#define YS_ROUND DEC_ROUND
    YS_ROUNDS(x0, x1, x2, x3, x4);
#undef YS_ROUND

    x0 = x1 = x2 = x3 = x4 = 0;
}

inline void YS_Islam::islam_ecb_encrypt(const uint8_t *data, uint32_t dataLen, const uint8_t key[16], uint8_t *cipher)
{
    if (dataLen % 16)
    {
        return;
    }
    sms4_key_t tKey;
    sms4_set_encrypt_key(&tKey, key);
    sms4_avx2_ecb_encrypt_blocks(data, cipher, dataLen / 16, &tKey);
}

inline void YS_Islam::encrypt(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t *outLen, const uint8_t key[16])
{
    islamAddPad(in, inLen, out, outLen);
    islam_ecb_encrypt(out, *outLen, key, out);
}

inline void YS_Islam::islam_ecb_decrypt(const uint8_t *cipher, uint32_t cipherLen, const uint8_t key[16], uint8_t *data)
{
    if (cipherLen % 16)
    {
        return;
    }
    sms4_key_t tKey;
    sms4_set_decrypt_key(&tKey, key);
    sms4_avx2_ecb_encrypt_blocks(cipher, data, cipherLen / 16, &tKey);
}

inline void YS_Islam::decrypt(const uint8_t *in, uint32_t inLen, uint8_t *out, uint32_t *outLen, const uint8_t key[16])
{
    islam_ecb_decrypt(in, inLen, key, out);
    *outLen = inLen;
    islamDelPad(out, outLen);
}

inline uint8_t *YS_Islam::getDefKey()
{
    static uint8_t islamDefKey[16] = {0xe7, 0xf7, 0x19, 0x85, 0x54, 0x92, 0xdb, 0xb9,
                                      0x00, 0x18, 0xa8, 0xa3, 0x9d, 0xaf, 0x1e, 0x3c};
    return islamDefKey;
}
inline void YS_Islam::islamAddPad(std::string &data)
{
    uint32_t uPadLen = 16 - (data.length() % 16);
    data.append(uPadLen, char(uPadLen));
}

inline void YS_Islam::islamAddPad(const uint8_t *data, uint32_t dataLen, uint8_t *out, uint32_t *outLen)
{
    uint8_t uPadLen = 16 - (dataLen % 16);
    memcpy(out, data, dataLen);
    memset(out + dataLen, uPadLen, uPadLen);
    *outLen = dataLen + uPadLen;
}

inline void YS_Islam::islamDelPad(std::string &data)
{
    uint32_t uPadLen = data[data.length() - 1];
    if (uPadLen > 16 || uPadLen > data.length())
    {
        return;
    }
    data = data.substr(0, data.length() - uPadLen);
}

inline void YS_Islam::islamDelPad(uint8_t *data, uint32_t *dataLen)
{
    uint8_t uPadLen = data[*dataLen - 1];
    if (uPadLen > 16 || uPadLen > *dataLen)
    {
        return;
    }
    *dataLen -= uPadLen;
}

inline int32_t YS_Islam::encrypt(std::string &data, std::string &out, const std::string &pin)
{
    if (pin.length() < 4)
    {
        return -1;
    }
    std::string src = pin + data;
    islamAddPad(src);
    std::unique_ptr<uint8_t> buf(new uint8_t[src.length()]);
    islam_ecb_encrypt((uint8_t *)src.c_str(), src.length(), getDefKey(), buf.get());
    std::string enc((const char *)buf.get(), src.length());
    out = "islam:" + YS_Base64::encode(enc);
    return 0;
}

inline int32_t YS_Islam::decrypt(std::string &enc, std::string &out, const std::string &pin)
{
    if (enc.length() < 22)
    {
        std::printf("enc len < 22");
        return -1;
    }

    if (enc.compare(0, 6, "islam:"))
    {
        std::printf("compare fail %s\n", enc.c_str());
        return -1;
    }

    std::string data = enc.substr(6);
    std::string bin  = YS_Base64::decode(data);
    if (bin.length() % 16)
    {
        std::printf("bin len = %zu [%s]\n", bin.length(), data.c_str());
        return -1;
    }
    std::unique_ptr<uint8_t> buf(new uint8_t[bin.length()]);
    islam_ecb_decrypt((uint8_t *)bin.c_str(), bin.length(), getDefKey(), buf.get());
    std::string dec((char *)buf.get(), bin.length());
    islamDelPad(dec);
    if (dec.compare(0, 4, pin))
    {
        return -1;
    }
    out = dec.substr(4);
    return 0;
}

#undef YS_GET_BLKS
#undef YS_PUT_BLKS
#undef SIMD_ROUND_TBOX
#undef YS_ROUNDS
#undef L32_
#undef S32
#undef ENC_ROUND
#undef DEC_ROUND
#undef ROUND_TBOX
} // namespace isaac

#endif