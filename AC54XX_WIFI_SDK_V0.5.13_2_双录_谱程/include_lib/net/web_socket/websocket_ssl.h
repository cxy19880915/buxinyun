#ifndef WEBSOCKET_SSL_H
#define WEBSOCKET_SSL_H

#include "websocket_define.h"
#include "websocket_api.h"
//#include "../mbedtls/config.h"
//#include "../mbedtls/platform.h"
//#include "../mbedtls/entropy.h"
//#include "../mbedtls/ctr_drbg.h"
//#include "../mbedtls/certs.h"
//#include "../mbedtls/ssl.h"
//#include "../mbedtls/net_sockets.h"


//struct websockets_mbedtls
//{
//    /*client*/
//    mbedtls_net_context server_fd;
//    mbedtls_entropy_context entropy;
//    mbedtls_ctr_drbg_context ctr_drbg;
//    mbedtls_ssl_context ssl;
//    mbedtls_ssl_config conf;
//    mbedtls_x509_crt cacert;
//    char ssl_fd;
//    /*server*/
//    mbedtls_net_context client_fd;//add client fd
//    mbedtls_x509_crt srvcert;
//    mbedtls_pk_context pkey;
//
//};
//
//typedef struct websockets_mbedtls WEBSOCKETS_MBTLS_INFO;

/**********client********************/
int  websockets_mbtls_client_init(struct websockets_mbedtls *wbsk_mbtls_info);
int  websockets_mbtls_client_connect(struct websockets_mbedtls *wbsk_mbtls_info, u8 *host, int port);
int  websockets_mbtls_client_handshack(struct websockets_mbedtls *wbsk_mbtls_info);
int  websockets_mbtls_client_write(struct websockets_mbedtls *wbsk_mbtls_info, u8 *buf, u64 *len, char type);
int  websockets_mbtls_client_read(struct websockets_mbedtls *wbsk_mbtls_info, u8 *buf, int len);
void websockets_mbtls_client_close(struct websockets_mbedtls *wbsk_mbtls_info);
void websockets_mbtls_client_exit(struct websockets_mbedtls *wbsk_mbtls_info);

/************server****************/
void websocket_mbedtls_serv_init(struct websockets_mbedtls *wbsk_mbtls_info);
int  websocket_mbedtls_serv_bind(struct websockets_mbedtls *wbsk_mbtls_info, int port);
int  websocket_mbedtls_serv_accept(struct websockets_mbedtls *wbsk_mbtls_info);
int  websockets_mbtls_serv_write(struct websockets_mbedtls *wbsk_mbtls_info, u8 *buf, u64 *len, char type);
int  websockets_mbtls_serv_read(struct websockets_mbedtls *wbsk_mbtls_info, u8 *buf, int len);
void websockets_mbtls_serv_close(struct websockets_mbedtls *wbsk_mbtls_info);
void websockets_mbtls_serv_exit(struct websockets_mbedtls *wbsk_mbtls_info);

#endif


