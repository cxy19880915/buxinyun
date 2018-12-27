#include "system/includes.h"

#define LOG_TAG "flash_wp"
#include "generic/log.h"


static int spi_flash_write_protect()
{
    int i;
    int id;

    if (!fdir_exist("mnt/spiflash")) {
        if (!mount("spiflash", "mnt/spiflash", "sdfile", NULL)) {
            return -EFAULT;
        }
    }

    FILE *file = fopen("mnt/spiflash/res/flash_cfg.bin", "r");
    if (!file) {
        return 0;
    }

    int len = flen(file);
    u32 *data = (u32 *)malloc(len);
    if (!data) {
        return -ENOMEM;
    }
    if (len != fread(file, data, len)) {
        return -EFAULT;
    }
    fclose(file);

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        return -EFAULT;
    }
    dev_ioctl(dev, IOCTL_GET_ID, (u32)&id);

    for (i = 1; i < len; i += 6) {
        if (data[i] == id) {
            u32 cmd = data[i + 1];
            log_v("flash_id = %x, write_protect\n", id);
            dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, cmd);
            goto __exit;
        }
    }
    log_w("flash_not_write_protect\n");

__exit:
    free(data);
    dev_close(dev);

    return 0;
}
__initcall(spi_flash_write_protect);


