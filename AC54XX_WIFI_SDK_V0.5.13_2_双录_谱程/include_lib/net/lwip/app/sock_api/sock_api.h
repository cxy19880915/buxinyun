#ifndef __SOCK_API_H__
#define __SOCK_API_H__

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os/os_compat.h"
#include "list.h"
#include "string.h"

enum sock_api_msg_type {
    SOCK_SEND_TO,
    SOCK_RECV_TO,
    SOCK_EVENT_ALWAYS,
    SOCK_CONNECT_SUCC,
    SOCK_CONNECT_FAIL,
    SOCK_UNREG,
};
struct sock_hdl {
    int sock;
    int (*cb_func)(enum sock_api_msg_type type, void *priv);
    void *priv;
    int connect_to;
    int send_to, send_to_flag;
    int recv_to, recv_to_flag;
    OS_MUTEX send_mtx;
    OS_MUTEX recv_mtx;
    //OS_MUTEX close_mtx;
    int quit;
    unsigned int magic;
};

void sock_clr_quit(void *sock_hdl);
void sock_set_quit(void *sock_hdl);
int sock_get_quit(void *sock_hdl);
int sock_select(void *sock_hdl, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *tv);
int sock_get_socket(void *hdl);
void *sock_reg(int domain, int type, int protocol, int (*cb_func)(enum sock_api_msg_type type, void *priv), void *priv);
void sock_unreg(void *sock_hdl);
void sock_set_recv_timeout(void *sock_hdl, unsigned int millsec);
int sock_recv_timeout(void *sock_hdl);
int sock_would_block(void *sock_hdl);
void sock_set_send_timeout(void *sock_hdl, unsigned int millsec);
int sock_send_timeout(void *sock_hdl);
int sock_recv(void *sock_hdl, char *buf, unsigned int len, int flag);
int sock_send(void *sock_hdl, char *buf, unsigned int len, int flag);
void *sock_accept(void *sock_hdl, struct sockaddr *addr, socklen_t *addrlen, int (*cb_func)(enum sock_api_msg_type type, void *priv), void *priv);
int sock_bind(void *sock_hdl, const struct sockaddr *name, socklen_t namelen);
int sock_listen(void *sock_hdl, int backlog);
void sock_set_connect_to(void *sock_hdl, int sec);
int sock_get_error(void *sock_hdl);
int sock_connect(void *sock_hdl, const struct sockaddr *name, socklen_t namelen);
int sock_select_rdset(void *sock_hdl);
int sock_fcntl(void *sock_hdl, int cmd, int val);
int socket_set_keepalive(void *sock_hdl, int keep_idle, int keep_intv, int keep_cnt);
int sock_set_reuseaddr(void *sock_hdl);
int sock_getsockopt(void *sock_hdl, int level, int optname, void *optval, socklen_t *optlen);

int sock_recvfrom(void *sock_hdl, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);

int sock_setsockopt(void *sock_hdl, int level, int optname, const void *optval, socklen_t optlen);

int sock_sendto(void *sock_hdl, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);

int sock_getpeername(void *sock_hdl, struct sockaddr *name, socklen_t *namelen);
int sock_getsockname(void *sock_hdl, struct sockaddr *name, socklen_t *namelen);

#endif

