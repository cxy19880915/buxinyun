#include "gSensor_manage.h"

#ifdef CONFIG_GSENSOR_ENABLE

const struct device_operations gsensor_dev_ops;
static void *iic = NULL;
#define iic_delay 10
unsigned char gravity_sensor_command(unsigned char w_chip_id, unsigned char register_address, unsigned char function_command)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        ret = 0;
        puts("\n gsen iic wr err 0\n");
        goto __gcend;
    }

    delay(iic_delay);

    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        ret = 0;
        puts("\n gsen iic wr err 1\n");
        goto __gcend;
    }

    delay(iic_delay);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, function_command)) {
        ret = 0;
        puts("\n gsen iic wr err 2\n");
        goto __gcend;
    }

__gcend:

    delay(iic_delay);
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;
}

unsigned char _gravity_sensor_get_data(unsigned char w_chip_id, unsigned char r_chip_id, unsigned char register_address)
{
    unsigned char regDat;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, w_chip_id)) {
        puts("\n gsen iic rd err 0\n");
        regDat = 0;
        goto __gdend;
    }


    delay(iic_delay);
//    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, register_address)) {
    if (dev_ioctl(iic, IIC_IOCTL_TX, register_address)) {
        puts("\n gsen iic rd err 1\n");
        regDat = 0;
        goto __gdend;
    }


    delay(iic_delay);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, r_chip_id)) {
        regDat = 0;
        puts("\n gsen iic rd err 2\n");
        goto __gdend;
    }

    delay(iic_delay);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&regDat)) {
        regDat = 0;
        puts("\n gsen iic rd err 3\n");
        goto __gdend;
    }
__gdend:

    delay(iic_delay);
    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return regDat;
}


//u8 gravity_sen_dev_cur;		/*当前挂载的Gravity sensor*/
int  _gravity_sensor_init(const struct dev_node *node, void *_data)
{
    return 0;
}

int  _gravity_sensor_open(const struct dev_node *node,  struct device **device, void *arg)
{

    if (!iic) {
        iic = dev_open("iic1", 0);
        if (!iic) {
            puts("\n  open iic1 for gsensor err\n");
            return -EINVAL;
        }
    }

    G_SENSOR_INTERFACE *c;


    list_for_each_gsensor(c) {
//        printf("\n gg==== %s\n", c->gsensor_ops->logo);
        if (0 == c->gsensor_ops->gravity_sensor_check()) {

            /*printf("\ncur g sensor logo:::  %s\n", c->gsensor_ops->logo);*/

            *device = &c->dev;

            (*device)->private_data = c;

            if (c->gsensor_ops->gravity_sensor_init) {
                c->gsensor_ops->gravity_sensor_init();
            }

            return 0;
        }
    }

    return -EINVAL;

}




int  g_sensor_gravity_sensity_ioctl(struct device *device, u32 cmd, u32 arg)
{

    G_SENSOR_INTERFACE *gsen = (G_SENSOR_INTERFACE *)device->private_data;

    if (gsen->gsensor_ops->gravity_sensor_sensity) {
        return gsen->gsensor_ops->gravity_sensor_sensity(cmd);
    }

    return -EINVAL;
}

//char g_sensor_gravity_read(struct device *device, void *buf, int len, u32 addr)
//{
//    G_SENSOR_INTERFACE *gsen = (G_SENSOR_INTERFACE *)device->private_data;
//    u8 tmp;
//    tmp= gsen->gsensor_ops.gravity_sensor_read(addr);
//    buf = &tmp;
//    return len;
//}
//unsigned char g_sensor_gravity_write(struct device *device, void *buf, int len, u32 addr)
//{
//
//    G_SENSOR_INTERFACE *gsen = (G_SENSOR_INTERFACE *)device->private_data;
//
//    return gsen->gsensor_ops.gravity_sensor_wirte(addr, buf[0]);
//}

const struct device_operations gsensor_dev_ops = {
    .init = _gravity_sensor_init,
    .open = _gravity_sensor_open,
//    .read = g_sensor_gravity_read,
//    .write = g_sensor_gravity_write,
    .ioctl = g_sensor_gravity_sensity_ioctl,
};


static volatile u8 gsen_active_flag;
/**
 * call get_gsen_active_flag()  before save file
 * */
u8 get_gsen_active_flag()
{
    return gsen_active_flag;
}

/**
 * call clear_gsen_active_flag() after save file
 * */
void clear_gsen_active_flag()
{
    gsen_active_flag = 0;
}

static void *gse = NULL;
static int gse_timer_hand = 0;
void *get_gse()
{
    return gse;
}

static void send_key_lock_file_msg()
{
    struct sys_event eve;
    eve.arg = "gsen_lock";
    eve.type = SYS_DEVICE_EVENT;
    puts("\n====lock\n");
    eve.u.dev.event = DEVICE_EVENT_CHANGE;
    sys_event_notify(&eve);//MSG FOR UI DISP
}

static volatile u8 park_guard = 0;
void set_park_guard(u8 p_flag)
{
    park_guard = p_flag;
}

u8 get_park_guard_status(u8 park_guard_en)
{
    if (park_guard_en) {
        if (park_guard) {
            park_guard = 0;
            if (get_gse()) {
                gsen_active_flag = 1;
                return true;
            }
        }
    }

    return false;
}
void usb_online_clear_park_wkup()
{
    get_park_guard_status(1);
    clear_gsen_active_flag();
}
void gravity_scan_process(void *par)
{
    /* putchar('a'); */
    if (par) {
        int ret = dev_ioctl(par, G_SENSOR_SCAN, 0); /***gravity scan if intruct activ ,return 0*/

        if (!ret) {
            //active
            if (gsen_active_flag == 0) {
                gsen_active_flag = 1;
                send_key_lock_file_msg();
            }
        }
    }
}


/**
 * restore OR menuSet sensity para
para:
    G_SENSOR_CLOSE = 0,
    G_SENSOR_HIGH,
    G_SENSOR_MEDIUM,
    G_SENSOR_LOW,
 * */
void set_gse_sensity(u8 sensity)
{
    if (get_gse()) {
        /*printf("\n========== sensity %d\n", sensity);*/
        dev_ioctl(get_gse(), sensity, 0); //
        if (!gse_timer_hand) {
            gse_timer_hand = sys_timer_add(get_gse(), gravity_scan_process, 100);
            if (!gse_timer_hand) {
                puts("\n gravity sensor timer fail \n");
            }
        }
    }
}
/**
*call set_parking_guard_wkpu() before power off
*/
//set_parking_guard_wkpu(menu_info.video_park_guard);

void set_parking_guard_wkpu(u8 park_guad)
{

    if (park_guad) {
        if (get_gse()) {
            puts("\n set parking on\n");
            dev_ioctl(get_gse(), G_SENSOR_LOW_POWER_MODE, 0); //

            if (gse_timer_hand) {
                sys_timer_del(gse_timer_hand);
                gse_timer_hand = 0;
            }
            dev_close(get_gse());
        }
    } else {
        if (get_gse()) {
            puts("\n set parking off\n");
            dev_ioctl(get_gse(), G_SENSOR_CLOSE, 0); //

            if (gse_timer_hand) {
                sys_timer_del(gse_timer_hand);
                gse_timer_hand = 0;
            }
            dev_close(get_gse());
        }
    }
}

static int gsensor_init()
{
    if (!get_gse()) {
        gse = dev_open("gsensor", 0);
        if (!gse) {
            puts("\n gensor mout fail, please check it\n");
        }
    }

    return 0;
}
late_initcall(gsensor_init);/**/

#endif // CONFIG_GSENSOR_ENABLE

