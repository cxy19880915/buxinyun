#ifndef _P2P_DEV__H
#define _P2P_DEV__H


extern const struct device_operations p2p_dev_ops;

#include "lwip/sockets.h"






#define RAW_DATA             0x00001
#define DATA_WITH_KCP        0x00002
#define DATA_WITH_TLS1_2     0x00004

#define TLS_IS_CLIENT        0x00001
#define TLS_IS_SERVER        0x00002


#define P2PIOC_HANDSHARK                _IOW('U', 0, sizeof(unsigned int))
#define P2PIOC_SERVER_ACCEPT                _IOW('U', 1, sizeof(unsigned int))



struct p2p_format {
    u8 type;  // 使用协议
    u32 channel;    //区分数据
    u32 prio; //当前句柄优先级
    u8 ntype;   //网络类型 CLIENT  SERVER
    struct sockaddr_in remote_addr;
    struct sockaddr_in local_addr;
};

#endif
