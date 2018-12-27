#include "power_ctrl.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "gSensor_manage.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "video_dec.h"
//#include "asm/lcd_config.h"

void *p_dev;
const struct power_platform_data *power_data;
int _power_init()
{
    return 0;
}
void *rtc_fd;
int _power_open(void *par)
{
    rtc_fd = dev_open("rtc", NULL);
    if (rtc_fd) {
        return 0;
    }
    return -1;
}

int _power_ioctrl(int cmd, int arg)
{
    int ret = -EINVAL;
    struct _pr_wkup ppp;
    struct _pr_main_en mian_ppp;
    switch (cmd) {
    case IOCTL_SET_WKUP_IO_PRX:
        ppp = power_data->usb_wkup;
        gpio_direction_input(ppp.gpio);
        gpio_set_pull_up(ppp.gpio, 0);
        gpio_set_pull_down(ppp.gpio, 0);
        dev_ioctl(rtc_fd, IOCTL_SET_WKUP_IO_PRX, (u32)&ppp);
        ppp = power_data->gsen_int_wkup;
        gpio_direction_input(ppp.gpio);
        gpio_set_pull_up(ppp.gpio, 0);
        gpio_set_pull_down(ppp.gpio, 0);
        ppp.port_en = arg;
        dev_ioctl(rtc_fd, IOCTL_SET_WKUP_IO_PRX, (u32)&ppp);

        break;
    case IOCTL_SET_WKUP_PRX_EN:
        mian_ppp = power_data->en_pr;
        dev_ioctl(rtc_fd, IOCTL_SET_WKUP_PRX_EN, (u32)&mian_ppp);

        break;
    case IOCTL_SET_READ_PDFLAG:
        return dev_ioctl(rtc_fd, IOCTL_SET_READ_PDFLAG, 0);
        break;
    case IOCTL_SET_READ_PRX_PND:
        if (!arg) {
            ppp = power_data->usb_wkup;
            ret = dev_ioctl(rtc_fd, IOCTL_SET_READ_PRX_PND,  ppp.gpio);
        } else {
            ppp = power_data->gsen_int_wkup;
            ret = dev_ioctl(rtc_fd, IOCTL_SET_READ_PRX_PND,  ppp.gpio);
        }

        break;
    case IOCTL_SET_CLEAR_PRX_PND:
        if (!arg) {
            ppp = power_data->usb_wkup;
            return dev_ioctl(rtc_fd, IOCTL_SET_CLEAR_PRX_PND, ppp.gpio);
        } else {
            ppp = power_data->gsen_int_wkup;
            return 	dev_ioctl(rtc_fd, IOCTL_SET_CLEAR_PRX_PND, ppp.gpio);
        }
        break;
    case IOCTL_SET_CLEAR_PR01_POWER:
        delay(100);
        dev_ioctl(rtc_fd, IOCTL_SET_CLEAR_PR01_POWER, arg);//power off
        break;
    default:
        break;
    }
    /*printf("ret %d\n", ret);*/
    return ret;
}

int _power_close()
{
    if (rtc_fd) {
        puts("close rtc\n");
        dev_close(rtc_fd);
        rtc_fd = NULL;
    }
    return 0;
}
_power_ops ppp_ops = {
    .power_init = _power_init,
    .power_open = _power_open,
    .power_ioctrl = _power_ioctrl,
    .power_close = _power_close,
};
REGISTER_POWER_DEV(power_d)
.power_ops = &ppp_ops,
};

int power_driver_init(const struct dev_node *node, void *_data)
{
    const struct power_platform_data *data = (const struct power_platform_data *)_data;
    if (data) {
        power_data = data;
        return 0;
    }

    return -EINVAL;
}

int power_open(const struct dev_node *node,  struct device **device, void *arg)
{
    power_device *p;
    list_for_each_power_dev(p) {
        if (p->power_ops->power_open) {
            int ret = p->power_ops->power_open(NULL);
            if (!ret) {
                *device = &p->dev;
                (*device)->private_data = p;
                return 0;
            }
        }
    }

    return -EINVAL;
}

int power_ioctl(struct device *device, u32 cmd, u32 arg)
{
    power_device *p_dev = (power_device *)device->private_data;
    if (p_dev->power_ops->power_ioctrl) {
        return p_dev->power_ops->power_ioctrl(cmd, arg);
    }
    return -EINVAL;
}
int power_close(struct device *device)
{
    power_device *p_dev = (power_device *)device->private_data;
    if (p_dev->power_ops->power_close) {
        p_dev->power_ops->power_close();
    }
    return 0;
}

const struct device_operations power_dev_ops = {
    .init = power_driver_init,
    .open = power_open,
    .ioctl = power_ioctl,
    .close = power_close,
};

/**********************************POWER CTRL API*************************************/
//开机时是否是被重力感应唤醒
#define  USB_WKUP 	BIT(1)
#define  PARK_WKUP	BIT(2)
static u8 wkup_reason(void *p_dev)
{
    int ret0;
    int ret1;
    u8 wk_up_reason = 0;
    ret0 = dev_ioctl(p_dev, IOCTL_SET_READ_PRX_PND,  0);
    ret1 = dev_ioctl(p_dev, IOCTL_SET_READ_PRX_PND,  1);
    if (ret0 != 0 || ret1 != 0) {
        log_i("pr 1/2/3 wakeup pending pr1 %d  pr2/3 %d\n", ret0, ret1);
        if (ret0) {
            dev_ioctl(p_dev, IOCTL_SET_CLEAR_PRX_PND, 0);
            return (wk_up_reason | USB_WKUP);
        }

        if (ret1) {
            dev_ioctl(p_dev, IOCTL_SET_CLEAR_PRX_PND, 1);
            return (wk_up_reason | PARK_WKUP);
        }
    }
    return 0;
}
//开机时电源控制 与唤醒原因记录
int first_time_power_on()
{
    int ret;
    u8 wk_up_reason;
    p_dev = dev_open("power", NULL);
    if (p_dev) {
        ret = dev_ioctl(p_dev, IOCTL_SET_CLEAR_PR01_POWER, 1);    //power on
    }

    if (read_power_key()) {
        puts("\n power start key press start\n");
        goto __END;
    }

    if (p_dev) {
        ret = dev_ioctl(p_dev, IOCTL_SET_READ_PDFLAG, 0);
        wk_up_reason = wkup_reason(p_dev);
        if ((ret == 0) && (wk_up_reason != 0)) {
            if (wk_up_reason & PARK_WKUP) {//停车守卫唤醒
                puts("gsen wk up\n");
#ifdef CONFIG_GSENSOR_ENABLE
                set_park_guard(1);
#endif
            }
        } else {
            if (!usb_is_charging()) {//第一次接电池上电不开机,仅接电池中途复位不开机
                puts("battery on first time or mid reset , power off\n");
                //port_wkup_ctrl(0);//power off
                //return 0;
            }
        }
    }
__END:
    puts("power on\n");

    if (p_dev) {
        /* dev_close(p_dev); */
        /* p_dev= NULL; */
    }
    PWR_CTL(1);
    return 0;
}

//关机前唤醒功能配置
void port_wkup_ctrl(u8 en)
{
    dev_ioctl(p_dev, IOCTL_SET_WKUP_IO_PRX, en);
    dev_ioctl(p_dev, IOCTL_SET_WKUP_PRX_EN, 0);
    dev_ioctl(p_dev, IOCTL_SET_CLEAR_PR01_POWER, 0);//power off

    PWR_CTL(0);
}


/***********************************auto close************************************************/
static u32 auto_close_time;
static u32 auto_close_flag;
static u32 auto_close_cnt;

void send_key_long_msg()
{
    struct sys_event eve;
    eve.type = SYS_KEY_EVENT;
    eve.u.key.event = KEY_EVENT_LONG;
    eve.u.key.value = KEY_POWER;
    sys_key_event_takeover(false, false); //让app响应key
    sys_event_notify(&eve);
}
extern u8 get_usb_in_status();
static void usb_auto_close_factor()
{
    if (get_usb_in_status()) {
        if (auto_close_cnt) {
            auto_close_cnt = 0;
        }
    }
}
u32 __attribute__((weak)) get_avin_parking_status()
{
    return 0;
}

extern struct video_rec_hdl rec_handler;
#define __this_rec 	(&rec_handler)
//录像且不在倒车状态时候，清空倒计时关机变量
void rec_auto_close_factor()
{
    if ((__this_rec->state == VIDREC_STA_START)
        || (get_avin_parking_status() != 0)
       ) {
        if (auto_close_cnt) {
            auto_close_cnt = 0;
        }
    }
}

extern struct video_dec_hdl dec_handler;
#define __this_dec 	(&dec_handler)
//回放时候，清空倒计时关机变量
void dec_auto_close_factor()
{
    if ((__this_dec->status == VIDEO_DEC_PLAYING)
        || (__this_dec->status == VIDEO_DEC_FF)
        || (__this_dec->status == VIDEO_DEC_FR)
        || (__this_dec->status == VIDEO_DEC_PAUSE)
       ) {
        if (auto_close_cnt) {
            auto_close_cnt = 0;
        }
    }
}

//倒计时关机控制函数
static void auto_power_off(void *priv)
{

    if (get_parking_status()) {
        auto_close_cnt = 0;
        return;
    }
    usb_auto_close_factor();
    rec_auto_close_factor();
    dec_auto_close_factor();
    if (auto_close_time && auto_close_flag == 0) {
        auto_close_cnt++;
        /* printf("\n auto_close_cnt %d\n", auto_close_cnt);  */
        if (auto_close_cnt >= auto_close_time) {
            auto_close_cnt = 0;
            auto_close_flag = 1;
            send_key_long_msg();

        }
    } else {
        auto_close_cnt = 0;
    }
}
//倒计时关机设置函数 提供给系统菜单调用
void auto_close_time_set(int sec)
{
    static int timer;
    if (sec) {
        auto_close_time = sec;
        if (!timer) {
            auto_close_flag = 0;
            timer = sys_timer_add(NULL, auto_power_off, 1000);
        }
    } else {
        auto_close_time = 0;
    }
}

//按键时，将倒计时关机变量清空
static void auto_close_event_handler(struct sys_event *event)
{

    if (auto_close_cnt) {
        auto_close_cnt = 0;
    }
}
SYS_EVENT_HANDLER(SYS_KEY_EVENT | SYS_TOUCH_EVENT, auto_close_event_handler, 0);



#define CHARGING_ON_CNT     15
#define CHARGING_OFF_CNT    1

#define CHARGING_STATE_OFF        0
#define CHARGING_STATE_ON         1

struct charger {
    u8 state;
    u8 prev_state;
    u16 on_cnt;
    u16 off_cnt;
    u16 off_sec_cnt;
};

static struct charger charger;

#define __this      (&charger)
//usb充电检测
static void usb_charging_detect(void *arg)
{
    struct sys_event event;

    if (usb_is_charging()) {
        if (__this->state == CHARGING_STATE_OFF) {
            __this->on_cnt++;
            if (__this->on_cnt > CHARGING_ON_CNT) {
                __this->state = CHARGING_STATE_ON;
            }
        } else if (__this->state == CHARGING_STATE_ON) {
            __this->off_cnt = 0;
        }
    } else {
        if (__this->state == CHARGING_STATE_ON) {
            __this->off_cnt++;
            if (__this->off_cnt > CHARGING_OFF_CNT) {
                __this->state = CHARGING_STATE_OFF;
            }
        } else {
            __this->on_cnt = 0;
        }
    }

    if (__this->state == __this->prev_state) {
        return;
    }

    __this->prev_state = __this->state;

    event.arg = "charger";
    event.type = SYS_DEVICE_EVENT;
    if (__this->state == CHARGING_STATE_ON) {
        event.u.dev.event = DEVICE_EVENT_IN;
        log_v("charging in\n");
	//  ui_lcd_light_on();  //增加	
        __this->off_sec_cnt = 0;
    } else if (__this->state == CHARGING_STATE_OFF) {
        event.u.dev.event = DEVICE_EVENT_OUT;
        __this->off_sec_cnt = 1;
        log_v("charging out\n");
//	 ui_lcd_light_on_low(); //增加
    }

    sys_event_notify(&event);
}

//拔掉usb充电线，10s倒计时关机
void charg_off_sec_det(void *par)
{
    static u32 cnt = 0;

    if (__this->off_sec_cnt) {
        if (++cnt >= 10) {
            cnt  = 0;
            __this->off_sec_cnt = 0;
            puts("charg off power close\n");
            send_key_long_msg();
        }
    } else {
        cnt = 0;
    }
}
//充电检测调用，由系统调用
static int charging_detect(void)
{
    memset(__this, 0x0, sizeof(struct charger));
    sys_timer_add(NULL, usb_charging_detect, 30);
    sys_timer_add(NULL, charg_off_sec_det, 1000);
    return 0;
}
late_initcall(charging_detect);


//清空usb充电检测,计数
void clear_usb_charg_off_sec_cnt()
{
    if (__this->off_sec_cnt) {
        __this->off_sec_cnt = 0;
    }
}
//按键触发清空,usb充电线拔出计数变量
static void charg_off_close_event_handler(struct sys_event *event)
{
    clear_usb_charg_off_sec_cnt();
}
SYS_EVENT_HANDLER(SYS_KEY_EVENT, charg_off_close_event_handler, 0);

//电池低电自动关机
static u32 cnt_flag = false;
static void lower_power_close_handler(struct sys_event *event)
{
    static u32 cnt = 0;
    if (!ASCII_StrCmp(event->arg, "powerdet", 8)) {
        if (event->u.dev.event == DEVICE_EVENT_CHANGE) {
            if (event->u.dev.value <= 1) {
                if (cnt++ > 5 && (cnt_flag != true)) {
                    cnt = 0;
                    cnt_flag = true;
                }
            } else {
                cnt = 0;
                cnt_flag = false;
            }
        }
    }
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, lower_power_close_handler, 4);

static void lower_power_close(void *par)
{
    static u32 cnt = 0;
    if (cnt_flag) {
        if (cnt++ > 5) {
            cnt = 0;
            if (!usb_is_charging()) {
                puts("low battery power close\n");
                send_key_long_msg();
            } else {
                cnt_flag = false;
            }
        }
    }
}

static int lower_power_close_ctrl()
{
    sys_timer_add(NULL, lower_power_close, 1000);
    return 0;
}
late_initcall(lower_power_close_ctrl);

