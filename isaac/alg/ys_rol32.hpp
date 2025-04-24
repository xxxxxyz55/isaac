#ifndef _YS_ROL32_H_
#define _YS_ROL32_H_

/*
 * Engage compiler specific rotate intrinsic function if available.
 */

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wstrict-aliasing"
#endif
// #if defined(_MSC_VER)
// #   pragma warning(disable:4100)
// #endif

#undef ROL32
#ifndef PEDANTIC
#if defined(_MSC_VER)
#define ROL32(a, n) _lrotl(a, n)
#elif defined(__ICC)
#define ROL32(a, n) _rotl(a, n)
#elif defined(__GNUC__) && __GNUC__ >= 2
/*
 * Some GNU C inline assembler templates. Note that these are
 * rotates by *constant* number of bits! But that's exactly
 * what we need here...
 *                                    <appro@fy.chalmers.se>
 */
#if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
#define ROL32(a, n) ({                   \
    register unsigned int ret;           \
    asm(                                 \
        "roll %1,%0"                     \
        : "=r"(ret)                      \
        : "I"(n), "0"((unsigned int)(a)) \
        : "cc");                         \
    ret;                                 \
})
#elif defined(_ARCH_PPC) || defined(_ARCH_PPC64) || \
    defined(__powerpc) || defined(__ppc__) || defined(__powerpc64__)
#define ROL32(a, n) ({         \
    register unsigned int ret; \
    asm(                       \
        "rlwinm %0,%1,%2,0,31" \
        : "=r"(ret)            \
        : "r"(a), "I"(n));     \
    ret;                       \
})
#elif defined(__s390x__)
#define ROL32(a, n) ({         \
    register unsigned int ret; \
    asm("rll %0,%1,%2"         \
        : "=r"(ret)            \
        : "r"(a), "I"(n));     \
    ret;                       \
})
#endif
#endif
#endif /* PEDANTIC */

#ifndef ROL32
#define ROL32(a, n) (((a) << (n)) | (((a) & 0xffffffff) >> (32 - (n))))
#endif

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
typedef __int64          i64;
typedef unsigned __int64 u64;
#define U64(C) C##UI64
#elif defined(__arch64__)
typedef long          i64;
typedef unsigned long u64;
#define U64(C) C##UL
#else
typedef long long          i64;
typedef unsigned long long u64;
#define U64(C) C##ULL
#endif

typedef unsigned int  u32;
typedef unsigned char u8;

#define STRICT_ALIGNMENT 1
#ifndef PEDANTIC
#if defined(__i386) || defined(__i386__) ||                     \
    defined(__x86_64) || defined(__x86_64__) ||                 \
    defined(_M_IX86) || defined(_M_AMD64) || defined(_M_X64) || \
    defined(__aarch64__) ||                                     \
    defined(__s390__) || defined(__s390x__)
#undef STRICT_ALIGNMENT
#endif
#endif

#if !defined(PEDANTIC) && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
#if defined(__GNUC__) && __GNUC__ >= 2
#if defined(__x86_64) || defined(__x86_64__)
#define BSWAP8(x) ({ u64 ret_=(x);                   \
                        asm ("bswapq %0"                \
                        : "+r"(ret_));   ret_; })
#define BSWAP4(x) ({ u32 ret_=(x);                   \
                        asm ("bswapl %0"                \
                        : "+r"(ret_));   ret_; })
#elif (defined(__i386) || defined(__i386__)) && !defined(I386_ONLY)
#define BSWAP8(x) ({ u32 lo_=(u64)(x)>>32,hi_=(x);   \
                        asm ("bswapl %0; bswapl %1"     \
                        : "+r"(hi_),"+r"(lo_));         \
                        (u64)hi_<<32|lo_; })
#define BSWAP4(x) ({ u32 ret_=(x);                   \
                        asm ("bswapl %0"                \
                        : "+r"(ret_));   ret_; })
#elif defined(__aarch64__)
#define BSWAP8(x) ({ u64 ret_;                       \
                        asm ("rev %0,%1"                \
                        : "=r"(ret_) : "r"(x)); ret_; })
#define BSWAP4(x) ({ u32 ret_;                       \
                        asm ("rev %w0,%w1"              \
                        : "=r"(ret_) : "r"(x)); ret_; })
#elif (defined(__arm__) || defined(__arm)) && !defined(STRICT_ALIGNMENT)
#define BSWAP8(x) ({ u32 lo_=(u64)(x)>>32,hi_=(x);   \
                        asm ("rev %0,%0; rev %1,%1"     \
                        : "+r"(hi_),"+r"(lo_));         \
                        (u64)hi_<<32|lo_; })
#define BSWAP4(x) ({ u32 ret_;                       \
                        asm ("rev %0,%1"                \
                        : "=r"(ret_) : "r"((u32)(x)));  \
                        ret_; })
#endif
#elif defined(_MSC_VER)
#if _MSC_VER >= 1300
#pragma intrinsic(_byteswap_uint64, _byteswap_ulong)
#define BSWAP8(x) _byteswap_uint64((u64)(x))
#define BSWAP4(x) _byteswap_ulong((u32)(x))
#elif defined(_M_IX86)
__inline u32 _bswap4(u32 val)
{
    _asm mov eax, val _asm bswap eax
}
#define BSWAP4(x) _bswap4(x)
#endif
#endif
#endif
#if defined(BSWAP4) && !defined(STRICT_ALIGNMENT)
#define GETU32(p)    BSWAP4(*(const u32 *)(p))
#define PUTU32(p, v) *(u32 *)(p) = BSWAP4(v)
#else
#define GETU32(p)    ((u32)(p)[0] << 24 | (u32)(p)[1] << 16 | (u32)(p)[2] << 8 | (u32)(p)[3])
#define PUTU32(p, v) ((p)[0] = (u8)((v) >> 24), (p)[1] = (u8)((v) >> 16), (p)[2] = (u8)((v) >> 8), (p)[3] = (u8)(v))
#endif

#endif