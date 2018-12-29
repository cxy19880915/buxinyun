#include "touch_panel_manager.h"

#ifdef CONFIG_TOUCH_PANEL_ENABLE

/* static struct touch_panel_points touch_points; */

struct touch_panel_info {
    void *iic;
    struct device device;
    struct sw_touch_panel_platform_data _touch_panel_data;
    TOUCH_PANEL_INTERFACE *interface;
};

static struct touch_panel_info fh;

u8 _touch_panel_write(u8 w_chip_id, u16 reg_addr, u8 *buf, u32 len)
{
    u8 ret = TRUE;

    if (!fh.iic) {
        ret = FALSE;
        puts(" touch_panel iic wr err -1\n");
        goto __gcend;
    }

    if (!len) {
        ret = FALSE;
        puts(" touch_panel iic wr err -2\n");
        goto __gcend;
    }

    dev_ioctl(fh.iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts(" touch_panel iic wr err 0\n");
        ret = FALSE;
        goto __gcend;
    }

    delay(10);

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX, reg_addr >> 8)) {
        puts(" touch_panel iic wr err 1\n");
        ret = FALSE;
        goto __gcend;
    }

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX, reg_addr & 0xff)) {
        puts(" touch_panel iic wr err 1\n");
        goto __gcend;
    }

    delay(10);

    int i;
    for (i = 0; i < len - 1; i++) {
        if (dev_ioctl(fh.iic, IIC_IOCTL_TX, buf[i])) {
            puts(" touch_panel iic wr err 2\n");
            ret = FALSE;
            goto __gcend;
        }
    }

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX_WITH_STOP_BIT, buf[len - 1])) {
        puts(" touch_panel iic wr err 3\n");
        ret = FALSE;
        goto __gcend;
    }

__gcend:

    dev_ioctl(fh.iic, IIC_IOCTL_STOP, 0);

    return ret;
}

u8 _touch_panel_read(u8 w_chip_id, u8 r_chip_id, u16 reg_addr, u8 *buf, u32 len)
{
    unsigned char ret = TRUE;

    if (!fh.iic) {
        ret = FALSE;
        puts(" touch_panel iic rd err -1\n");
        goto __gdend;
    }

    if (!len) {
        ret = FALSE;
        puts(" touch_panel iic rd err -2\n");
        goto __gdend;
    }

    dev_ioctl(fh.iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts(" touch_panel iic rd err 0\n");
        ret = FALSE;
        goto __gdend;
    }

    delay(10);

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX, reg_addr >> 8)) {
        puts(" touch_panel iic rd err 1\n");
        ret = FALSE;
        goto __gdend;
    }

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX, reg_addr & 0xff)) {
        puts(" touch_panel iic rd err 1\n");
        ret = FALSE;
        goto __gdend;
    }

    delay(10);

    if (dev_ioctl(fh.iic, IIC_IOCTL_TX_WITH_START_BIT, r_chip_id)) {
        ret = FALSE;
        puts(" touch_panel iic rd err 2\n");
        goto __gdend;
    }

    delay(10);

    int i;
    for (i = 0; i < len - 1; i++) {
        if (dev_ioctl(fh.iic, IIC_IOCTL_RX_WITH_ACK, (u32)&buf[i])) {
            ret = FALSE;
            puts(" touch_panel iic rd err 3\n");
            goto __gdend;
        }
    }

    if (dev_ioctl(fh.iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&buf[len - 1])) {
        ret = FALSE;
        puts(" touch_panel iic rd err 4\n");
        goto __gdend;
    }
__gdend:

    dev_ioctl(fh.iic, IIC_IOCTL_STOP, 0);

    return ret;
}


/* static int touch_panel_init(); */
static int _touch_panel_init(const struct dev_node *node, void *_data)
{
    /* _touch_panel_data = (struct sw_touch_panel_platform_data *)_data; */
    /* touch_panel_init(); */
    memset(&fh, 0, sizeof(struct touch_panel_info));
    memcpy(&fh._touch_panel_data, _data, sizeof(struct sw_touch_panel_platform_data));
    return 0;
}

static int _touch_panel_open(const struct dev_node *node, struct device **device, void *arg)
{
    TOUCH_PANEL_INTERFACE *c;
    int err;

    if (fh.interface) {
        puts("touch panel is already opened\n");
        return -EFAULT;
    }
    if (!fh.iic) {
        fh.iic = dev_open(fh._touch_panel_data.iic_dev, 0);
        if (!fh.iic) {
            puts("\n  open iic dev for touch panel err\n");
            return -EINVAL;
        }
    }

    list_for_each_touch_panel(c) {
        if (c->touch_panel_ops->touch_panel_check) {
            err = c->touch_panel_ops->touch_panel_check(&fh._touch_panel_data);
            if (err == TRUE) {
                printf("\ncur touch panel logo: %s\n\n", c->logo);
                if (c->touch_panel_ops->touch_panel_init) {
                    err = c->touch_panel_ops->touch_panel_init();
                    if (err == TRUE) {
                        fh.interface = c;
                        *device = &fh.device;
                        (*device)->private_data = &fh;
                        return 0;
                    }
                }
            }
        }
    }

    return -EINVAL;
}


static int _touch_panel_ioctl(struct device *device, u32 cmd, u32 arg)
{

    TOUCH_PANEL_INTERFACE *_touch_panel = ((struct touch_panel_info *)device->private_data)->interface;

    if (_touch_panel->touch_panel_ops->touch_panel_ioctl) {
        return _touch_panel->touch_panel_ops->touch_panel_ioctl(cmd, arg);
    }

    return -EINVAL;
}

/*
int touch_panel_read(struct device *device, void *buf, u32 len, u32 arg)
{

    TOUCH_PANEL_INTERFACE *_touch_panel = ((struct touch_panel_info *)device->private_data)->interface;

    if (_touch_panel->touch_panel_ops->touch_panel_read) {
        return _touch_panel->touch_panel_ops->touch_panel_read(buf, len, arg);
    }

    return -EINVAL;
}
*/
const struct device_operations touch_panel_dev_ops = {
    .init = _touch_panel_init,
    .open = _touch_panel_open,
    //.read = touch_panel_read,
//    .write = touch_panel_write,
    .ioctl = _touch_panel_ioctl,
};






static void *touch_panel = NULL;

static int touch_panel_startup_done(void)
{
    if (touch_panel) {
        return 1;
    } else {
        return 0;
    }
}

static void touch_panel_startup_task(void *arg)
{
    int err;
    u32 time;
    if (!touch_panel) {
        puts("\n====== init touch_panel ======\n");
        touch_panel = dev_open("touch_panel", 0);
        if (!touch_panel) {
            puts("\n touch panel mount fail, please check it\n");
        }
    }
    while (1) {
        os_time_dly(10);
    }
}

static int touch_panel_startup_finish(void *arg)
{
    os_task_del("touch_panel_startup");
    return 0;
}


static int touch_panel_startup(void)
{
    int err;
    err = os_task_create(touch_panel_startup_task, 0, 30, 256, 0, "touch_panel_startup");
    if (err) {
        printf("\nstart touch_panel_startup task fail\n\n");
        return -1;
    }
    wait_completion(touch_panel_startup_done, touch_panel_startup_finish, 0);
    return 0;
}
__initcall(touch_panel_startup);

#endif //  CONFIG_TOUCH_PANEL_ENABLE


