#include "server/server_core.h"
#include "system/includes.h"
#include "generic/typedef.h"
#include "errno-base.h"
#include "asm/imb_server.h"
#include "os/os_api.h"
#include "asm/cpu.h"
#include "generic/gpio.h"

OS_SEM imb_sem;
struct imbreq ireq = {0};

static void imb_request_handler(struct server_req *sreq)
{
    struct imbreq *req = (struct imbreq *)sreq->arg;

    /* dma_copy((u8 *)req->dst, (u8 *)req->src, req->len); */
    switch (sreq->type) {
    case IMB_CONV:
        break;
    default:
        break;
    }

    server_req_complete(sreq);
}

void convert_per_line(u32 src, u32 dst, u32 width) __attribute__((noinline));
void convert_per_line(u32 src, u32 dst, u32 width)
{
    u32 tmp;

    __asm__ volatile(
        " %1 = [%0++]\n\t"
        " .Lepeat_begin : \n\t"
        " rep %3 {\n\t"
        "  r5_r4 = mts0(%1, %1) # %1 = [%0++]\n\t"
        "  d[%2++] = r5_r4\n\t"
        "}\n\t"
        "if(%3!=0) goto .Lepeat_begin \n\t"
        :"=&r"(src), "=&r"(tmp), "=&r"(dst), "=&r"(width)
        :"3"(width), "1"(tmp), "0"(src), "2"(dst)
        :"r5", "r4"
    );
}

void yuv420_interpolation(struct imbreq *ireq)
{
    u32 h;
    u8 *sY, *sCb, *sCr;
    u8 *ssY, *ssCb, *ssCr;
    u8 *dY, *dCb, *dCr;

    sY  = ireq->src;
    sCb = &ireq->src[ireq->src_width * ireq->src_height];
    sCr = &ireq->src[ireq->src_width * ireq->src_height * 5 / 4];

    ssY  = &sY[ireq->src_y * ireq->src_width + ireq->src_x];
    ssCb = &sCb[(ireq->src_y / 2) * (ireq->src_width / 2) + ireq->src_x / 2];
    ssCr = &sCr[(ireq->src_y / 2) * (ireq->src_width / 2) + ireq->src_x / 2];

    dY  = ireq->dst;
    dCb	= &ireq->dst[ireq->dst_width * ireq->dst_height];
    dCr = &ireq->dst[ireq->dst_width * ireq->dst_height * 5 / 4];

    for (h = 0; h < ireq->dst_height; h ++) {
        convert_per_line((u32)&ssY[h * ireq->src_width], (u32)&dY[h * ireq->dst_width], (ireq->dst_width / 2) / 4);
        convert_per_line((u32)&ssCb[h * ireq->src_width / 4], (u32)&dCb[h * ireq->dst_width / 4], (ireq->dst_width / 2) / 4 / 4);
        convert_per_line((u32)&ssCr[h * ireq->src_width / 4], (u32)&dCr[h * ireq->dst_width / 4], (ireq->dst_width / 2) / 4 / 4);
    }
    /* flush_dcache(dY,ireq->dst_width * ireq->dst_height * 3 / 2); */
}

static void imb_server_task(void *arg)
{
    int res;
    u32 msg[32];
    u32 h = 0;
    u8 *sY, *sCb, *sCr;
    u8 *dY, *dCb, *dCr;

    while (1) {
        os_sem_pend(&imb_sem, 0);

        /* PORTB_DIR &= ~BIT(13); */
        /* PORTB_OUT |= BIT(13); */

        ireq.busy = true;
        yuv420_interpolation(&ireq);
        ireq.buf_index = !ireq.buf_index;
        ireq.busy = false;

        /* PORTB_OUT &= ~ BIT(13); */
    }
}


static void *imb_server_open(void *private, void *p)
{
    int err;

    err = task_create(imb_server_task, NULL, "imb_server");
    if (err != OS_NO_ERR) {
        task_delete("imb_server");
        goto _err;
    }

    os_sem_create(&imb_sem, 0);

    return (void *)1;

_err:
    return NULL;
}

static void imb_server_close(void *private)
{
    while (OS_TASK_NOT_EXIST != os_task_del_req("imb_server")) {
        os_taskq_post("imb_server", 1, 0);
        os_time_dly(1);
    }
}

SERVER_REGISTER(imb_copy) = {
    .name = "imb_server",
    .reqlen = 0,
    .open = imb_server_open,
    .close = imb_server_close,
};
