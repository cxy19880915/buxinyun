
#ifndef WEBSOCKET_DEF_H
#define WEBSOCKET_DEF_H

#include "string.h"

typedef unsigned char 	    u8;
typedef unsigned short 	    u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

#define     BIT(n)              (1UL << (n))

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))

/**********************************************/

#define WEBSOCKET_MODE      0   //不加密
#define WEBSOCKETS_MODE     1   //SSL加密

/*************************************/

#define CLIENNT_CHECK_INDEX         0
#define SERVER_CHECK_INDEX          1

#define SERVER_MASK_DISEN           3
#define CLIENNT_MASK_EN             4

#define HAND_SHAKE_BUF_MAX          2048
#define REQUEST_LEN_MAX             1462
#define FRAME_LEN_MAX               1462
#define ALL_FRAME_MAX               1462*5
#define DEFEULT_SERVER_PORT         8000
#define WEB_SOCKET_KEY_LEN_MAX      1024
#define RESPONSE_HEADER_LEN_MAX     1024
#define LINE_MAX                    256

#define WEBSOCKET_MAIN_THREAD_STK_SIZE	1024
#define WEBSOCKET_CLI_THREAD_STK_SIZE	1024

#define websocket_socket_force_close(sk_fd)  {shutdown(sk_fd,2);closesocket(sk_fd);sk_fd=0;}

char *websocket_packData(u8 *buf, u64 *len, char type, char mask_en); //websocket 数据打包函数

#endif



