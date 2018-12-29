#ifndef _KCP_API_H
#define _KCP_API_H


#include "kcp/ikcp.h"
#include "generic/typedef.h"
#include "os/os_api.h"

struct kcp_struct {
    ikcpcb *kcp_hdl;

    OS_MUTEX pmutex;
    OS_SEM psendsem;
    OS_SEM precvsem;


    int kcp_conv;
    int (*kcp_output)(const char *, int, struct IKCPCB *, void *);
    int (*kcp_input)(ikcpcb *, const char *, long);
    void (*kcp_update)(ikcpcb *kcp, unsigned int (*iclock)(void));
    void *private_data;
};



struct _kcp_ops {

    int (*kinit)(struct kcp_struct *);
    int (*kwrite)(struct kcp_struct *, u8 *, u32, u32);
    int (*kioctl)(struct kcp_struct *, u32, u32);
    int (*kread)(struct kcp_struct *, u8 *, u32, u32);
    int (*kuninit)(struct kcp_struct *);

};




extern const struct _kcp_ops kcp_ops;



#endif
