


#include "kcp_api.h"
#include "common/common.h"


static int ikcp_sem_del(void *psem)
{
    return os_sem_del(psem, OS_DEL_ALWAYS);
}

static int ikcp_sem_post(void *psem)
{
    if (os_sem_query(psem) == 0) {
        os_sem_post(psem);
    }

    return 0;
}
static int ikcp_sem_pend(void *psem, int timeout)
{
    int ret;
    ret =  os_sem_pend(psem, timeout / 10);
    if (ret) {
        /*printf("kcp sem [0x%x]  pend to, ret = %d !!! \r\n", psem, ret);*/
    }
    if (ret != OS_TIMEOUT) {
        return ret;
    }
    return 0;
}

static int ikcp_mutex_lock(void *mutex)
{
    return os_mutex_pend(mutex, 0);

}
static void ikcp_mutex_unlock(void *mutex)
{
    os_mutex_post(mutex);
}

static void ikcp_mutex_del(void *mutex)
{
    os_mutex_del(mutex, OS_DEL_ALWAYS);
}



static int kcp_init(struct kcp_struct *fh)
{

#define UDP_MAX_MTU  1419//1472 /*UDP 1472, DTLS 1419  */
#define UDP_MAX_MSS (UDP_MAX_MTU-IKCP_HEAD_SIZE)
#define UDP_MAX_AGGREGATION_PKT_SIZE (1*UDP_MAX_MTU)

    ikcpcb *kcp_hdl = NULL;
    kcp_hdl = ikcp_create(fh->kcp_conv, (void *)fh);
    if (kcp_hdl == NULL) {
        goto exit;
    }

    kcp_hdl->output = (int (*)(const char *, int, struct IKCPCB *, void *))fh->kcp_output;
    ikcp_wndsize(kcp_hdl, 666, 512);    //set snd recv windows size
    ikcp_nodelay(kcp_hdl, 0, 2, 2, 0);    //set kcp mode

    ikcp_setmtu(kcp_hdl, UDP_MAX_MTU, UDP_MAX_AGGREGATION_PKT_SIZE);

    os_mutex_create(&fh->pmutex);
    ikcp_set_mutex_lock_func(kcp_hdl, ikcp_mutex_lock, ikcp_mutex_unlock, ikcp_mutex_del, &fh->pmutex);

    os_sem_create(&fh->psendsem, 0);
    ikcp_set_send_block(kcp_hdl, 0, ikcp_sem_post, ikcp_sem_pend, ikcp_sem_del, 0, &fh->psendsem);

    os_sem_create(&fh->precvsem, 0);
    ikcp_set_recv_block(kcp_hdl, 0, ikcp_sem_post, ikcp_sem_pend, ikcp_sem_del, 0, &fh->precvsem);



    fh->kcp_hdl = kcp_hdl;
    fh->kcp_input = ikcp_input;
    fh->kcp_update = ikcp_update;



    return 0;

exit:
    return -1;
}

static int kcp_send(struct kcp_struct *fh, u8 *buf, u32 len, u32 flag)
{

    int ret = 0;
    ret = ikcp_send(fh->kcp_hdl, buf, len);
    if (!ret) {
        ikcp_update(fh->kcp_hdl, timer_get_ms);
    }
    return ret;
}


static int kcp_recv(struct kcp_struct *fh, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;
    ret = ikcp_recv(fh->kcp_hdl, (const char *)buf, len);
    return ret;
}

static int kcp_uninit(struct kcp_struct *fh)
{
    ikcp_release(fh->kcp_hdl);
    return 0;
}

const struct _kcp_ops kcp_ops = {
    .kinit = kcp_init,
    .kwrite = kcp_send,
    .kioctl = NULL,
    .kread  = kcp_recv,
    .kuninit = kcp_uninit,

};
