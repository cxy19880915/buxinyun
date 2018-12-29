
#ifndef _ABNORMAL_OFF_H_
#define _ABNORMAL_OFF_H_

#include <stdarg.h>
#include "generic/typedef.h"
#include "os/os_api.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/prot/ethernet.h"
#include <streaming_media_server/fenice_config.h>

typedef struct _FENICE_BANORMAL_OFF {
    unsigned char flag;
    unsigned char pend_flag;
    OS_SEM sem;
    OS_MUTEX mutex;
} FENICE_BANORMAL_OFF;


typedef struct _FENICE_TRANSFER {
    unsigned int type;  /* app层控制采用的协议类型--TCP/默认 */
    unsigned int port;  /* app层配置的端口号 */
    int (*exit)(void);  /* 关闭底层硬件 */
    int (*setup)(void);  /* 开启底层硬件 */
    int (*info)(struct fenice_source_info *info);
} FENICE_TRANSFER;

#endif

