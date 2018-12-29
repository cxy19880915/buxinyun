#ifndef __TLS_API_H
#define __TLS_API_H

/*---------------------------------------------------------------------------------*/

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#include "lwip.h"
#include "os/os_api.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#define mbedtls_printf     printf
#define mbedtls_fprintf    fprintf
#define mbedtls_time_t     time_t
#endif


#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/timing.h"

#include "mbedtls/x509.h"
#include "mbedtls/ssl_cookie.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif
#if defined(MBEDTLS_SSL_TICKET_C)
#include "mbedtls/ssl_ticket.h"
#endif

#include "os/os_api.h"

#define TLS_SET_CA   0x1
#define TLS_SERVER_ACCEPT  0x2


struct tls_struct {
    /***********client*********************/
    mbedtls_net_context dtls_fd;
    mbedtls_ssl_context ssl;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_timing_delay_context timer;
    char public_ip[15]; //client 理解为将要连接的远端IP   server理解为本地绑定IP
    char public_port[6];//同上

    /*************server******************/
    mbedtls_net_context  client_fd;
    mbedtls_net_context  listen_fd;
    mbedtls_ssl_cookie_ctx cookie_ctx;

    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context cache;
#endif
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_ticket_context ticket_ctx;
#endif

    OS_MUTEX mutex;

    void *fd; //原始socket句柄


};




struct _tls_ops {

    int (*dinit)(struct tls_struct *);
    int (*handshake)(struct tls_struct *);
    int (*dwrite)(struct tls_struct *, u8 *, u32, u32);
    int (*dread)(struct tls_struct *, u8 *, u32, u32);
    int (*dioctl)(struct tls_struct *, u32, u32);
    int (*duninit)(struct tls_struct *);

};


extern const struct _tls_ops tls_cli_ops;
extern const struct _tls_ops tls_ser_ops;
extern const  struct _tls_ops tls_raw_ops;


#endif


