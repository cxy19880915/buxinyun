#ifndef __MBEDTLS_CONFIG_H__
#define __MBEDTLS_CONFIG_H__


//#include "config-default.h"

#include "mbedtls/configs/config-mini-tls1_1.h"


#define mbedtls_time       time
#define mbedtls_time_t     time_t
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf

#define MBEDTLS_AES_SETKEY_ENC_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_DECRYPT_ALT
#define MBEDTLS_SELF_TEST //MODIFY BY SHUNJIAN

#define MBEDTLS_SHA1_PROCESS_ALT
#define MBEDTLS_SSL_EXPORT_KEYS
//#define MBEDTLS_CIPHER_TLS_RSA_WITH_AES_256_CBC_SHA256
//#define MBEDTLS_CIPHER_TLS_RSA_WITH_AES_256_CBC_SHA

#endif //__MBEDTLS_CONFIG_H__
