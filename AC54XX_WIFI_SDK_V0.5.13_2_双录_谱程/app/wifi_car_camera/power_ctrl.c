#include "power_ctrl.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "gSensor_manage.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "video_dec.h"

extern struct power_platform_data power_data;

void system_shutdown(u8 force)
{

    int park_en = db_select("par");

#ifdef CONFIG_GSENSOR_ENABLE
    set_parking_guard_wkpu(park_en);//gsensor parking guard
#endif
    if (force) {
        power_data.gsen_int_wkup.port_en = 1;
    } else {
        power_data.gsen_int_wkup.port_en = park_en;
    }
    rtc_wkup_ctrl(&power_data);
}

//唤醒原因记录
static int wkup_reaon_check()
{
    if (read_power_key()) {
        rtc_wkup_reason(&power_data, true);
        puts("power start key press start\n");
        goto __END;
    }

    u32 tmp = rtc_wkup_reason(&power_data, false);
    if (tmp & PARK_IO_WKUP) {
#ifdef CONFIG_GSENSOR_ENABLE
        set_park_guard(1);
#endif
    } else if ((tmp & ABNORMAL_RESET) || tmp & BAT_POWER_FIRST) {
        if (!usb_is_charging()) {//第一次接电池上电不开机,仅接电池中途复位不开机
            puts("battery on first time or mid reset , power off\n");
            system_shutdown(0);
        }
    }

__END:
    PWR_CTL(1);
    return 0;
}
platform_initcall(wkup_reaon_check);

/***********************************auto close************************************************/
static u32 auto_close_time;
static u32 auto_close_cnt;

void send_key_long_msg()
{
    struct sys_event eve;
    eve.type = SYS_KEY_EVENT;
    eve.u.key.event = KEY_EVENT_LONG;
    eve.u.key.value = KEY_POWER;
    sys_key_event_enable();  //防止倒车状态等按键不响应态低电关机、拔出充电线关机失效
    sys_key_event_takeover(false, false); //让app响应key
    sys_event_notify(&eve);
}
extern u8 get_usb_in_status();
static int auto_close_usb_is_busy()
{
    return get_usb_in_status();
}
u32 __attribute__((weak)) get_avin_parking_status()
{
    return 0;
}

extern struct video_rec_hdl rec_handler;
#define __this_rec     (&rec_handler)
//录像且不在倒车状态时候，清空倒计时关机变量
static int auto_close_rec_is_busy()
{
    if ((__this_rec->state == VIDREC_STA_START)
        || (get_avin_parking_status() != 0)
       ) {
        return 1;
    }
    return 0;
}

extern struct video_dec_hdl dec_handler;
#define __this_dec     (&dec_handler)
//回放时候，清空倒计时关机变量
static int auto_close_dec_is_busy()
{
    if ((__this_dec->status == VIDEO_DEC_PLAYING)
        || (__this_dec->status == VIDEO_DEC_FF)
        || (__this_dec->status == VIDEO_DEC_FR)
        || (__this_dec->status == VIDEO_DEC_PAUSE)
       ) {
        return 1;
    }
    return 0;
}

//倒计时关机控制函数
static void auto_power_off(void *priv)
{
    if (get_parking_status()) {
        auto_close_cnt = 0;
        return;
    }
    if (auto_close_usb_is_busy() || auto_close_rec_is_busy() || auto_close_dec_is_busy()) {
        auto_close_cnt = 0;
        return;
    }
    if (auto_close_time) {
        /* auto_close_cnt++; */
        /* printf("\n auto_close_cnt %d\n", auto_close_cnt);  */
        if (auto_close_cnt >= auto_close_time) {
            auto_close_cnt = 0;
            auto_close_time = 0;
            send_key_long_msg();

        }
    } else {
        auto_close_cnt = 0;
    }
}
//倒计时关机设置函数 提供给系统菜单调用
void auto_close_time_set(int sec)
{
    if (sec) {
        auto_close_time = sec;
    } else {
        auto_close_time = 0;
    }
}

static int auto_close_init(void)
{
    int timer;
    timer = sys_timer_add(NULL, auto_power_off, 1000);
    if (!timer) {
        log_e("register auto poweroff fail\n");
        return -1;
    }
    return 0;
}
late_initcall(auto_close_init);

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
		printf("==========================123====================\n");
    } else if (__this->state == CHARGING_STATE_OFF) {
        event.u.dev.event = DEVICE_EVENT_OUT;
		printf("==========================234====================\n");
        log_v("charging out\n");
    }

    sys_event_notify(&event);
}

//拔掉usb充电线，10s倒计时关机
void charg_off_sec_det(void *par)
{
    if (__this->state == CHARGING_STATE_OFF) {
        __this->off_sec_cnt++;
        if (__this->off_sec_cnt >= 10) {
            __this->off_sec_cnt = 0;
            puts("charg off power close\n");
            send_key_long_msg();
        }
    } else {
        __this->off_sec_cnt = 0;
    }
}
//充电检测调用，由系统调用
static int charging_detect(void)
{
    memset(__this, 0x0, sizeof(struct charger));
	//__this->state = 0xff;

    sys_timer_add(NULL, usb_charging_detect, 30);
    sys_timer_add(NULL, charg_off_sec_det, 1000);
    return 0;
}
late_initcall(charging_detect);


//清空usb拔出充电计数
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
SYS_EVENT_HANDLER(SYS_KEY_EVENT | SYS_TOUCH_EVENT, charg_off_close_event_handler, 0);

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

