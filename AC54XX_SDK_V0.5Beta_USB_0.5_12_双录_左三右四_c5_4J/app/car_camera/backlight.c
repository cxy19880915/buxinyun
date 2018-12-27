#include "device/lcd_driver.h"
#include "system/includes.h"
#include "power_ctrl.h"
#include "ui/includes.h"
#include "server/ui_server.h"
#include "video_rec.h"

static u8 lcd_pro_flag = 0; /* 屏保触发标志，1：已经触发 */
static u16 lcd_protect_time = 0; /* 屏保触发时间，单位秒 */
static u16 lcd_pro_cnt = 0;
static int timer = 0;


static void ui_lcd_light_on(void)
{
    puts("====ui_lcd_light_on====\n");
    lcd_backlight_ctrl(true);
}

static void ui_lcd_light_off(void)
{
    puts("====ui_lcd_light_off====\n");
    lcd_backlight_ctrl(false);
}


/*
 * 屏幕保护计时器
 */
static void lcd_pro_kick(void *priv)
{
    if (get_parking_status()) {
        //倒车不屏保
        lcd_pro_cnt = 0;
        return;
    }
    if (lcd_protect_time && lcd_pro_flag == 0) {
        lcd_pro_cnt++;
        if (lcd_pro_cnt >= lcd_protect_time) {
            puts("\n\n\n********lcd_pro_kick********\n\n\n");
            lcd_pro_cnt = 0;
            lcd_pro_flag = 1;
            ui_lcd_light_off();
        }
    } else {
        lcd_pro_cnt = 0;
    }
}

void ui_lcd_light_time_set(int sec)
{
    /*printf("ui_lcd_light_time_set sec:%d\n", sec);*/

    if (sec) {
        lcd_protect_time = sec;
        if (!timer) {
            timer = sys_timer_add(NULL, lcd_pro_kick, 1000);
        }
    } else {
        lcd_protect_time = 0;
    }
}

static void backlight_event_handler(struct sys_event *event)
{
    clear_usb_charg_off_sec_cnt();

    lcd_pro_cnt = 0;
    if (lcd_pro_flag) {   //by peng ���ӱ��⿪��ֻ�е�Դ����Ӧ
        //if (event->type == SYS_KEY_EVENT) 
        if (event->type == SYS_KEY_EVENT && event->u.key.event == KEY_EVENT_CLICK) 
		{
            ui_lcd_light_on();
            lcd_pro_flag = 0;
            sys_key_event_consume(&(event->u.key)); /* 背光关闭时，按键只是打开背光 */
        } else if (event->type == SYS_TOUCH_EVENT) {
            sys_touch_event_consume(&(event->u.touch)); /* 背光关闭时，触摸只是打开背光 */
            if (event->u.touch.event == ELM_EVENT_TOUCH_UP) {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        }
    } else if (event->type == SYS_KEY_EVENT
               && event->u.key.event == KEY_EVENT_CLICK
               && event->u.key.value == KEY_POWER) {
        lcd_pro_flag = 1;
        ui_lcd_light_off();
        sys_key_event_consume(&(event->u.key));
    }
}
SYS_EVENT_HANDLER(SYS_KEY_EVENT | SYS_TOUCH_EVENT, backlight_event_handler, 4);


static void backlight_charge_event_handler(struct sys_event *event)
{

    if (!ASCII_StrCmp(event->arg, "charger", 7)
        || (!ASCII_StrCmp(event->arg, "parking", 7))) {
        if (event->u.dev.event == DEVICE_EVENT_IN) {
            if (lcd_pro_flag) {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
            if (lcd_pro_flag) {
                ui_lcd_light_on();
                lcd_pro_flag = 0;
            }
        }
    }

    if (!strncmp(event->arg, "parking", 7)) {
        if (event->u.dev.event == DEVICE_EVENT_IN) {
            ui_lcd_light_on();
            lcd_pro_flag = 0;
        }
    }

}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, backlight_charge_event_handler, 4);

