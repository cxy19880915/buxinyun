

#include "tls_api.h"
#define DEBUG_LEVEL 1

static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    ((void) level);
    //mbedtls_printf( "%s:%04d: %s", line, str);
}

/*---------------------------------------------------------------------------------*/



static int dtls_client_init(struct tls_struct *tls)
{
    log_d("dtls_client_init\n");
    int ret, len;
    const char *pers = "dtls_cli2223";

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init(&tls->dtls_fd);
    os_mutex_create(&tls->dtls_fd.mutex);
    mbedtls_ssl_init(&tls->ssl);
    mbedtls_ssl_config_init(&tls->conf);
    mbedtls_x509_crt_init(&tls->cacert);
    mbedtls_ctr_drbg_init(&tls->ctr_drbg);

    mbedtls_printf("\n  . Seeding the random number generator...");

    mbedtls_entropy_init(&tls->entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&tls->ctr_drbg, mbedtls_entropy_func, &tls->entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    /*
     * 0. Load certificates
     */
    mbedtls_printf("  . Loading the CA root certificate ...");

    ret = mbedtls_x509_crt_parse(&tls->cacert, (const unsigned char *) mbedtls_test_cas_pem,
                                 mbedtls_test_cas_pem_len);
    if (ret < 0) {
        mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        goto exit;
    }

    mbedtls_printf(" ok (%d skipped)\n", ret);
#if 0
    if ((ret = mbedtls_net_connect_bind(&tls->dtls_fd, AF_INET, SOCK_DGRAM, 0, 0, 0)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_net_connect_bind returned %d\n\n", ret);
        goto exit;
    }
#endif

    /*
     * 1. Start the connection
     */
    mbedtls_printf("  . Connecting to udp ip %s port %s...", tls->public_ip, tls->public_port);

    if ((ret = mbedtls_net_connect(&tls->dtls_fd, tls->public_ip,
                                   tls->public_port, MBEDTLS_NET_PROTO_UDP)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    /*
     * 2. Setup stuff
     */
    mbedtls_printf("  . Setting up the DTLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&tls->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto exit;
    }

    /* OPTIONAL is usually a bad choice for security, but makes interop easier
     * in this simplified example, in which the ca chain is hardcoded.
     * Production code should set a proper ca chain and use REQUIRED. */
    mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&tls->conf, &tls->cacert, NULL);
    mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->ctr_drbg);
    mbedtls_ssl_conf_dbg(&tls->conf, my_debug, 0);

    if ((ret = mbedtls_ssl_setup(&tls->ssl, &tls->conf)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_set_hostname(&tls->ssl, "localhost")) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_set_bio(&tls->ssl, &tls->dtls_fd,
                        mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    mbedtls_ssl_set_timer_cb(&tls->ssl, &tls->timer, mbedtls_timing_set_delay,
                             mbedtls_timing_get_delay);

    mbedtls_printf(" ok\n");
    return 0;

exit:

#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf);
    }
#endif

    mbedtls_net_free(&tls->dtls_fd);

    mbedtls_x509_crt_free(&tls->cacert);
    mbedtls_ssl_free(&tls->ssl);
    mbedtls_ssl_config_free(&tls->conf);
    mbedtls_ctr_drbg_free(&tls->ctr_drbg);
    mbedtls_entropy_free(&tls->entropy);


    /* Shell can not handle large exit numbers -> 1 for errors */
    if (ret < 0) {
        ret = 1;
    }

    return -1;
}


static int dtls_handshake(struct tls_struct *tls)
{
    int ret = 0;
    int flags = 0;
    char vrfy_buf[512];
    /*
     * 4. Handshake
     */
    mbedtls_printf("  . Performing the SSL/TLS handshake...");

    do {
        ret = mbedtls_ssl_handshake(&tls->ssl);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
             ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
        return -1;
    }

    mbedtls_printf(" ok\n");

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf("  . Verifying peer X.509 certificate...");

    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
     * handshake would not succeed if the peer's cert is bad.  Even if we used
     * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(&tls->ssl)) != 0) {

        mbedtls_printf(" failed\n");

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

        mbedtls_printf("%s\n", vrfy_buf);
    } else {
        mbedtls_printf(" ok\n");
    }
    tls->dtls_fd.handshake_ok = 1;
    return 0;
}

static int dtls_write(struct tls_struct *tls, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;
    /*printf("%s  %d\n",__func__,__LINE__);*/
    os_mutex_pend(&tls->dtls_fd.mutex, 0);
    /*do {*/
//        printf("%s  %d\n",__func__,__LINE__);
    ret = mbedtls_ssl_write(&tls->ssl, (unsigned char *) buf, len);
//        printf("%s  %d\n",__func__,__LINE__);
    /*} while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);*/
//printf("%s  %d\n",__func__,__LINE__);
    if (ret < 0) {
        mbedtls_printf(" mbedtls_ssl_write returned %d\n", ret);
        os_mutex_post(&tls->dtls_fd.mutex);
        return -1;
    }
    os_mutex_post(&tls->dtls_fd.mutex);
    /*printf("%s  %d\n",__func__,__LINE__);*/
    return 0;

}



static int dtls_read(struct tls_struct *tls, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;
    /*printf("%s  %d\n",__func__,__LINE__);*/
    os_mutex_pend(&tls->dtls_fd.mutex, 0);
    /*do {*/
//        printf("%s  %d\n",__func__,__LINE__);
    ret = mbedtls_ssl_read(&tls->ssl, (unsigned char *) buf, len);
//        printf("%s  %d\n",__func__,__LINE__);
    /*} while (ret == MBEDTLS_ERR_SSL_WANT_READ);*/
//    printf("%s  %d\n",__func__,__LINE__);
    if (ret < 0) {
        mbedtls_printf("mbedtls_ssl_read returned %d\n", ret);
        os_mutex_post(&tls->dtls_fd.mutex);
        return -1;
    }
    os_mutex_post(&tls->dtls_fd.mutex);
    /*printf("%s  %d\n",__func__,__LINE__);*/
    return ret;
}


static int dtls_ioctl(struct tls_struct *tls, u32 cmd, u32 arg)
{
    int ret = 0;
    switch (cmd) {
    case TLS_SET_CA:
        break;
    default:
        break;

    }
    return ret;
}


static int dtls_close(struct tls_struct *tls)
{
    mbedtls_net_free(&tls->dtls_fd);
    mbedtls_x509_crt_free(&tls->cacert);
    mbedtls_ssl_free(&tls->ssl);
    mbedtls_ssl_config_free(&tls->conf);
    mbedtls_ctr_drbg_free(&tls->ctr_drbg);
    mbedtls_entropy_free(&tls->entropy);
    return 0;
}


const  struct _tls_ops tls_cli_ops = {
    .dinit = dtls_client_init,
    .handshake = dtls_handshake,
    .dwrite = dtls_write,
    .dread = dtls_read,
    .dioctl = NULL,
    .duninit = dtls_close,
};





