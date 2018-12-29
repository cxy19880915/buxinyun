

#include "tls_api.h"
#define DEBUG_LEVEL 0

static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    ((void) level);
    //mbedtls_printf( "%s:%04d: %s", line, str);
}

/*---------------------------------------------------------------------------------*/



static int dtls_server_init(struct tls_struct *tls)
{
    printf("dtls_server_init\n");
    const char *pers = "dtls_server";
    int ret = 0;
    mbedtls_net_init(&tls->listen_fd);
    mbedtls_net_init(&tls->client_fd);
    os_mutex_create(&tls->client_fd.mutex);
    mbedtls_ssl_init(&tls->ssl);
    mbedtls_ssl_config_init(&tls->conf);
    mbedtls_ssl_cookie_init(&tls->cookie_ctx);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init(&tls->cache);
#endif
    mbedtls_x509_crt_init(&tls->srvcert);
    mbedtls_pk_init(&tls->pkey);
    mbedtls_entropy_init(&tls->entropy);
    mbedtls_ctr_drbg_init(&tls->ctr_drbg);
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_ticket_init(&tls->ticket_ctx);
#endif

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    /*
     * 1. Load the certificates and private RSA key
     */
    printf("\n  . Loading the server cert. and key...");

    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse(&tls->srvcert, (const unsigned char *) mbedtls_test_srv_crt,
                                 mbedtls_test_srv_crt_len);
    if (ret != 0) {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
        goto exit;
    }

    ret = mbedtls_x509_crt_parse(&tls->srvcert, (const unsigned char *) mbedtls_test_cas_pem,
                                 mbedtls_test_cas_pem_len);
    if (ret != 0) {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
        goto exit;
    }

    ret =  mbedtls_pk_parse_key(&tls->pkey, (const unsigned char *) mbedtls_test_srv_key,
                                mbedtls_test_srv_key_len, NULL, 0);
    if (ret != 0) {
        printf(" failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
        goto exit;
    }

    printf(" ok\n");

    /*
     * 2. Setup the "listening" UDP socket
     */
    printf("  . Bind on udp/*/4433 ...");
    mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    if ((ret = mbedtls_net_bind(&tls->listen_fd, NULL, tls->public_port, MBEDTLS_NET_PROTO_UDP)) != 0) {
        printf(" failed\n  ! mbedtls_net_bind returned %d\n\n", ret);
        goto exit;
    }

    printf(" ok\n");

    /*
     * 3. Seed the RNG
     */
    printf("  . Seeding the random number generator...");

    if ((ret = mbedtls_ctr_drbg_seed(&tls->ctr_drbg, mbedtls_entropy_func, &tls->entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    printf(" ok\n");

    /*
     * 4. Setup stuff
     */
    printf("  . Setting up the DTLS data...");

    if ((ret = mbedtls_ssl_config_defaults(&tls->conf,
                                           MBEDTLS_SSL_IS_SERVER,
                                           MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->ctr_drbg);
//    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&tls->conf, &tls->cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set);
#endif
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    if ((ret = mbedtls_ssl_ticket_setup(&tls->ticket_ctx,
                                        mbedtls_ctr_drbg_random, &tls->ctr_drbg,
                                        MBEDTLS_CIPHER_AES_256_GCM,
                                        86400)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_ticket_setup returned %d\n\n", ret);
        goto exit;
    }
    mbedtls_ssl_conf_session_tickets_cb(&tls->conf,
                                        mbedtls_ssl_ticket_write,
                                        mbedtls_ssl_ticket_parse,
                                        &tls->ticket_ctx);
#endif

    mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&tls->conf, tls->srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&tls->conf, &tls->srvcert, &tls->pkey)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_cookie_setup(&tls->cookie_ctx,
                                        mbedtls_ctr_drbg_random, &tls->ctr_drbg)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_cookie_setup returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_conf_dtls_cookies(&tls->conf, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check,
                                  &tls->cookie_ctx);

    if ((ret = mbedtls_ssl_setup(&tls->ssl, &tls->conf)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_set_timer_cb(&tls->ssl, &tls->timer, mbedtls_timing_set_delay,
                             mbedtls_timing_get_delay);

    printf(" ok\n");

    return 0;
exit:
    mbedtls_net_free(&tls->client_fd);
    mbedtls_net_free(&tls->listen_fd);

    mbedtls_x509_crt_free(&tls->srvcert);
    mbedtls_pk_free(&tls->pkey);
    mbedtls_ssl_free(&tls->ssl);
    mbedtls_ssl_config_free(&tls->conf);
    mbedtls_ssl_cookie_free(&tls->cookie_ctx);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&tls->cache);
#endif
    mbedtls_ctr_drbg_free(&tls->ctr_drbg);
    mbedtls_entropy_free(&tls->entropy);
    return -1;

}


static int dtls_handshake(struct tls_struct *tls)
{
    char vrfy_buf[512];
    int ret = 0;
    /*
     * 5. Handshake
     */
    printf("  . Performing the DTLS handshake...");

    do {
        ret = mbedtls_ssl_handshake(&tls->ssl);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
             ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
        printf(" hello verification requested\n");
        ret = 0;
        return 1;
    } else if (ret != 0) {
        printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
        return -1;
    }

    printf(" ok\n");
    printf("ca verify\n\n\n");
    u32 flags;
    if ((flags = mbedtls_ssl_get_verify_result(&tls->ssl)) != 0) {
        mbedtls_printf(" failed\n");

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
        mbedtls_printf("%s\n", vrfy_buf);
    } else {
        mbedtls_printf(" ok\n");
    }

    tls->client_fd.handshake_ok = 1;
    return 0;

}




static int dtls_write(struct tls_struct *tls, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    os_mutex_pend(&tls->client_fd.mutex, 0);
    /*do {*/
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    ret = mbedtls_ssl_write(&tls->ssl, (unsigned char *) buf, len);
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    /*} while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);*/
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    if (ret < 0) {
        mbedtls_printf(" mbedtls_ssl_write returned %d\n", ret);
        os_mutex_post(&tls->client_fd.mutex);
        return -1;
    }
    os_mutex_post(&tls->client_fd.mutex);
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    return 0;

}



static int dtls_read(struct tls_struct *tls, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    os_mutex_pend(&tls->client_fd.mutex, 0);
    /*do {*/
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    ret = mbedtls_ssl_read(&tls->ssl, (unsigned char *) buf, len);
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    /*} while (ret == MBEDTLS_ERR_SSL_WANT_READ);*/
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    if (ret < 0) {
        mbedtls_printf("mbedtls_ssl_read returned %d\n", ret);
        os_mutex_post(&tls->client_fd.mutex);
        return -1;
    }
    os_mutex_post(&tls->client_fd.mutex);
    /*printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);*/
    return ret;
}

static int dtls_server_accpet(struct tls_struct *tls)
{

    unsigned char client_ip[16] = { 0 };
    size_t cliip_len;
    int ret = 0;

    mbedtls_net_free(&tls->client_fd);

    mbedtls_ssl_session_reset(&tls->ssl);

    /*
     * 3. Wait until a client connects
     */
    printf("  . Waiting for a remote connection ...");

    if ((ret = mbedtls_net_accept(&tls->listen_fd, &tls->client_fd,
                                  client_ip, sizeof(client_ip), &cliip_len)) != 0) {
        printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
        return -1;
    }

    /* For HelloVerifyRequest cookies */
    if ((ret = mbedtls_ssl_set_client_transport_id(&tls->ssl,
               client_ip, cliip_len)) != 0) {
        printf(" failed\n  ! "
               "mbedtls_ssl_set_client_transport_id() returned -0x%x\n\n", -ret);
        return -1;
    }

    mbedtls_ssl_set_bio(&tls->ssl, &tls->client_fd,
                        mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    printf(" ok\n");
    return 0;


}
static int dtls_ioctl(struct tls_struct *tls, u32 cmd, u32 arg)
{
    printf("%s\n", __func__);
    int ret = 0;
    switch (cmd) {
    case TLS_SET_CA:
        break;
    case TLS_SERVER_ACCEPT:
        ret = dtls_server_accpet(tls);
        break;
    default:
        break;

    }
    return ret;
}


static int dtls_close(struct tls_struct *tls)
{
    mbedtls_net_free(&tls->client_fd);
    mbedtls_net_free(&tls->listen_fd);

    mbedtls_x509_crt_free(&tls->srvcert);
    mbedtls_pk_free(&tls->pkey);
    mbedtls_ssl_free(&tls->ssl);
    mbedtls_ssl_config_free(&tls->conf);
    mbedtls_ssl_cookie_free(&tls->cookie_ctx);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&tls->cache);
#endif
    mbedtls_ctr_drbg_free(&tls->ctr_drbg);
    mbedtls_entropy_free(&tls->entropy);

    return 0;
}


const  struct _tls_ops tls_ser_ops = {
    .dinit = dtls_server_init,
    .handshake = dtls_handshake,
    .dwrite = dtls_write,
    .dread = dtls_read,
    .dioctl = dtls_ioctl,
    .duninit = dtls_close,
};





