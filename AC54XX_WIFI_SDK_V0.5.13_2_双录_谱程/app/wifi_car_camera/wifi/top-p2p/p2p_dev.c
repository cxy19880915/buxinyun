#include <stdlib.h>
#include "device/device.h"
#include "fs/fs.h"
#include "jiffies.h"
#include "sock_api/sock_api.h"
#include "os/os_compat.h"
#include "system/event.h"
#include "system/timer.h"
#include "common/common.h"

#include "tls_api.h"
#include "kcp_api.h"
#include "p2p_dev.h"




struct p2p_dev_control {
    OS_MUTEX mutex;
    struct list_head dev_list;
};

#define P2P_INITING 0x1
#define P2P_HANDSHARKE 0x2

struct p2p_fh {
    struct list_head entry;
    u32 ref;

    //for tls
    struct tls_struct tls;
    struct _tls_ops   *tls_ops;

    //for kcp
    struct kcp_struct kcp;
    struct _kcp_ops   *kcp_ops;

    u32 channel;    //区分数据
    u32 prio; //当前句柄优先级
    u8 type;

    struct sockaddr_in remote_addr;

    u32 status;
    OS_MUTEX mutex;

    struct device device;
    void *private_data;
};

static struct p2p_dev_control p2p_dev;

#define __this  (&p2p_dev)

#define list_for_each_p2p(fh) \
        list_for_each_entry(fh, &__this->dev_list, entry)

#define list_add_p2p(fh) \
        list_add(&fh->entry, &__this->dev_list)

#define list_del_p2p(fh) \
        list_del(&fh->entry);


/*实现KCP 对外接口 */
static int tls_output(const char *buf, int len, struct IKCPCB *pcb, void *user)
{
    int ret = 0;
    struct kcp_struct *kcp = (struct kcp_struct *)user;
    struct p2p_fh *fh = (struct p2p_fh *)kcp->private_data;
    ret = fh->tls_ops->dwrite(&fh->tls, buf, len, 0);

    if (ret < 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
        return -1;
    }

    return 0;
}


static void kcp_local_thread_update(void *priv)
{
    struct kcp_struct *kcp = (struct kcp_struct *)priv;
    struct p2p_fh *fh = (struct p2p_fh *)kcp->private_data;

    while (1) {
        msleep(20);
        kcp->kcp_update(kcp->kcp_hdl, timer_get_ms);
    }

}

static void kcp_local_thread(void *priv)
{
#define KCP_MAX_BUFFER_SIZE (44 * 1472)
    struct kcp_struct *kcp = (struct kcp_struct *)priv;
    struct p2p_fh *fh = (struct p2p_fh *)kcp->private_data;
    int ret;
    char buf[KCP_MAX_BUFFER_SIZE];

    while (1) {
        if (fh->status & P2P_HANDSHARKE || fh->type & RAW_DATA) {
            ret = fh->tls_ops->dread(&fh->tls, buf, KCP_MAX_BUFFER_SIZE, 0);
            if (ret <= 0) {
                printf(" DDD_READ RET ERR = %d \r\n", ret);
                break;
            }
            kcp->kcp_input(kcp->kcp_hdl, buf, ret);
        } else {
            msleep(100);
        }
    }

}


/******************************************************************/


static int p2p_dev_init(const struct dev_node *node, void *_data)
{
    os_mutex_create(&__this->mutex);
    INIT_LIST_HEAD(&__this->dev_list);
    return 0;
}


static int p2p_dev_open(const struct dev_node *node, struct device **device, void *arg)
{
    int ret;
    struct p2p_fh *fh = NULL;
    struct p2p_format *p = (struct p2p_format *)arg;
    log_i("p2p_dev_open\n");
    os_mutex_pend(&__this->mutex, 0);
    list_for_each_p2p(fh) {
        if (fh->remote_addr.sin_port == p->remote_addr.sin_port) {
            fh->ref++;
            *device = &fh->device;
            (*device)->private_data = fh;
            os_mutex_post(&__this->mutex);
            return 0;
        }
    }

    fh = (struct p2p_fh *)zalloc(sizeof(*fh));
    if (!fh) {
        os_mutex_post(&__this->mutex);
        return -ENOMEM;
    }

    fh->channel = p->channel;
    fh->prio = p->prio;
    fh->type = p->type;
    memcpy(&fh->remote_addr, &p->remote_addr, sizeof(struct sockaddr));




    if (p->type & DATA_WITH_TLS1_2) {
        log_i("use tls1.2 \n");

        //client ops
        if (p->ntype & TLS_IS_CLIENT) {
            fh->tls_ops = &tls_cli_ops;
            inet_ntoa_r(p->remote_addr.sin_addr.s_addr, fh->tls.public_ip, sizeof(fh->tls.public_ip));
            itoa(ntohs(p->remote_addr.sin_port), fh->tls.public_port, 10);
            ret =  fh->tls_ops->dinit(&fh->tls);
            if (ret) {
                log_e(" tls init fail\n");
                free(fh);
                return -EINVAL;
            }
        } else if (p->ntype & TLS_IS_SERVER) {

            //server ops
            fh->tls_ops = &tls_ser_ops;

            inet_ntoa_r(p->local_addr.sin_addr.s_addr, fh->tls.public_ip, sizeof(fh->tls.public_ip));
            itoa(ntohs(p->local_addr.sin_port), fh->tls.public_port, 10);
            ret =  fh->tls_ops->dinit(&fh->tls);
            if (ret) {
                log_e(" tls init fail\n");
                free(fh);
                return -EINVAL;
            }

        } else {
            log_e("no set net type \n");
            free(fh);
            return -1;
        }
    } else if (p->type & RAW_DATA) {
        fh->tls_ops = &tls_raw_ops;

        if (p->ntype & TLS_IS_CLIENT) {
            inet_ntoa_r(p->remote_addr.sin_addr.s_addr, fh->tls.public_ip, sizeof(fh->tls.public_ip));
            itoa(ntohs(p->remote_addr.sin_port), fh->tls.public_port, 10);
        } else if (p->ntype & TLS_IS_SERVER) {
            inet_ntoa_r(p->local_addr.sin_addr.s_addr, fh->tls.public_ip, sizeof(fh->tls.public_ip));
            itoa(ntohs(p->local_addr.sin_port), fh->tls.public_port, 10);
        }
        ret =  fh->tls_ops->dinit(&fh->tls);
        if (ret) {
            log_e(" tls init fail\n");
            free(fh);
            return -EINVAL;
        }

    }

    if (p->type & DATA_WITH_KCP) {
        log_i("use kcp \n");
        fh->kcp_ops   = &kcp_ops;
        fh->kcp.kcp_conv = 1;
        fh->kcp.kcp_output = tls_output;
        fh->kcp.private_data = fh;
        ret = fh->kcp_ops->kinit(&fh->kcp);
        if (ret) {
            log_e("kcp init fail\n");
            free(fh);
            return -EINVAL;
        }

        //create recv thread
        thread_fork("kcp_recv_thread_ddd", 6, 0x8000, 0, 0, kcp_local_thread, (void *)&fh->kcp);
        thread_fork("kcp_local_thread_update", 5, 0x4000, 0, 0, kcp_local_thread_update, (void *)&fh->kcp);

    }


    *device = &fh->device;
    (*device)->private_data = fh;
    fh->ref = 1;

    os_mutex_post(&__this->mutex);
    fh->status = P2P_INITING;
    return 0;

}


static int p2p_dev_ioctl(struct device *device, u32 cmd, u32 arg)
{
    int ret = 0;
    struct p2p_fh *fh = (struct p2p_fh *)device->private_data;

    switch (cmd) {
    case P2PIOC_HANDSHARK:
        log_i("P2PIOC_HANDSHARK\n");
        if (fh->tls_ops->handshake) {
            ret = fh->tls_ops->handshake(&fh->tls);
            if (!ret) {
                fh->status = P2P_HANDSHARKE;
            }
        }
        break;
    case P2PIOC_SERVER_ACCEPT:
        ret = fh->tls_ops->dioctl(&fh->tls, TLS_SERVER_ACCEPT, 0);
        break;
    default:
        break;
    }

    return ret;
}


static int p2p_dev_read(struct device *device, void *buf, u32 len, u32 flag)
{
    int ret = 0;
    struct p2p_fh *fh = (struct p2p_fh *)device->private_data;
    if (fh->type & DATA_WITH_KCP) {
        ret = fh->kcp_ops->kread(&fh->kcp, buf, len, flag);
    } else {
        ret = fh->tls_ops->dread(&fh->tls, buf, len, flag);
    }

    return ret;


}

static int p2p_dev_write(struct device *device, void *buf, u32 len, u32 flag)
{
    int ret = 0;
    struct p2p_fh *fh = (struct p2p_fh *)device->private_data;
    if (fh->type & DATA_WITH_KCP) {
        ret = fh->kcp_ops->kwrite(&fh->kcp, buf, len, flag);
    } else {
        ret = fh->tls_ops->dwrite(&fh->tls, buf, len, flag);
    }
    return ret;
}

static int p2p_dev_close(struct device *device)
{
    struct p2p_fh *fh = (struct p2p_fh *)device->private_data;
    if (--fh->ref == 0) {
        log_i("ipc dev close sock fd \n");
        fh->tls_ops->duninit(&fh->tls);
        if (fh->type & DATA_WITH_KCP) {
            fh->kcp_ops->kuninit(&fh->kcp);
        }
        list_del_p2p(fh);
        free(fh);
    }
    return 0;
}




const struct device_operations p2p_dev_ops = {
    .init  =  p2p_dev_init,
    .open  =  p2p_dev_open,
    .ioctl =  p2p_dev_ioctl,
    .close =  p2p_dev_close,
    .read   = p2p_dev_read,
    .write   = p2p_dev_write,
};


