
#include "device/av10_spi.h"

//ISP_CS
#define ISP_CS_H(spi)        \
        do { \
            gpio_direction_output(spi->pin_cs, 1); \
        }while(0)

#define ISP_CS_L(spi)       \
        do { \
            gpio_direction_output(spi->pin_cs, 0); \
        }while(0)


#define ISP_CS_OUT(spi)     	\
        do { \
            gpio_direction_output(spi->pin_cs, -1); \
        }while(0)

#define ISP_CS_IN(spi)     \
        do { \
            gpio_direction_input(spi->pin_cs); \
        }while(0)

#define ISP_CS_R(spi)      \
        gpio_read(spi->pin_cs)

//ISP_CLK
#define ISP_CLK_H(spi)      \
        do { \
            gpio_direction_output(spi->pin_clk, 1); \
        }while(0)

#define ISP_CLK_L(spi)      \
        do { \
            gpio_direction_output(spi->pin_clk, 0); \
        }while(0)
//ISP_DO
#define ISP_DO_H(spi)       \
        do { \
            gpio_direction_output(spi->pin_out, 1); \
            gpio_set_hd(spi->pin_out, 1); \
        }while(0)

#define ISP_DO_L(spi)      \
        do { \
            gpio_direction_output(spi->pin_out, 0); \
            gpio_set_hd(spi->pin_out, 1); \
        }while(0)

#define ISP_DO_IN(spi)      \
        do { \
            gpio_direction_input(spi->pin_out); \
            gpio_set_pull_up(spi->pin_out, 1); \
            gpio_set_pull_down(spi->pin_out, 0); \
        }while(0)

#define ISP_DO_R(spi)      \
        gpio_read(spi->pin_out)

//ISP_DI
#define ISP_DI_IN(spi)     \
        do { \
            gpio_direction_input(spi->pin_in); \
            gpio_set_pull_up(spi->pin_in, 1); \
            gpio_set_pull_down(spi->pin_in, 0); \
        }while(0)

#define ISP_DI_R(spi)       \
        gpio_read(spi->pin_in)

/******************************************************************************************/
static int spi_init(struct dev_node *node, void *_data)
{
    struct avin_spi_device *p;
    const struct sw_spi_platform_data *data = (const struct sw_spi_platform_data *)_data;

    list_for_each_avin_spi_device(p) {
        p->spi_dat = &data->spi;
        if (p->ops->init) {
            /*puts("\n av10 spi init 0\n");*/
            p->ops->init(&data->spi);
            return 0;
        }
    }

    return -EINVAL;
}

static int  spi_open(struct dev_node *node,  struct device **device, void *arg)
{
    struct avin_spi_device *p;
    list_for_each_avin_spi_device(p) {
        *device = &p->dev;
        (*device)->private_data = p;

        os_mutex_create(&p->mutex);

        return 0;
    }

    return -EINVAL;
}

static int spi_read(struct device *device, void *buf, int len, u32 addr)
{
    struct avin_spi_device *spi = (struct avin_spi_device *)device->private_data;
    u32 ret;
    if (spi->ops->read) {
        os_mutex_pend(&spi->mutex, 0);
        ret = spi->ops->read(spi->spi_dat, buf, len);
        os_mutex_post(&spi->mutex);
        return ret;
    }
    return -EINVAL;
}


static int spi_write(struct device *device, void *buf, int len, u32 addr)
{
    struct avin_spi_device *spi = (struct avin_spi_device *)device->private_data;
    if (spi->ops->write) {
        os_mutex_pend(&spi->mutex, 0);
        spi->ops->write(spi->spi_dat, buf, len);
        os_mutex_post(&spi->mutex);
        return 0;
    }

    return -EINVAL;
}
static int spi_ioctl(struct device *device, int cmd, int arg)
{
    struct avin_spi_device *spi = (struct avin_spi_device *)device->private_data;
    if (spi->ops->ioctl) {
        os_mutex_pend(&spi->mutex, 0);
        spi->ops->ioctl(spi->spi_dat, cmd, arg);
        os_mutex_post(&spi->mutex);
        return 0;
    }
    return -EINVAL;
}

//static
const struct device_operations _spi_dev_ops = {
    .init 	= spi_init,
    .open 	= spi_open,
    .read 	= spi_read,
    .write 	= spi_write,
    .ioctl 	= spi_ioctl,
};
/******************************************************************************************/

static u16 KEY_DELAY = 600;
//------------------------------
//  SPI send
//------------------------------
static void _spi_send(const struct software_spi *spi, void *buf, int len)
{
    u8 i;

    u8 *dat = (u8 *)buf;
    for (i = 0; i < 8; i++) {
        if (dat[0] & BIT(7)) {
            ISP_DO_H(spi);
        } else {
            ISP_DO_L(spi);
        }
        dat[0] <<= 1;

        ISP_CLK_H(spi);
        delay(KEY_DELAY);
        ISP_CLK_L(spi);
        delay(KEY_DELAY);
    }
}

//------------------------------
//  SPI receive
//------------------------------
static u8 _spi_receive(const struct software_spi *spi, void *buf, int len)
{
    ISP_DO_IN(spi);
    u8 dat = 0;
    u8 i;
    for (i = 0; i < 8; i++) {
        ISP_CLK_H(spi);
        dat <<= 1;
        if (ISP_DI_R(spi)) {
            dat |= BIT(0);
        }
        delay(KEY_DELAY);
        ISP_CLK_L(spi);
        delay(KEY_DELAY);
    }

    return dat;
}

static u8 _spi_ioctrl(const struct software_spi *spi, int cmd, int arg)
{
    if (cmd) {
        ISP_CS_H(spi);
    } else {
        ISP_CS_L(spi);
    }

    return 0;
}

//-----------------------------
//      av10 spi1
//-----------------------------
static void _spi_ini(const struct software_spi *spi)
{
    //printf("\nAV10 spi io initial \n");
    ISP_CS_H(spi);
    ISP_CS_IN(spi);
    ISP_DO_L(spi);
    ISP_CLK_L(spi);
    ISP_DO_IN(spi);

    /*printf("\nspi cs %d\nspi clk %d\nspi di %d\nspi do %d\n", spi->pin_cs, spi->pin_clk, spi->pin_in, spi->pin_out);*/

}

static const struct avin_spi_operations sw_spi_ops = {
    .init 	= _spi_ini,
    .read 	= _spi_receive,
    .write 	= _spi_send,
    .ioctl 	= _spi_ioctrl,
};

REGISTER_AVIN_SPI_DEVICE(&sw_spi_ops);
/******************************************************************************************/

extern void *get_spi();

void spi1_send(u8 dat)
{
    if (get_spi()) {
        dev_write(get_spi(), &dat, 1);
    }
}

u8 spi1_receive(u8 buf)
{
    if (get_spi()) {
        return dev_read(get_spi(), &buf, 1);
    }

    return 0;
}

u8 spi_cs_ctrl(u8 cmd)
{
    if (get_spi()) {
        dev_ioctl(get_spi(), cmd, 0);
    }

    return 0;
}
/******************************************************************************************/

//------------------------------
//  av10 tx
//------------------------------
void av10_drv_tx(u16 addr, u8 wdat)
{

    u8 cmd = 0x04;
    u8 dummy = 0x00;
    u8 adr;
    cmd = cmd | addr >> 8;
    adr = (u8)addr;

    spi_cs_ctrl(0);
    delay(KEY_DELAY);
    spi1_send(cmd);
    spi1_send(adr);
    spi1_send(wdat);
    spi1_send(dummy);
    spi_cs_ctrl(1);

}

//------------------------------
//  av10 rx
//------------------------------
u8 av10_drv_rx(u16 addr)
{

    u8 tmp;
    u8 cmd = 0x02;
    u8 dummy = 0x00;
    u8 adr;
    cmd = cmd | addr >> 8;
    adr = (u8)addr;
    spi_cs_ctrl(0);
    delay(KEY_DELAY);
    spi1_send(cmd);
    spi1_send(adr);
    spi1_send(dummy);
    tmp = spi1_receive(dummy);
    spi_cs_ctrl(1);

    return tmp;

}

//------------------------------
//  av10 reset
//------------------------------
void av10_drv_rst(u16 addr, u8 wdat)
{

    u8 cmd = 0x06;
    u8 dummy = 0x00;
    u8 adr;
    cmd = cmd | addr >> 8;
    adr = (u8)addr;
    spi_cs_ctrl(0);
    delay(KEY_DELAY);
    spi1_send(cmd);
    spi1_send(adr);
    spi1_send(wdat);
    spi1_send(dummy);
    spi_cs_ctrl(1);

}

//------------------------------
//  av10 mov
//------------------------------
void av10_f_mov(u16 addr, u8 wdat)
{
    av10_drv_tx(addr, wdat);
}

//------------------------------
//  av10 or
//------------------------------
void av10_f_or(u16 addr, u8 wdat)
{
    u8 tmp;
    tmp = av10_drv_rx(addr);
    av10_drv_tx(addr, tmp | wdat);
}

//------------------------------
//  av10 and
//------------------------------
void av10_f_and(u16 addr, u8 wdat)
{
    u8 tmp;
    tmp = av10_drv_rx(addr);
    av10_drv_tx(addr, tmp & wdat);
}

//------------------------------
//  av10 xor
//------------------------------
void av10_f_xor(u16 addr, u8 wdat)
{
    u8 tmp;
    tmp = av10_drv_rx(addr);
    av10_drv_tx(addr, tmp ^ wdat);
}



