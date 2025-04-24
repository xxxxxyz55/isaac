#ifndef _YS_SM3_DRBG_H_
#define _YS_SM3_DRBG_H_

#include "ys_sm3.hpp"
#include <mutex>
#include <memory>
#include "../tool/ys_timer.hpp"
#if _WIN32
#include <wincrypt.h>
#include <Windows.h>
#else
#endif

namespace isaac {

#define SM3_SEED_LEN 55

// sm3 随机数发生器
class YS_Sm3Drbg
{
public:
    // 获取随机数
    static void randBytes(uint32_t len, uint8_t *data);

    // 获取随机数
    static std::string random(uint32_t len);

private:
    uint8_t  _v[SM3_SEED_LEN] = {};
    uint8_t  _c[SM3_SEED_LEN] = {};
    uint64_t _counter         = 0;

    void init(const uint8_t *entropy, uint32_t entropyLen,
              const uint8_t *nonce, uint32_t nonceLen,
              const uint8_t *psersonalstr, uint32_t personalstrLen);

    void reseedDef();

    void generate(const uint8_t *additional, uint32_t additionalLen,
                  uint32_t outlen, uint8_t *out);

    void reseed(const uint8_t *entropy, uint32_t entropyLen,
                const uint8_t *additional, uint32_t additionalLen);

    static void random(uint32_t len, uint8_t *buf);

    void sm3Df(uint8_t *material, uint32_t materialLen, uint32_t outLen, uint8_t *out);

    void add(uint8_t *R, const uint8_t *A, uint32_t seedLen);

    void add1(uint8_t *R, size_t seedLen);

    void sm3Gen(uint32_t outLen, uint8_t *out);
};

inline void YS_Sm3Drbg::init(const uint8_t *entropy, uint32_t entropyLen,
                      const uint8_t *nonce, uint32_t nonceLen,
                      const uint8_t *psersonalstr, uint32_t personalstrLen)
{
    uint8_t *pBuf = new uint8_t[entropyLen + nonceLen + personalstrLen];
    uint32_t len  = 0;
    memcpy(pBuf + len, entropy, entropyLen);
    len += entropyLen;
    memcpy(pBuf + len, nonce, nonceLen);
    len += nonceLen;
    memcpy(pBuf + len, psersonalstr, personalstrLen);
    len += personalstrLen;

    sm3Df(pBuf, len, SM3_SEED_LEN, _v);
    uint8_t uBuf[128];
    uBuf[0] = 0x00;
    memcpy(uBuf + 1, _v, SM3_SEED_LEN);
    sm3Df(uBuf, len, 1 + SM3_SEED_LEN, _c);

    _counter = 1;

    delete[] pBuf;
}

inline void YS_Sm3Drbg::reseed(const uint8_t *entropy, uint32_t entropyLen,
                        const uint8_t *additional, uint32_t additionalLen)
{
    return init(_v, SM3_SEED_LEN, entropy, entropyLen, additional, additionalLen);
}

inline void YS_Sm3Drbg::reseedDef()
{
    uint8_t ent[16];
    random(sizeof(ent), ent);
    init(_v, SM3_SEED_LEN, ent, sizeof(ent), nullptr, 0);
}

#define PUTU64(p, V)                \
    ((p)[0] = (uint8_t)((V) >> 56), \
     (p)[1] = (uint8_t)((V) >> 48), \
     (p)[2] = (uint8_t)((V) >> 40), \
     (p)[3] = (uint8_t)((V) >> 32), \
     (p)[4] = (uint8_t)((V) >> 24), \
     (p)[5] = (uint8_t)((V) >> 16), \
     (p)[6] = (uint8_t)((V) >> 8),  \
     (p)[7] = (uint8_t)(V))

inline void YS_Sm3Drbg::generate(const uint8_t *additional, uint32_t additionalLen,
                          uint32_t outlen, uint8_t *out)
{
    if (_counter > (uint64_t)1 << 48)
    {
        reseedDef();
    }

    uint8_t T[SM3_SEED_LEN];

    if (additionalLen)
    {
        uint8_t prefix = 0x02;
        uint8_t md[SM3_MD_LEN];
        YS_Sm3  ctx;
        ctx.init();
        ctx.update(&prefix, sizeof(prefix));
        ctx.update(_v, SM3_SEED_LEN);
        ctx.update(additional, additionalLen);
        ctx.final(md);

        memset(T, 0, SM3_SEED_LEN - SM3_MD_LEN);
        memcpy(T + SM3_SEED_LEN - SM3_MD_LEN, md, SM3_MD_LEN);
        add(_v, T, SM3_SEED_LEN);
    }

    sm3Gen(outlen, out);

    {
        uint8_t prefix = 0x03;
        YS_Sm3  ctx;
        uint8_t md[SM3_MD_LEN];
        ctx.init();
        ctx.update(&prefix, sizeof(prefix));
        ctx.update(_v, SM3_SEED_LEN);
        ctx.final(md);

        memset(T, 0, SM3_SEED_LEN - SM3_MD_LEN);
        memcpy(T + SM3_SEED_LEN - SM3_MD_LEN, md, SM3_MD_LEN);
        add(_v, T, SM3_SEED_LEN);
    }
    add(_v, _c, SM3_SEED_LEN);

    memset(T, 0, SM3_SEED_LEN - sizeof(uint64_t));
    PUTU64(T + SM3_SEED_LEN - sizeof(uint64_t), _counter);
    add(_v, T, SM3_SEED_LEN);
    _counter++;
}
#undef PUTU64

inline void YS_Sm3Drbg::random(uint32_t len, uint8_t *buf)
{
    if (len == 0)
    {
        return;
    }
#if _WIN32
    HCRYPTPROV hCryptProv;

    if (CryptAcquireContextA(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT) != TRUE ||
        CryptGenRandom(hCryptProv, (DWORD)len, buf) != TRUE ||
        CryptReleaseContext(hCryptProv, 0) != TRUE)
    {
        std::printf("window gen random fail.\n");
        exit(1);
    }
#else

    std::unique_ptr<FILE, int (*)(FILE *)> fp(
        fopen("/dev/urandom", "rb"),
        ::fclose);

    if (!fp || fread(buf, 1, len, fp.get()) != len)
    {
        std::printf("urandom fail.\n");
        exit(1);
    }
#endif
}

inline void YS_Sm3Drbg::randBytes(uint32_t len, uint8_t *data)
{
    static YS_Sm3Drbg drbg;
    static bool       fInit = false;
    static std::mutex lock;

    std::unique_lock<std::mutex> lck(lock);
    if (!fInit)
    {
        uint8_t ent[16];
        auto    tm = YS_Timer::getTimeMs();
        random(sizeof(ent), ent);
        drbg.init(ent, sizeof(ent), (uint8_t *)&tm, sizeof(tm), nullptr, 0);
        fInit = true;
    }
    drbg.generate(nullptr, 0, len, data);
}

inline std::string YS_Sm3Drbg::random(uint32_t len)
{
    std::unique_ptr<uint8_t> buf(new uint8_t[len]);
    randBytes(len, buf.get());
    return std::string((char *)buf.get(), len);
}

inline void YS_Sm3Drbg::sm3Df(uint8_t *material, uint32_t materialLen, uint32_t outLen, uint8_t *out)
{
    uint8_t outbits[4];
    YS_Sm3  ctx;
    PUTU32(outbits, outLen << 3);

    uint8_t  counter = 0x01;
    uint8_t  md[32];
    uint32_t len;

    while (outLen > 0)
    {
        ctx.init();
        ctx.update(&counter, sizeof(counter));
        ctx.update(outbits, sizeof(outbits));
        ctx.update(material, materialLen);
        ctx.final(md);
        len = 32;

        if (outLen < len)
        {
            len = outLen;
        }
        memcpy(out, md, len);
        out += len;
        outLen -= len;
        counter++;
    }
}

inline void YS_Sm3Drbg::add(uint8_t *R, const uint8_t *A, uint32_t seedLen)
{
    int temp = 0;
    while (seedLen--)
    {
        temp += R[seedLen] + A[seedLen];
        R[seedLen] = temp & 0xff;
        temp >>= 8;
    }
}

inline void YS_Sm3Drbg::add1(uint8_t *R, size_t seedLen)
{
    int temp = 1;
    while (seedLen--)
    {
        temp += R[seedLen];
        R[seedLen] = temp & 0xff;
        temp >>= 8;
    }
}

inline void YS_Sm3Drbg::sm3Gen(uint32_t outLen, uint8_t *out)
{
    uint8_t  data[SM3_SEED_LEN];
    uint8_t  md[SM3_MD_LEN];
    uint32_t len;

    memcpy(data, _v, SM3_SEED_LEN);
    while (outLen > 0)
    {
        YS_Sm3 ctx;
        ctx.init();
        ctx.update(data, SM3_SEED_LEN);
        ctx.final(md);
        len = 32;
        if (outLen < len)
        {
            len = outLen;
        }
        memcpy(out, md, len);
        out += len;
        outLen -= len;

        add1(data, SM3_SEED_LEN);
    }
}

#undef SM3_SEED_LEN

} // namespace isaac

#endif