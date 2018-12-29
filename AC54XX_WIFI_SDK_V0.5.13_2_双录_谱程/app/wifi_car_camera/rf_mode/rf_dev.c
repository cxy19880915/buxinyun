#include "device/rf_dev.h"
#include "device/device.h"

static struct rf_platform_data *data = NULL;
static struct rf_phy_device *phy_dev = NULL;

static bool rf_module_online(const struct dev_node *node)
{
    if (phy_dev) {
        if (phy_dev->ops->online) {
            return phy_dev->ops->online(node);
        }
    }
    return false;
}



static int rf_module_init(struct dev_node *node, void *priv_data)
{
    data = (struct rf_platform_data *)priv_data;
    struct rf_phy_device *p = NULL;
    printf("rf module -> %s\n", node->name);
#if 1
    for (p = rf_phy_device_begin; p < rf_phy_device_end; p++) {
        if (!strcmp((const char *)data->name, p->name)) {
            printf("rf name -> %s \n", data->name);
            phy_dev = p;
            phy_dev->ops->init(node, data);

            return 0;
        }
    }
#endif
    printf("no rf module\n");
    return -1;

}

static int  rf_module_open(const struct dev_node *node,  struct device **device, void *arg)
{
    if (phy_dev) {
        if (phy_dev->ops->open) {
            return phy_dev->ops->open(node, device, arg);
        }
    }
    return -EINVAL;
}

static int rf_module_read(struct device *dev, void *buf, u32 len, u32 addr)
{
    if (phy_dev) {
        if (phy_dev->ops->read) {
            return phy_dev->ops->read(dev, buf, len, addr);
        }
    }
    return -EINVAL;
}

static int rf_module_write(struct device *dev, void *buf, u32 len, u32 addr)
{

    if (phy_dev) {
        if (phy_dev->ops->write) {
            return phy_dev->ops->write(dev, buf, len, addr);
        }
    }
    return -EINVAL;
}

static int rf_module_ioctl(struct device *device, u32 cmd, u32 arg)
{
    if (phy_dev) {
        if (phy_dev->ops->ioctl) {
            return phy_dev->ops->ioctl(device, cmd, arg);
        }
    }
    return -EINVAL;
}

static int rf_module_close(struct device *device)
{

    if (phy_dev) {
        if (phy_dev->ops->close) {
            phy_dev->ops->close(device);
        }
        phy_dev = NULL;
    }
    return 0;
}

const struct device_operations rf_phy_dev_ops = {
    .online = rf_module_online,
    .init   = rf_module_init,
    .open   = rf_module_open,
    .read   = rf_module_read,
    .write  = rf_module_write,
    .ioctl  = rf_module_ioctl,
    .close  = rf_module_close,
};

