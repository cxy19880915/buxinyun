#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"

#include "app_config.h"


static char *const sd_list[][2] = {
    { "sd0", CONFIG_SD0_PATH },
    { "sd1", CONFIG_SD1_PATH },
    { "sd2", CONFIG_SD2_PATH },
};


extern int upgrade_file_detect(const char *path);

int storage_device_ready()
{
    return fdir_exist(CONFIG_STORAGE_PATH);
}

static u32 class;
static u8 fs_mount = 0;

int mount_sd_to_fs(const char *name)
{
    struct imount *mt;
    int id = ((char *)name)[2] - '0';
    const char *dev  = sd_list[id][0];
    const char *path = sd_list[id][1];

    if (fs_mount) {
        return 0;
    }
    fs_mount = 1;

    void *fd = dev_open(dev, 0);
    if (!fd) {
        fs_mount = 0;
        return -EFAULT;
    }
    dev_ioctl(fd, SD_IOCTL_GET_CLASS, (u32)&class);
    if (class == SD_CLASS_10) {
        puts("sd_class: 10\n");
    } else {
        puts("sd is not class 10\n");
    }
    dev_close(fd);

    mt = mount(dev, path, "fat", NULL);
    if (!mt) {
        puts("mount fail\n");
        fs_mount = 0;
        return -EFAULT;
    }

    return 0;
}
/*
 * sd卡插拔事件处理
 */
static void sd_event_handler(struct sys_event *event)
{
    int id = ((char *)event->arg)[2] - '0';
    const char *dev  = sd_list[id][0];
    const char *path = sd_list[id][1];

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        mount_sd_to_fs(event->arg);
        break;
    case DEVICE_EVENT_OUT:
        printf("%s: out\n", dev);
        unmount(path);
        fs_mount = 0;
        break;
    }
}


static void device_event_handler(struct sys_event *event)
{
    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        sd_event_handler(event);
    } else if (!ASCII_StrCmp(event->arg, "usb", 4)) {

    }
}
/*
 * 静态注册设备事件回调函数，优先级为0
 */
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, device_event_handler, 0);





