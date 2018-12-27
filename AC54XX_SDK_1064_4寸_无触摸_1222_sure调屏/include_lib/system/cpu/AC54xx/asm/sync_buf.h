#ifndef __SYNC_BUF_H__
#define __SYNC_BUF_H__

#include "typedef.h"
#include "list.h"
#include "spinlock.h"
#include "asm/imd.h"


enum {
    BUF_IDLE,
    BUF_READ,
    BUF_WRITE,
    BUF_READY,
};

struct sb_buffer {
    struct list_head entry;
    u8 index;
    volatile u8 write_status;
    volatile u8 read_status;
    u8 *baddr;
};

struct sb_buffer *sync_buf_init(struct imd_dmm_info *info);
struct sb_buffer *sync_buf_read();
struct sb_buffer *sync_buf_write();
struct sb_buffer *sync_buf_write_done();
void sync_buf_free(struct sb_buffer *head);

#endif
