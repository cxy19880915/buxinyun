#include "ui/includes.h"
#include "system/includes.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "menu_parm_api.h"
#include "video_system.h"
#include "storage_device.h"
#include "app_database.h"
#include "res.h"
#include "asm/lcd_config.h"

#include "video_rec.h"

#ifdef CONFIG_UI_STYLE_JL02_ENABLE


#define STYLE_NAME  JL02

struct sys_menu_info {
    u8 lcd_protect;
    u8 auto_off;
    u8 led_fre_hz;
    u8 key_voice;
    u8 language;
    u8 tv_mod;
    u8 lane_det;
    u8 hl_warnning;

    u8 sys_menu_status;
    u8 page_exit;
    u8 reset_flag;
    u8 battery_val;
    u8 battery_char;

    u8 onkey_mod;
    u8 onkey_sel;

    u8 time_set_pos;
    u8 cid_set_pos;

    int vanish_line;
    int car_head_line;
};


static struct sys_menu_info handler = {0};
#define __this 	(&handler)

extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec 4:sys*/

static u8 mode_sel = 0;
static u8 btn_warning_show_f = 0;//confirm框5秒结束后置1
static u8 confirm_f = 0;//车牌是否定位到确认 1:是 0:否
static u8 cur_sel_item = 0;//当前列表选项框位置
static u8 lane_setting = 0;//正在设置车道偏移

int lane_entry_flag = 0;

static int onkey_sel_item[3] = {
    SYS_BTN_REC,
    SYS_BTN_FILES,
    SYS_BTN_HOME,
};
const int sel_pic2[2] = {
    SYS_PIC_C2_1,
    SYS_PIC_C2_2,
};
const int sel_pic3[3] = {
    SYS_PIC_C3_1,
    SYS_PIC_C3_2,
    SYS_PIC_C3_3,
};
const int sel_pic4[4] = {
    SYS_PIC_C4_1,
    SYS_PIC_C4_2,
    SYS_PIC_C4_3,
    SYS_PIC_C4_4,
};
const int sel_layout2[2] = {
    SYS_LAY_C2_1,
    SYS_LAY_C2_2,
};
const int sel_layout3[3] = {
    SYS_LAY_C3_1,
    SYS_LAY_C3_2,
    SYS_LAY_C3_3,
};
const int sel_layout4[4] = {
    SYS_LAY_C4_1,
    SYS_LAY_C4_2,
    SYS_LAY_C4_3,
    SYS_LAY_C4_4,
};
const int sel_timePic[6] = {
    0,
    SYS_PIC_SEL_YEAR,
    SYS_PIC_SEL_MONTH,
    SYS_PIC_SEL_DAY,
    SYS_PIC_SEL_HOUR,
    SYS_PIC_SEL_MIN,
};
const int language_txt[13] = {
    SYS_FORM_TXT_01,
    SYS_FORM_TXT_02,
    SYS_FORM_TXT_03,
    SYS_FORM_TXT_04,
    SYS_FORM_TXT_05,
    SYS_FORM_TXT_06,
    SYS_FORM_TXT_07,
    SYS_FORM_TXT_08,
    SYS_FORM_TXT_09,
    SYS_FORM_TXT_10,
    SYS_FORM_TXT_11,
    SYS_FORM_TXT_12,
    SYS_FORM_TXT_13,
};
const int cid_btn_id0[31] = {
    SYS_BTN_CID_PRO_01,
    SYS_BTN_CID_PRO_02,
    SYS_BTN_CID_PRO_03,
    SYS_BTN_CID_PRO_04,
    SYS_BTN_CID_PRO_05,
    SYS_BTN_CID_PRO_06,
    SYS_BTN_CID_PRO_07,
    SYS_BTN_CID_PRO_08,
    SYS_BTN_CID_PRO_09,
    SYS_BTN_CID_PRO_10,
    SYS_BTN_CID_PRO_11,
    SYS_BTN_CID_PRO_12,
    SYS_BTN_CID_PRO_13,
    SYS_BTN_CID_PRO_14,
    SYS_BTN_CID_PRO_15,
    SYS_BTN_CID_PRO_16,
    SYS_BTN_CID_PRO_17,
    SYS_BTN_CID_PRO_18,
    SYS_BTN_CID_PRO_19,
    SYS_BTN_CID_PRO_20,
    SYS_BTN_CID_PRO_21,
    SYS_BTN_CID_PRO_22,
    SYS_BTN_CID_PRO_23,
    SYS_BTN_CID_PRO_24,
    SYS_BTN_CID_PRO_25,
    SYS_BTN_CID_PRO_26,
    SYS_BTN_CID_PRO_27,
    SYS_BTN_CID_PRO_28,
    SYS_BTN_CID_PRO_29,
    SYS_BTN_CID_PRO_30,
    SYS_BTN_CID_PRO_31
};
const int cid_btn_id1[36] = {
    SYS_BTN_CID_KB_1,
    SYS_BTN_CID_KB_2,
    SYS_BTN_CID_KB_3,
    SYS_BTN_CID_KB_4,
    SYS_BTN_CID_KB_5,
    SYS_BTN_CID_KB_6,
    SYS_BTN_CID_KB_7,
    SYS_BTN_CID_KB_8,
    SYS_BTN_CID_KB_9,
    SYS_BTN_CID_KB_0,
    SYS_BTN_CID_KB_Q,
    SYS_BTN_CID_KB_W,
    SYS_BTN_CID_KB_E,
    SYS_BTN_CID_KB_R,
    SYS_BTN_CID_KB_T,
    SYS_BTN_CID_KB_Y,
    SYS_BTN_CID_KB_U,
    SYS_BTN_CID_KB_I,
    SYS_BTN_CID_KB_O,
    SYS_BTN_CID_KB_P,
    SYS_BTN_CID_KB_A,
    SYS_BTN_CID_KB_S,
    SYS_BTN_CID_KB_D,
    SYS_BTN_CID_KB_F,
    SYS_BTN_CID_KB_G,
    SYS_BTN_CID_KB_H,
    SYS_BTN_CID_KB_J,
    SYS_BTN_CID_KB_K,
    SYS_BTN_CID_KB_L,
    SYS_BTN_CID_KB_Z,
    SYS_BTN_CID_KB_X,
    SYS_BTN_CID_KB_C,
    SYS_BTN_CID_KB_V,
    SYS_BTN_CID_KB_B,
    SYS_BTN_CID_KB_N,
    SYS_BTN_CID_KB_M,
};
static void set_cid_onkey(u8 key_value);
/************************************************************
                         系统设置
 ************************************************************/
/*
 * system光源频率设置
 */
static const u8 table_system_led_fre[] = {
    50,
    60,
};

/*
 * system自动关机设置
 */
static const u8 table_system_auto_close[] = {
    0,
    3,
    5,
    10,
};

/*
 * system屏幕保护设置
 */
static const u16 table_system_lcd_protect[] = {
    0,
    30,
    60,
    120,
};

/*
 * system语言设置
 */
static const u8 table_system_language[] = {
    Chinese_Simplified,  /* 简体中文 */
    Chinese_Traditional, /* 繁体中文 */
    English,             /* 英文 */
};

/*
 * system光源频率设置
 */
static const u8 table_system_tv_mod[] = {
    TVM_PAL,
    TVM_NTSC,
};

enum {
    PAGE_SHOW = 0,
    MODE_SW_EXIT,
    HOME_SW_EXIT,
};


enum SYS_MENU {
    SYS_MENU_LCDOFF = 0,
    SYS_MENU_AUTOOFF,
    SYS_MENU_FREQUENCY,
    SYS_MENU_KEYVOICE,
    SYS_MENU_LANGUAGE,
    //SYS_MENU_TVMODE,
    SYS_MENU_FORMAT,
    SYS_MENU_DATETIME,
    SYS_MENU_CARNUM,
    SYS_MENU_LANEDET,
    SYS_MENU_HL_WARNNING,
    
    SYS_MENU_RESET,
    SYS_MENU_VERSION,
    SYS_MENU_NONE,
};

#define SYS_SETTING_ITEM   SYS_MENU_NONE
/*******************************************************************/
/*                           时间设置
*******************************************************************/
enum SYS_MENU_TIME {
    SYS_MENU_TIME_NONE = 0,
    SYS_MENU_TIME_YEAR,
    SYS_MENU_TIME_MONTH,
    SYS_MENU_TIME_DAY,
    SYS_MENU_TIME_HOUR,
    SYS_MENU_TIME_MIN,
    SYS_MENU_TIME_SET,
};
static struct sys_time t;
static struct utime ts, tu, td;
static u8 day_max = 31;
static u8 day_set = 31;

static void sys_menu_tim_show()
{

    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        /* memset(t,0,sizeof(struct sys_time)); */
        t.year = 2000;
        t.month = 1;
        t.day = 1;
        t.hour = 0;
        t.min = 0;
        t.sec = 0;
    } else {
        dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)&t);
        UI_ONTOUCH_DEBUG("get_sys_time: %d-%d-%d %d:%d:%d\n",
                         t.year,
                         t.month,
                         t.day,
                         t.hour,
                         t.min,
                         t.sec);
        dev_close(fd);
    }

    ts.year = t.year;
    ts.month = t.month;
    ts.day = t.day;
    ts.hour = t.hour;
    ts.min = t.min;
    ts.sec = 0;

    if (t.month == 1 || t.month == 3 || t.month == 5 || t.month == 7 || t.month == 8 || t.month == 10 || t.month == 12) {
        day_max = 31;
    } else if (t.month == 4 || t.month == 6 || t.month == 9 || t.month == 11) {
        day_max = 30;
    } else if (t.month == 2) {
        if (t.year % 4 == 0 && t.year % 100 != 0) {
            day_max = 29;
        } else if (t.year % 400 == 0) {
            day_max = 29;
        } else {
            day_max = 28;
        }
    }
    day_set = t.day;

    tu.year = (t.year - 1) >= 2000 ? (t.year - 1) : 2099;
    tu.month = (t.month - 1) >= 1 ? (t.month - 1) : 12;
    tu.day = (t.day - 1) >= 1 ? (t.day - 1) : day_max;
    tu.hour = (t.hour - 1) >= 0 ? (t.hour - 1) : 23;
    tu.min = (t.min - 1) >= 0 ? (t.min - 1) : 59;
    tu.sec = 0;

    td.year = (t.year + 1) <= 2099 ? (t.year + 1) : 2000;
    td.month = (t.month + 1) <= 12 ? (t.month + 1) : 1;
    td.day = (t.day + 1) <= day_max ? (t.day + 1) : 1;
    td.hour = (t.hour + 1) <= 23 ? (t.hour + 1) : 0;
    td.min = (t.min + 1) <= 59 ? (t.min + 1) : 0;
    td.sec = 0;

    ui_time_update_by_id(SYS_TIM_SET_YEAR, &ts);
    ui_time_update_by_id(SYS_TIM_SET_MONTH, &ts);
    ui_time_update_by_id(SYS_TIM_SET_DAY, &ts);
    ui_time_update_by_id(SYS_TIM_SET_HOUR, &ts);
    ui_time_update_by_id(SYS_TIM_SET_MIN, &ts);

    ui_time_update_by_id(SYS_TIM_UP_YEAR, &tu);
    ui_time_update_by_id(SYS_TIM_UP_MONTH, &tu);
    ui_time_update_by_id(SYS_TIM_UP_DAY, &tu);
    ui_time_update_by_id(SYS_TIM_UP_HOUR, &tu);
    ui_time_update_by_id(SYS_TIM_UP_MIN, &tu);

    ui_time_update_by_id(SYS_TIM_DOWN_YEAR, &td);
    ui_time_update_by_id(SYS_TIM_DOWN_MONTH, &td);
    ui_time_update_by_id(SYS_TIM_DOWN_DAY, &td);
    ui_time_update_by_id(SYS_TIM_DOWN_HOUR, &td);
    ui_time_update_by_id(SYS_TIM_DOWN_MIN, &td);
}

static void sys_menu_tim_change(enum SYS_MENU_TIME ymdhm, s16 value)
{
    s32 tmp;

    switch (ymdhm) {
    case SYS_MENU_TIME_YEAR:
        tmp = t.year + value;
        ts.year = tmp % 100 + 2000;
        if (ts.year == t.year) {
            break;
        }
        tu.year = (ts.year - 1) >= 2000 ? (ts.year - 1) : 2099;
        td.year = (ts.year + 1) <= 2099 ? (ts.year + 1) : 2000;
        ui_time_update_by_id(SYS_TIM_SET_YEAR, &ts);
        ui_time_update_by_id(SYS_TIM_UP_YEAR, &tu);
        ui_time_update_by_id(SYS_TIM_DOWN_YEAR, &td);
        break;
    case SYS_MENU_TIME_MONTH:
        tmp = t.month + value;
        ts.month = (tmp + 119) % 12 + 1;
        if (ts.month == t.month) {
            break;
        }
        tu.month = (ts.month - 1) >= 1 ? (ts.month - 1) : 12;
        td.month = (ts.month + 1) <= 12 ? (ts.month + 1) : 1;
        ui_time_update_by_id(SYS_TIM_SET_MONTH, &ts);
        ui_time_update_by_id(SYS_TIM_UP_MONTH, &tu);
        ui_time_update_by_id(SYS_TIM_DOWN_MONTH, &td);
        break;
    case SYS_MENU_TIME_DAY:
        tmp = t.day + value;
        ts.day = (tmp + day_max * 10 - 1) % day_max + 1;
        if (ts.day == t.day) {
            break;
        }
        tu.day = (ts.day - 1) >= 1 ? (ts.day - 1) : day_max;
        td.day = (ts.day + 1) <= day_max ? (ts.day + 1) : 1;
        ui_time_update_by_id(SYS_TIM_SET_DAY, &ts);
        ui_time_update_by_id(SYS_TIM_UP_DAY, &tu);
        ui_time_update_by_id(SYS_TIM_DOWN_DAY, &td);
        break;
    case SYS_MENU_TIME_HOUR:
        tmp = t.hour + value;
        ts.hour = (tmp + 240) % 24;
        if (ts.hour == t.hour) {
            break;
        }
        tu.hour = (ts.hour - 1) >= 0 ? (ts.hour - 1) : 23;
        td.hour = (ts.hour + 1) <= 23 ? (ts.hour + 1) : 0;
        ui_time_update_by_id(SYS_TIM_SET_HOUR, &ts);
        ui_time_update_by_id(SYS_TIM_UP_HOUR, &tu);
        ui_time_update_by_id(SYS_TIM_DOWN_HOUR, &td);
        break;
    case SYS_MENU_TIME_MIN:
        tmp = t.min + value;
        ts.min = (tmp + 600) % 60;
        if (ts.min == t.min) {
            break;
        }
        tu.min = (ts.min - 1) >= 0 ? (ts.min - 1) : 59;
        td.min = (ts.min + 1) <= 59 ? (ts.min + 1) : 0;
        ui_time_update_by_id(SYS_TIM_SET_MIN, &ts);
        ui_time_update_by_id(SYS_TIM_UP_MIN, &tu);
        ui_time_update_by_id(SYS_TIM_DOWN_MIN, &td);
        break;
    case SYS_MENU_TIME_SET:
        //如果修改了年月，则判断最大日数
        if (ts.year != t.year || ts.month != t.month) {
            if (ts.month == 1 || ts.month == 3 || ts.month == 5 || ts.month == 7 || ts.month == 8 || ts.month == 10 || ts.month == 12) {
                day_max = 31;
            } else if (ts.month == 4 || ts.month == 6 || ts.month == 9 || ts.month == 11) {
                day_max = 30;
            } else if (ts.month == 2) {
                if (ts.year % 4 == 0 && ts.year % 100 != 0) {
                    day_max = 29;
                } else if (ts.year % 400 == 0) {
                    day_max = 29;
                } else {
                    day_max = 28;
                }
            }
        }

        //如果修改了日，则保存，否则还原
        if (ts.day != t.day) {
            day_set = ts.day;
        } else {
            ts.day = day_set;
            if (ts.day > day_max) {
                ts.day = day_max;
            }
            if (ts.day != t.day) {
                ui_time_update_by_id(SYS_TIM_SET_DAY, &ts);
            }
        }

        //更新日显示
        if (ts.day > day_max) {
            ts.day = day_max;
            ui_time_update_by_id(SYS_TIM_SET_DAY, &ts);
        }
        tmp = (ts.day - 1) >= 1 ? (ts.day - 1) : day_max;
        if (tmp != tu.day) {
            tu.day = tmp;
            ui_time_update_by_id(SYS_TIM_UP_DAY, &tu);
        }
        tmp = (ts.day + 1) <= day_max ? (ts.day + 1) : 1;
        if (tmp != td.day) {
            td.day = tmp;
            ui_time_update_by_id(SYS_TIM_DOWN_DAY, &td);
        }

        //保存时间到t
        t.year = ts.year;
        t.month = ts.month;
        t.day = ts.day;
        t.hour = ts.hour;
        t.min = ts.min;
        t.sec = 0;

        break;
    default:
        break;
    }
}

/*******************************************************************/
/*                           车牌设置
*******************************************************************/
enum SYS_MENU_CID {
    SYS_MENU_CID_NONE = 0,
    SYS_MENU_CID_PRO,
    SYS_MENU_CID_1,
    SYS_MENU_CID_2,
    SYS_MENU_CID_3,
    SYS_MENU_CID_4,
    SYS_MENU_CID_5,
    SYS_MENU_CID_6,
    SYS_MENU_CID_PREV,
    SYS_MENU_CID_NEXT,
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
    '9'
};
static u8 sys_menu_cid_status[7];
static u8 sys_menu_cid_setting = SYS_MENU_CID_NONE;
static void sys_menu_cid_get()
{
    u32 tmp = db_select("cna");
    printf("tmp0=%x\n", tmp);
    sys_menu_cid_status[0] = index_of_table16(tmp >> 16, TABLE(province_gb2312));
    sys_menu_cid_status[1] = index_of_table8((tmp >> 8) & 0xff, TABLE(num_table));
    sys_menu_cid_status[2] = index_of_table8((tmp >> 0) & 0xff, TABLE(num_table));
    tmp = db_select("cnb");
    printf("tmp1=%x\n", tmp);
    sys_menu_cid_status[3] = index_of_table8((tmp >> 24) & 0xff, TABLE(num_table));
    sys_menu_cid_status[4] = index_of_table8((tmp >> 16) & 0xff, TABLE(num_table));
    sys_menu_cid_status[5] = index_of_table8((tmp >> 8) & 0xff, TABLE(num_table));
    sys_menu_cid_status[6] = index_of_table8((tmp >> 0) & 0xff, TABLE(num_table));
    for (int i = 0; i < 6; i++) {
        printf("s[%d]=%d", i, sys_menu_cid_status[i]);
    }
}

static void sys_menu_cid_save()
{
    u32 tmp;
    tmp = ((province_gb2312[sys_menu_cid_status[0]] << 16) |
           (num_table[sys_menu_cid_status[1]] << 8) |
           (num_table[sys_menu_cid_status[2]] << 0));
    db_update("cna", tmp);
    tmp = ((num_table[sys_menu_cid_status[3]] << 24) |
           (num_table[sys_menu_cid_status[4]] << 16) |
           (num_table[sys_menu_cid_status[5]] << 8) |
           (num_table[sys_menu_cid_status[6]] << 0));
    db_update("cnb", tmp);
    db_flush();
}

static void sys_menu_cid_show()
{
    sys_menu_cid_get();

    ui_pic_show_image_by_id(SYS_PIC_CID_0, sys_menu_cid_status[0] + 1);
    ui_pic_show_image_by_id(SYS_PIC_CID_1, sys_menu_cid_status[1] + 1);
    ui_pic_show_image_by_id(SYS_PIC_CID_2, sys_menu_cid_status[2] + 1);
    ui_pic_show_image_by_id(SYS_PIC_CID_3, sys_menu_cid_status[3] + 1);
    ui_pic_show_image_by_id(SYS_PIC_CID_4, sys_menu_cid_status[4] + 1);
    ui_pic_show_image_by_id(SYS_PIC_CID_5, sys_menu_cid_status[5] + 1);
    ui_pic_show_image_by_id(SYS_PIC_CID_6, sys_menu_cid_status[6] + 1);
    sys_menu_cid_setting = SYS_MENU_CID_PRO;
}
static void sys_menu_cid_toggle(void *p)
{
    static u8 flag = 0;
    static u8 last = SYS_MENU_CID_NONE;
    if (flag) {
        switch (sys_menu_cid_setting) {
        case SYS_MENU_CID_NONE:
            break;
        case SYS_MENU_CID_PRO:
            ui_pic_show_image_by_id(SYS_PIC_CID_0, 0);
            break;
        case SYS_MENU_CID_1:
            ui_pic_show_image_by_id(SYS_PIC_CID_1, 0);
            break;
        case SYS_MENU_CID_2:
            ui_pic_show_image_by_id(SYS_PIC_CID_2, 0);
            break;
        case SYS_MENU_CID_3:
            ui_pic_show_image_by_id(SYS_PIC_CID_3, 0);
            break;
        case SYS_MENU_CID_4:
            ui_pic_show_image_by_id(SYS_PIC_CID_4, 0);
            break;
        case SYS_MENU_CID_5:
            ui_pic_show_image_by_id(SYS_PIC_CID_5, 0);
            break;
        case SYS_MENU_CID_6:
            ui_pic_show_image_by_id(SYS_PIC_CID_6, 0);
            break;
        }
    } else {
        if (last != sys_menu_cid_setting) {
            switch (last) {
            case SYS_MENU_CID_NONE:
                break;
            case SYS_MENU_CID_PRO:
                ui_pic_show_image_by_id(SYS_PIC_CID_0, sys_menu_cid_status[0] + 1);
                break;
            case SYS_MENU_CID_1:
                ui_pic_show_image_by_id(SYS_PIC_CID_1, sys_menu_cid_status[1] + 1);
                break;
            case SYS_MENU_CID_2:
                ui_pic_show_image_by_id(SYS_PIC_CID_2, sys_menu_cid_status[2] + 1);
                break;
            case SYS_MENU_CID_3:
                ui_pic_show_image_by_id(SYS_PIC_CID_3, sys_menu_cid_status[3] + 1);
                break;
            case SYS_MENU_CID_4:
                ui_pic_show_image_by_id(SYS_PIC_CID_4, sys_menu_cid_status[4] + 1);
                break;
            case SYS_MENU_CID_5:
                ui_pic_show_image_by_id(SYS_PIC_CID_5, sys_menu_cid_status[5] + 1);
                break;
            case SYS_MENU_CID_6:
                ui_pic_show_image_by_id(SYS_PIC_CID_6, sys_menu_cid_status[6] + 1);
                break;
            }
        }
        switch (sys_menu_cid_setting) {
        case SYS_MENU_CID_NONE:
            break;
        case SYS_MENU_CID_PRO:
            ui_pic_show_image_by_id(SYS_PIC_CID_0, sys_menu_cid_status[0] + 1);
            break;
        case SYS_MENU_CID_1:
            ui_pic_show_image_by_id(SYS_PIC_CID_1, sys_menu_cid_status[1] + 1);
            break;
        case SYS_MENU_CID_2:
            ui_pic_show_image_by_id(SYS_PIC_CID_2, sys_menu_cid_status[2] + 1);
            break;
        case SYS_MENU_CID_3:
            ui_pic_show_image_by_id(SYS_PIC_CID_3, sys_menu_cid_status[3] + 1);
            break;
        case SYS_MENU_CID_4:
            ui_pic_show_image_by_id(SYS_PIC_CID_4, sys_menu_cid_status[4] + 1);
            break;
        case SYS_MENU_CID_5:
            ui_pic_show_image_by_id(SYS_PIC_CID_5, sys_menu_cid_status[5] + 1);
            break;
        case SYS_MENU_CID_6:
            ui_pic_show_image_by_id(SYS_PIC_CID_6, sys_menu_cid_status[6] + 1);
            break;
        }
    }
    last = sys_menu_cid_setting;
    flag = !flag;
}
static void sys_menu_cid_set(u8 value)
{
    switch (sys_menu_cid_setting) {
    case SYS_MENU_CID_PRO:
        sys_menu_cid_status[0] = value;
        break;
    case SYS_MENU_CID_1:
        sys_menu_cid_status[1] = value;
        break;
    case SYS_MENU_CID_2:
        sys_menu_cid_status[2] = value;
        break;
    case SYS_MENU_CID_3:
        sys_menu_cid_status[3] = value;
        break;
    case SYS_MENU_CID_4:
        sys_menu_cid_status[4] = value;
        break;
    case SYS_MENU_CID_5:
        sys_menu_cid_status[5] = value;
        break;
    case SYS_MENU_CID_6:
        sys_menu_cid_status[6] = value;
        break;
    }
}
static void sys_menu_cid_selete(u8 item)
{
    if (item == sys_menu_cid_setting) {
        return;
    }
    switch (item) {
    case SYS_MENU_CID_PRO:
        ui_hide(SYS_LAY_CID_KB);
        ui_show(SYS_LAY_CID_PRO);
        sys_menu_cid_setting = item;
        break;
    case SYS_MENU_CID_1:
    case SYS_MENU_CID_2:
    case SYS_MENU_CID_3:
    case SYS_MENU_CID_4:
    case SYS_MENU_CID_5:
    case SYS_MENU_CID_6:
        ui_hide(SYS_LAY_CID_PRO);
        ui_show(SYS_LAY_CID_KB);
        sys_menu_cid_setting = item;
        break;
    case SYS_MENU_CID_PREV:
        sys_menu_cid_setting--;
        if (sys_menu_cid_setting < SYS_MENU_CID_PRO) {
            sys_menu_cid_setting = SYS_MENU_CID_6;
        }
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_hide(SYS_LAY_CID_KB);
            ui_show(SYS_LAY_CID_PRO);
        } else {
            ui_hide(SYS_LAY_CID_PRO);
            ui_show(SYS_LAY_CID_KB);
        }
        break;
    case SYS_MENU_CID_NEXT:
        sys_menu_cid_setting++;
        if (sys_menu_cid_setting > SYS_MENU_CID_6) {
            sys_menu_cid_setting = SYS_MENU_CID_PRO;
        }
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_hide(SYS_LAY_CID_KB);
            ui_show(SYS_LAY_CID_PRO);
        } else {
            ui_hide(SYS_LAY_CID_PRO);
            ui_show(SYS_LAY_CID_KB);
        }
        break;
    }
}

/*******************************************************************/
/*                           格式化、默认设置
*******************************************************************/
static void sys_warning_tick(void *_p)
{
    int *p = (int *)_p;
    int item = *p;

    if (__this->sys_menu_status == SYS_MENU_FORMAT && !storage_device_ready()) {
        if (item < 0) {
            ui_hide(SYS_BTN_WARNING_CONFIRM);
            btn_warning_show_f = 0;
            ui_show(SYS_PIC_WARNING_CONFIRM);
        }
        (*p) = 5;
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 2);
        ui_text_show_index_by_id(SYS_TXT_WARNING_CONFIRM, 0);
        return;
    } else if (item == 5) {
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 0);
    }

    if (!item) {
        ui_hide(SYS_PIC_WARNING_CONFIRM);
        ui_show(SYS_BTN_WARNING_CONFIRM);
        btn_warning_show_f = 1;
        if (__this->sys_menu_status == SYS_MENU_RESET) {
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 1);
        }
    }
    ui_text_show_index_by_id(SYS_TXT_WARNING_CONFIRM, 5 - item);
    (*p)--;
}
static int get_child_item_num(u8 sys_menu_status)
{
    switch (sys_menu_status) {
    case SYS_MENU_AUTOOFF:
    case SYS_MENU_LCDOFF:
        return ARRAY_SIZE(table_system_auto_close);
    case SYS_MENU_FREQUENCY:
    case SYS_MENU_KEYVOICE:
    //case SYS_MENU_TVMODE:
    case SYS_MENU_FORMAT:
    case SYS_MENU_RESET:
    case SYS_MENU_HL_WARNNING:
    case SYS_MENU_LANEDET:
        return 2;
    case SYS_MENU_LANGUAGE:
        return ARRAY_SIZE(table_system_language);
    default:
        break;
    }
    return 0;
}
int menu_sys_format_set(u8 sel_item);
int menu_sys_default_set(u8 sel_item);
static void set_format_reset_func(void)
{

    switch (__this->sys_menu_status) {
        ui_hide(SYS_BTN_WARNING_CONFIRM);
        btn_warning_show_f = 0;
        ui_hide(SYS_TXT_WARNING_CONFIRM);
    case SYS_MENU_FORMAT:
        if (storage_device_ready()) {
            menu_sys_format_set(1);
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 3);
        }
        break;
    case SYS_MENU_RESET:
        menu_sys_default_set(1);
        sys_fun_restore();
        __this->reset_flag = 1;
        ui_hide(SYS_LAY);
        ui_show(SYS_LAY);
        __this->lcd_protect = index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect));
        __this->auto_off    = index_of_table8(db_select("aff"), TABLE(table_system_auto_close));
        __this->led_fre_hz  = index_of_table8(db_select("fre"), TABLE(table_system_led_fre));
        __this->key_voice   = db_select("kvo");
        __this->language    = index_of_table8(db_select("lag"), TABLE(table_system_language));
        __this->tv_mod      = index_of_table8(db_select("tvm"), TABLE(table_system_tv_mod));
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 7);

        break;
    }
}

/*
 * (begin)提示框显示接口 ********************************************
 */
enum box_msg {
    BOX_MSG_POWER_OFF = 1,
    BOX_MSG_NO_POWER,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __sys_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(SYS_LAY_MESSAGEBOX);
        }
    } else if (id == 0) { /* 没有指定ID，强制隐藏 */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(SYS_LAY_MESSAGEBOX);
        }
    }
}
static void __sys_msg_timeout_func(void *priv)
{
    __sys_msg_hide((enum box_msg)priv);
}
static void __sys_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__main_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(SYS_LAY_MESSAGEBOX);
            ui_text_show_index_by_id(SYS_TXT_MESSAGEBOX, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __sys_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(SYS_LAY_MESSAGEBOX);
        }
        ui_text_show_index_by_id(SYS_TXT_MESSAGEBOX, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __sys_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */
/*******************************************************************/
/*                           菜单显示
/*******************************************************************/

static void sys_menu_show(enum SYS_MENU item)
{
    switch (item) {
    case SYS_MENU_LCDOFF:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_01);
        ui_highlight_element_by_id(SYS_FORM_TXT_01);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 0);
        ui_show(SYS_LAY_C4);
        ui_text_show_index_by_id(SYS_TXT_C4_2, 0);
        ui_text_show_index_by_id(SYS_TXT_C4_3, 0);
        ui_text_show_index_by_id(SYS_TXT_C4_4, 0);
        switch (__this->lcd_protect) {
        case 0:
            ui_show(SYS_PIC_C4_1);
            break;
        case 1:
            ui_show(SYS_PIC_C4_2);
            break;
        case 2:
            ui_show(SYS_PIC_C4_3);
            break;
        case 3:
            ui_show(SYS_PIC_C4_4);
            break;
        }
        break;
    case SYS_MENU_AUTOOFF:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_02);
        ui_highlight_element_by_id(SYS_FORM_TXT_02);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 1);
        ui_show(SYS_LAY_C4);
        ui_text_show_index_by_id(SYS_TXT_C4_2, 1);
        ui_text_show_index_by_id(SYS_TXT_C4_3, 1);
        ui_text_show_index_by_id(SYS_TXT_C4_4, 1);
        switch (__this->auto_off) {
        case 0:
            ui_show(SYS_PIC_C4_1);
            break;
        case 1:
            ui_show(SYS_PIC_C4_2);
            break;
        case 2:
            ui_show(SYS_PIC_C4_3);
            break;
        case 3:
            ui_show(SYS_PIC_C4_4);
            break;
        }
        break;
    case SYS_MENU_FREQUENCY:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_03);
        ui_highlight_element_by_id(SYS_FORM_TXT_03);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 2);
        ui_show(SYS_LAY_C2);
        ui_text_show_index_by_id(SYS_TXT_C2_1, 0);
        ui_text_show_index_by_id(SYS_TXT_C2_2, 0);
        switch (__this->led_fre_hz) {
        case 0:
            ui_show(SYS_PIC_C2_1);
            break;
        case 1:
            ui_show(SYS_PIC_C2_2);
            break;
        }
        break;
    case SYS_MENU_KEYVOICE:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_04);
        ui_highlight_element_by_id(SYS_FORM_TXT_04);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 3);
        ui_show(SYS_LAY_C2);
        ui_text_show_index_by_id(SYS_TXT_C2_1, 1);
        ui_text_show_index_by_id(SYS_TXT_C2_2, 1);
        switch (__this->key_voice) {
        case 0:
            ui_show(SYS_PIC_C2_1);
            break;
        case 1:
            ui_show(SYS_PIC_C2_2);
            break;
        }
        break;
    case SYS_MENU_LANGUAGE:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_05);
        ui_highlight_element_by_id(SYS_FORM_TXT_05);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 4);
        ui_show(SYS_LAY_C3);
        switch (__this->language) {
        case 0:
            ui_show(SYS_PIC_C3_1);
            break;
        case 1:
            ui_show(SYS_PIC_C3_2);
            break;
        case 2:
            ui_show(SYS_PIC_C3_3);
            break;
        }
        break;
    #if 0    
    case SYS_MENU_TVMODE:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_06);
        ui_highlight_element_by_id(SYS_FORM_TXT_06);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 5);
        ui_show(SYS_LAY_C2);
        ui_text_show_index_by_id(SYS_TXT_C2_1, 2);
        ui_text_show_index_by_id(SYS_TXT_C2_2, 2);
        switch (__this->tv_mod) {
        case 0:
            ui_show(SYS_PIC_C2_1);
            break;
        case 1:
            ui_show(SYS_PIC_C2_2);
            break;
        }
        break;
    #endif    
    case SYS_MENU_DATETIME:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_07);
        ui_highlight_element_by_id(SYS_FORM_TXT_07);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 6);
        ui_show(SYS_LAY_TIM);
        /* >>>>>>>>>>>>>显示当前时间 */
        /* sys_menu_tim_show(); */
        break;
    case SYS_MENU_CARNUM:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_08);
        ui_highlight_element_by_id(SYS_FORM_TXT_08);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 7);
        ui_show(SYS_LAY_CID);
        /* >>>>>>>>>>>>>显示当前车牌，第一位闪烁 */
        /* sys_menu_cid_show(); */
        break;
    case SYS_MENU_LANEDET:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_09);
        ui_highlight_element_by_id(SYS_FORM_TXT_09);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 8);
        ui_show(SYS_LAY_C2);
        ui_text_show_index_by_id(SYS_TXT_C2_1, 1);
        ui_text_show_index_by_id(SYS_TXT_C2_2, 1);
        switch (__this->lane_det) {
        case 0:
            ui_show(SYS_PIC_C2_1);
            break;
        case 1:
            ui_show(SYS_PIC_C2_2);
            break;
        }
        break;
    case SYS_MENU_HL_WARNNING:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_10);
        ui_highlight_element_by_id(SYS_FORM_TXT_10);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 9);
        ui_show(SYS_LAY_C2);
        ui_text_show_index_by_id(SYS_TXT_C2_1, 1);
        ui_text_show_index_by_id(SYS_TXT_C2_2, 1);
        switch (__this->hl_warnning) {
        case 0:
            ui_show(SYS_PIC_C2_1);
            break;
        case 1:
            ui_show(SYS_PIC_C2_2);
            break;
        }
        break;
    case SYS_MENU_FORMAT:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_06);
        ui_highlight_element_by_id(SYS_FORM_TXT_06);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 05);
        ui_show(SYS_LAY_WARNING);
        if (storage_device_ready()) {
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 0);
        } else {
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 2);
        }
        /* >>>>>>>>>>>>倒计时五秒，隐藏SYS_PIC_WARNING_CONFIRM,显示SYS_BTN_WARNING_CONFIRM */
        break;
    case SYS_MENU_RESET:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_11);
        ui_highlight_element_by_id(SYS_FORM_TXT_11);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 05);
        ui_show(SYS_LAY_WARNING);
        if (__this->reset_flag) {
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 7);
            ui_text_show_index_by_id(SYS_TXT_WARNING_CONFIRM, 5);
        } else {
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 1);
        }
        /* >>>>>>>>>>>>倒计时五秒，隐藏SYS_PIC_WARNING_CONFIRM,显示SYS_BTN_WARNING_CONFIRM */
        break;
    case SYS_MENU_VERSION:
        __this->sys_menu_status = item;
        ui_highlight_element_by_id(SYS_FORM_PIC_12);
        ui_highlight_element_by_id(SYS_FORM_TXT_12);
        ui_text_show_index_by_id(SYS_TXT_TITLE, 10);
        ui_show(SYS_LAY_VERSION);
        /* >>>>>>>>>>>>显示版本号 */
        #if 1//(SLELE_5XX == JL5601)
        ui_text_set_str_by_id(SYS_TXT_VERSION, "ascii", "VERSION: 5.9.6J 20180316");
        #endif
        #if 0//(SLELE_5XX == JL5401)
        ui_text_set_str_by_id(SYS_TXT_VERSION, "ascii", "VERSION: 5.7.4 20170925");
        #endif
        break;
    }
}

static void sys_menu_hide()
{
    switch (__this->sys_menu_status) {
    case SYS_MENU_LCDOFF:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_01);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_01);
        ui_hide(SYS_LAY_C4);
        break;
    case SYS_MENU_AUTOOFF:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_02);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_02);
        ui_hide(SYS_LAY_C4);
        break;
    case SYS_MENU_FREQUENCY:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_03);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_03);
        ui_hide(SYS_LAY_C2);
        break;
    case SYS_MENU_KEYVOICE:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_04);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_04);
        ui_hide(SYS_LAY_C2);
        break;
    case SYS_MENU_LANGUAGE:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_05);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_05);
        ui_hide(SYS_LAY_C3);
        break;
    #if 0    
    case SYS_MENU_TVMODE:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_06);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_06);
        ui_hide(SYS_LAY_C2);
        break;
    #endif    
    case SYS_MENU_DATETIME:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_07);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_07);
        ui_hide(SYS_LAY_TIM);
        break;
    case SYS_MENU_CARNUM:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_08);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_08);
        ui_hide(SYS_LAY_CID);
        break;
    case SYS_MENU_LANEDET:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_09);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_09);
        ui_hide(SYS_LAY_C2);
        break;
    case SYS_MENU_HL_WARNNING:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_10);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_10);
        ui_hide(SYS_LAY_C2);
        break;
    case SYS_MENU_FORMAT:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_06);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_06);
        ui_hide(SYS_LAY_WARNING);
        break;
    case SYS_MENU_RESET:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_11);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_11);
        ui_hide(SYS_LAY_WARNING);
        __this->reset_flag = 0;
        break;
    case SYS_MENU_VERSION:
        ui_no_highlight_element_by_id(SYS_FORM_PIC_12);
        ui_no_highlight_element_by_id(SYS_FORM_TXT_12);
        ui_hide(SYS_LAY_VERSION);
        break;
    default:
        __this->sys_menu_status = 0;
        break;
    }
}


int sys_menu(int item)
{
    if (item == __this->sys_menu_status || item < 0 || item >= SYS_MENU_NONE) {
        return 0;
    } else {
        sys_menu_hide();
        sys_menu_show(item);
    }
    return 0;
}




/*
 * (begin)系统设置各项操作请求
 */

int menu_sys_lcd_pro_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "pro";
    it.exdata = table_system_lcd_protect[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":lcd_protect set fail!\n");
        return -1;
    }
    __this->lcd_protect = sel_item;
    printf("set lcd_protect ok: %d\n", __this->lcd_protect);
    return 0;
}
int menu_sys_auto_off_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "aff";
    it.exdata = table_system_auto_close[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":auto_off set fail!\n");
        return -1;
    }
    __this->auto_off = sel_item;
    printf("set auto_off ok: %d\n", __this->auto_off);

    return 0;
}
int menu_sys_led_fre_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "fre";
    it.exdata = table_system_led_fre[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":led_fre set fail!\n");
        return -1;
    }
    __this->led_fre_hz = sel_item;
    printf("set led_fre ok: %d\n", __this->led_fre_hz);

    return 0;
}
int menu_sys_key_voice_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "kvo";
    it.exdata = sel_item;
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":key_voice set fail!\n");
        return -1;
    }
    __this->key_voice = sel_item;
    printf("set key_voice ok: %d\n", __this->key_voice);

    return 0;
}
int menu_sys_hlight_warning_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "hlw";
    it.exdata = sel_item;
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":head light warning set fail!\n");
        return -1;
    }
    __this->hl_warnning = sel_item;

    return 0;
}
int menu_sys_language_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "lag";
    it.exdata = table_system_language[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":language set fail!\n");
        return -1;
    }
    __this->language = sel_item;
    printf("set language ok: %d\n", __this->language);

    return 0;
}
int menu_sys_tv_mod_set(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "tvm";
    it.exdata = table_system_tv_mod[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":tv_mod set fail!\n");
        return -1;
    }
    __this->tv_mod = sel_item;
    printf("set tv_mod ok: %d\n", __this->tv_mod);

    return 0;
}

void menu_lane_det_set(int value)
{
    struct intent it;
    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_SET_CONFIG;
    it.data = "lan";
    it.exdata = value;
    start_app(&it);
    if (value) {
        __this->lane_det = 1;
    } else {
        __this->lane_det = 0;
    }
}

void format_callback(void *priv, int err)
{
    if (err) {
        printf("format err!!\n");
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 5);
    } else {
        printf("format success!!\n");
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 4);
    }
}

int menu_sys_format_set(u8 sel_item)
{
    struct intent it;
    if (sel_item == 1) {
        init_intent(&it);
        it.name	= "video_system";
        it.action = ACTION_SYSTEM_SET_CONFIG;
        it.data = "frm";
        start_app_async(&it, format_callback, NULL);
    }
    return 0;
}
void default_callback(void *priv, int err)
{
    if (err) {
        printf("default err!!\n");
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 8);
    } else {
        printf("default success!!\n");
        ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 7);
    }
}
int menu_sys_default_set(u8 sel_item)
{
    if (sel_item == 1) {
        struct sys_time time;
        
        void *fd = dev_open("rtc", NULL);
        if (!fd) {
            return -EFAULT;
        }
        time.year = 2017;
        time.month = 1;
        time.day = 1;
        time.hour = 12;
        time.min = 0;
        time.sec = 0,
        dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&time);
        dev_close(fd);
        
        db_reset();
        default_callback(NULL, 0);
    }
    return 0;
}


int menu_sys_version_set(u8 sel_item)
{

    return 0;
}

/***************************** 返回HOME按钮动作 ************************************/
static int sys_backhome_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys back to home ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->page_exit = HOME_SW_EXIT;
        init_intent(&it);
        app = get_current_app();
        if (app) {
            it.name = "video_system";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_HOME)
.ontouch = sys_backhome_ontouch,
};

/***************************** 返回REC按钮动作 ************************************/
static int sys_to_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys to rec ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        init_intent(&it);
        app = get_current_app();
        if (app) {
            __this->page_exit = MODE_SW_EXIT;
            it.name = "video_system";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app

            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_MAIN;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_REC)
.ontouch = sys_to_rec_ontouch,
};
static int sys_to_file_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys to file ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        init_intent(&it);
        app = get_current_app();
        if (app) {
            it.name = "video_system";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app

            it.name = "video_dec";
            it.action = ACTION_VIDEO_DEC_MAIN;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_FILES)
.ontouch = sys_to_file_ontouch,
};

/*
 * battery事件处理函数
 */
static void battery_event_handler(struct sys_event *event, void *priv)
{
    static u8 ten_sec_off = 0;
    if (event->type == SYS_KEY_EVENT || event->type == SYS_TOUCH_EVENT) {
        if (ten_sec_off) {
            ten_sec_off = 0;
            __sys_msg_hide(0);
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
                    /* ui_battery_level_change(__this->battery_val, 0); */
                }
            }
        } else if (!ASCII_StrCmp(event->arg, "charger", 7)) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                /* ui_battery_level_change(100, 1); */
                __this->battery_char = 1;
                if (ten_sec_off) {
                    ten_sec_off = 0;
                    __sys_msg_hide(0);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                /* ui_battery_level_change(__this->battery_val, 0); */
                __this->battery_char = 0;
                __sys_msg_show(BOX_MSG_POWER_OFF, 0);
                ten_sec_off = 1;
            }
        }
    }
}

static void no_power_msg_box_timer(void *priv)
{
    static u32 cnt = 0;
    if (__this->battery_val <= 20 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            puts("no power show.\n");
            __sys_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            puts("no power hide.\n");
            __sys_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __sys_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
    }
}
static int highlight_no_cid_item(int p);







/***************************** 系统菜单页面回调 ************************************/
static int sys_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err;
    static u32 timer_handle = 0;
    struct intent it;
    static u16 id = 0;
    static void *fd = NULL;
    switch (e) {
    case ON_CHANGE_INIT:
        sys_cur_mod = 4;
        memset(__this, 0, sizeof(struct sys_menu_info));
        __this->sys_menu_status = SYS_MENU_NONE;
        __this->lcd_protect = index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect));
        __this->auto_off    = index_of_table8(db_select("aff"), TABLE(table_system_auto_close));
        __this->led_fre_hz  = index_of_table8(db_select("fre"), TABLE(table_system_led_fre));
        __this->key_voice   = db_select("kvo");
        __this->language    = index_of_table8(db_select("lag"), TABLE(table_system_language));
        __this->tv_mod      = index_of_table8(db_select("tvm"), TABLE(table_system_tv_mod));
        __this->vanish_line    = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->lane_det    = !!db_select("lan");
        __this->hl_warnning = db_select("hlw");

        if (lane_setting) {
            lane_setting = 0;
            ui_set_call(sys_menu, SYS_MENU_LANEDET);
            __this->onkey_mod = 0;
            __this->onkey_sel = SYS_MENU_LANEDET;
        } else {
            ui_set_call(sys_menu, 0);
            __this->onkey_mod = 0;
            __this->onkey_sel = 0;
        }
        sys_key_event_takeover(true, false);
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
        timer_handle = sys_timer_add(NULL, no_power_msg_box_timer, 1000);
        id = register_sys_event_handler(SYS_DEVICE_EVENT | SYS_KEY_EVENT | SYS_TOUCH_EVENT, 200, fd, battery_event_handler);

        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        if (timer_handle) {
            sys_timer_del(timer_handle);
            timer_handle = 0;
        }
        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE);
        }
        __sys_msg_hide(0);//强制隐藏消息框
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_SYS)
.onchange = sys_page_onchange,
 .ontouch = NULL,
};
int language_set_highlight(int p)
{
    if (__this->onkey_mod == 1) {
        if (__this->sys_menu_status == SYS_MENU_LANGUAGE) {
            ui_highlight_element_by_id(sel_layout3[__this->language]);
        }
    }
    return 0;
}
/***************************** 第一次显示系统菜单 ************************************/
static int sys_table_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    /* UI_ONTOUCH_DEBUG("\n***sys table onchange %d***\n",e); */
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, -1);
        break;
    case ON_CHANGE_TRY_OPEN_DC:
        ui_core_element_on_focus(ui_core_get_element_by_id(SYS_WIN), true);
        break;
    case ON_CHANGE_FIRST_SHOW:
        /* ui_set_call(sys_menu_show, __this->sys_menu_status); */
        sys_menu_show(__this->sys_menu_status);//降低刷新效果
        return true;
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        ui_set_call(language_set_highlight, 0);
        break;
    default:
        return false;
    }
    return false;
}

/***************************** 选择系统菜单 ************************************/
static int sys_table_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys setting table ontouch**");
    int sel_item;
    int i;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sel_item = ui_grid_cur_item(grid);
        printf("sel_item=%d\n", sel_item);
        if (sel_item < 0 || sel_item > SYS_SETTING_ITEM) {
            printf("sel item=%d\n", sel_item);
            break;
        }
        if (__this->onkey_mod == 2) {
            ui_no_highlight_element_by_id(onkey_sel_item[mode_sel]);
            mode_sel = 0;
        } else if (__this->onkey_mod == 1) {
            if (__this->onkey_sel == sel_item) {
                if (__this->sys_menu_status == SYS_MENU_FORMAT || __this->sys_menu_status == SYS_MENU_RESET) {
                    ui_no_highlight_element_by_id(SYS_BTN_WARNING_CONFIRM);

                } else if (__this->sys_menu_status == SYS_MENU_DATETIME) {
                    if (__this->time_set_pos > SYS_MENU_TIME_YEAR && __this->time_set_pos < SYS_MENU_TIME_SET) {
                        ui_hide(sel_timePic[__this->time_set_pos]);
                    } else if (__this->time_set_pos == SYS_MENU_TIME_SET) {
                        ui_no_highlight_element_by_id(SYS_BTN_TIM_CONFIRM);
                    }

                } else if (__this->sys_menu_status == SYS_MENU_CARNUM) {
                    if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
                        ui_no_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
                    } else {
                        ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
                        if (confirm_f) {
                            confirm_f = 0;
                            ui_no_highlight_element_by_id(SYS_BTN_CID_CONFIRM);
                        }
                    }
                } else {
                    switch (__this->sys_menu_status) {
                    case SYS_MENU_LCDOFF:
                    case SYS_MENU_AUTOOFF:
                        ui_no_highlight_element_by_id(sel_layout4[cur_sel_item]);
                        break;
                    case SYS_MENU_FREQUENCY:
                    case SYS_MENU_KEYVOICE:
                    //case SYS_MENU_TVMODE:
                        ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
                        break;
                    case SYS_MENU_LANGUAGE:
                        ui_no_highlight_element_by_id(sel_layout3[cur_sel_item]);
                        break;
                    }
                }
            }
        }
        __this->onkey_sel = sel_item;
        __this->onkey_mod = 0;
        ui_set_call(sys_menu, sel_item);

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_FORM)
.ontouch = sys_table_ontouch,
 .onchange = sys_table_onchange,
};



/***************************** 显示系统时钟设置界面 ************************************/
static int sys_time_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        sys_menu_tim_show();
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_LAY_TIM)
.onchange = sys_time_onchange,
};

/***************************** 显示系统时钟界面操作 ************************************/
static int sys_time_ontouch(void *ctr, struct element_touch_event *e)
{
#define SYS_TIME_TOUCH_BASE_LEN 30
    static enum SYS_MENU_TIME ymdhm = SYS_MENU_TIME_NONE;
    static u32 y = 0;
    static s32 value;

    UI_ONTOUCH_DEBUG("**sys time ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        y = e->pos.y;
        if (e->pos.x > 810 && e->pos.x < 958 && e->pos.y > 91 && e->pos.y < 235) {
            ymdhm = SYS_MENU_TIME_YEAR;
        } else if (e->pos.x > 958 && e->pos.x < 1050 && e->pos.y > 91 && e->pos.y < 235) {
            ymdhm = SYS_MENU_TIME_MONTH;
        } else if (e->pos.x > 1050 && e->pos.x < 1150 && e->pos.y > 91 && e->pos.y < 235) {
            ymdhm = SYS_MENU_TIME_DAY;
        } else if (e->pos.x > 810 && e->pos.x < 958 && e->pos.y > 257 && e->pos.y < 400) {
            ymdhm = SYS_MENU_TIME_HOUR;
        } else if (e->pos.x > 958 && e->pos.x < 1050 && e->pos.y > 257 && e->pos.y < 400) {
            ymdhm = SYS_MENU_TIME_MIN;
        }
        return true;
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        if (y >= e->pos.y) {
            value = (y - e->pos.y) / SYS_TIME_TOUCH_BASE_LEN;
        } else {
            value = -((e->pos.y - y) / SYS_TIME_TOUCH_BASE_LEN);
        }
        sys_menu_tim_change(ymdhm, value);
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ymdhm = SYS_MENU_TIME_SET;
        value = (y - e->pos.y) / SYS_TIME_TOUCH_BASE_LEN;
        sys_menu_tim_change(ymdhm, value);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_PIC_TIM)
.ontouch = sys_time_ontouch,
};

/***************************** 显示系统时钟确认修改 ************************************/
static int sys_time_confirm(void *ctr, struct element_touch_event *e)
{
    char tim_str[30];
    struct intent it;
    int err;
    UI_ONTOUCH_DEBUG("**sys time confirm ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->onkey_mod == 1) {
            if (__this->time_set_pos > SYS_MENU_TIME_YEAR && __this->time_set_pos < SYS_MENU_TIME_SET) {
                ui_hide(sel_timePic[__this->time_set_pos]);
            } else if (__this->time_set_pos == SYS_MENU_TIME_SET) {
                ui_no_highlight_element_by_id(SYS_BTN_TIM_CONFIRM);
            }
            __this->onkey_mod = 0;
        }
        void *fd = dev_open("rtc", NULL);
        if (!fd) {
            return -EFAULT;
        }
        dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&t);
        dev_close(fd);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_TIM_CONFIRM)
.ontouch = sys_time_confirm,
};



/***************************** 显示车牌设置界面 ************************************/
static int sys_cid_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static int cid_timer = 0;
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        sys_menu_cid_show();
        if (cid_timer) {
            printf("CID_TIMER repeatedly add");
            break;
        }
        cid_timer = sys_timer_add(NULL, sys_menu_cid_toggle, 500);
        if (!cid_timer) {
            UI_ONTOUCH_DEBUG("CID_TIMER add failed\n");
        }
        break;
    case ON_CHANGE_RELEASE:
        if (cid_timer) {
            sys_timer_del(cid_timer);
            cid_timer = 0;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_LAY_CID)
.onchange = sys_cid_onchange,
};

/***************************** 车牌切换 ************************************/
static int highlight_no_cid_item(int p)
{
    if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
        ui_no_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
    } else {
        ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
    }
    if (sys_menu_cid_setting == SYS_MENU_CID_1) {
        __this->cid_set_pos = 10;
    } else {
        __this->cid_set_pos = 0;
    }
    if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
        ui_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
    } else {
        ui_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
    }
    return 0;
}
static int sys_cid_change(void *ctr, struct element_touch_event *e)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    UI_ONTOUCH_DEBUG("**sys cid change ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        switch (pic->elm.id) {
        case SYS_PIC_CID_0:
            sys_menu_cid_selete(SYS_MENU_CID_PRO);
            break;
        case SYS_PIC_CID_1:
            sys_menu_cid_selete(SYS_MENU_CID_1);
            break;
        case SYS_PIC_CID_2:
            sys_menu_cid_selete(SYS_MENU_CID_2);
            break;
        case SYS_PIC_CID_3:
            sys_menu_cid_selete(SYS_MENU_CID_3);
            break;
        case SYS_PIC_CID_4:
            sys_menu_cid_selete(SYS_MENU_CID_4);
            break;
        case SYS_PIC_CID_5:
            sys_menu_cid_selete(SYS_MENU_CID_5);
            break;
        case SYS_PIC_CID_6:
            sys_menu_cid_selete(SYS_MENU_CID_6);
            break;
        }
        sys_menu_cid_toggle(NULL);
        if (__this->onkey_mod == 1) {
            ui_set_call(highlight_no_cid_item, 0);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_0)
.ontouch = sys_cid_change,
};
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_1)
.ontouch = sys_cid_change,
};
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_2)
.ontouch = sys_cid_change,
};
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_3)
.ontouch = sys_cid_change,
};
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_4)
.ontouch = sys_cid_change,
};
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_5)
.ontouch = sys_cid_change,
};
REGISTER_UI_EVENT_HANDLER(SYS_PIC_CID_6)
.ontouch = sys_cid_change,
};

static int sys_cid_prev(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys cid change ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sys_menu_cid_selete(SYS_MENU_CID_PREV);
        sys_menu_cid_toggle(NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PREV)
.ontouch = sys_cid_prev,
};

static int sys_cid_next(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys cid change ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sys_menu_cid_selete(SYS_MENU_CID_NEXT);
        sys_menu_cid_toggle(NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_NEXT)
.ontouch = sys_cid_next,
};

static int sys_cid_set_pro(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    const int btn_id[31] = {
        SYS_BTN_CID_PRO_01,
        SYS_BTN_CID_PRO_02,
        SYS_BTN_CID_PRO_03,
        SYS_BTN_CID_PRO_04,
        SYS_BTN_CID_PRO_05,
        SYS_BTN_CID_PRO_06,
        SYS_BTN_CID_PRO_07,
        SYS_BTN_CID_PRO_08,
        SYS_BTN_CID_PRO_09,
        SYS_BTN_CID_PRO_10,
        SYS_BTN_CID_PRO_11,
        SYS_BTN_CID_PRO_12,
        SYS_BTN_CID_PRO_13,
        SYS_BTN_CID_PRO_14,
        SYS_BTN_CID_PRO_15,
        SYS_BTN_CID_PRO_16,
        SYS_BTN_CID_PRO_17,
        SYS_BTN_CID_PRO_18,
        SYS_BTN_CID_PRO_19,
        SYS_BTN_CID_PRO_20,
        SYS_BTN_CID_PRO_21,
        SYS_BTN_CID_PRO_22,
        SYS_BTN_CID_PRO_23,
        SYS_BTN_CID_PRO_24,
        SYS_BTN_CID_PRO_25,
        SYS_BTN_CID_PRO_26,
        SYS_BTN_CID_PRO_27,
        SYS_BTN_CID_PRO_28,
        SYS_BTN_CID_PRO_29,
        SYS_BTN_CID_PRO_30,
        SYS_BTN_CID_PRO_31
    };
    u8 i;
    UI_ONTOUCH_DEBUG("**sys cid set pro ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        for (i = 0; i < 31; i++) {
            if (btn->elm.id == btn_id[i]) {
                if (sys_menu_cid_status[0] != i) {
                    sys_menu_cid_status[0] = i;
                    sys_menu_cid_toggle(NULL);
                }
                break;
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_01)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_02)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_03)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_04)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_05)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_06)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_07)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_08)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_09)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_10)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_11)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_12)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_13)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_14)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_15)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_16)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_17)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_18)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_19)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_20)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_21)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_22)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_23)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_24)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_25)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_26)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_27)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_28)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_29)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_30)
.ontouch = sys_cid_set_pro,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_PRO_31)
.ontouch = sys_cid_set_pro,
};

static int sys_cid_set(void *ctr, struct element_touch_event *e)
{
    struct button *btn = (struct button *)ctr;
    const int btn_id[36] = {
        SYS_BTN_CID_KB_A,
        SYS_BTN_CID_KB_B,
        SYS_BTN_CID_KB_C,
        SYS_BTN_CID_KB_D,
        SYS_BTN_CID_KB_E,
        SYS_BTN_CID_KB_F,
        SYS_BTN_CID_KB_G,
        SYS_BTN_CID_KB_H,
        SYS_BTN_CID_KB_I,
        SYS_BTN_CID_KB_J,
        SYS_BTN_CID_KB_K,
        SYS_BTN_CID_KB_L,
        SYS_BTN_CID_KB_M,
        SYS_BTN_CID_KB_N,
        SYS_BTN_CID_KB_O,
        SYS_BTN_CID_KB_P,
        SYS_BTN_CID_KB_Q,
        SYS_BTN_CID_KB_R,
        SYS_BTN_CID_KB_S,
        SYS_BTN_CID_KB_T,
        SYS_BTN_CID_KB_U,
        SYS_BTN_CID_KB_V,
        SYS_BTN_CID_KB_W,
        SYS_BTN_CID_KB_X,
        SYS_BTN_CID_KB_Y,
        SYS_BTN_CID_KB_Z,
        SYS_BTN_CID_KB_0,
        SYS_BTN_CID_KB_1,
        SYS_BTN_CID_KB_2,
        SYS_BTN_CID_KB_3,
        SYS_BTN_CID_KB_4,
        SYS_BTN_CID_KB_5,
        SYS_BTN_CID_KB_6,
        SYS_BTN_CID_KB_7,
        SYS_BTN_CID_KB_8,
        SYS_BTN_CID_KB_9
    };
    u8 i;
    UI_ONTOUCH_DEBUG("**sys cid set pro ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        for (i = 0; i < 36; i++) {
            if (btn->elm.id == btn_id[i]) {
                if (sys_menu_cid_setting == SYS_MENU_CID_1 && i >= 26) {
                    break;
                }
                if (sys_menu_cid_status[sys_menu_cid_setting - 1] != i) {
                    sys_menu_cid_status[sys_menu_cid_setting - 1] = i;
                    sys_menu_cid_toggle(NULL);
                }
                break;
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_A)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_B)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_C)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_D)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_E)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_F)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_G)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_H)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_I)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_J)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_K)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_L)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_M)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_N)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_O)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_P)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_Q)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_R)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_S)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_T)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_U)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_V)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_W)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_X)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_Y)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_Z)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_1)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_2)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_3)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_4)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_5)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_6)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_7)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_8)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_9)
.ontouch = sys_cid_set,
};
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_KB_0)
.ontouch = sys_cid_set,
};

/***************************** 车牌确认修改 ************************************/
static int sys_cid_confirm(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys cid confirm ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->onkey_mod == 1) {
            if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
                ui_no_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
            } else if (confirm_f) {
                confirm_f = 0;
                ui_no_highlight_element_by_id(SYS_BTN_CID_CONFIRM);
            } else {
                ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
            }
        }
        __this->cid_set_pos = 0;
        __this->onkey_mod = 0;
        sys_menu_cid_save();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_CID_CONFIRM)
.ontouch = sys_cid_confirm,
};
static s8 cid_set_pos_remap(u8 pos)
{
    if (pos < 9) {
        return 27 + pos;
    }
    switch (pos) {
    case 9:
        return 26;
        break;
    case 10:
        return 16;
        break;
    case 11:
        return 22;
        break;
    case 12:
        return 4;
        break;
    case 13:
        return 17;
        break;
    case 14:
        return 19;
        break;
    case 15:
        return 24;
        break;
    case 16:
        return 20;
        break;
    case 17:
        return 8;
        break;
    case 18:
        return 14;
        break;
    case 19:
        return 15;
        break;
    case 20:
        return 0;
        break;
    case 21:
        return 18;
        break;
    case 22:
        return 3;
        break;
    case 23:
        return 5;
        break;
    case 24:
        return 6;
        break;
    case 25:
        return 7;
        break;
    case 26:
        return 9;
        break;
    case 27:
        return 10;
        break;
    case 28:
        return 11;
        break;
    case 29:
        return 25;
        break;
    case 30:
        return 23;
        break;
    case 31:
        return 2;
        break;
    case 32:
        return 21;
        break;
    case 33:
        return 1;
        break;
    case 34:
        return 13;
        break;
    case 35:
        return 12;
        break;
    default:
        break;
    }
    return -1;
}

static int highlight_cid_lay_item(int p)
{
    ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
    if (sys_menu_cid_setting == SYS_MENU_CID_1) {
        __this->cid_set_pos = 10;
    } else {
        __this->cid_set_pos = 0;
    }
    ui_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
    return 0;

}
static void set_cid_onkey(u8 key_value)
{
    s8 remap_cid_selnum;
    u8 sel_item_num;
    u8 cur_cid_setting = 0;
    if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
        sel_item_num = ARRAY_SIZE(cid_btn_id0);
        cur_cid_setting = SYS_MENU_CID_PRO;
        confirm_f = 0;
    } else {
        sel_item_num = ARRAY_SIZE(cid_btn_id1);
        cur_cid_setting = sys_menu_cid_setting;
    }
    switch (key_value) {
    case KEY_DOWN:
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_no_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
        } else {
            ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
        }
        __this->cid_set_pos++;
        if (__this->cid_set_pos >= sel_item_num) {
            __this->cid_set_pos = 0;
        }
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
        } else {
            ui_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
        }
        break;
    case KEY_UP:
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_no_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
        } else {
            ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
        }
        if (__this->cid_set_pos == 0) {
            __this->cid_set_pos = sel_item_num;
        }
        __this->cid_set_pos--;
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
        } else {
            ui_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
        }
        break;
    case KEY_OK:
        if (confirm_f) {
            sys_menu_cid_save();
            ui_no_highlight_element_by_id(SYS_BTN_CID_CONFIRM);
            confirm_f = 0;
            __this->onkey_mod = 0;
            sys_menu_cid_selete(SYS_MENU_CID_PRO);
            sys_menu_cid_toggle(NULL);
            break;
        }
        if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            if (sys_menu_cid_status[sys_menu_cid_setting - 1] != __this->cid_set_pos) {
                sys_menu_cid_status[sys_menu_cid_setting - 1] = __this->cid_set_pos;
                sys_menu_cid_toggle(NULL);
            }
        } else if (sys_menu_cid_setting == SYS_MENU_CID_1) {
            if (__this->cid_set_pos < 10) {
                break;
            }
            remap_cid_selnum = cid_set_pos_remap(__this->cid_set_pos);
            if (remap_cid_selnum < 0) {
                break;
            }
            if (sys_menu_cid_status[sys_menu_cid_setting - 1] != remap_cid_selnum) {
                sys_menu_cid_status[sys_menu_cid_setting - 1] = remap_cid_selnum;
                sys_menu_cid_toggle(NULL);
            }
        } else {
            remap_cid_selnum = cid_set_pos_remap(__this->cid_set_pos);
            if (remap_cid_selnum < 0) {
                break;
            }
            if (sys_menu_cid_status[sys_menu_cid_setting - 1] != remap_cid_selnum) {
                sys_menu_cid_status[sys_menu_cid_setting - 1] = remap_cid_selnum;
                sys_menu_cid_toggle(NULL);
            }
        }
        cur_cid_setting++;
        if (cur_cid_setting > SYS_MENU_CID_6) {
            ui_no_highlight_element_by_id(cid_btn_id1[__this->cid_set_pos]);
            ui_highlight_element_by_id(SYS_BTN_CID_CONFIRM);
            confirm_f = 1;
            break;
        }
        sys_menu_cid_selete(cur_cid_setting);
        sys_menu_cid_toggle(NULL);
        ui_set_call(highlight_cid_lay_item, 0);
        break;
    case KEY_MODE:
        __this->onkey_mod = 0;
        if (confirm_f) {
            ui_no_highlight_element_by_id(SYS_BTN_CID_CONFIRM);
            confirm_f = 0;
        } else if (sys_menu_cid_setting == SYS_MENU_CID_PRO) {
            ui_no_highlight_element_by_id(cid_btn_id0[__this->cid_set_pos]);
        }
        sys_menu_cid_selete(SYS_MENU_CID_PRO);
        sys_menu_cid_toggle(NULL);
        break;
    }
}
/***************************** 选择设置菜单 ************************************/
static int sys_set2_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, -1);
    case ON_CHANGE_FIRST_SHOW:
        return true;
        break;
    default:
        return false;
    }
    return false;
}
static int highlight_sel_item(int p)
{
    switch (__this->sys_menu_status) {
    case SYS_MENU_LCDOFF:
    case SYS_MENU_AUTOOFF:
        ui_highlight_element_by_id(sel_layout4[p]);
        break;
    case SYS_MENU_FREQUENCY:
    case SYS_MENU_KEYVOICE:
    //case SYS_MENU_TVMODE:
    case SYS_MENU_LANEDET:
    case SYS_MENU_HL_WARNNING:
        ui_highlight_element_by_id(sel_layout2[p]);
        break;
    case SYS_MENU_LANGUAGE:
        ui_highlight_element_by_id(sel_layout3[p]);
        break;
    default:
        break;
    }
    return 0;
}

static void open_set_lane_page(void *p, int err)
{
    struct intent it;
    lane_setting = 1;
    puts("\n =============car lane set =========\n");
    init_intent(&it);
    it.name = "video_rec";
    /* it.action = ACTION_VIDEO_REC_MAIN; */
    it.action = ACTION_VIDEO_REC_MAIN;
    it.data   = "lan_setting";
    start_app(&it);
}
static void close_set_lane_page(void *p, int err)
{
    struct intent it;
    int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
    int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
    puts("\n =============car lane set exit=========\n");
    init_intent(&it);
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    menu_lane_det_set(top << 16 | bottom);
    printf("lane set top=%d , bottom=%d\n", top, bottom);
}

static int sys_set2_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys set2 ontouch**");
    int sel_item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sel_item = ui_grid_cur_item(grid);
        if (sel_item < 0 || sel_item > 1) {
            break;
        }
        switch (__this->sys_menu_status) {
        case SYS_MENU_FREQUENCY:
            if (__this->onkey_mod == 0) {
                if (sel_item == __this->led_fre_hz) {
                    break;
                }
            }
            ui_hide(sel_pic2[__this->led_fre_hz]);
            if (__this->onkey_mod  == 1) {
                ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
            }
            menu_sys_led_fre_set(sel_item);
            ui_show(sel_pic2[sel_item]);
            if (__this->onkey_mod  == 1) {
                ui_set_call(highlight_sel_item, sel_item);
            }
            cur_sel_item = __this->led_fre_hz;
            break;
        case SYS_MENU_KEYVOICE:
            if (__this->onkey_mod == 0) {
                if (sel_item == __this->key_voice) {
                    break;
                }
            }
            ui_hide(sel_pic2[__this->key_voice]);
            if (__this->onkey_mod  == 1) {
                ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
            }
            menu_sys_key_voice_set(sel_item);
            if (__this->onkey_mod  == 1) {
                ui_set_call(highlight_sel_item, sel_item);
            }
            ui_show(sel_pic2[sel_item]);
            cur_sel_item = __this->key_voice;
            break;
        #if 0    
        case SYS_MENU_TVMODE:
            if (__this->onkey_mod == 0) {
                if (sel_item == __this->tv_mod) {
                    break;
                }
            }
            ui_hide(sel_pic2[__this->tv_mod]);
            if (__this->onkey_mod  == 1) {
                ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
            }
            menu_sys_tv_mod_set(sel_item);
            if (__this->onkey_mod  == 1) {
                ui_set_call(highlight_sel_item, sel_item);
            }
            ui_show(sel_pic2[sel_item]);
            cur_sel_item = __this->tv_mod;
            break;
        #endif    
        case SYS_MENU_LANEDET:
            if (sel_item == 1) {
                struct intent it;
                __this->lane_det = 1;
                ui_hide(ui_get_current_window_id());
                init_intent(&it);
                it.name = "video_system";
                it.action = ACTION_BACK;
                start_app_async(&it, open_set_lane_page, NULL);
                break;
            } else {
                ui_hide(sel_pic2[__this->lane_det]);
                if (__this->onkey_mod  == 1) {
                    ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
                }
                menu_lane_det_set(0);
                if (__this->onkey_mod  == 1) {
                    ui_set_call(highlight_sel_item, sel_item);
                }
                ui_show(sel_pic2[sel_item]);
                cur_sel_item = __this->lane_det;
            }
            break;
        case SYS_MENU_HL_WARNNING:
            if (__this->onkey_mod == 0) {
                if (sel_item == __this->hl_warnning) {
                    break;
                }
            }
            ui_hide(sel_pic2[__this->hl_warnning]);
            if (__this->onkey_mod  == 1) {
                ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
            }
            menu_sys_hlight_warning_set(sel_item);
            if (__this->onkey_mod  == 1) {
                ui_set_call(highlight_sel_item, sel_item);
            }
            ui_show(sel_pic2[sel_item]);
            cur_sel_item = __this->hl_warnning;
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_LIST_C2)
.onchange = sys_set2_onchange,
 .ontouch = sys_set2_ontouch,
};

static int sys_set3_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, -1);
        break;
    case ON_CHANGE_FIRST_SHOW:
        return true;
        break;
    default:
        return false;
    }
    return false;
}
static int sys_set3_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys set ontouch**");
    int sel_item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sel_item = ui_grid_cur_item(grid);
        if (sel_item < 0 || sel_item > 2) {
            break;
        }
        switch (__this->sys_menu_status) {
        case SYS_MENU_LANGUAGE:
            if (__this->language != sel_item) {
                menu_sys_language_set(sel_item);
                //整体刷新提升刷新速度
                ui_hide(SYS_LAY);
                ui_show(SYS_LAY);
            }
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_LIST_C3)
.onchange = sys_set3_onchange,
 .ontouch = sys_set3_ontouch,
};

static int sys_set4_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, -1);
    case ON_CHANGE_FIRST_SHOW:
        return true;
        break;
    default:
        return false;
    }
    return false;
}
static int sys_set4_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys set ontouch**");
    int sel_item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        sel_item = ui_grid_cur_item(grid);
        if (sel_item < 0 || sel_item > 3) {
            break;
        }
        switch (__this->sys_menu_status) {
        case SYS_MENU_LCDOFF:
            if (__this->onkey_mod == 0) {
                if (sel_item == __this->lcd_protect) {
                    break;
                }
            }
            ui_hide(sel_pic4[__this->lcd_protect]);
            if (__this->onkey_mod  == 1) {
                ui_no_highlight_element_by_id(sel_layout4[cur_sel_item]);
            }
            menu_sys_lcd_pro_set(sel_item);
            if (__this->onkey_mod  == 1) {
                ui_set_call(highlight_sel_item, sel_item);
            }
            ui_show(sel_pic4[sel_item]);
            cur_sel_item = __this->lcd_protect;
            break;
        case SYS_MENU_AUTOOFF:
            if (__this->onkey_mod == 0) {
                if (sel_item == __this->auto_off) {
                    break;
                }
            }
            ui_hide(sel_pic4[__this->auto_off]);
            if (__this->onkey_mod  == 1) {
                ui_no_highlight_element_by_id(sel_layout4[cur_sel_item]);
            }
            menu_sys_auto_off_set(sel_item);
            if (__this->onkey_mod  == 1) {
                ui_set_call(highlight_sel_item, sel_item);
            }
            ui_show(sel_pic4[sel_item]);
            cur_sel_item = __this->auto_off;
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_LIST_C4)
.onchange = sys_set4_onchange,
 .ontouch = sys_set4_ontouch,
};

/***************************** 格式化、默认设置菜单 ************************************/
static int sys_warning_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static int warning_timer = 0;
    static int timer_countdown = 4;
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        timer_countdown = 4;
        if (warning_timer) {
            UI_ONTOUCH_DEBUG("WARNING_TIMER repeatedly add");
            break;
        }
        if (__this->sys_menu_status == SYS_MENU_RESET && __this->reset_flag) {
            break;
        }
        warning_timer = sys_timer_add(&timer_countdown, sys_warning_tick, 1000);
        if (!warning_timer) {
            UI_ONTOUCH_DEBUG("WARNING_TIMER add failed\n");
        }
        break;
    case ON_CHANGE_RELEASE:
        if (warning_timer) {
            sys_timer_del(warning_timer);
            UI_ONTOUCH_DEBUG("WARNING_TIMER del");
            warning_timer = 0;
            btn_warning_show_f = 0;
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_PIC_WARNING_CONFIRM)
.onchange = sys_warning_onchange,
};

static int sys_warning_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    int err;
    struct intent it;
    UI_ONTOUCH_DEBUG("**sys warning confirm ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        switch (__this->sys_menu_status) {
            ui_hide(SYS_BTN_WARNING_CONFIRM);
            btn_warning_show_f = 0;
            ui_hide(SYS_TXT_WARNING_CONFIRM);
        case SYS_MENU_FORMAT:
            if (storage_device_ready()) {
                menu_sys_format_set(1);
                ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 3);
            }
            break;
        case SYS_MENU_RESET:
            menu_sys_default_set(1);
            sys_fun_restore();
            __this->reset_flag = 1;
            ui_hide(SYS_LAY);
            ui_show(SYS_LAY);
            __this->lcd_protect = index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect));
            __this->auto_off    = index_of_table8(db_select("aff"), TABLE(table_system_auto_close));
            __this->led_fre_hz  = index_of_table8(db_select("fre"), TABLE(table_system_led_fre));
            __this->key_voice   = db_select("kvo");
            __this->language    = index_of_table8(db_select("lag"), TABLE(table_system_language));
            __this->tv_mod      = index_of_table8(db_select("tvm"), TABLE(table_system_tv_mod));
            __this->vanish_line    = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
            __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
            __this->lane_det    = !!db_select("lan");
            __this->hl_warnning = db_select("hlw");
            ui_text_show_index_by_id(SYS_TXT_WARNING_BODY, 7);

            break;
        }
        if (__this->onkey_mod == 1) {
            ui_no_highlight_element_by_id(SYS_BTN_WARNING_CONFIRM);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(SYS_BTN_WARNING_CONFIRM)
.ontouch = sys_warning_confirm_ontouch,
};



static int sys_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    static u8 child_item_sel = 0;
    static u8 child_item_num = 0;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }

    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    if (__this->onkey_mod == 0) {
        switch (e->value) {
        case KEY_DOWN:
            __this->onkey_sel++;
            if (__this->onkey_sel > SYS_SETTING_ITEM - 1) {
                __this->onkey_sel = 0;
            }
            ui_set_call(sys_menu, __this->onkey_sel);
            break;
        case KEY_UP:
            if (__this->onkey_sel == 0) {
                __this->onkey_sel = SYS_SETTING_ITEM;
            }
            __this->onkey_sel--;
            ui_set_call(sys_menu, __this->onkey_sel);
            break;
        case KEY_OK:
            if (__this->sys_menu_status == SYS_MENU_VERSION) {
                break;
            } else if (__this->sys_menu_status == SYS_MENU_FORMAT || __this->sys_menu_status == SYS_MENU_RESET) {
                if (btn_warning_show_f == 0) {
                    break;
                }
                ui_highlight_element_by_id(SYS_BTN_WARNING_CONFIRM);
            } else if (__this->sys_menu_status == SYS_MENU_CARNUM) {
                ui_set_call(highlight_no_cid_item, 0);
            } else if (__this->sys_menu_status == SYS_MENU_DATETIME) {
                ui_show(sel_timePic[1]);
            }
            __this->onkey_mod = 1;
            child_item_sel = 0;
            __this->time_set_pos = SYS_MENU_TIME_YEAR;
            child_item_num = get_child_item_num(__this->sys_menu_status);
            switch (__this->sys_menu_status) {
            case SYS_MENU_LCDOFF:
                ui_highlight_element_by_id(sel_layout4[__this->lcd_protect]);
                cur_sel_item = __this->lcd_protect;
                break;
            case SYS_MENU_AUTOOFF:
                ui_highlight_element_by_id(sel_layout4[__this->auto_off]);
                cur_sel_item = __this->auto_off;
                break;
            case SYS_MENU_FREQUENCY:
                ui_highlight_element_by_id(sel_layout2[__this->led_fre_hz]);
                cur_sel_item = __this->led_fre_hz;
                break;
            case SYS_MENU_KEYVOICE:
                ui_highlight_element_by_id(sel_layout2[__this->key_voice]);
                cur_sel_item = __this->key_voice;
                break;
            case SYS_MENU_HL_WARNNING:
                printf("hl_warnning====%d\n", __this->hl_warnning);
                ui_highlight_element_by_id(sel_layout2[__this->hl_warnning]);
                cur_sel_item = __this->hl_warnning;
                break;
            case SYS_MENU_LANEDET:
                ui_highlight_element_by_id(sel_layout2[__this->lane_det]);
                cur_sel_item = __this->lane_det;
                break;
            case SYS_MENU_LANGUAGE:
                ui_highlight_element_by_id(sel_layout3[__this->language]);
                cur_sel_item = __this->language;
                break;
            #if 0    
            case SYS_MENU_TVMODE:
                ui_highlight_element_by_id(sel_layout2[__this->tv_mod]);
                cur_sel_item = __this->tv_mod;
                break;
            #endif    
            }
            break;
        case KEY_MODE:
            __this->onkey_mod = 2;
            mode_sel = 0;
            ui_highlight_element_by_id(SYS_BTN_REC);
            break;
        default:
            break;
        }
    } else if (__this->onkey_mod == 1) {
        switch (e->value) {
        case KEY_DOWN:
            switch (__this->sys_menu_status) {
            case SYS_MENU_LCDOFF:
            case SYS_MENU_AUTOOFF:
                child_item_sel = cur_sel_item;
                ui_no_highlight_element_by_id(sel_layout4[child_item_sel]);
                child_item_sel++;
                if (child_item_sel >= child_item_num) {
                    child_item_sel = 0;
                }
                ui_highlight_element_by_id(sel_layout4[child_item_sel]);
                cur_sel_item = child_item_sel;
                break;
            case SYS_MENU_FREQUENCY:
            case SYS_MENU_KEYVOICE:
            //case SYS_MENU_TVMODE:
            case SYS_MENU_HL_WARNNING:
            case SYS_MENU_LANEDET:
                child_item_sel = cur_sel_item;
                ui_no_highlight_element_by_id(sel_layout2[child_item_sel]);
                child_item_sel++;
                if (child_item_sel >= child_item_num) {
                    child_item_sel = 0;
                }
                ui_highlight_element_by_id(sel_layout2[child_item_sel]);
                cur_sel_item = child_item_sel;
                break;
            case SYS_MENU_LANGUAGE:
                child_item_sel = cur_sel_item;
                ui_no_highlight_element_by_id(sel_layout3[child_item_sel]);
                child_item_sel++;
                if (child_item_sel >= child_item_num) {
                    child_item_sel = 0;
                }
                ui_highlight_element_by_id(sel_layout3[child_item_sel]);
                cur_sel_item = child_item_sel;
                break;
            case SYS_MENU_DATETIME:
                sys_menu_tim_change(__this->time_set_pos, 1);
                sys_menu_tim_change(SYS_MENU_TIME_SET, 0);
                break;
            case SYS_MENU_CARNUM:
                set_cid_onkey(KEY_DOWN);
                break;
            }
            break;
        case KEY_UP:
            switch (__this->sys_menu_status) {
            case SYS_MENU_LCDOFF:
            case SYS_MENU_AUTOOFF:
                child_item_sel = cur_sel_item;
                ui_no_highlight_element_by_id(sel_layout4[child_item_sel]);
                if (child_item_sel == 0) {
                    child_item_sel = child_item_num;
                }
                child_item_sel--;
                ui_highlight_element_by_id(sel_layout4[child_item_sel]);
                cur_sel_item = child_item_sel;
                break;
            case SYS_MENU_FREQUENCY:
            case SYS_MENU_KEYVOICE:
            //case SYS_MENU_TVMODE:
            case SYS_MENU_HL_WARNNING:
            case SYS_MENU_LANEDET:
                child_item_sel = cur_sel_item;
                ui_no_highlight_element_by_id(sel_layout2[child_item_sel]);
                if (child_item_sel == 0) {
                    child_item_sel = child_item_num;
                }
                child_item_sel--;
                ui_highlight_element_by_id(sel_layout2[child_item_sel]);
                cur_sel_item = child_item_sel;
                break;
            case SYS_MENU_LANGUAGE:
                child_item_sel = cur_sel_item;
                ui_no_highlight_element_by_id(sel_layout3[child_item_sel]);
                if (child_item_sel == 0) {
                    child_item_sel = child_item_num;
                }
                child_item_sel--;
                ui_highlight_element_by_id(sel_layout3[child_item_sel]);
                cur_sel_item = child_item_sel;
                break;
            case SYS_MENU_DATETIME:
                sys_menu_tim_change(__this->time_set_pos, -1);
                sys_menu_tim_change(SYS_MENU_TIME_SET, 0);
                break;
            case SYS_MENU_CARNUM:
                set_cid_onkey(KEY_UP);
                break;
            }
            break;
        case KEY_OK:
            if (__this->sys_menu_status == SYS_MENU_DATETIME) {
                ui_hide(sel_timePic[__this->time_set_pos]);
                __this->time_set_pos++;
                if (__this->time_set_pos == SYS_MENU_TIME_SET) {
                    ui_highlight_element_by_id(SYS_BTN_TIM_CONFIRM);
                }
                if (__this->time_set_pos > SYS_MENU_TIME_SET) {
                    ui_no_highlight_element_by_id(SYS_BTN_TIM_CONFIRM);
                    void *fd = dev_open("rtc", NULL);
                    if (!fd) {
                        return -1;
                    }
                    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&t);
                    dev_close(fd);
                } else {
                    ui_show(sel_timePic[__this->time_set_pos]);
                    break;
                }
            } else if (__this->sys_menu_status == SYS_MENU_FORMAT || __this->sys_menu_status == SYS_MENU_RESET) {
                ui_no_highlight_element_by_id(SYS_BTN_WARNING_CONFIRM);
                if (child_item_sel == 0) {
                    set_format_reset_func();
                }
                __this->onkey_mod = 0;
            } else if (__this->sys_menu_status == SYS_MENU_CARNUM) {
                set_cid_onkey(KEY_OK);
                break;
            } else {
                switch (__this->sys_menu_status) {
                case SYS_MENU_LCDOFF:
                    if (__this->lcd_protect != cur_sel_item) {
                        ui_hide(sel_pic4[__this->lcd_protect]);
                        menu_sys_lcd_pro_set(cur_sel_item);
                        ui_show(sel_pic4[cur_sel_item]);
                    }
                    break;
                case SYS_MENU_AUTOOFF:
                    if (__this->auto_off != cur_sel_item) {
                        ui_hide(sel_pic4[__this->auto_off]);
                        menu_sys_auto_off_set(cur_sel_item);
                        ui_show(sel_pic4[cur_sel_item]);
                    }
                    break;
                case SYS_MENU_FREQUENCY:
                    if (__this->led_fre_hz != cur_sel_item) {
                        ui_hide(sel_pic2[__this->led_fre_hz]);
                        menu_sys_led_fre_set(cur_sel_item);
                        ui_show(sel_pic2[cur_sel_item]);
                    }
                    break;
                case SYS_MENU_KEYVOICE:
                    if (__this->key_voice != cur_sel_item) {
                        ui_hide(sel_pic2[__this->key_voice]);
                        menu_sys_key_voice_set(cur_sel_item);
                        ui_show(sel_pic2[cur_sel_item]);
                    }
                    break;
                case SYS_MENU_HL_WARNNING:
                    if (__this->hl_warnning != cur_sel_item) {
                        ui_hide(sel_pic2[__this->hl_warnning]);
                        menu_sys_hlight_warning_set(cur_sel_item);
                        ui_show(sel_pic2[cur_sel_item]);
                    }
                    break;
                case SYS_MENU_LANEDET:
                    if (cur_sel_item == 0 && __this->lane_det != cur_sel_item) {
                        ui_hide(sel_pic2[__this->lane_det]);
                        menu_lane_det_set(0);
                        ui_show(sel_pic2[cur_sel_item]);
                    } else if (cur_sel_item == 1) {
                        //打开车道设置界面
                        __this->lane_det = 1;
                        ui_hide(ui_get_current_window_id());
                        init_intent(&it);
                        it.name = "video_system";
                        it.action = ACTION_BACK;
                        start_app_async(&it, open_set_lane_page, NULL);
                    }
                    break;
                #if 0    
                case SYS_MENU_TVMODE:
                    if (__this->tv_mod != cur_sel_item) {
                        ui_hide(sel_pic2[__this->tv_mod]);
                        menu_sys_tv_mod_set(cur_sel_item);
                        ui_show(sel_pic2[cur_sel_item]);
                    }
                    break;
                #endif    
                case SYS_MENU_LANGUAGE:
                    if (__this->language != cur_sel_item) {
                        menu_sys_language_set(cur_sel_item);
                        ui_hide(SYS_LAY);
                        ui_show(SYS_LAY);
                    }
                    break;
                }
            }
            break;
        case KEY_MODE:
            if (__this->sys_menu_status == SYS_MENU_FORMAT || __this->sys_menu_status == SYS_MENU_RESET) {
                ui_no_highlight_element_by_id(SYS_BTN_WARNING_CONFIRM);
            }
            if (__this->sys_menu_status == SYS_MENU_DATETIME) {
                if (__this->time_set_pos > SYS_MENU_TIME_YEAR && __this->time_set_pos < SYS_MENU_TIME_SET) {
                    ui_hide(sel_timePic[__this->time_set_pos]);
                } else if (__this->time_set_pos == SYS_MENU_TIME_SET) {
                    ui_no_highlight_element_by_id(SYS_BTN_TIM_CONFIRM);
                }
            }
            if (__this->sys_menu_status == SYS_MENU_CARNUM) {
                set_cid_onkey(KEY_MODE);
                break;
            }
            __this->onkey_mod = 0;
            switch (__this->sys_menu_status) {
            case SYS_MENU_LCDOFF:
            case SYS_MENU_AUTOOFF:
                ui_no_highlight_element_by_id(sel_layout4[cur_sel_item]);
                break;
            case SYS_MENU_FREQUENCY:
            case SYS_MENU_KEYVOICE:
            //case SYS_MENU_TVMODE:
            case SYS_MENU_LANEDET:
            case SYS_MENU_HL_WARNNING:
                ui_no_highlight_element_by_id(sel_layout2[cur_sel_item]);
                break;
            case SYS_MENU_LANGUAGE:
                ui_no_highlight_element_by_id(sel_layout3[cur_sel_item]);
                break;
            }
            break;
        }

    } else if (__this->onkey_mod == 2) {
        switch (e->value) {
        case KEY_OK:
            init_intent(&it);
            if (mode_sel == ARRAY_SIZE(onkey_sel_item) - 1) {
                __this->page_exit = HOME_SW_EXIT;
            } else {
                __this->page_exit = MODE_SW_EXIT;
            }
            it.name = "video_system";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
            if (mode_sel == 0) {
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app_async(&it, NULL, NULL); //不等待直接启动app
            } else if (mode_sel == 1) {
                it.name = "video_dec";
                it.action = ACTION_VIDEO_DEC_MAIN;
                start_app_async(&it, NULL, NULL); //不等待直接启动app
            }
            break;
        case KEY_DOWN:
            ui_no_highlight_element_by_id(onkey_sel_item[mode_sel]);
            mode_sel++;
            if (mode_sel > ARRAY_SIZE(onkey_sel_item) - 1) {
                mode_sel = 0;
            }
            ui_highlight_element_by_id(onkey_sel_item[mode_sel]);
            break;
        case KEY_UP:
            ui_no_highlight_element_by_id(onkey_sel_item[mode_sel]);
            if (mode_sel == 0) {
                mode_sel = ARRAY_SIZE(onkey_sel_item);
            }
            mode_sel--;
            ui_highlight_element_by_id(onkey_sel_item[mode_sel]);
            break;
        case KEY_MODE:
            ui_no_highlight_element_by_id(onkey_sel_item[mode_sel]);
            __this->onkey_mod = 0;
            break;

        }
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(SYS_WIN)
.onkey = sys_onkey,
};



static int lane_set_txt_top_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_INIT:
        __this->vanish_line = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        if (__this->vanish_line < 20) {
            __this->vanish_line = 20;
        }
        if (__this->car_head_line > LCD_DEV_HIGHT - 20 || __this->car_head_line < __this->vanish_line + 30) {
            __this->car_head_line = LCD_DEV_HIGHT - 20 ;
        }

        break;
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->vanish_line;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(LANE_TXT_TOP)
.onchange = lane_set_txt_top_onchange,
};
static int lane_set_txt_bottom_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct draw_context *dc = (struct draw_context *)arg;

    switch (e) {
    case ON_CHANGE_INIT:
        __this->vanish_line = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        if (__this->vanish_line < 20) {
            __this->vanish_line = 20;
        }
        if (__this->car_head_line > LCD_DEV_HIGHT - 20 || __this->car_head_line < __this->vanish_line + 30) {
            __this->car_head_line = LCD_DEV_HIGHT - 20;
        }

        break;
    case ON_CHANGE_SHOW:
        dc->rect.top = __this->car_head_line - 22;
        dc->draw.top = dc->rect.top;
        break;
    default:
        return false;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(LANE_TXT_BOTTOM)
.onchange = lane_set_txt_bottom_onchange,
};


static char lane_set_arg[128];
static u8 lane_move = 0;
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
static int lane_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        __this->vanish_line   = ((db_select("lan") >> 16) & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->car_head_line = (db_select("lan") & 0x0000ffff) * LCD_DEV_HIGHT / 352;
        __this->onkey_mod = 3;
        __this->onkey_sel = 0;
        sys_key_event_takeover(true, false);
        break;
    case ON_CHANGE_RELEASE:
       
        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE); 
            ///video0_disp_start(1280, 480, 0, 0);
        }
        break;    
    }
    return false;
}
static int lane_set_ontouch(void *ctr, struct element_touch_event *e)
{
    int y = e->pos.y;
    int tmp;
    UI_ONTOUCH_DEBUG("**lane ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        if (y > __this->vanish_line - 30 &&
            y < __this->vanish_line + 30) {
            lane_move = 1;
            __this->onkey_sel = 0;
            UI_ONTOUCH_DEBUG(" MOVE TOP\n");
            send_lane_det_setting_msg("lane_set_close", 0);
        } else if (y > __this->car_head_line - 30 &&
                   y < __this->car_head_line + 30) {
            lane_move = 2;
            __this->onkey_sel = 1;
            UI_ONTOUCH_DEBUG(" MOVE BOTTOM\n");
            send_lane_det_setting_msg("lane_set_close", 0);
        }
        return true;
        break;
    case ELM_EVENT_TOUCH_MOVE:
    case ELM_EVENT_TOUCH_HOLD:
        if (lane_move == 1) {
            tmp = __this->vanish_line;
            if (y <= 20) {
                __this->vanish_line = 20;
            } else if (y > __this->car_head_line - 30) {
                __this->vanish_line = __this->car_head_line - 30;
            } else {
                __this->vanish_line = y;
            }
            if (tmp != __this->vanish_line) {
                UI_ONTOUCH_DEBUG("TOP : %3d\n", __this->vanish_line);
                ui_hide(LANE_TXT_TOP);
                ui_show(LANE_TXT_TOP);

                //vanish_y
            }
        } else if (lane_move == 2) {
            tmp = __this->car_head_line;
            if (y >= LCD_DEV_HIGHT - 20) {
                __this->car_head_line = LCD_DEV_HIGHT - 20;
            } else if (y < __this->vanish_line + 30) {
                __this->car_head_line = __this->vanish_line + 30;
            } else {
                __this->car_head_line = y;
            }
            if (tmp != __this->car_head_line) {
                UI_ONTOUCH_DEBUG("BOTTOM : %3d\n", __this->vanish_line);
                ui_hide(LANE_TXT_BOTTOM);
                ui_show(LANE_TXT_BOTTOM);
                //car_head_y
            }
        } else {
            UI_ONTOUCH_DEBUG("\n");
        }
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (lane_move) {
            lane_move = 0;
            int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
            int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
            printf("top=%d\n", top);
            printf("bottom=%d\n", bottom);
            send_lane_det_setting_msg("lane_set_open", top << 16 | bottom);
        }
        break;
    }
    return false;
}
static int lane_set_onkey(void *ctr, struct element_key_event *e)
{

    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }

    if (e->event == KEY_EVENT_CLICK) {
        if (__this->onkey_mod == 3) {
            switch (e->value) {
            case KEY_DOWN:
                if (__this->onkey_sel == 0) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->vanish_line > __this->car_head_line - 30) {
                            __this->vanish_line = __this->car_head_line - 30;
                        } else {
                            __this->vanish_line += 1;
                        }
                        ui_hide(LANE_TXT_TOP);
                        ui_show(LANE_TXT_TOP);
                    }
                } else if (__this->onkey_sel == 1) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->car_head_line >= LCD_DEV_HIGHT - 20) {
                            __this->car_head_line = LCD_DEV_HIGHT - 20;
                        } else {
                            __this->car_head_line += 1;
                        }
                        ui_hide(LANE_TXT_BOTTOM);
                        ui_show(LANE_TXT_BOTTOM);
                    }
                }
                break;
            case KEY_UP:
                if (__this->onkey_sel == 0) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->vanish_line <= 30) {
                            __this->vanish_line = 30;
                        } else {
                            __this->vanish_line -= 1;
                        }
                        ui_hide(LANE_TXT_TOP);
                        ui_show(LANE_TXT_TOP);
                    }
                } else if (__this->onkey_sel == 1) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->car_head_line < __this->vanish_line + 30) {
                            __this->car_head_line = __this->vanish_line + 30;
                        } else {
                            __this->car_head_line -= 1;
                        }
                        ui_hide(LANE_TXT_BOTTOM);
                        ui_show(LANE_TXT_BOTTOM);
                    }
                }
                break;
            case KEY_OK:
                if (__this->onkey_sel == 2) {
                    struct intent it;
                    ui_hide(ui_get_current_window_id());
                    init_intent(&it);
                    it.name = "video_rec";
                    it.action = ACTION_BACK;
                    start_app_async(&it, close_set_lane_page, NULL);
                } else {
                    if (lane_move == 3) {
                        int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
                        int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
                        printf("top=%d\n", top);
                        printf("bottom=%d\n", bottom);

                        send_lane_det_setting_msg("lane_set_open", top << 16 | bottom);
                        __this->onkey_sel = !__this->onkey_sel;
                        lane_move = 0;
                    }
                }
                break;
            case KEY_MODE:
                if (__this->onkey_sel == 2) {
                    ui_no_highlight_element_by_id(LANE_BTN_RETURN);
                    __this->onkey_sel = 0;
                    if (lane_move == 3) {
                        lane_move = 0;
                    }
                } else {
                    ui_highlight_element_by_id(LANE_BTN_RETURN);
                    __this->onkey_sel = 2;
                    if (lane_move == 3) {
                        int top = __this->vanish_line * 352 / LCD_DEV_HIGHT;
                        int bottom = __this->car_head_line * 352 / LCD_DEV_HIGHT;
                        printf("top=%d\n", top);
                        printf("bottom=%d\n", bottom);
                        send_lane_det_setting_msg("lane_set_open", top << 16 | bottom);
                        lane_move = 0;
                    }
                }
                break;
            }
        }
    } else if (e->event == KEY_EVENT_HOLD) {
        if (__this->onkey_mod == 3) {
            switch (e->value) {
            case KEY_DOWN:
                if (__this->onkey_sel == 0) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->vanish_line > __this->car_head_line - 20) {
                            __this->vanish_line = __this->car_head_line - 30;
                        } else {
                            __this->vanish_line += 10;
                        }
                        ui_hide(LANE_TXT_TOP);
                        ui_show(LANE_TXT_TOP);
                    }
                } else if (__this->onkey_sel == 1) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->car_head_line >= 450) {
                            __this->car_head_line = LCD_DEV_HIGHT - 20;
                        } else {
                            __this->car_head_line += 10;
                        }
                        ui_hide(LANE_TXT_BOTTOM);
                        ui_show(LANE_TXT_BOTTOM);
                    }
                }
                break;
            case KEY_UP:
                if (__this->onkey_sel == 0) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->vanish_line <= 30) {
                            __this->vanish_line = 20;
                        } else {
                            __this->vanish_line -= 10;
                        }
                        ui_hide(LANE_TXT_TOP);
                        ui_show(LANE_TXT_TOP);
                    }
                } else if (__this->onkey_sel == 1) {
                    if (!lane_move) {
                        send_lane_det_setting_msg("lane_set_close", 0);
                        lane_move = 3;
                    }
                    if (lane_move == 3) {
                        if (__this->car_head_line < __this->vanish_line + 40) {
                            __this->car_head_line = __this->vanish_line + 30;
                        } else {
                            __this->car_head_line -= 10;
                        }
                        ui_hide(LANE_TXT_BOTTOM);
                        ui_show(LANE_TXT_BOTTOM);
                    }
                }
                break;
            }
        }
    }
    return false;

}
REGISTER_UI_EVENT_HANDLER(LANE_LAY)
.onchange = lane_set_onchange,
 .ontouch = lane_set_ontouch,
  .onkey = lane_set_onkey,
};


#if 0
static int video_lane_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    struct application *app;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    switch (e) {
    case ON_CHANGE_INIT:
        
        break;
    case ON_CHANGE_RELEASE:
       
        if (__this->page_exit == HOME_SW_EXIT) {

            ui_show(ID_WINDOW_MAIN_PAGE); 
            ///video0_disp_start(1280, 480, 0, 0);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_LANE)
.onchange = video_lane_onchange,
 .ontouch = NULL,
};
#endif

static int lane_return_ontouch(void *ctr, struct element_touch_event *e)
{
    struct application *app;
    
    UI_ONTOUCH_DEBUG("**lane return ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        struct intent it;
        #if 1
        ui_hide(ui_get_current_window_id());
        init_intent(&it);
        if(lane_entry_flag == 1)  
        {
            printf("+++++++++++++++++++++++++++++++\n");
            lane_entry_flag = 0;
            #if 0
            __this->page_exit = HOME_SW_EXIT;
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, close_set_lane_page, NULL); //不等待直接启动app
            #endif
            __this->page_exit = HOME_SW_EXIT;
            init_intent(&it);
            
            app = get_current_app();
        
            printf("===============app_name=%s======================\n", app->name);
            if (app) {
                it.name = "video_rec";
                it.action = ACTION_BACK;
                start_app_async(&it, NULL, NULL); //不等待直接启动app
            }
        
                //it.name = "video_rec";
                //it.action = ACTION_VIDEO_REC_MAIN;
                //start_app_async(&it, NULL, NULL);
          
        }
        else
        {
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, close_set_lane_page, NULL);
        }
        #endif
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LANE_BTN_RETURN)
.ontouch = lane_return_ontouch,
};

#endif
