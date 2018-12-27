#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"

#include "app_config.h"

enum {
    SD_UNMOUNT,
    SD_MOUNT_SUSS,
    SD_MOUNT_FAILD,
};

static char *const sd_list[] = {
    "sd0",
    "sd1",
    "sd2",
};

static u32 class;
static u8 fs_mount = SD_UNMOUNT;
static OS_MUTEX sd_mutex = OS_MUTEX_INIT();


int mount_sd_to_fs(const char *name)
{
    int err = 0;
    struct imount *mt;
    int id = ((char *)name)[2] - '0';
    const char *dev  = sd_list[id];

    err = os_mutex_pend(&sd_mutex, 0);
    if (err) {
        return -EFAULT;
    }


    if (fs_mount == SD_MOUNT_SUSS) {
        goto __exit;
    }
    if (fs_mount == SD_MOUNT_FAILD) {
        err = -EFAULT;
        goto __exit;
    }
    if (!dev_online(dev)) {
        err = -EFAULT;
        goto __exit;
    }

    void *fd = dev_open(dev, 0);
    if (!fd) {
        err = -EFAULT;
        goto __err;
    }
    dev_ioctl(fd, SD_IOCTL_GET_CLASS, (u32)&class);
    if (class == SD_CLASS_10) {
        puts("sd_class: 10\n");
    } else {
        puts("sd is not class 10\n");
    }
    dev_close(fd);

    mt = mount(dev, CONFIG_STORAGE_PATH, "fat", NULL);
    if (!mt) {
        puts("mount fail\n");
        err = -EFAULT;
    } else {
        puts("mount sd suss\n");
    }

__err:
    fs_mount = err ? SD_MOUNT_FAILD : SD_MOUNT_SUSS;
__exit:
    os_mutex_post(&sd_mutex);

    return err;
}

void unmount_sd_to_fs(const char *path)
{
    os_mutex_pend(&sd_mutex, 0);

    unmount(path);
    fs_mount = SD_UNMOUNT;

    os_mutex_post(&sd_mutex);
}

int storage_device_ready()
{
    if (fs_mount == SD_UNMOUNT && dev_online(SDX_DEV)) {
        mount_sd_to_fs(SDX_DEV);
    }

    return fdir_exist(CONFIG_STORAGE_PATH);
}

int storage_device_format()
{
    int err;

    unmount_sd_to_fs(CONFIG_STORAGE_PATH);

    err = f_format(SDX_DEV, "fat", 32 * 1024);
    if (err == 0) {
        mount_sd_to_fs(SDX_DEV);
    }

    return err;
}


/*
 * sd卡插拔事件处理
 */
static void sd_event_handler(struct sys_event *event)
{
    int id = ((char *)event->arg)[2] - '0';
    const char *dev  = sd_list[id];

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        mount_sd_to_fs(event->arg);
        break;
    case DEVICE_EVENT_OUT:
        printf("%s: out\n", dev);
        unmount_sd_to_fs(CONFIG_STORAGE_PATH);
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





