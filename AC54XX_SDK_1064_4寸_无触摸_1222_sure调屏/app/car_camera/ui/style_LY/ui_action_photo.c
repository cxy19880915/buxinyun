#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "app_database.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"


#ifdef CONFIG_UI_STYLE_LY_ENABLE


extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */



extern int menu_sys_lcd_pro_set(u8 sel_item);
extern int menu_sys_auto_off_set(u8 sel_item);
extern int menu_sys_led_fre_set(u8 sel_item);
extern int menu_sys_key_voice_set(u8 sel_item);
extern int menu_sys_language_set(u8 sel_item);
extern int menu_sys_date_set(u8 sel_item);
extern int menu_sys_tv_mod_set(u8 sel_item);
extern int menu_sys_format_set(u8 sel_item);
extern int menu_sys_default_set(u8 sel_item);

#define STYLE_NAME  LY





struct tph_menu_info {
    u8 cur_camera_id;
    u8 menu_status;
    u8 battery_val;     /* 电池电量 0-100 */
    u8 battery_char;    /* 是否充电中 1充电 0没充电 */
    u8 take_photo_busy; /* 是否正在拍照 */
    u8 hlight_show_status;  /* 前照灯显示状态 */

};

static struct tph_menu_info handler = {0};
#define __this 	(&handler)


static struct server *audio = NULL;
static FILE *file;


/************************************************************
					  拍照模式设置
************************************************************/

/*
 * photo拍摄延时方式设置
 */
static const u8 table_photo_delay_mode_camera0[] = {
    0,
    2,
    5,
    10,
};
static void get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    dev_close(fd);
}
/*
 * photo拍摄分辨率设置
 */
static const u8 table_photo_res_camera0[] = {
    PHOTO_RES_VGA, 		/* 640*480 */
    PHOTO_RES_1D3M,  	/* 1280*960 */
    PHOTO_RES_2M,		/* 1920*1080 */
    PHOTO_RES_3M, 		/* 2048*1536 */
    PHOTO_RES_5M, 		/* 2592*1944 */
    PHOTO_RES_8M, 		/* 3264*2448 */
    PHOTO_RES_10M, 		/* 3648*2736 */
    PHOTO_RES_12M, 		/* 4032*3024 */
};


/*
 * photo图像质量设置
 */
static const u8 table_photo_quality_camera0[] = {
    PHOTO_QUA_HI,
    PHOTO_QUA_MD,
    PHOTO_QUA_LO,
};

/*
 * photo图像锐度设置
 */
static const u8 table_photo_acutance_camera0[] = {
    PHOTO_ACU_HI,
    PHOTO_ACU_MD,
    PHOTO_ACU_LO,
};

/*
 * photo图像白平衡设置
 */
static const u8 table_photo_white_balance_camera0[] = {
    PHOTO_WBL_AUTO,     /* 自动 */
    PHOTO_WBL_SUN,      /* 日光 */
    PHOTO_WBL_CLDY,     /* 阴天 */
    PHOTO_WBL_TSL,      /* 钨丝灯 */
    PHOTO_WBL_FLUL,     /* 荧光灯 */
};

/*
 * photo图像ISO设置
 */
static const u16 table_photo_iso_camera0[] = {
    0,
    100,
    200,
    400,
};

/*
 * photo曝光补偿设置
 */
static const u8 table_photo_exposure_camera0[] = {
    3,
    2,
    1,
    0,
    (u8) - 1,
    (u8) - 2,
    (u8) - 3,
};


/*
 * photo快速预览设置
 */
static const u8 table_photo_quick_scan_camera0[] = {
    0,
    2,
    5,
};


/*
 * photo色彩设置
 */
static const u8 table_photo_color_camera0[] = {
    PHOTO_COLOR_NORMAL,
    PHOTO_COLOR_WB,
    PHOTO_COLOR_OLD,
};


static u32 photo_db_select(const char *table)
{
    if (__this->cur_camera_id == 0) {
        return db_select(table);
    }
    return 0;
}



/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_NO_POWER = 1,
    BOX_MSG_MEM_ERR,
    BOX_MSG_NEED_FORMAT,
    BOX_MSG_INSERT_SD,
    BOX_MSG_DEFAULT_SET,
    BOX_MSG_FORMATTING,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __tph_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_TPH);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_TPH);
        }
    }
}

static void __tph_msg_timeout_func(void *priv)
{
    __tph_msg_hide((enum box_msg)priv);
}
static void __tph_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__tph_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            ui_show(LAYER_MSG_TPH);
            ui_text_show_index_by_id(TEXT_MSG_TPH, msg - 1);
            msg_show_f = 1;
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __tph_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_TPH);
        }
        ui_text_show_index_by_id(TEXT_MSG_TPH, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __tph_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */


static int tph_camera_sw_handler(const char *type, u32 args)
{

    ASSERT(type != NULL, "tph_camera_sw_handler err.");

    if (!strcmp(type, "id")) {
        __this->cur_camera_id = args;
        ui_show(TEXT_REMAIN_NUM_TPH);
        /*
         * VGA
         */
        ui_show(TEXT_RES_TPH);
    }
    return 0;
}


static int tph_no_card_handler(const char *type, u32 arg)
{
    __tph_msg_show(BOX_MSG_INSERT_SD, 3000);
    return 0;
}
static int tph_fs_err_handler(const char *type, u32 arg)
{
    __tph_msg_show(BOX_MSG_NEED_FORMAT, 3000);
    return 0;
}
static int tph_take_photo_in_handler(const char *type, u32 arg)
{
    __this->take_photo_busy = 1;
    __tph_msg_hide(0);
    ui_pic_show_image_by_id(PIC_LOGO_TPH, 1);

    return 0;
}
static int tph_take_photo_out_handler(const char *type, u32 arg)
{
    os_time_dly(50);
    ui_pic_show_image_by_id(PIC_LOGO_TPH, 0);
    ui_show(TEXT_REMAIN_NUM_TPH);
    __this->take_photo_busy = 0;

    return 0;
}

static int tph_delay_take_handler(const char *type, u32 arg)
{
    __this->take_photo_busy = 1;
    __tph_msg_hide(0);
    if (!strcmp(type, "ms")) {
        printf("ms:%d\n", arg);
        ui_show(LAYER_DELAY_TPH);
        switch (arg) {
        case 2000:
            ui_show(ANI_DELAY_2S_TPH);

            break;
        case 5000:
            ui_show(ANI_DELAY_5S_TPH);

            break;
        case 10000:
            ui_show(ANI_DELAY_10S_TPH);
            break;
        }
    }
    return 0;
}
static int ani_delay_2s_onchange(void *_ani, enum element_change_event e, void *arg)
{
    if (e == ON_CHANGE_ANIMATION_END) {
        ui_hide(ANI_DELAY_2S_TPH);
        ui_hide(LAYER_DELAY_TPH);
        sys_key_event_enable();
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ANI_DELAY_2S_TPH)
.onchange = ani_delay_2s_onchange,
};
static int ani_delay_5s_onchange(void *_ani, enum element_change_event e, void *arg)
{
    if (e == ON_CHANGE_ANIMATION_END) {
        ui_hide(ANI_DELAY_5S_TPH);
        ui_hide(LAYER_DELAY_TPH);
        sys_key_event_enable();
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ANI_DELAY_5S_TPH)
.onchange = ani_delay_5s_onchange,
};
static int ani_delay_10s_onchange(void *_ani, enum element_change_event e, void *arg)
{
    if (e == ON_CHANGE_ANIMATION_END) {
        ui_hide(ANI_DELAY_10S_TPH);
        ui_hide(LAYER_DELAY_TPH);
        sys_key_event_enable();
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ANI_DELAY_10S_TPH)
.onchange = ani_delay_10s_onchange,
};

extern void play_voice_file(const char *file_name);
static int tph_headlight_on_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("tph_headlight_on_handler\n");
        if (__this->hlight_show_status == 0) {
            __this->hlight_show_status = 1;
            ui_show(ANI_FLIG_TPH);//show head light
            play_voice_file("mnt/spiflash/audlogo/olight.adp");
        }
    } else {
        __this->hlight_show_status = 0;
    }

    return 0;
}
static int tph_headlight_off_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("tph_headlight_off_handler\n");
        ui_hide(ANI_FLIG_TPH);//hide head light
        __this->hlight_show_status = 0;
    }
    return 0;
}

/*
 * 拍照模式的APP状态响应回调
 */
static const struct uimsg_handl tph_msg_handler[] = {

    { "swCAM",      tph_camera_sw_handler       },
    { "fsErr",      tph_fs_err_handler          },
    { "noCard",     tph_no_card_handler         },
    { "tphin",      tph_take_photo_in_handler   },
    { "tphout",     tph_take_photo_out_handler  },
    { "dlyTPH",     tph_delay_take_handler      },
    { "HlightOn",   tph_headlight_on_handler    },
    { "HlightOff",  tph_headlight_off_handler   },
    { NULL, NULL }
};




static void tph_tell_app_exit_menu(void)
{

    int err;
    struct intent it;
    init_intent(&it);
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_CHANGE_STATUS;
    it.data = "exitMENU";
    err = start_app(&it);
    if (err) {
        /*ASSERT(err == 0, ":tph exitMENU\n");*/
    }
}
static int tph_ask_app_open_menu(void)
{

    int err;
    struct intent it;

    init_intent(&it);
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_CHANGE_STATUS;
    it.data = "opMENU:";
    err = start_app(&it);
    if (err) {
        return err;
    }
    if (!strcmp(it.data, "opMENU:dis")) {
        return -1;
    } else if (!strcmp(it.data, "opMENU:en")) {
    } else {
        ASSERT(0, "opMENU err\n");
    }
    return 0;
}

void tph_exit_menu_post(void)
{
    ui_hide(LAYER_MENU_TPH);
    ui_show(LAYER_UP_TPH);
    ui_show(LAYER_DOWN_TPH);
    tph_tell_app_exit_menu();
    __this->menu_status = 0;
}


/*****************************拍照模式页面回调 ************************************/
int tph_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        /*
         * 注册APP消息响应
         */
       //    printf("\nlklklklklklk\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_TPH, tph_msg_handler);

        __this->cur_camera_id = 0;
        sys_cur_mod = 2;
        __this->hlight_show_status = 0;
        break;
    case ON_CHANGE_RELEASE:

        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        break;
    default:
        return false;
    }
    return false;
}



int tph_mode_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            if (tph_ask_app_open_menu() == (int) - 1) {
                break;
            }
            __tph_msg_hide(0);
            ui_hide(LAYER_UP_TPH);
            ui_hide(LAYER_DOWN_TPH);
            ui_show(LAYER_MENU_TPH);
            ui_show(LAYOUT_MN_TPH_TPH);
            sys_key_event_takeover(true, false);
            __this->menu_status = 1;

            break;
        default:
            return false;
        }
        break;
    default:
        if (__this->menu_status) {
            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
                tph_exit_menu_post();
                sys_key_event_takeover(false, true);
            }
        }

        return false;
    }

    return true;
}



REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_TPH)
.onchange = tph_mode_onchange,
 .onkey = tph_mode_onkey,
  .ontouch = NULL,
};

#if 1
/*****************************布局up回调 ************************************/
static int tph_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct intent it;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */
         #if 1
        item = index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0));
        if (item != 0) {
            ui_pic_show_image_by_id(PIC_DELAY_TPH, item - 1);
        } else {
            ui_hide(PIC_DELAY_TPH);
        }
        if (db_select("cyt") != 0) {
            ui_show(PIC_REPEAT_TPH);
        } else {
            ui_hide(PIC_REPEAT_TPH);
        }
		#endif
       ui_pic_show_image_by_id(PIC_QUA_TPH, index_of_table8(db_select("qua"), TABLE(table_photo_quality_camera0)));
        ui_pic_show_image_by_id(PIC_WBL_TPH, index_of_table8(db_select("wbl"), TABLE(table_photo_white_balance_camera0)));
        ui_pic_show_image_by_id(PIC_ISO_TPH, index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0)));
        ui_pic_show_image_by_id(PIC_EXP_TPH, index_of_table8(db_select("pexp"), TABLE(table_photo_exposure_camera0)));
        ui_pic_show_image_by_id(PIC_SHAKE_TPH, db_select("sok"));


        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_UP_TPH)
.onchange = tph_layout_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#endif

/***************************** 分辨率文字动作 ************************************/
static int text_tph_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        index = index_of_table8(photo_db_select("pres"), TABLE(table_photo_res_camera0));
        ui_text_set_index(text, index);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_RES_TPH)
.onchange = text_tph_res_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};






/***************************** 系统时间控件动作 ************************************/

static int timer_sys_tph_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:

        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;

        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_TPH)
.onchange = timer_sys_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 系统日期控件动作 ************************************/
static int timer_sys_date_tph_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_YEAR_TPH)
.onchange = timer_sys_date_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/*
 * sd卡事件处理函数
 */
static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            ui_pic_show_image_by_id(PIC_SD_TPH, 1);
            ui_show(TEXT_REMAIN_NUM_TPH);
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            ui_pic_show_image_by_id(PIC_SD_TPH, 0);
            ui_show(TEXT_REMAIN_NUM_TPH);
            break;
        default:
            break;
        }
    }
}

extern int storage_device_ready();
/***************************** SD 卡图标动作 ************************************/
static int pic_sd_tph_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u16 id = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        if (storage_device_ready() == 0) {
            ui_pic_set_image_index(pic, 0);
        } else {
            ui_pic_set_image_index(pic, 1);
        }
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_SD_TPH)
.onchange = pic_sd_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




static void no_power_msg_box_timer(void *priv)
{
    static u8 cnt = 0;
    if (__this->battery_val <= 20
        && __this->menu_status == 0
        && __this->battery_char == 0
        && __this->take_photo_busy == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            __tph_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            __tph_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __tph_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
    }
}

/*
 * battery事件处理函数
 */
static void battery_event_handler(struct sys_event *event, void *priv)
{
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
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
            }
        }
    }
}
/***************************** 电池控件动作 ************************************/
static int battery_tph_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_battery *battery = (struct ui_battery *)ctr;
    static u16 id = 0;
    static u32 timer_handle = 0;
    static void *fd = NULL;
    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, fd, battery_event_handler);
        break;
    case ON_CHANGE_FIRST_SHOW:
        fd = dev_open("powerdet", NULL);
        if (fd) {
            u32 power_level = 0;
            dev_ioctl(fd, POWER_DET_GET_LEVEL, (u32)&power_level);
            printf("power_level : %d\n", power_level);
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
REGISTER_UI_EVENT_HANDLER(BATTERY_TPH)
.onchange = battery_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};





/***************************** ISO 图标动作 ************************************/
static int pic_iso_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0));
        ui_pic_set_image_index(pic, index);
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_ISO_TPH)
.onchange = pic_iso_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 曝光度 图标动作 ************************************/
static int pic_exp_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("pexp"), TABLE(table_photo_exposure_camera0));
        ui_pic_set_image_index(pic, index);
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_EXP_TPH)
.onchange = pic_exp_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 白平衡 图标动作 ************************************/
static int pic_wbl_onchange(void *ctr, enum element_change_event e, void *arg2)
{
    int index;
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("wbl"), TABLE(table_photo_white_balance_camera0));
        ui_pic_set_image_index(pic, index);
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_WBL_TPH)
.onchange = pic_wbl_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 图像质量 图标动作 ************************************/
static int pic_quality_onchange(void *ctr, enum element_change_event e, void *arg2)
{
    int index;
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("qua"), TABLE(table_photo_quick_scan_camera0));
        ui_pic_set_image_index(pic, index);

        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_QUA_TPH)
.onchange = pic_quality_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static char remain_num_str[10];
/***************************** 剩余拍照张数文字动作 ************************************/
static int text_tph_remain_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    struct intent it;
    int err;
    switch (e) {
    case ON_CHANGE_INIT:

        break;
    case ON_CHANGE_SHOW_PROBE:
        puts("tph_remain_num show probe.\n");

        if (storage_device_ready() == 0) {
            text->str = "00000";
            return TRUE;
        }
        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_CHANGE_STATUS;
        it.data = "reNUM:";
        err = start_app(&it);
        if (err) {
            break;
        }
        if (strlen(it.data) < sizeof(remain_num_str)) {
            strcpy(remain_num_str, it.data);
            printf("remain_num_str: %s\n", remain_num_str);
            text->str = remain_num_str;
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(TEXT_REMAIN_NUM_TPH)
.onchange = text_tph_remain_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/
static int tph_menu_tph_table[] = {
    LAYOUT_MN_DELAY_TPH,
    LAYOUT_MN_RES_TPH,
    LAYOUT_MN_REPEAT_TPH,
    LAYOUT_MN_QUA_TPH,
    LAYOUT_MN_ACU_TPH,
    LAYOUT_MN_WBL_TPH,
    LAYOUT_MN_COLOR_TPH,
    LAYOUT_MN_ISO_TPH,
    LAYOUT_MN_EXP_TPH,
    LAYOUT_MN_SHAKE_TPH,
    LAYOUT_MN_SCAN_TPH,
    LAYOUT_MN_LABEL_TPH,
};

static int tph_menu_tph_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    static int sel_item = 0;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        ASSERT(sel_item < (sizeof(tph_menu_tph_table) / sizeof(int)));
        ui_show(tph_menu_tph_table[sel_item]);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_show(ID_WINDOW_VIDEO_SYS);
        ui_hide(LAYER_MENU_TPH);
        break;

    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_TPH_TPH)
.onkey = tph_menu_tph_onkey,
 .ontouch = NULL,
};


/***************************** 拍摄方式设置 ************************************/
static int menu_tph_method_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_tph_method_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "phm";
        it.exdata = table_photo_delay_mode_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_DELAY_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_DELAY_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_DELAY_TPH)
.onchange = menu_tph_method_onchange,
 .onkey = menu_tph_method_onkey,
  .ontouch = NULL,
};


/***************************** 分辨率设置 ************************************/

static int menu_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_res_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "pres";
        it.exdata = table_photo_res_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_RES_TPH);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_RES_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_RES_TPH)
.onchange = menu_res_onchange,
 .onkey = menu_res_onkey,
  .ontouch = NULL,
};


/***************************** 连拍设置 ************************************/

static int menu_repeat_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("cyt"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_repeat_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "cyt";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_REPEAT_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_REPEAT_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_REPEAT_TPH)
.onchange = menu_repeat_onchange,
 .onkey = menu_repeat_onkey,
  .ontouch = NULL,
};



/***************************** 图像质量设置 ************************************/
static int menu_quality_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("qua"), TABLE(table_photo_quality_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_quality_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "qua";
        it.exdata = table_photo_quality_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_QUA_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_QUA_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_QUA_TPH)
.onchange = menu_quality_onchange,
 .onkey = menu_quality_onkey,
  .ontouch = NULL,
};


/***************************** 锐度设置 ************************************/

static int menu_acutance_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("acu"), TABLE(table_photo_acutance_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_acutance_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "acu";
        it.exdata = table_photo_acutance_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_ACU_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_ACU_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_ACU_TPH)
.onchange = menu_acutance_onchange,
 .onkey = menu_acutance_onkey,
  .ontouch = NULL,
};


/***************************** 白平衡设置 ************************************/
static int menu_wbl_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("wbl"), TABLE(table_photo_white_balance_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_wbl_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "wbl";
        it.exdata = table_photo_white_balance_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_WBL_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_WBL_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_WBL_TPH)
.onchange = menu_wbl_onchange,
 .onkey = menu_wbl_onkey,
  .ontouch = NULL,
};



/***************************** 色彩设置 ************************************/
static int menu_color_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("col"), TABLE(table_photo_color_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_color_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "col";
        it.exdata = table_photo_color_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_COLOR_TPH);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_COLOR_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_COLOR_TPH)
.onchange = menu_color_onchange,
 .onkey = menu_color_onkey,
  .ontouch = NULL,
};




/***************************** ISO设置 ************************************/
static int menu_iso_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_iso_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "iso";
        it.exdata = table_photo_iso_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_ISO_TPH);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_ISO_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_ISO_TPH)
.onchange = menu_iso_onchange,
 .onkey = menu_iso_onkey,
  .ontouch = NULL,
};




/***************************** 曝光补偿设置 ************************************/
static int menu_exposure_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("pexp"), TABLE(table_photo_exposure_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_exposure_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "pexp";
        it.exdata = table_photo_exposure_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_EXP_TPH);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_EXP_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_EXP_TPH)
.onchange = menu_exposure_onchange,
 .onkey = menu_exposure_onkey,
  .ontouch = NULL,
};




/***************************** 防手抖设置 ************************************/
static int menu_hand_shake_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("sok"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_hand_shake_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "sok";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_SHAKE_TPH);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_SHAKE_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SHAKE_TPH)
.onchange = menu_hand_shake_onchange,
 .onkey = menu_hand_shake_onkey,
  .ontouch = NULL,
};


/***************************** 快速浏览设置 ************************************/
static int menu_quick_scan_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("sca"), TABLE(table_photo_quick_scan_camera0));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_quick_scan_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "sca";
        it.exdata = table_photo_quick_scan_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_SCAN_TPH);
        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_SCAN_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_SCAN_TPH)
.onchange = menu_quick_scan_onchange,
 .onkey = menu_quick_scan_onkey,
  .ontouch = NULL,
};


/***************************** 日期标签设置 ************************************/
static int menu_date_label_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("pdat"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_date_label_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_photo";
        it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
        it.data = "pdat";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            break;
        }
        ui_hide(LAYOUT_MN_LABEL_TPH);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        tph_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LABEL_TPH);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LABEL_TPH)
.onchange = menu_date_label_onchange,
 .onkey = menu_date_label_onkey,
  .ontouch = NULL,
};






#endif
