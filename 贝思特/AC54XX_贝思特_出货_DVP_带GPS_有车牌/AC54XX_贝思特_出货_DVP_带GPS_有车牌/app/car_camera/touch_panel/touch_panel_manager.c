#include "touch_panel_manager.h"

#ifdef CONFIG_TOUCH_PANEL_ENABLE

const struct device_operations touch_panel_dev_ops;
static void *iic = NULL;
static void *touch_panel = NULL;
static struct sw_touch_panel_platform_data *_touch_panel_data = NULL;
static struct touch_panel_points touch_points;

unsigned char _touch_panel_write(unsigned char w_chip_id, unsigned short register_address, unsigned char *buf, unsigned int len)
{
    unsigned char ret = true;

    if (!iic) {
        ret = false;
        puts("\n touch_panel iic wr err -1");
        goto __gcend;
    }

    if (!len) {
        ret = false;
        puts("\n touch_panel iic wr err -2");
        goto __gcend;
    }

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n touch_panel iic wr err 0\n");
        ret = false;
        goto __gcend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 8)) {
        puts("\n touch_panel iic wr err 1\n");
        ret = false;
        goto __gcend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n touch_panel iic wr err 1\n");
        goto __gcend;
    }

    delay(10);

    int i;
    for (i = 0; i < len - 1; i++) {
        if (dev_ioctl(iic, IIC_IOCTL_TX, buf[i])) {
            puts("\n touch_panel iic wr err 2\n");
            ret = false;
            goto __gcend;
        }
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, buf[len - 1])) {
        puts("\n touch_panel iic wr err 3\n");
        ret = false;
        goto __gcend;
    }

__gcend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;
}


unsigned char _touch_panel_read_32BIT(unsigned char w_chip_id, unsigned char r_chip_id, unsigned int register_address, unsigned char *buf, unsigned int len)
{
    unsigned char ret = true;

    if (!iic) {
        ret = false;
        puts("\n touch_panel iic rd err -1");
        goto __gdend;
    }

    if (!len) {
        ret = false;
        puts("\n touch_panel iic rd err -2");
        goto __gdend;
    }

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n touch_panel iic rd err 0\n");
        ret = false;
        goto __gdend;
    }

    delay(10);
    #if 0
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 24)) {
        puts("\n touch_panel iic rd err 1\n");
        ret = false;
        goto __gdend;
    }
    #endif
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 16)) {
        puts("\n touch_panel iic rd err 1\n");
        ret = false;
        goto __gdend;
    }
    
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 8)) {
        puts("\n touch_panel iic rd err 1\n");
        ret = false;
        goto __gdend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n touch_panel iic rd err 1\n");
        ret = false;
        goto __gdend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, r_chip_id)) {
        ret = false;
        puts("\n touch_panel iic rd err 2\n");
        goto __gdend;
    }

    delay(10);

    int i;
    for (i = 0; i < len - 1; i++) {
        if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_ACK, (u32)&buf[i])) {
            ret = false;
            puts("\n touch_panel iic rd err 3\n");
            goto __gdend;
        }
    }

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&buf[len - 1])) {
        ret = false;
        puts("\n touch_panel iic rd err 4\n");
        goto __gdend;
    }
__gdend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;
}

unsigned char _touch_panel_write_32BIT(unsigned char w_chip_id, unsigned int register_address, unsigned char *buf, unsigned int len)
{
    unsigned char ret = true;

    if (!iic) {
        ret = false;
        puts("\n touch_panel iic wr err -1");
        goto __gcend;
    }

    if (!len) {
        ret = false;
        puts("\n touch_panel iic wr err -2");
        goto __gcend;
    }

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n touch_panel iic wr err 0\n");
        ret = false;
        goto __gcend;
    }

    delay(10);
    #if 0
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 24)) {
        puts("\n touch_panel iic wr err 1\n");
        ret = false;
        goto __gcend;
    }
    #endif
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 16)) {
        puts("\n touch_panel iic wr err 1\n");
        ret = false;
        goto __gcend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 8)) {
        puts("\n touch_panel iic wr err 1\n");
        ret = false;
        goto __gcend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n touch_panel iic wr err 1\n");
        goto __gcend;
    }

    delay(10);

    int i;
    for (i = 0; i < len - 1; i++) {
        if (dev_ioctl(iic, IIC_IOCTL_TX, buf[i])) {
            puts("\n touch_panel iic wr err 2\n");
            ret = false;
            goto __gcend;
        }
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, buf[len - 1])) {
        puts("\n touch_panel iic wr err 3\n");
        ret = false;
        goto __gcend;
    }

__gcend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;
}

unsigned char _touch_panel_read(unsigned char w_chip_id, unsigned char r_chip_id, unsigned short register_address, unsigned char *buf, unsigned int len)
{
    unsigned char ret = true;

    if (!iic) {
        ret = false;
        puts("\n touch_panel iic rd err -1");
        goto __gdend;
    }

    if (!len) {
        ret = false;
        puts("\n touch_panel iic rd err -2");
        goto __gdend;
    }

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n touch_panel iic rd err 0\n");
        ret = false;
        goto __gdend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address >> 8)) {
        puts("\n touch_panel iic rd err 1\n");
        ret = false;
        goto __gdend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n touch_panel iic rd err 1\n");
        ret = false;
        goto __gdend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, r_chip_id)) {
        ret = false;
        puts("\n touch_panel iic rd err 2\n");
        goto __gdend;
    }

    delay(10);

    int i;
    for (i = 0; i < len - 1; i++) {
        if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_ACK, (u32)&buf[i])) {
            ret = false;
            puts("\n touch_panel iic rd err 3\n");
            goto __gdend;
        }
    }

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&buf[len - 1])) {
        ret = false;
        puts("\n touch_panel iic rd err 4\n");
        goto __gdend;
    }
__gdend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;
}


static int touch_panel_init();
int  _touch_panel_init(const struct dev_node *node, void *_data)
{
    _touch_panel_data = (struct sw_touch_panel_platform_data *)_data;
    //touch_panel_init();
    return 0;

}

int  _touch_panel_open(const struct dev_node *node,  struct device **device, void *arg)
{

    if (!iic) {
        iic = dev_open(_touch_panel_data->iic_dev, 0);
        if (!iic) {
            puts("\n  open iic dev for touch panel err\n");
            return -EINVAL;
        }
    }

    TOUCH_PANEL_INTERFACE *c;


    list_for_each_touch_panel(c) {
        if (TRUE == c->touch_panel_ops->touch_panel_check(_touch_panel_data)) {

            printf("\ncur touch panel logo:::  %s\n", c->touch_panel_ops->logo);

            *device = &c->dev;

            (*device)->private_data = c;

            if (c->touch_panel_ops->touch_panel_init) {
                c->touch_panel_ops->touch_panel_init();
            }

            return 0;
        }
    }

    return -EINVAL;

}


int _touch_panel_ioctl(struct device *device, u32 cmd, u32 arg)
{

    TOUCH_PANEL_INTERFACE *_touch_panel = (TOUCH_PANEL_INTERFACE *)device->private_data;

    if (_touch_panel->touch_panel_ops->touch_panel_ioctl) {
        return _touch_panel->touch_panel_ops->touch_panel_ioctl(cmd, arg);
    }

    return -EINVAL;
}
/*
int _touch_panel_read(struct device *device, void *buf, u32 len, u32 arg)
{

    TOUCH_PANEL_INTERFACE *_touch_panel = (TOUCH_PANEL_INTERFACE *)device->private_data;

    if (_touch_panel->touch_panel_ops->touch_panel_read) {
        return _touch_panel->touch_panel_ops->touch_panel_read(buf, len, arg);
    }

    return -EINVAL;
}
*/
const struct device_operations touch_panel_dev_ops = {
    .init = _touch_panel_init,
    .open = _touch_panel_open,
    //.read = _touch_panel_read,
//    .write = touch_panel_write,
    .ioctl = _touch_panel_ioctl,
};


void *get_touch_panel()
{
    return touch_panel;
}

//#ifdef FLASE_POWER_OFF
void dev_close_touch()
{
    dev_close(touch_panel);   
}

//#endif
static int touch_panel_init()
{
    puts("\n  init touch_panel ======");
    if (!get_touch_panel()) {
        touch_panel = dev_open("touch_panel", 0);
        if (!touch_panel) {
            puts("\n touch panel mout fail, please check it\n");
        }
    }

    return 0;
}
late_initcall(touch_panel_init); 


#endif //  CONFIG_TOUCH_PANEL_ENABLE


