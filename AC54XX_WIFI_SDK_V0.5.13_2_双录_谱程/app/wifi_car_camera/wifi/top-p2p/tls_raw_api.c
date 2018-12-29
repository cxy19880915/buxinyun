

#include "tls_api.h"



extern int atoi(const char *__nptr);

static int dtls_client_init(struct tls_struct *tls)
{
    log_d("dtls_raw_init\n");
    struct sockaddr_in addr;
    int ret = 0;
    tls->fd = sock_reg(AF_INET, SOCK_DGRAM, 0, NULL, NULL);
    if (tls->fd == NULL) {
        printf("%s %d->Error in socket()\n", __func__, __LINE__);
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);// 远端IP
    addr.sin_port = htons(atoi(tls->public_port));

    ret = sock_bind(tls->fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        return -1;
    }

    u32 millsec = 100;

    sock_setsockopt(tls->fd, SOL_SOCKET, SO_SNDTIMEO, (const void *)&millsec, sizeof(millsec));
    return 0;

}

static int dtls_write(struct tls_struct *tls, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;
    struct sockaddr_in addr;


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(tls->public_ip);// 远端IP
    addr.sin_port = htons(atoi(tls->public_port));
    ret = sock_sendto(tls->fd, buf, len, flag, (struct sockaddr *)&addr, sizeof(addr));
    if (ret <= 0) {
        return -1;
    }
    return 0;

}


static int dtls_read(struct tls_struct *tls, u8 *buf, u32 len, u32 flag)
{
    int ret = 0;

    ret = sock_recv(tls->fd, buf, len, flag);
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

    sock_unreg(tls->fd);
    return 0;
}


const  struct _tls_ops tls_raw_ops = {
    .dinit = dtls_client_init,
    .handshake = NULL,
    .dwrite = dtls_write,
    .dread = dtls_read,
    .dioctl = NULL,
    .duninit = dtls_close,
};



