#include "sock_api/sock_api.h"
#include "os/os_compat.h"
#include "common/common.h"

#include "tls_api.h"
#include "kcp_api.h"
#include "p2p_dev.h"








void tls_kcp_cli()
{
    fh->tls_ops = &tls_cli_ops;
    inet_ntoa_r(p->remote_addr.sin_addr.s_addr, fh->tls.public_ip, sizeof(fh->tls.public_ip));
    itoa(ntohs(p->remote_addr.sin_port), fh->tls.public_port, 10);


    ret =  fh->tls_ops->dinit(&fh->tls);
    if (ret) {
        log_e(" tls init fail\n");
        free(fh);
        return -EINVAL;
    }
}


}
