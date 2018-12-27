#include "ui/includes.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "app_database.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"
#include "asm/lcd_config.h"
#include "res.h"


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


static u8 __car_num_set_by_ascii(const char *str);

#define STYLE_NAME  LY



struct video_menu_info {
    volatile char if_in_rec;    /* 是否正在录像 */
    u8 lock_file_flag;          /* 是否当前文件被锁 */

    u8 menu_status;             /*0 menu off, 1 menu on*/
    u8 battery_val;
    u8 battery_char;
    u8 hlight_show_status;  /* 前照灯显示状态 */

    int vanish_line;
    int car_head_line;
};

static struct video_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct video_menu_info))

static struct server *audio = NULL;
static FILE *file;
static int rec_time = 0;
/************************************************************
				    	录像模式设置
************************************************************/
/*
 * rec分辨率设置
 */
static const u8 table_video_resolution[] = {
    VIDEO_RES_1080P,
    VIDEO_RES_720P,
    VIDEO_RES_VGA,
};


/*
 * rec循环录像设置
 */
static const u8 table_video_cycle[] = {
    0,
    3,
    5,
    10,
};


/*
 * rec曝光补偿设置
 */
static const u8 table_video_exposure[] = {
    3,
    2,
    1,
    0,
    (u8) - 1,
    (u8) - 2,
    (u8) - 3,
};


/*
 * rec重力感应设置
 */
static const u8 table_video_gravity[] = {
    GRA_SEN_OFF,
    GRA_SEN_LO,
    GRA_SEN_MD,
    GRA_SEN_HI,
};



/*
 * rec间隔录影设置, ms
 */
static const u16 table_video_gap[] = {
    0,
    100,
    200,
    500,
};

static const u16 province_gb2312[] = {
    0xA9BE, 0xFEC4, 0xA8B4, 0xA6BB, 0xF2BD, //京，宁，川，沪，津
    0xE3D5, 0xE5D3, 0xE6CF, 0xC1D4, 0xA5D4, //浙，渝，湘，粤，豫
    0xF3B9, 0xD3B8, 0xC9C1, 0xB3C2, 0xDABA, //贵，赣，辽，鲁，黑
    0xC2D0, 0xD5CB, 0xD8B2, 0xF6C3, 0xFABD, //新，苏，藏，闽，晋
    0xEDC7, 0xBDBC, 0xAABC, 0xF0B9, 0xCAB8, //琼，冀，吉，桂，甘，
    0xEECD, 0xC9C3, 0xF5B6, 0xC2C9, 0xE0C7, //皖，蒙，鄂，陕，青，
    0xC6D4                                  //云
};

static const u8 num_table[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9', ' ',
};


struct car_num {
    const char *mark;
    u32 text_id;
    u32 text_index;
};

struct car_num_str {
    u8 province;
    u8 town;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 f;
};

struct car_num text_car_num_table[] = {
    {"province", TEXT_MN_CN_PROVINCE, 0}, /* 京 */
    {"town",     TEXT_MN_CN_TOWN,     0}, /* A */
    {"a",        TEXT_MN_CN_A,        0}, /* 1 */
    {"b",        TEXT_MN_CN_B,        0}, /* 2 */
    {"c",        TEXT_MN_CN_C,        0}, /* 3 */
    {"d",        TEXT_MN_CN_D,        0}, /* 4 */
    {"e",        TEXT_MN_CN_E,        0}, /* 5 */
    {"f",        TEXT_MN_CN_F,        0}, /* 5 */
};

enum sw_dir {
    /*
     * 切换方向
     */
    DIR_NEXT = 1,
    DIR_PREV,
    DIR_SET,
};
enum set_mod {
    /*
     * 加减方向
     */
    MOD_ADD = 1,
    MOD_DEC,
    MOD_SET,
};
static u16 car_num_set_p = 0xff;
/*
 * text_car_num_table的当前设置指针
 */

/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_NO_POWER = 1,
    BOX_MSG_MEM_ERR,
    BOX_MSG_NO_MEM,
    BOX_MSG_NEED_FORMAT,
    BOX_MSG_INSERT_SD,
    BOX_MSG_DEFAULT_SET,
    BOX_MSG_FORMATTING,
    BOX_MSG_GSENSOR_ERR
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __rec_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REC);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REC);
        }
    }
}
static void __rec_msg_timeout_func(void *priv)
{
    __rec_msg_hide((enum box_msg)priv);
}
static void __rec_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__rec_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REC);
            ui_text_show_index_by_id(TEXT_MSG_REC, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __rec_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REC);
        }
        ui_text_show_index_by_id(TEXT_MSG_REC, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __rec_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

/*****************************布局上部回调 ************************************/
static int video_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */


        if (__this->lock_file_flag == 1) {
            ui_show(PIC_LOCK_REC);
        }

        if (db_select("mot")) {
            ui_show(PIC_MOTDET_REC);
        } else {
            ui_hide(PIC_MOTDET_REC);
        }

        index = db_select("gra");
        if (index != 0) {
            ui_pic_show_image_by_id(PIC_GRAVITY_REC, index - 1);
        } else {
            ui_hide(PIC_GRAVITY_REC);
        }

        if (db_select("par")) {
            ui_show(PIC_PARK_REC);
        } else {
            ui_hide(PIC_PARK_REC);
        }

        if (db_select("wdr")) {
            ui_show(PIC_HDR_REC);
        } else {
            ui_hide(PIC_HDR_REC);
        }

        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));

        if (index != 0) {
            ui_pic_show_image_by_id(PIC_CYC_REC, index - 1);
        } else {
            ui_hide(PIC_CYC_REC);
        }
/*
        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        if (index) {
            ui_show(PIC_GAP_REC);
        } else {
            ui_hide(PIC_GAP_REC);
        }
*/
        if (__this->if_in_rec) {
            puts("show run rec timer.\n");
            ui_show(TIMER_RUN_REC);
        } else {
            ui_show(TIMER_REMAI_REC);
        }
        index = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
        ui_pic_show_image_by_id(PIC_EXP_REC, index);

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_UP_REC)
.onchange = video_layout_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

int car_num_index[] = 
{
    TEXT_CAR_TOWN,
    TEXT_CAR_A,
    TEXT_CAR_B,
    TEXT_CAR_C,
    TEXT_CAR_D,
    TEXT_CAR_E,
    TEXT_CAR_F,
};
/*****************************布局下部回调 ************************************/
static int video_layout_down_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u32 a, b;
    u8 c;
    u8 i;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */
        a = db_select("cna");
        b = db_select("cnb");
	    c = db_select("cnc");
        if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
        {
            text_car_num_table[0].text_index = index_of_table16(a >> 16, TABLE(province_gb2312));
        }
        else
        {
            text_car_num_table[0].text_index = index_of_table8(((a >> 16) & 0xff), TABLE(num_table));
            text_car_num_table[0].text_index += 31;
        }
        
        text_car_num_table[1].text_index  = index_of_table8((a >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[2].text_index  = index_of_table8((a >> 0) & 0xff, TABLE(num_table));
        text_car_num_table[3].text_index  = index_of_table8((b >> 24) & 0xff, TABLE(num_table));
        text_car_num_table[4].text_index  = index_of_table8((b >> 16) & 0xff, TABLE(num_table));
        text_car_num_table[5].text_index  = index_of_table8((b >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[6].text_index  = index_of_table8((b >> 0) & 0xff, TABLE(num_table));
	    text_car_num_table[7].text_index  = index_of_table8(c , TABLE(num_table));

        if (db_select("num")) 
        {
            //printf("===================%d============\n",text_car_num_table[0].text_index);
            if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
            {
                if(text_car_num_table[0].text_index >= 31)
                {
                    text_car_num_table[0].text_index = 0;
                }
                ui_text_show_index_by_id(TEXT_CAR_PROVINCE, text_car_num_table[0].text_index);
            }
            else
            {
                if(text_car_num_table[0].text_index < 31 || text_car_num_table[0].text_index > 67)
                {
                    text_car_num_table[0].text_index = 31;
                }
                ui_text_show_index_by_id(TEXT_CAR_PROVINCE, text_car_num_table[0].text_index);     
            }

            for(i = 0; i < 7; i++)
            {
                if( text_car_num_table[i].text_index == 36)
                {
                    ui_hide(car_num_index[i]);
                }
                else
                {
                    ui_text_show_index_by_id(car_num_index[i], text_car_num_table[i+1].text_index);    
                }
            }
            #if 0
            ui_text_show_index_by_id(TEXT_CAR_TOWN, text_car_num_table[1].text_index);
            ui_text_show_index_by_id(TEXT_CAR_A,    text_car_num_table[2].text_index);
            ui_text_show_index_by_id(TEXT_CAR_B,    text_car_num_table[3].text_index);
            ui_text_show_index_by_id(TEXT_CAR_C,    text_car_num_table[4].text_index);
            ui_text_show_index_by_id(TEXT_CAR_D,    text_car_num_table[5].text_index);
            ui_text_show_index_by_id(TEXT_CAR_E,    text_car_num_table[6].text_index);
	        ui_text_show_index_by_id(TEXT_CAR_F,    text_car_num_table[7].text_index);
            #endif
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DOWN_REC)
.onchange = video_layout_down_onchange,
 .onkey = NULL,
  .ontouch = NULL,
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

static void show_sys_time(void *arg)
{
	struct sys_time sys_time;
	get_sys_time(&sys_time);
	ui_time_update_by_id(TIMER_HOUR_REC, &sys_time);	
}

/***************************** 系统时间控件动作 ************************************/
static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
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
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_REC)
.onchange = timer_sys_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/***************************** 系统日期控件动作 ************************************/
static int timer_sys_date_rec_onchange(void *ctr, enum element_change_event e, void *arg)
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
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_YEAR_REC)
.onchange = timer_sys_date_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/*
 * 录像计时的定时器,1s
 */
static void rec_cnt_handler(int rec_time_sec)
{
    struct utime time_r;
    int cyc_time = db_select("cyc");
    if (cyc_time == 0) {
        cyc_time = 30;
    }
	

		
    rec_time = rec_time_sec % (cyc_time * 60);
	#if 0	
	if(rec_time%2)
		gpio_direction_output(IO_PORTH_12, 1);		
	else
		gpio_direction_output(IO_PORTH_12, 0);
	#endif	
    time_r.hour = rec_time / 60 / 60;
    time_r.min = rec_time / 60 % 60;
    time_r.sec = rec_time % 60;
    ui_time_update_by_id(TIMER_RUN_REC, &time_r);

}

/*
 * sd卡事件处理函数
 */
static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            ui_pic_show_image_by_id(PIC_SD_REC, 1);
            ui_show(TIMER_REMAI_REC);
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            ui_pic_show_image_by_id(PIC_SD_REC, 0);
            ui_show(TIMER_REMAI_REC);
            break;
        default:
            break;
        }
    } else if (!strcmp(event->arg, "video_rec_time")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE:
            rec_cnt_handler(event->u.dev.value);
            break;
        default:
            break;
        }
    }

}

extern int storage_device_ready();
/***************************** SD 卡图标动作 ************************************/
static int pic_sd_rec_onchange(void *ctr, enum element_change_event e, void *arg)
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
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 250, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_SD_REC)
.onchange = pic_sd_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void no_power_msg_box_timer(void *priv)
{
    static u8 cnt = 0;
    if (__this->battery_val <= 20 && __this->menu_status == 0 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            __rec_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            __rec_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __rec_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
    }
	show_sys_time(0);
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
static int battery_rec_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(BATTERY_REC)
.onchange = battery_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 录像剩余时间控件动作 ************************************/
static int timer_rec_remain_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct intent it;
    int err, i, s;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_SHOW_PROBE:
        if (storage_device_ready() == 0) {
            /*
             * 第一次显示需要判断一下SD卡是否在线
             */
            time->hour = 0;
            time->min = 0;
            time->sec = 0;
            break;
        }

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
        it.data = "reTIME:";
        err = start_app(&it);
        if (err) {
            puts("\n err == 0, :rec reTIME\n");
            break;
        }
        i = 0;
        s = 10;
        time->hour = 0;
        while (it.data[i] != '.' && it.data[i] != '\0') {
            if (it.data[i] >= '0' && it.data[i] <= '9') {
                time->hour += ((it.data[i] - '0') * s);
            }
            i++;
            s = s / 10;
        }
        i++;
        s = 10;
        time->min = 0;
        while (it.data[i] != '.' && it.data[i] != '\0') {
            if (it.data[i] >= '0' && it.data[i] <= '9') {
                time->min += ((it.data[i] - '0') * s);
            }
            i++;
            s = s / 10;
        }
        i++;
        s = 10;
        time->sec = 0;
        while (it.data[i] != '.' && it.data[i] != '\0') {
            if (it.data[i] >= '0' && it.data[i] <= '9') {
                time->sec += ((it.data[i] - '0') * s);
            }
            i++;
            s = s / 10;
        }
        printf("reTIME hour:%d, min:%d, sec:%d\n", time->hour, time->min, time->sec);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_REMAI_REC)
.onchange = timer_rec_remain_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 录像时间控件动作 ************************************/
static int timer_rec_red_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_HIDE:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_SHOW_PROBE:
        if ((time->sec % 2) == 0) {
			gpio_direction_output(IO_PORTH_12, 1);	
            ui_show(PIC_RED_DOT_REC);
        } else {
        	gpio_direction_output(IO_PORTH_12, 0);	
            ui_hide(PIC_RED_DOT_REC);
        }
        break;
    case ON_CHANGE_SHOW_POST:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_RUN_REC)
.onchange = timer_rec_red_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/***************************** MIC 图标动作 ************************************/
static int pic_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("mic"));
        /*
         * 禁止录音
         */
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(PIC_MIC_REC)
.onchange = pic_mic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 分辨率文字动作 ************************************/
static int text_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        ui_text_set_index(text, index);
        return TRUE;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_RES_REC)
.onchange = text_res_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/*
 * (begin)APP状态变更，UI响应回调
 */
static int rec_on_handler(const char *type, u32 arg)
{
    __this->if_in_rec = TRUE;
    ui_hide(TIMER_REMAI_REC);
    ui_show(TIMER_RUN_REC);
    return 0;
}

static int rec_off_handler(const char *type, u32 arg)
{
    __this->if_in_rec = FALSE;
    rec_time = 0;
    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(PIC_LOCK_REC);
    }
	gpio_direction_output(IO_PORTH_12, 1);
	
    ui_hide(TIMER_RUN_REC);
    ui_hide(PIC_RED_DOT_REC);
    ui_show(TIMER_REMAI_REC);
    return 0;
}

static int rec_save_handler(const char *type, u32 arg)
{
    log_w("rec_save_handler");
    struct utime time_r;
    time_r.hour = 0;
    time_r.min = 0;
    time_r.sec = 0;
    ui_time_update_by_id(TIMER_RUN_REC, &time_r);

    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(PIC_LOCK_REC);
    }

    return 0;
}
static int rec_no_card_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_INSERT_SD, 3000);
    return 0;
}
static int rec_fs_err_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_NEED_FORMAT, 3000);
    return 0;
}
static int rec_fs_full_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_NO_MEM, 3000);
    return 0;
}

static int rec_on_mic_handler(const char *type, u32 arg)
{
    ui_pic_show_image_by_id(PIC_MIC_REC, 1);

    return 0;
}
static int rec_off_mic_handler(const char *type, u32 arg)
{
    ui_pic_show_image_by_id(PIC_MIC_REC, 0);
    return 0;
}
static int rec_lock_handler(const char *type, u32 arg)
{
    __this->lock_file_flag = 1;
    ui_show(PIC_LOCK_REC);
    return 0;
}
static int rec_unlock_handler(const char *type, u32 arg)
{
    __this->lock_file_flag = 0;
    ui_hide(PIC_LOCK_REC);
    return 0;
}

extern void play_voice_file(const char *file_name);
static int rec_headlight_on_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("rec_headlight_on_handler\n");
        if (__this->hlight_show_status == 0) {
            __this->hlight_show_status = 1;
            ui_show(ANI_FLIG_REC);//show head light
            play_voice_file("mnt/spiflash/audlogo/lane.adp");
			os_time_dly(100);
			play_voice_file("mnt/spiflash/audlogo/lane.adp");
			//ui_hide(ANI_FLIG_REC);
        }
    } else {
        __this->hlight_show_status = 0;
    }

    return 0;
}
static int rec_headlight_off_handler(const char *type, u32 arg)
{
    if (__this->menu_status == 0) {
        puts("rec_headlight_off_handler\n");
        ui_hide(ANI_FLIG_REC);//hide head light
        __this->hlight_show_status = 0;
    }
    return 0;
}
void show_gra_err(const char *type, u32 arg)
{
	__rec_msg_show(BOX_MSG_GSENSOR_ERR, 0);	
}

#ifdef GSENSOR_UP  

static int rec_on_park_handler(const char *type, u32 arg)
{
	struct intent it;
	//printf("===rec_on_park_handler====\n");
    ui_show(PIC_PARK_REC);
	
	init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "par";
    it.exdata = 1;
    start_app(&it);
	
    return 0;
}

static int rec_off_park_handler(const char *type, u32 arg)
{
	struct intent it;
	//printf("===rec_off_park_handler====\n");	
    ui_hide(PIC_PARK_REC);
	init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "par";
    it.exdata = 0;
    start_app(&it);
	
    return 0;
}

#endif  

/*
 * 录像模式的APP状态响应回调
 */
static const struct uimsg_handl rec_msg_handler[] = {
    { "onREC",          rec_on_handler       }, /* 开始录像 */
    { "offREC",         rec_off_handler      }, /* 停止录像 */
    { "saveREC",        rec_save_handler     }, /* 保存录像 */
    { "noCard",         rec_no_card_handler  }, /* 没有SD卡 */
    { "fsErr",          rec_fs_err_handler   }, /* 需要格式化 */
    { "onMIC",          rec_on_mic_handler   }, /* 打开录音 */
    { "offMIC",         rec_off_mic_handler  }, /* 关闭录音 */
    { "lockREC",        rec_lock_handler     }, /* 锁文件 */
    { "unlockREC",      rec_unlock_handler   }, /* 解锁文件 */
    { "HlightOn",      rec_headlight_on_handler   },
    { "HlightOff",     rec_headlight_off_handler  },
    { "fsfull",          rec_fs_full_handler   }, 
    { "gsenerr",            show_gra_err},
#ifdef GSENSOR_UP   
    { "onPARK",          rec_on_park_handler   }, /* 停车守卫开 */
    { "offPARK",         rec_off_park_handler  }, /* 停车守卫关 */
#endif      
    { NULL, NULL},      /* 必须以此结尾！ */
};
/*
 * (end)
 */



/*
 * (begin)UI状态变更主动请求APP函数
 */
static void rec_tell_app_exit_menu(void)
{

    int err;
    struct intent it;
    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
    it.data = "exitMENU";
    err = start_app(&it);
    if (err) {
        printf("res exit menu err! %d\n", err);
        /* ASSERT(err == 0, ":rec exitMENU\n"); */
    }
}

static int rec_ask_app_open_menu(void)
{

    int err;
    struct intent it;

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_STATUS;
    it.data = "opMENU:";
    err = start_app(&it);
    if (err) {
        printf("res ask menu err! %d\n", err);
        return -1;
        /* ASSERT(err == 0, ":rec opMENU fail! %d\n", err); */
    }
    if (!strcmp(it.data, "opMENU:dis")) {
        return -1;
    } else if (!strcmp(it.data, "opMENU:en")) {
    } else {
        ASSERT(0, "opMENU err\n");
    }
    return 0;
    /*
     * 返回0则打开菜单
     */
}



static void menu_lane_det_set(int value)
{
    struct intent it;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "lan";
    it.exdata = value;
    start_app(&it);
}
static void open_set_lane_page(void *p, int err)
{
    struct intent it;
    puts("\n =============car lane set =========\n");
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    it.data   = "lan_setting";
    start_app(&it);
}
static void close_set_lane_page(void *p)
{
    struct intent it;
    ui_hide(ui_get_current_window_id());
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_BACK;
    start_app(&it);
    puts("\n =============car lane set exit=========\n");
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}
static char lane_set_arg[128];
static void send_lane_det_setting_msg(const char *arg, u32 lane)
{
    struct sys_event eve;
    memset(lane_set_arg, 0, sizeof(lane_set_arg));
    sprintf(lane_set_arg, "%s:%d", arg, lane);
    puts(lane_set_arg);
    eve.arg = lane_set_arg;
    eve.type = SYS_DEVICE_EVENT;
    eve.u.dev.event = DEVICE_EVENT_CHANGE;
    sys_event_notify(&eve);
}
/*
 * (end)
 */


void rec_exit_menu_post(void)
{
    /*
     * 退出菜单，回到APP
     */
    ui_hide(LAYER_MENU_REC);
    ui_show(LAYER_UP_REC);
    ui_show(LAYER_DOWN_REC);
    rec_tell_app_exit_menu();

    __this->menu_status = 0;

}

static void video_rec_ui_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "parking")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            if (__this->menu_status) {
                sys_key_event_takeover(false, false);
            }
            break;
        default:
            break;
        }
    }
}

/*****************************录像模式页面回调 ************************************/
static int video_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item;
    static int id = 0;
    const char *str = NULL;
    struct intent it;
    int ret;
    u8 timeout;

    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***rec mode onchange init***\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_REC, rec_msg_handler);
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, video_rec_ui_event_handler);
        /*
         * 注册APP消息响应
         */
        sys_cur_mod = 1;
        __this->hlight_show_status = 0;
        break;
    case ON_CHANGE_RELEASE:
        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}

static int video_mode_onkey(void *ctr, struct element_key_event *e)
{
    struct window *win = (struct window *)ctr;
    static char flag = 0;
    int err;
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            if (__this->if_in_rec == TRUE) {
                break;
            }
            if (rec_ask_app_open_menu() == (int) - 1) {
                break;
            }
            __rec_msg_hide(0);
            ui_hide(LAYER_UP_REC);
            ui_hide(LAYER_DOWN_REC);
            ui_show(LAYER_MENU_REC);
            ui_show(LAYOUT_MN_REC_REC);
            /*
             * 显示菜单1
             */
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
                rec_exit_menu_post();
                sys_key_event_takeover(false, true);
            }
        }
        return true;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onchange = video_mode_onchange,
 .onkey = video_mode_onkey,
  .ontouch = NULL,
};

static int parking_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_PARKING, rec_msg_handler); /* 注册APP消息响应 */
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_PARKING)
.onchange = parking_page_onchange,
 .ontouch = NULL,
};
/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/
static int video_menu_rec_table[] = {
    LAYOUT_MN_RES_REC,
    //LAYOUT_MN_TWO_REC,
    LAYOUT_MN_CYC_REC,
    //LAYOUT_MN_GAP_REC,
    LAYOUT_MN_HDR_REC,
    LAYOUT_MN_EXP_REC,
    LAYOUT_MN_MOTD_REC,
    LAYOUT_MN_MIC_REC,
    LAYOUT_MN_LABEL_REC,
    LAYOUT_MN_GRAV_REC,
    LAYOUT_MN_PARK_REC,
    //LAYOUT_MN_NUM_REC,
    //LAYOUT_MN_LANE_REC,
    LAYOUT_MN_FLIG_REC,
};


	

static int video_menu_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    static int sel_item = 0;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        ASSERT(sel_item < (sizeof(video_menu_rec_table) / sizeof(int)));
        ui_show(video_menu_rec_table[sel_item]);

        break;
    case KEY_DOWN:
        return FALSE;
        /*
         * 向后分发消息
         */

        break;
    case KEY_UP:
        return FALSE;
        /*
         * 向后分发消息
         */

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_show(ID_WINDOW_VIDEO_SYS);
        ui_hide(LAYER_MENU_REC);
        break;

    default:
        return false;
        break;
    }

    return true;
    /*
     * 不向后分发消息
     */
}


REGISTER_UI_EVENT_HANDLER(VLIST_REC_REC)
.onkey = video_menu_rec_onkey,
 .ontouch = NULL,
};



/***************************** 分辨率设置 ************************************/

static int menu_res_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        ui_grid_set_item(grid, item);
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
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "res";
        it.exdata = table_video_resolution[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_RES_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_RES_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_RES_REC)
.onchange = menu_res_onchange,
 .onkey = menu_res_onkey,
  .ontouch = NULL,
};

#if 0
/***************************** 双路录像设置 ************************************/
static int menu_double_route_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("two"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_double_route_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "two";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("two onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_TWO_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TWO_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_TWO_REC)
.onchange = menu_double_route_onchange,
 .onkey = menu_double_route_onkey,
  .ontouch = NULL,
};
#endif

/***************************** 循环录影设置 ************************************/
static int menu_cyc_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_cyc_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "cyc";
        it.exdata = table_video_cycle[sel_item];
        err = start_app(&it);
        if (err) {
            printf("cyc onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_CYC_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CYC_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_CYC_REC)
.onchange = menu_cyc_rec_onchange,
 .onkey = menu_cyc_rec_onkey,
  .ontouch = NULL,
};


#if 0
/***************************** 间隔录像设置 ************************************/
static int menu_gap_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_gap_rec_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "gap";
        it.exdata = table_video_gap[sel_item];
        err = start_app(&it);
        if (err) {
            printf("gap onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_GAP_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_GAP_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_GAP_REC)
.onchange = menu_gap_rec_onchange,
 .onkey = menu_gap_rec_onkey,
  .ontouch = NULL,
};
#endif


/***************************** 动态范围设置 ************************************/
static int menu_wdr_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("wdr"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_wdr_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "wdr";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("wdr onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_HDR_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_HDR_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_HDR_REC)
.onchange = menu_wdr_onchange,
 .onkey = menu_wdr_onkey,
  .ontouch = NULL,
};



/***************************** 曝光补偿设置 ************************************/

static int menu_exposure_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
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
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "exp";
        it.exdata = table_video_exposure[sel_item];
        err = start_app(&it);
        if (err) {
            printf("exp onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_EXP_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_EXP_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_EXP_REC)
.onchange = menu_exposure_onchange,
 .onkey = menu_exposure_onkey,
  .ontouch = NULL,
};



/***************************** 运动检测设置 ************************************/

static int menu_motdet_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("mot"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_motdet_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "mot";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("mot onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_MOTD_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_MOTD_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MOTD_REC)
.onchange = menu_motdet_onchange,
 .onkey = menu_motdet_onkey,
  .ontouch = NULL,
};



/***************************** 录音设置 ************************************/

static int menu_audio_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("mic"));
        break;
    default:
        return FALSE;
    }

    return FALSE;
}

static int menu_audio_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "mic";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("audio onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_MIC_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_MIC_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MIC_REC)
.onchange = menu_audio_onchange,
 .onkey = menu_audio_onkey,
  .ontouch = NULL,
};


/***************************** 日期标签设置 ************************************/

static int menu_date_label_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("dat"));
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
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "dat";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("date label onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_LABEL_REC);

        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LABEL_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LABEL_REC)
.onchange = menu_date_label_onchange,
 .onkey = menu_date_label_onkey,
  .ontouch = NULL,
};


/***************************** 重力感应设置 ************************************/

static int menu_gravity_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        index = index_of_table8(db_select("gra"), TABLE(table_video_gravity));
        ui_grid_set_item(grid, index);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_gravity_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "gra";
        it.exdata = table_video_gravity[sel_item];
        err = start_app(&it);
        if (err) {
            printf("gravity onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_GRAV_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_GRAV_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_GRAV_REC)
.onchange = menu_gravity_onchange,
 .onkey = menu_gravity_onkey,
  .ontouch = NULL,
};



/***************************** 停车守卫设置 ************************************/
static int menu_parking_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("par"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_parking_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "par";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("park onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_MN_PARK_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_PARK_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_PARK_REC)
.onchange = menu_parking_onchange,
 .onkey = menu_parking_onkey,
  .ontouch = NULL,
};


/***************************** 车牌号码开关设置 ************************************/
static int menu_car_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("num"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_car_num_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "num";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("res car num err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_NUM_REC);
        if (sel_item) {
            ui_show(LAYOUT_MN_CARNUM_SET_REC);
        }

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_NUM_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_NUM_REC)
.onchange = menu_car_num_onchange,
 .onkey = menu_car_num_onkey,
  .ontouch = NULL,
};
/***************************** 车道偏移开关设置 ************************************/
static int menu_lane_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, !!db_select("lan"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_lane_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        if (sel_item == 0) {
            menu_lane_det_set(0);
            ui_hide(LAYOUT_MN_LANE_REC);
        } else {
            struct intent it;
            ui_hide(ui_get_current_window_id());
            rec_exit_menu_post();
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, open_set_lane_page, NULL);
            break;
        }

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LANE_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LANE_REC)
.onchange = menu_lane_onchange,
 .onkey = menu_lane_onkey,
  .ontouch = NULL,
};
/***************************** 前照灯开关设置 ************************************/
static int menu_flig_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("hlw"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_flig_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "video_rec";
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = "hlw";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("head light warning err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_FLIG_REC);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_FLIG_REC);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_FLIG_REC)
.onchange = menu_flig_onchange,
 .onkey = menu_flig_onkey,
  .ontouch = NULL,
};
/***************************** 车牌号码设置 ************************************/

static u16 __car_num_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);
    while (p < table_sum) {
        if (!strcmp(mark, text_car_num_table[p].mark)) {
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __car_num_reset(void)
{
    car_num_set_p = 0xff;
    return 0;
}
static u8 __car_num_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = car_num_set_p;
    u16 p;

    table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);

    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (car_num_set_p >= (table_sum - 1)) {
            car_num_set_p = 0;
        } else {
            car_num_set_p++;
        }
        break;
    case DIR_PREV:
        if (car_num_set_p == 0) {
            car_num_set_p = (table_sum - 1);
        } else {
            car_num_set_p--;
        }
        break;
    case DIR_SET:
        p = __car_num_search_by_mark(mark);
        if (p == (u16) - 1) {
            return -1;
        }
        car_num_set_p = p;

        break;
    }
    if (prev_set_p != 0xff) {
        ui_no_highlight_element_by_id(text_car_num_table[prev_set_p].text_id);
    }
    ui_highlight_element_by_id(text_car_num_table[car_num_set_p].text_id);

    return 0;

}
static u8 __car_num_update_show(u8 pos) /* 更新单个时间控件的时间 */
{
    ui_text_show_index_by_id(text_car_num_table[pos].text_id, text_car_num_table[pos].text_index);
    return 0;
}
static u8 __car_num_set_value(enum set_mod mod, u16 value)
{

    u8 p;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    switch (mod) {
    case MOD_ADD:
        switch (car_num_set_p) {
        case 0: /* province */
            if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
            {
                value = text_car_num_table[car_num_set_p].text_index + 1;
                if (value >= 31) {
                    value = 0;
                }
            }
            else
            {
                value = text_car_num_table[car_num_set_p].text_index + 1;
                if (value >= 68) 
                {
                    value = 31;
                }
            }
            printf("123 = %d\n", value);
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
	    case 7: /* f */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= sizeof(num_table)/sizeof(num_table[0])/*(('Z' - 'A' + 1) + ('9' - '0' + 1))*/) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (car_num_set_p) {
        case 0: /* province */
            if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
            {
                value = text_car_num_table[car_num_set_p].text_index;
                if (value == 0) {
                    value = 30;
                } else {
                    value--;
                }
            }
            else
            {
                value = text_car_num_table[car_num_set_p].text_index;
                if (value == 31) {
                    value = 67;
                } else {
                    value--;
                }
            }
            printf("456 = %d\n", value);
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
	    case 7: /* f */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = sizeof(num_table)/sizeof(num_table[0]) - 1;  //('Z' - 'A' + 1) + ('9' - '0' + 1) - 1;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (car_num_set_p) {
        case 0: /* province */
            ASSERT(value >= 0 && value <= 31, "car num set value err!\n");
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
	    case 7: /* f */
            ASSERT(value >= 0 && value <= (('Z' - 'A' + 1) + ('9' - '0' + 1) - 1));
            break;
        default:
            ASSERT(0, "mod_set car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    }
    text_car_num_table[car_num_set_p].text_index = value;

    printf("car_num_set p:%d, value:%d\n", car_num_set_p, value);
    __car_num_update_show(car_num_set_p); /* 更新当前位显示 */

    return 0;
}

static u8 __car_num_get(struct car_num_str *num)
{
    num->province = text_car_num_table[0].text_index;
    num->town = text_car_num_table[1].text_index;
    num->a = text_car_num_table[2].text_index;
    num->b = text_car_num_table[3].text_index;
    num->c = text_car_num_table[4].text_index;
    num->d = text_car_num_table[5].text_index;
    num->e = text_car_num_table[6].text_index;
    num->f = text_car_num_table[7].text_index;

    return 0;
}

static u8 __car_num_set_by_ascii(const char *str)
{
    u16 i;
    u16 province;
    /* ASSERT(strlen(str) == 8, "car num err!"); */

    memcpy((char *)&province, str, 2);
    text_car_num_table[0].text_index = 0;
    for (i = 0; i < (sizeof(province_gb2312) / sizeof(u16)); i++) {
        if (province_gb2312[i] == province) {
            text_car_num_table[0].text_index = i;
            break;
        }
    }
    text_car_num_table[1].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[2]) {
            text_car_num_table[1].text_index = i;
            break;
        }
    }
    text_car_num_table[2].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[3]) {
            text_car_num_table[2].text_index = i;
            break;
        }
    }
    text_car_num_table[3].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[4]) {
            text_car_num_table[3].text_index = i;
            break;
        }
    }
    text_car_num_table[4].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[5]) {
            text_car_num_table[4].text_index = i;
            break;
        }
    }
    text_car_num_table[5].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[6]) {
            text_car_num_table[5].text_index = i;
            break;
        }
    }
    text_car_num_table[6].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[7]) {
            text_car_num_table[6].text_index = i;
            break;
        }
    }

    text_car_num_table[7].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[8]) {
            text_car_num_table[7].text_index = i;
			//printf("345  %s", str[8]);
			//printf("345  %d\n", text_car_num_table[7].text_index);
            break;
        }

    }
    return 0;

}

int menu_rec_car_num_set_lang(void)
{
    struct intent it;
    int err;
    struct car_num_str num;
    u32 part_a,part_b,part_c;
    __car_num_get(&num);
    if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
    {
        part_a = (province_gb2312[0] <<16) |
                 (num_table[num.town] << 8) | (num_table[num.a] <<0)  /*num_table[num.b]*/;
    }
    else
    {
        part_a = (num_table[0] <<16) |
                 (num_table[num.town] << 8) | (num_table[num.a] <<0)  /*num_table[num.b]*/;    
    }
    
    part_b = (num_table[num.b] << 24) | (num_table[num.c] << 16) |
                 (num_table[num.d] << 8)  |  (num_table[num.e] << 0) /*| num_table[num.f]*/;
    part_c = num_table[num.f];

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "cna";
    it.exdata = part_a;
    err = start_app(&it);
    if (err) {
        printf("num set onkey err! %d\n", err);
        return -1;
    }
    it.data = "cnb";
    it.exdata = part_b;
    err = start_app(&it);

    it.data = "cnc";
    it.exdata = part_c;
    err = start_app(&it);
    return 0;
}

int menu_rec_car_num_set(void)
{
    struct intent it;
    int err;
    struct car_num_str num;
    u32 part_a,part_b,part_c;
    __car_num_get(&num);
    if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
    {
        part_a = (province_gb2312[num.province] <<16) |
                 (num_table[num.town] << 8) | (num_table[num.a] <<0)  /*num_table[num.b]*/;
    }
    else
    {
        //printf("=========%c===%d===========\n", num_table[num.province-31], num.province);
        part_a = (num_table[num.province-31] <<16) |
                 (num_table[num.town] << 8) | (num_table[num.a] <<0)  /*num_table[num.b]*/;    
    }
    
    part_b = (num_table[num.b] << 24) | (num_table[num.c] << 16) |
                 (num_table[num.d] << 8)  |  (num_table[num.e] << 0) /*| num_table[num.f]*/;
    part_c = num_table[num.f];

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "cna";
    it.exdata = part_a;
    err = start_app(&it);
    if (err) {
        printf("num set onkey err! %d\n", err);
        return -1;
    }
    it.data = "cnb";
    it.exdata = part_b;
    err = start_app(&it);

    it.data = "cnc";
    it.exdata = part_c;
    err = start_app(&it);
    return 0;
}

static int menu_layout_car_num_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __car_num_switch(DIR_SET, "province");
        break;
    default:
        return false;
    }
    return false;
}
static int menu_layout_car_num_set_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        __car_num_switch(DIR_NEXT, NULL);

        break;
    case KEY_DOWN:
        __car_num_set_value(MOD_ADD, 0);
        return FALSE;

        break;
    case KEY_UP:
        __car_num_set_value(MOD_DEC, 0);
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        rec_exit_menu_post();
        sys_key_event_takeover(false, false);
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CARNUM_SET_REC);
        menu_rec_car_num_set();

        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_CARNUM_SET_REC)
.onchange = menu_layout_car_num_set_onchange,
 .onkey = menu_layout_car_num_set_onkey,
  .ontouch = NULL,
};




/*
 * (begin)菜单中车牌号码的显示初始化动作 ********************************************
 */
static int text_menu_car_num_province_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if( (ui_language_get() == Chinese_Simplified) || (ui_language_get() == Chinese_Traditional) )
        {
            if(text_car_num_table[0].text_index >= 31)
            {
                text_car_num_table[0].text_index = 0;
            } 
        }
        else
        {
            if(text_car_num_table[0].text_index < 31 || text_car_num_table[0].text_index > 67)
            {
                text_car_num_table[0].text_index = 31;
            }      
        }
        ui_text_set_index(text, text_car_num_table[0].text_index);  
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_PROVINCE)
.onchange = text_menu_car_num_province_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_town_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[1].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_TOWN)
.onchange = text_menu_car_num_town_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_a_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[2].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_A)
.onchange = text_menu_car_num_a_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_b_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[3].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_B)
.onchange = text_menu_car_num_b_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_c_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[4].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_C)
.onchange = text_menu_car_num_c_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_d_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[5].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_D)
.onchange = text_menu_car_num_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_e_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[6].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_E)
.onchange = text_menu_car_num_e_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#if 1
static int text_menu_car_num_f_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[7].text_index);
		//printf("123777=====%d\n", text_car_num_table[7].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_F)
.onchange = text_menu_car_num_f_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif
/*
 * (end)---------------------------------------------------------------
 */




static u8 lane_set_flag = 0;
static int menu_lane_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __this->vanish_line    = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        if (__this->vanish_line < 30) {
            __this->vanish_line = 30;
        }
        if (__this->car_head_line > LCD_DEV_HIGHT - 30 || __this->car_head_line < __this->vanish_line + 30) {
            __this->car_head_line = LCD_DEV_HIGHT - 30 ;
        }
        lane_set_flag = 0;
        break;
    default:
        return false;
    }
    return false;
}

#define LANE_LINSE_STEP_SMALL  3
#define LANE_LINSE_STEP_BIG    30
static int menu_lane_set_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_DOWN:
        if (!(lane_set_flag & 0x02)) {
            lane_set_flag |= 0x02;
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        if ((lane_set_flag & 0x01) == 0) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->vanish_line += LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->vanish_line += LANE_LINSE_STEP_BIG;
            }
            __this->vanish_line = __this->vanish_line > __this->car_head_line - 30 ? __this->car_head_line - 30 : __this->vanish_line;
            ui_hide(TEXT_LANE_TOP);
            ui_show(TEXT_LANE_TOP);
        } else if ((lane_set_flag & 0x01) == 1) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->car_head_line += LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->car_head_line += LANE_LINSE_STEP_BIG;
            }
            __this->car_head_line = __this->car_head_line > LCD_DEV_HIGHT - 30 ? LCD_DEV_HIGHT - 30 : __this->car_head_line;
            ui_hide(TEXT_LANE_BOTTOM);
            ui_show(TEXT_LANE_BOTTOM);
        }
        break;
    case KEY_UP:
        if (!(lane_set_flag & 0x02)) {
            lane_set_flag |= 0x02;
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        if ((lane_set_flag & 0x01) == 0) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->vanish_line -= LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->vanish_line -= LANE_LINSE_STEP_BIG;
            }
            __this->vanish_line = __this->vanish_line < 30 ? 30 : __this->vanish_line;
            ui_hide(TEXT_LANE_TOP);
            ui_show(TEXT_LANE_TOP);
        } else if ((lane_set_flag & 0x01) == 1) {
            if (e->event == KEY_EVENT_CLICK) {
                __this->car_head_line -= LANE_LINSE_STEP_SMALL;
            } else if (e->event == KEY_EVENT_HOLD) {
                __this->car_head_line -= LANE_LINSE_STEP_BIG;
            }
            __this->car_head_line = __this->car_head_line < __this->vanish_line + 30 ? __this->vanish_line + 30 : __this->car_head_line;
            ui_hide(TEXT_LANE_BOTTOM);
            ui_show(TEXT_LANE_BOTTOM);
        }
        break;
    case KEY_OK:
        if (lane_set_flag & 0x02) {
            lane_set_flag &= 0x01;
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            menu_lane_det_set(top << 16 | bottom);
            printf("lane set top=%d , bottom=%d\n", top, bottom);

            send_lane_det_setting_msg("lane_set_open", top << 16 | bottom);
        }
        lane_set_flag = !lane_set_flag;
        break;
    case KEY_MODE:
        if (lane_set_flag & 0x02 || lane_set_flag == 0) {
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            menu_lane_det_set(top << 16 | bottom);
            printf("lane set top=%d , bottom=%d\n", top, bottom);
        }
        sys_key_event_takeover(false, false);
        close_set_lane_page(NULL);
        break;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_LANE_REC)
.onchange = menu_lane_set_onchange,
 .onkey = menu_lane_set_onkey,
  .ontouch = NULL,
};


static int lane_set_txt_top_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->vanish_line;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_LANE_TOP)
.onchange = lane_set_txt_top_onchange,
};
static int lane_set_txt_bottom_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->car_head_line - 30;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_LANE_BOTTOM)
.onchange = lane_set_txt_bottom_onchange,
};




#endif
