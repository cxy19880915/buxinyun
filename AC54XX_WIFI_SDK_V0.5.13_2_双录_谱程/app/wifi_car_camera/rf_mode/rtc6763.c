#include "rtc6763.h"
struct rtc676x_data {
    u8 irq_status;
    u8 tim_status;
    OS_MUTEX spi_mutex;
    OS_SEM rx_video_sem;
    OS_SEM rx_audio_sem;
    OS_SEM irq_sem;
    rtc676x_gpio_interrupt_info intr_info;
};


struct rtc676x_data *rtc676xdata = NULL;
static void *port_wakeup_hdl;

const struct device_operations rtc6763_dev_ops;
static void *dev_spi = NULL;

extern void delay_2ms(int cnt);

void rtc676x_reset(void)
{

    /* gpio_direction_output(IO_PORTD_05, 1); */
    /* delay_2ms(5);// 大于5ms */
    gpio_direction_output(RTC6763_RESET_PORT, 0);
    delay_2ms(5);// 大于5ms
    gpio_direction_output(RTC6763_RESET_PORT, 1);
    delay_2ms(5);// 大于5ms
}

void rtc676x_spi_cs_low(void)
{
    dev_ioctl(dev_spi, IOCTL_SPI_SET_CS, 0);
}
void rtc676x_spi_cs_high(void)
{
    dev_ioctl(dev_spi, IOCTL_SPI_SET_CS, 1);
}



int rtc676x_gpio_interrupt_enable(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON0 |= BIT(event);
    return 0;
}
int rtc676x_gpio_interrupt_disable(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON0 &= ~BIT(event);
    return 0;
}
int rtc676x_gpio_interrupt_clear(u8 event)
{
    if (event > 17) {
        return -1;
    }
    WKUP_CON2 |= BIT(event);
    return 0;
}
int rtc676x_gpio_check_interrupt(u8 event)
{
    if (event > 17) {
        return -1;
    }
    if (BIT(event)&WKUP_CON3) {
        return 1;
    }
    return 0;
}
static inline void rtc676x_spi_isr_status(u8 *status_value, u8 *tim_status_value, u8 *config3_value)
{
    int err;
    u8 status_read[2] = {RF_STATUS | 0x80, 0xFF};
    u8 status_read_buf[2] = {0};
    u8 status_write_buf[2] = {RF_STATUS, 0xFF};
    u8 tim_status_read[2] = {RF_TIM_STATUS | 0x80, 0xFF};
    u8 tim_status_read_buf[2] = {0};
    u8 tim_status_write_buf[2] = {RF_TIM_STATUS, 0xFF};
    u8 config3_read[2] = {RF_CONFIG3 | 0x80, 0xFF};
    u8 config3_read_buf[2] = {0};
    if (status_value) {
        rtc676x_spi_cs_low();
        err = dev_write(dev_spi, status_read, 2);
        err = dev_read(dev_spi, status_read_buf, 2);
        rtc676x_spi_cs_high();
    }
    if (tim_status_value) {
        rtc676x_spi_cs_low();
        err = dev_write(dev_spi, tim_status_read, 2);
        err = dev_read(dev_spi, tim_status_read_buf, 2);
        rtc676x_spi_cs_high();
    }
    if (config3_value) {
        rtc676x_spi_cs_low();
        err = dev_write(dev_spi, config3_read, 2);
        err = dev_read(dev_spi, config3_read_buf, 2);
        rtc676x_spi_cs_high();
    }
    if (status_value) {
        *status_value = status_read_buf[1];
    }
    if (tim_status_value) {
        *tim_status_value = tim_status_read_buf[1];
    }
    if (config3_value) {
        *config3_value = config3_read_buf[1];
    }

}
void rtc676x_int_isr(void *priv)
{
    struct rtc676x_data *data = priv;
    if (rtc676x_gpio_check_interrupt(0)) {
        rtc676x_spi_isr_status(&data->irq_status, &data->tim_status, NULL);
        rtc676x_gpio_interrupt_disable(0);
        os_sem_post(&data->irq_sem);

    }
    rtc676x_gpio_interrupt_clear(0);
}
int rtc676x_gpio_interrupt_register(rtc676x_gpio_interrupt_info *intr_info)
{
    IOMC2 &= ~(0x7f);
    IOMC2 |=  0x1C;  //PORTB_13

    if (intr_info->interrupt_edge_control) {
        WKUP_CON1 |= BIT(intr_info->interrupt_event);
    } else {

        WKUP_CON1 &= ~BIT(intr_info->interrupt_event);
    }
    rtc676x_gpio_interrupt_clear(intr_info->interrupt_event);
    port_wakeup_hdl = (void *)port_wakeup_reg(rtc676x_int_isr, rtc676xdata);
    return 0;
}
static void rtc676x_task(void *p)
{
    struct rtc676x_data *data = p;
    while (1) {

        os_sem_pend(&data->irq_sem, 0);
        printf("\nrtc6763 int trig\n");
        printf("data->irq_status %d,data->tim_status%d", data->irq_status, data->tim_status);
        rtc676x_gpio_interrupt_enable(0);
    }

}


void rtc676x_cfg(void)
{
    u8 reg_rf_status, reg_tim_status;
    u8 i, err;
    u8 temp1_cfg[5] = {0x1f, 0x04, 0x00, 0x00, 0x00};
    u8 temp2_cfg[4] = {0x32, 0x12, 0x33, 0x04};
    u8 temp3_cfg[2] = {0x01, 0x01};
    u8 temp4_cfg[2] = {0x02, 0xff};
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp4_cfg, 2);
    rtc676x_spi_cs_high();
    temp4_cfg[0] = 0x13;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp4_cfg, 2);
    rtc676x_spi_cs_high();

    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp3_cfg, 2);
    rtc676x_spi_cs_high();

    temp3_cfg[0] = 0x32;
    temp3_cfg[1] = 0x1c;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp3_cfg, 2);
    rtc676x_spi_cs_high();



    for (i = 0; i < sizeof(pmu_cfg);) {
        rtc676x_spi_cs_low();
        err = 	dev_write(dev_spi, &pmu_cfg[i], 2);
        rtc676x_spi_cs_high();
        i += 2;
        rtc676x_spi_cs_low();
        err = 	dev_write(dev_spi, &pmu_cfg[i], 2);
        rtc676x_spi_cs_high();
        i += 2;
    }

    for (i = 0; i < sizeof(rf_cfg);) {
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x22);
        err = 	dev_write(dev_spi, &rf_cfg[i], 1);
        rtc676x_spi_cs_high();
        i += 1;
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x24);
        err = 	dev_write(dev_spi, &rf_cfg[i], 4);
        rtc676x_spi_cs_high();
        i += 4;
    }




    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, &temp2_cfg[2], 2);
    rtc676x_spi_cs_high();

    for (i = 0; i < 3; i++) {
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x22);
        err = 	dev_write(dev_spi, temp1_cfg, 1);
        rtc676x_spi_cs_high();
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x24);
        err = 	dev_write(dev_spi, &temp1_cfg[1], 4);
        rtc676x_spi_cs_high();
    }

    temp2_cfg[3] = 0;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, &temp2_cfg[2], 2);
    rtc676x_spi_cs_high();



    temp1_cfg[1] = 0x07;
    for (i = 0; i < 3; i++) {
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x22);
        err = 	dev_write(dev_spi, temp1_cfg, 1);
        rtc676x_spi_cs_high();
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x24);
        err = 	dev_write(dev_spi, &temp1_cfg[1], 4);
        rtc676x_spi_cs_high();
    }



    temp1_cfg[1] = 0x08;
    for (i = 0; i < 3; i++) {
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x22);
        err = 	dev_write(dev_spi, temp1_cfg, 1);
        rtc676x_spi_cs_high();
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x24);
        err = 	dev_write(dev_spi, &temp1_cfg[1], 4);
        rtc676x_spi_cs_high();
    }
    temp2_cfg[1] = 0x6;
    temp2_cfg[3] = 0x40;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, &temp2_cfg[2], 2);
    rtc676x_spi_cs_high();



    temp2_cfg[1] = 0x12;
    temp2_cfg[3] = 0x4;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, &temp2_cfg[2], 2);
    rtc676x_spi_cs_high();



    for (i = 0; i < sizeof(bb_cfg);) {
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0xe);
        err = 	dev_write(dev_spi, &bb_cfg[i], 1);
        rtc676x_spi_cs_high();
        i += 1;
        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x10);
        err = 	dev_write(dev_spi, &bb_cfg[i], 1);
        rtc676x_spi_cs_high();
        i += 1;
    }
    for (i = 0; i < 6; i++) {

        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, i | BIT(7));
        dev_read(dev_spi, temp2_cfg, 1);
        rtc676x_spi_cs_high();
        printf("\n reg %d value:0x%x \n", i, temp2_cfg[0]);
    }


    temp2_cfg[0] = 0x0;
    temp2_cfg[1] = 0xF0;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x1;
    temp2_cfg[1] = 0xc1;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x5;
    temp2_cfg[1] = 0x5;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp1_cfg[0] = 0x0b;
    temp1_cfg[1] = 0xbc;
    temp1_cfg[2] = 0x1;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 3);
    rtc676x_spi_cs_high();
    temp1_cfg[0] = 0x26;
    temp1_cfg[1] = 0x94;
    temp1_cfg[2] = 0x2;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 3);
    rtc676x_spi_cs_high();
    temp1_cfg[0] = 0x27;
    temp1_cfg[1] = 0xa3;
    temp1_cfg[2] = 0x2;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 3);
    rtc676x_spi_cs_high();
    temp1_cfg[0] = 0x28;
    temp1_cfg[1] = 0xfd ;
    temp1_cfg[2] = 0x2;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 3);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x9;
    temp2_cfg[1] = 0x0;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x36;
    temp2_cfg[1] = 0x20;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x25;
    temp2_cfg[1] = 0x04;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x0;
    temp2_cfg[1] = 0xf2;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x36;
    temp2_cfg[1] = 0x20;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();

    temp1_cfg[0] = 0x7;
    temp1_cfg[1] = 0x88 ;
    temp1_cfg[2] = 0x13;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 3);
    rtc676x_spi_cs_high();
    temp1_cfg[0] = 0x8;
    temp1_cfg[1] = 0xe2 ;
    temp1_cfg[2] = 0x04;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 3);
    rtc676x_spi_cs_high();

    temp1_cfg[0] = 0xc;
    temp1_cfg[1] = 0x8d ;
    temp1_cfg[2] = 0x0;
    temp1_cfg[3] = 0x0;
    temp1_cfg[4] = 0xff;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 5);
    rtc676x_spi_cs_high();

    temp1_cfg[0] = 0xd;
    temp1_cfg[1] = 0x5c ;
    temp1_cfg[2] = 0x0;
    temp1_cfg[3] = 0x0;
    temp1_cfg[4] = 0xff;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 5);
    rtc676x_spi_cs_high();
    temp1_cfg[0] = 0x21;
    temp1_cfg[1] = 0x8d ;
    temp1_cfg[2] = 0x0;
    temp1_cfg[3] = 0x0;
    temp1_cfg[4] = 0xff;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp1_cfg, 5);
    rtc676x_spi_cs_high();



    temp2_cfg[0] = 0x2B;
    temp2_cfg[1] = 0x6;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    temp2_cfg[0] = 0x29;
    temp2_cfg[1] = 0x4;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
    //--------------------0x31-----
    for (i = 0; i < sizeof(g_cfg);) {
        rtc676x_spi_cs_low();
        err = 	dev_write(dev_spi, &g_cfg[i], 5);
        rtc676x_spi_cs_high();
        i += 5;
    }
    for (i = 0; i < 6; i++) {

        rtc676x_spi_cs_low();
        dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, i | BIT(7));
        dev_read(dev_spi, temp2_cfg, 1);
        rtc676x_spi_cs_high();
        printf("\n reg %d value:0x%x \n", i, temp2_cfg[0]);
    }


    temp2_cfg[0] = 0x1;
    temp2_cfg[1] = 0xf0;
    rtc676x_spi_cs_low();
    err = 	dev_write(dev_spi, temp2_cfg, 2);
    rtc676x_spi_cs_high();
}



int  rtc676x_init(const struct dev_node *node, void *_data)
{
    gpio_direction_output(RTC6763_RESET_PORT, 1);
    return 0;
}

int  rtc676x_open(const struct dev_node *node,  struct device **device, void *arg)
{
    int err;
    struct rtc676x_data *data = NULL;
    puts("\n open rtc6763\n");
    dev_spi = dev_open("spi2", 0);
    if (!dev_spi) {
        printf("\n rtc6763 spi dev open err\n");
        return -EINVAL;
    }
    rtc676x_reset();
    // test rtc6763
#if 1
    u8 temp[2] = {0xA5, 0x5A} ;
    puts("\n test rtc6763\n");
    rtc676x_spi_cs_low();
    dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x16);
    err = 	dev_write(dev_spi, temp, 2);


    rtc676x_spi_cs_high();
#endif
    u8 temp2[2] = {0x10, 0x01} ;
    rtc676x_spi_cs_low();
    dev_ioctl(dev_spi, IOCTL_SPI_SEND_BYTE, 0x96);
    /* dev_ioctl(dev_spi,IOCTL_SPI_SEND_BYTE,0x81); */
    err =	dev_read(dev_spi, temp2, 2);
    rtc676x_spi_cs_high();
    printf("read rtc6763:%x;%x \n", temp2[0], temp2[1]);

    if (rtc676xdata) {
        printf("\n rtc676x had open \n");
        goto err_close_spidev;
    }

    rtc676x_cfg();


    data = zalloc(sizeof(struct rtc676x_data));
    if (data == NULL) {
        printf("\n malloc err  \n");
        goto err_close_spidev;
    }
    os_sem_create(&data->irq_sem, 0);


    data->intr_info.interrupt_event = 0;//事件0
    data->intr_info.interrupt_edge_control = 1;//0 上升 1 下降
    data->intr_info.isr_func = rtc676x_int_isr;
    data->irq_status = 1;
    data->tim_status = 1;
    rtc676xdata = data;
    rtc676x_gpio_interrupt_register(&data->intr_info);

    err = task_create(rtc676x_task, data, "rf_task");
    if (err) {
        printf("\n creat rtc676x task err \n ");
        return -EINVAL;
    }

    rtc676x_gpio_interrupt_enable(0);

    return 0;
err_close_spidev:
    if (dev_spi) {
        dev_close(dev_spi);
    }

    return -EINVAL;
}

int rtc676x_ioctl(struct device *device, u32 cmd, u32 arg)
{

    /* int err = 0; */
    switch (cmd) {
    case 1:
        break;
    default:
        break;
    }
    return -EINVAL;
}



const struct device_operations rtc6763_dev_ops = {
    /* .online = NULL; */
    .init = rtc676x_init,
    .open = rtc676x_open,
    .read = NULL,
    .write = NULL,
    .ioctl = rtc676x_ioctl,
    /* .close = NULL; */
};


static void *gse = NULL;
static int check_rtc6763()
{
    printf("\n---------check_rtc6763----\n\n");
    gse = dev_open("rtc6763", 0);
    if (!gse) {
        puts("\n rtc6763 mout fail, please check it\n");
    }

    return 0;
}
/* late_initcall(check_rtc6763);#<{(||)}># */

