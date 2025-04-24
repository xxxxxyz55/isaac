#ifndef _YS_SSL_H_
#define _YS_SSL_H_

#ifndef YS_SUPPORT_SSL
#define YS_SUPPORT_SSL 0
#endif

#include "tool/ys_timer.hpp"
#include "tool/ys_util.hpp"

typedef struct ssl_st     SSL;
typedef struct ssl_ctx_st SSL_CTX;

#if YS_SUPPORT_SSL

#ifndef YS_SUPPORT_GMTLS
#define YS_SUPPORT_GMTLS 1
#endif

extern "C" {

#define WITH_OPENSSL_H 0

#if WITH_OPENSSL_H
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/stack.h"
#include "openssl/x509.h"
#else

typedef struct ssl_method_st SSL_METHOD;
typedef struct ssl_cipher_st SSL_CIPHER;

#define SSLv23_method TLS_method
const SSL_METHOD            *TLS_method(void);
const SSL_METHOD            *TLSv1_method(void);
const SSL_METHOD            *TLSv1_1_method(void);
const SSL_METHOD            *TLSv1_2_method(void);

#define SSL_VERIFY_NONE                 0x00
#define SSL_VERIFY_PEER                 0x01
#define SSL_VERIFY_FAIL_IF_NO_PEER_CERT 0x02
#define SSL_VERIFY_CLIENT_ONCE          0x04

SSL_CTX *SSL_CTX_new(const SSL_METHOD *meth);
SSL     *SSL_new(SSL_CTX *ctx);
int      SSL_set_fd(SSL *s, int fd);
void     SSL_free(SSL *ssl);

typedef struct x509_store_ctx_st X509_STORE_CTX;
typedef int (*SSL_verify_cb)(int preverify_ok, X509_STORE_CTX *x509_ctx);
void SSL_CTX_set_verify(SSL_CTX *ctx, int mode, SSL_verify_cb callback);

int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,
                                  const char *CApath);

int  SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type);
int  SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type);
void SSL_CTX_free(SSL_CTX *);

int SSL_accept(SSL *ssl);
int SSL_connect(SSL *ssl);
int SSL_read(SSL *ssl, void *buf, int num);
int SSL_write(SSL *ssl, const void *buf, int num);

int SSL_get_error(const SSL *s, int ret_code);

long              SSL_get_verify_result(const SSL *ssl);
const SSL_CIPHER *SSL_get_current_cipher(const SSL *s);
const char       *SSL_CIPHER_get_name(const SSL_CIPHER *c);

const char       *ERR_func_error_string(unsigned long e);
const char       *ERR_reason_error_string(unsigned long e);
unsigned long     ERR_get_error(void);

#if YS_SUPPORT_GMTLS
const SSL_METHOD *GMTLS_method(void);
#endif

#define SSL_ERROR_NONE       0
#define SSL_ERROR_SSL        1
#define SSL_ERROR_WANT_READ  2
#define SSL_ERROR_WANT_WRITE 3

#endif
}
#endif

class YS_SslCtx
{
    SSL_CTX *_ctx;

public:
    YS_SslCtx(SSL_CTX *ctx) { _ctx = ctx; }
    ~YS_SslCtx()
    {
#if YS_SUPPORT_SSL
        if (_ctx) SSL_CTX_free(_ctx);
#endif
    }
    SSL_CTX *ctx() { return _ctx; }
};
using YS_SslCtxPtr = std::shared_ptr<YS_SslCtx>;

#include <string>
#include "ys_cli_log.h"

namespace isaac {

class YS_Ssl
{
public:
    static YS_SslCtxPtr newCtx(const std::string ca,
                               const std::string sign, const std::string signKey,
                               bool verifyClient = false)
    {
#if YS_SUPPORT_SSL
        SSL_CTX *pCtx = SSL_CTX_new(SSLv23_method());
        if (!pCtx)
        {
            CLI_DEBUG("ssl ctx new fail.");
            return nullptr;
        }

        int32_t mode = SSL_VERIFY_NONE;
        if (!ca.empty())
        {
            mode |= SSL_VERIFY_PEER;
            if (verifyClient)
            {
                mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
            }

            if (!SSL_CTX_load_verify_locations(pCtx, ca.c_str(), nullptr))
            {
                CLI_ERROR("ssl load ca fail [{}]", opensslErr());
            }
        }

        SSL_CTX_set_verify(pCtx, mode, nullptr);

        if (!sign.empty())
        {
            if (SSL_CTX_use_certificate_file(pCtx, sign.c_str(), 1) <= 0)
            {
                SSL_CTX_free(pCtx);
                CLI_ERROR("ssl use cert fail, [{}].", opensslErr());
                return nullptr;
            }
        }

        if (!signKey.empty())
        {
            if (SSL_CTX_use_PrivateKey_file(pCtx, signKey.c_str(), 1) <= 0)
            {
                SSL_CTX_free(pCtx);
                CLI_ERROR("ssl use key fail.");
                return nullptr;
            }
        }

        return std::make_shared<YS_SslCtx>(pCtx);
#else
        return nullptr;
#endif
    }
    // gmtls enc必须带加密拓展用途
    static YS_SslCtxPtr newGmCtx(const std::string ca,
                                 const std::string sign, const std::string signKey,
                                 const std::string enc, const std::string encKey,
                                 bool verifyClient = false)
    {
#if YS_SUPPORT_GMTLS
        SSL_CTX *pCtx = SSL_CTX_new(GMTLS_method());
        if (!pCtx)
        {
            CLI_DEBUG("ssl ctx new fail.");
            return nullptr;
        }

        int32_t mode = SSL_VERIFY_NONE;
        if (!ca.empty())
        {
            mode |= SSL_VERIFY_PEER;
            if (verifyClient)
            {
                mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
            }

            if (!SSL_CTX_load_verify_locations(pCtx, ca.c_str(), nullptr))
            {
                CLI_ERROR("ssl load ca fail [{}]", opensslErr());
            }
        }

        SSL_CTX_set_verify(pCtx, mode, nullptr);

        if (!sign.empty())
        {
            if (SSL_CTX_use_certificate_file(pCtx, sign.c_str(), 1) <= 0)
            {
                SSL_CTX_free(pCtx);
                CLI_ERROR("ssl use cert fail.");
                return nullptr;
            }
        }

        if (!signKey.empty())
        {
            if (SSL_CTX_use_PrivateKey_file(pCtx, signKey.c_str(), 1) <= 0)
            {
                SSL_CTX_free(pCtx);
                CLI_ERROR("ssl use key fail.");
                return nullptr;
            }
        }

        if (!enc.empty())
        {
            if (SSL_CTX_use_certificate_file(pCtx, enc.c_str(), 1) <= 0)
            {
                SSL_CTX_free(pCtx);
                CLI_ERROR("ssl use cert fail.");
                return nullptr;
            }
        }

        if (!encKey.empty())
        {
            if (SSL_CTX_use_PrivateKey_file(pCtx, encKey.c_str(), 1) <= 0)
            {
                SSL_CTX_free(pCtx);
                CLI_ERROR("ssl use key fail.");
                return nullptr;
            }
        }

        return std::make_shared<YS_SslCtx>(pCtx);
#else
        return nullptr;
#endif
    }

    static SSL *newSSl(YS_SslCtxPtr pCtx, int32_t fd)
    {
#if YS_SUPPORT_SSL
        SSL *pSsl = SSL_new(pCtx->ctx());
        if (!pSsl)
        {
            return nullptr;
        }
        if (!SSL_set_fd(pSsl, fd))
        {
            CLI_ERROR("ssl set fd fail [{}].", opensslErr());
        }
        return pSsl;
#else
        return nullptr;
#endif
    }

    static void freeSSL(SSL *pSsl)
    {
#if YS_SUPPORT_SSL
        SSL_free(pSsl);
#endif
    }

    static SSL *accept(YS_SslCtxPtr pCtx, int32_t fd, long tmout = 3000)
    {
#if YS_SUPPORT_SSL
        SSL *pSsl = SSL_new(pCtx->ctx());
        if (pSsl)
        {
            if (!SSL_set_fd(pSsl, fd))
            {
                CLI_ERROR("ssl set fd fail [{}].", opensslErr());
            }

            int32_t iRet;
            int32_t sslErr;

            iRet = SSL_accept(pSsl);
            while (iRet <= 0)
            {
                sslErr = SSL_get_error(pSsl, iRet);
                bool res;
                if (sslErr == SSL_ERROR_WANT_READ)
                {
                    res = YS_Util::canRead(fd, tmout);
                }
                else if (sslErr == SSL_ERROR_WANT_WRITE)
                {
                    res = YS_Util::canWrite(fd, tmout);
                }
                else
                {
                    CLI_ERROR("ssl accept fail err= {} {}.", sslErr, opensslErr());
                    SSL_free(pSsl);
                    return nullptr;
                }

                if (!res)
                {
                    CLI_ERROR("ssl accept timeout.");
                    SSL_free(pSsl);
                    return nullptr;
                }
                iRet = SSL_accept(pSsl);
            }

            CLI_DEBUG("ssl accep ok");
            if (SSL_get_verify_result(pSsl) == 0)
            {
                // CLI_DEBUG("cert verify ok.");
            }
            else
            {
                // CLI_ERROR("cert verify error.");
            }
        }
        return pSsl;
#else
        return nullptr;
#endif
    }

    static SSL *connect(YS_SslCtxPtr pCtx, int32_t fd, long tmout = 3000)
    {
#if YS_SUPPORT_SSL
        SSL *pSsl = SSL_new(pCtx->ctx());
        if (pSsl)
        {
            if (!SSL_set_fd(pSsl, fd))
            {
                CLI_ERROR("ssl set fd fail [{}].", opensslErr());
            }

            int32_t iRet;
            int32_t sslErr;
            iRet = SSL_connect(pSsl);
            while (iRet <= 0)
            {
                sslErr = SSL_get_error(pSsl, iRet);
                bool res;
                if (sslErr == SSL_ERROR_WANT_READ)
                {
                    res = YS_Util::canRead(fd, tmout);
                }
                else if (sslErr == SSL_ERROR_WANT_WRITE)
                {
                    res = YS_Util::canWrite(fd, tmout);
                }
                else
                {
                    CLI_ERROR("ssl accept fail err= {} {}.", sslErr, opensslErr());
                    SSL_free(pSsl);
                    return nullptr;
                }

                if (!res)
                {
                    CLI_ERROR("ssl accept timeout.");
                    SSL_free(pSsl);
                    return nullptr;
                }
                iRet = SSL_connect(pSsl);
            }

            // CLI_DEBUG("ssl connect OK");
            if (SSL_get_verify_result(pSsl) == 0)
            {
                // CLI_DEBUG("cert verify ok.");
            }
            else
            {
                //     CLI_ERROR("cert verify error.");
            }
        }
        // CLI_DEBUG("ssl cipher : {}", SSL_CIPHER_get_name(SSL_get_current_cipher(pSsl)));
        return pSsl;
#else
        return nullptr;
#endif
    }

    static int32_t write(SSL *pSsl, const char *buf, uint32_t len)
    {
#if YS_SUPPORT_SSL
        return SSL_write(pSsl, buf, len);
#else
        return -1;
#endif
    }

    static int32_t read(SSL *pSsl, char *buf, uint32_t len)
    {
#if YS_SUPPORT_SSL
        return SSL_read(pSsl, buf, len);
#else
        return -1;
#endif
    }

    static bool isSslEngine(SSL *pSsl, int32_t code)
    {
#if YS_SUPPORT_SSL
        int32_t iRet = SSL_get_error(pSsl, code);
        if (iRet != SSL_ERROR_NONE &&
            iRet != SSL_ERROR_WANT_WRITE &&
            iRet != SSL_ERROR_WANT_READ)
        {
            CLI_ERROR("ssl get error {}", iRet);
            return false;
        }
        else
        {
            return true;
        }
#else
        return false;
#endif
    }

    static std::string opensslErr()
    {
#if YS_SUPPORT_SSL
        std::string sErr;
        int32_t     iRet = ERR_get_error();

        while (iRet)
        {
            sErr += ERR_func_error_string(iRet);
            sErr += "\t";
            sErr += ERR_reason_error_string(iRet);
            iRet = ERR_get_error();
            sErr += "\t";
        }
        return sErr;
#else
        return "";
#endif
    }
    static bool isGmssl(SSL *pSsl)
    {
        if (pSsl)
        {
#if YS_SUPPORT_SSL
            std::string alg = SSL_CIPHER_get_name(SSL_get_current_cipher(pSsl));
            if (alg == "SM2-WITH-SMS4-SM3")
            {
                return true;
            }
#endif
        }
        return false;
    }
};

} // namespace isaac

#endif