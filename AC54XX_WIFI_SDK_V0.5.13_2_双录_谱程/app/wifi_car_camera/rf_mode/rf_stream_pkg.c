#include "common/common.h"
#include "system/spinlock.h"
#include "os/os_api.h"

#include "system/includes.h"

#include "server/rt_stream_pkg.h"
#include "server/stream_core.h"

struct rt_stream_info *rf_vpkg_open(const char *path, const char *mode)
{
    struct rt_stream_info *info = (struct rt_stream_info *)calloc(1, sizeof(struct rt_stream_info));
    u8 type = 0;
    if (info == NULL) {
        printf("%s %d->Error in malloc()\n", __func__, __LINE__);
        return NULL;
    }
    if (!strcmp(mode, "1")) {
        type = 1;
    } else if (!strcmp(mode, "0")) {
        type = 0;
    }

    info->fd = dev_open("rf_phy", (void *)type);
    return info;
}

int rf_send_frame(struct rt_stream_info *info, char *buffer, size_t len, u8 type)
{
    int ret = 0;
    u32 t_len = len;
    ret = dev_write(info->fd, buffer, len);
    if (!ret) {
        return t_len;
    }
    return ret;
}

int rf_vpkg_close(struct rt_stream_info *info)
{
    dev_close(info->fd);
    free(info);
    return 0;
}

REGISTER_NET_VIDEO_STREAM_SUDDEV(rf_video_stream_sub) = {
    .name = "rf",
    .open = rf_vpkg_open,
    .write = rf_send_frame,
    .close = rf_vpkg_close,
};


