#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "system/includes.h"
#include "menu_parm_api.h"



#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

REGISTER_UI_STYLE(STYLE_NAME)


struct main_page_info {
    u8 battery_val;
    u8 battery_char;
    s8 onkey_sel;
};
static int onkey_sel_item[3] = {
    HOME_BTN_RECORDER,
    HOME_BTN_FILE,
    HOME_BTN_SETTINGS,
};
static int onkey_sel_item1[3] = {
    HOME_TXT_RECORDER,
    HOME_TXT_FILES,
    HOME_TXT_SETTINGS,
};

extern volatile char if_in_rec; /* 是否正在录像 */
extern int rec_cnt;
static struct main_page_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct main_page_info))

/*
 * (begin)提示框显示接口 ********************************************
 */
enum box_msg {
    BOX_MSG_POWER_OFF = 1,
    BOX_MSG_NO_POWER,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __main_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(HOME_LAY_MESSAGEBOX);
        }
    } else if (id == 0) { /* 没有指定ID，强制隐藏 */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(HOME_LAY_MESSAGEBOX);
        }
    }
}
static void __main_msg_timeout_func(void *priv)
{
    __main_msg_hide((enum box_msg)priv);
}
static void __main_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__main_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(HOME_LAY_MESSAGEBOX);
            ui_text_show_index_by_id(HOME_TXT_MESSAGEBOX, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __main_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(HOME_LAY_MESSAGEBOX);
        }
        ui_text_show_index_by_id(HOME_TXT_MESSAGEBOX, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __main_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */
static void get_system_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    /* printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec); */
    dev_close(fd);
}
int ReturnWeekDay(unsigned int iYear, unsigned int iMonth, unsigned int iDay)
{
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    if (iMonth == 1 || iMonth == 2) {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    } else {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }

    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);        //iWeek为负时取模
    if (iWeek == 0) {     //星期日不作为一周的第一天
        iWeek = 7;
    }

    return iWeek;
}

static void no_power_msg_box_timer(void *priv)
{
    static u32 cnt = 0;
    if (__this->battery_val <= 20 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            puts("no power show.\n");
            __main_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            puts("no power hide.\n");
            __main_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __main_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
    }
}
/*
 * battery事件处理函数
 */
static void battery_event_handler(struct sys_event *event, void *priv)
{
    static u8 ten_sec_off = 0;
    if (event->type == SYS_KEY_EVENT || event->type == SYS_TOUCH_EVENT) {
        if (ten_sec_off) {
            ten_sec_off = 0;
            __main_msg_hide(0);
            return;
        }
    }

    if (event->type == SYS_DEVICE_EVENT) {
        if (!ASCII_StrCmp(event->arg, "powerdet", 8)) {
            if (event->u.dev.event == DEVICE_EVENT_CHANGE) {
                __this->battery_val = event->u.dev.value * 20;
                if (__this->battery_val > 100) {
                    __this->battery_val = 100;
                }
                if (__this->battery_char == 0) {
                    ui_battery_level_change(__this->battery_val, 0);
                }
            }
        } else if (!ASCII_StrCmp(event->arg, "charger", 7)) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                ui_battery_level_change(100, 1);
                __this->battery_char = 1;
                if (ten_sec_off) {
                    ten_sec_off = 0;
                    __main_msg_hide(0);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
                __main_msg_show(BOX_MSG_POWER_OFF, 0);
                ten_sec_off = 1;
            }
        }
    }
}

static int rec_off_handler(const char *type, u32 arg)
{
    puts("rec_off_handler.\n");
    if_in_rec = FALSE;

    rec_cnt = 0;
    return 0;
}
static int rec_save_handler(const char *type, u32 arg)
{
    //主界面循环录像计数清0
    return 0;
}
/*
 * 主界面响应回调
 */
static const struct uimsg_handl main_msg_handler[] = {
    { "offREC",         rec_off_handler      }, /* 停止录像 */
    { "saveREC",        rec_save_handler     }, /* 保存录像 */
    // { NULL, NULL},      /* 必须以此结尾！ */
};

static int main_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    struct application *app;
    int ret;
    /* UI_ONTOUCH_DEBUG("\nmain page onchange \n"); */
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_MAIN_PAGE, main_msg_handler);
        malloc_stats();
        break;
    default:
        return false;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MAIN_PAGE)
.onchange = main_page_onchange,
};
/***************************** 星期文字动作 ************************************/
static int text_week_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        printf("\nit is week %d\n", ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day));
        ui_text_set_index(text, ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(HOME_TXT_WEEKDAY)
.onchange = text_week_onchange,
 .ontouch = NULL,
};

/*****************************主界面电池控件动作 ************************************/
static int battery_main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_battery *battery = (struct ui_battery *)ctr;
    static u16 id = 0;
    static u32 timer_handle = 0;
    static void *fd = NULL;
    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT | SYS_KEY_EVENT | SYS_TOUCH_EVENT, 200, fd, battery_event_handler);
        break;
    case ON_CHANGE_FIRST_SHOW:
        fd = dev_open("powerdet", NULL);
        if (fd) {
            u32 power_level = 0;
            dev_ioctl(fd, POWER_DET_GET_LEVEL, (u32)&power_level);
            __this->battery_val = power_level * 20;
            if (__this->battery_val > 100) {
                __this->battery_val = 100;
            }
            dev_close(fd);
        }
        __this->battery_char = (usb_is_charging() ? 1 : 0);
        ui_battery_level_change(__this->battery_val, __this->battery_char);
        timer_handle = sys_timer_add(NULL, no_power_msg_box_timer, 1000);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        if (timer_handle) {
            sys_timer_del(timer_handle);
            timer_handle = 0;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BAT)
.onchange = battery_main_onchange,
 .ontouch = NULL,
};
/****************************主界面时间控件动作 ************************************/
static int timer_sys_main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_TIM_TIME)
.onchange = timer_sys_main_onchange,
 .ontouch = NULL,
};
/*****************************主界面系统日期控件动作 ************************************/
static int timer_sys_date_main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;

    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_TIM_DATE)
.onchange = timer_sys_date_main_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int car_recorder_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**car recorder ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        ui_highlight_element_by_id(HOME_TXT_RECORDER);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        app = get_current_app();
        if (app) {
            if (!strcmp(app->name, "video_rec")) {
                ui_hide(ui_get_current_window_id());
                ui_show(ID_WINDOW_VIDEO_REC);
            }
        } else {
            ui_hide(ui_get_current_window_id());
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_MAIN;
            start_app_async(&it, NULL, NULL);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_RECORDER)
.ontouch = car_recorder_ontouch,
};
static void app_action_back_ok(void *p, int err)
{
    if (err == 0) {
        puts("---app action back ok\n");
        ui_hide(ui_get_current_window_id());
    } else {
        printf("---app action back faild: %d\n", err);
    }
}
static int system_setting_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys setting ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        ui_highlight_element_by_id(HOME_TXT_SETTINGS);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        app = get_current_app();
        if (app) {
            init_intent(&it);
            it.name = app->name;
            it.action = ACTION_BACK;
            start_app_async(&it, app_action_back_ok, NULL);
        } else {
            ui_hide(ui_get_current_window_id());
        }
        init_intent(&it);
        it.name = "video_system";
        it.action = ACTION_SYSTEM_MAIN;
        start_app_async(&it, NULL, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_SETTINGS)
.ontouch = system_setting_ontouch,
};
static void file_browse_ok(void *p, int err)
{
    puts("open file browser ok!!!\n");
    sys_touch_event_enable();
}
static int file_browse_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file_browse  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        ui_highlight_element_by_id(HOME_TXT_FILES);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sys_touch_event_disable();
        app = get_current_app();
        if (app) {
            init_intent(&it);
            it.name = app->name;
            it.action = ACTION_BACK;
            start_app_async(&it, app_action_back_ok, NULL);
        } else {
            ui_hide(ui_get_current_window_id());
        }
        init_intent(&it);
        it.name = "video_dec";
        it.action = ACTION_VIDEO_DEC_MAIN;
        start_app_async(&it, file_browse_ok, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_FILE)
.ontouch = file_browse_ontouch,
};







static int main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("main_onchange\n");
        sys_key_event_takeover(true, false);
        __this->onkey_sel = 0;
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }

    return false;
}
static int main_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }
    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    switch (e->value) {
    case KEY_UP:
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        __this->onkey_sel --;
        if (__this->onkey_sel < 1) {
            __this->onkey_sel = 3;
        }
        ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
        ui_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        break;
    case KEY_DOWN:
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        __this->onkey_sel ++;
        if (__this->onkey_sel > 3) {
            __this->onkey_sel = 1;
        }
        ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
        ui_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        break;
    case KEY_OK:
        if (__this->onkey_sel) {
            struct intent it;
            switch (__this->onkey_sel) {
            case 1:
                ui_hide(ui_get_current_window_id());
                init_intent(&it);
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app_async(&it, NULL, NULL);
                break;
            case 2:
                ui_hide(ui_get_current_window_id());
                sys_touch_event_disable();
                init_intent(&it);
                it.name = "video_dec";
                it.action = ACTION_VIDEO_DEC_MAIN;
                start_app_async(&it, file_browse_ok, NULL);
                break;
            case 3:
                ui_hide(ui_get_current_window_id());
                init_intent(&it);
                it.name = "video_system";
                it.action = ACTION_SYSTEM_MAIN;
                start_app(&it);
                break;
            }
            return true;
        }
        break;
    case KEY_MODE:
        return true;
        break;
    default:
        return false;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(HOME_WIN)
.onchange = main_onchange,
 .onkey = main_onkey,
};

#endif
