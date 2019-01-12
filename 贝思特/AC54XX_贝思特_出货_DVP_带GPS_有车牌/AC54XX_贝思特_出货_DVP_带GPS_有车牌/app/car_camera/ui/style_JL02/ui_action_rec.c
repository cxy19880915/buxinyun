#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "system/includes.h"
#include "menu_parm_api.h"
#include "app_database.h"
#include "gps.h"
#include "key_voice.h"

#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

#define REC_RUNNING_TO_HOME     1  //录像时返回主界面

#define ENC_MENU_HIDE_ENABLE    1  //选定菜单后子菜单收起

struct rec_menu_info {
    char resolution;
    char double_route;
    char mic;
    char gravity;
    char motdet;
    char park_guard;
    char wdr;
    char cycle_rec;
    char car_num;
    char dat_label;
    char exposure;
    char gap_rec;

    u8 lock_file_flag; /* 是否当前文件被锁 */

    u8 page_exit;  /*页面退出方式  1切模式退出  2返回HOME退出 */
    u8 menu_status;/*0 menu off, 1 menu on*/
    s8 enc_menu_status;
    u8 battery_val;
    u8 battery_char;

    u8 onkey_mod;
    s8 onkey_sel;
    u8 key_disable;
    u8 hlight_show_status;  /* 前照灯显示状态 */
	   int dog_timer;
};


volatile char if_in_rec; /* 是否正在录像 */
static struct rec_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct rec_menu_info))

volatile u8 main_lock_flag;
u8 all_back = 0;


const static int onkey_sel_item[3] = {
    ENC_PIC_SETTING,
    ENC_BTN_VIDEO,
    ENC_BTN_HOME,
};

const static int onkey_sel_setting[12] = {
    ENC_SET_PIC_1_1,
    ENC_SET_PIC_1_2,
    ENC_SET_PIC_1_3,
    ENC_SET_PIC_1_4,
    ENC_SET_PIC_2_1,
    ENC_SET_PIC_2_2,
    ENC_SET_PIC_2_3,
    ENC_SET_PIC_2_4,
    ENC_SET_PIC_3_1,
    ENC_SET_PIC_3_2,
    ENC_SET_PIC_3_3,
    ENC_SET_PIC_3_4,
};
const static int onkey_sel_setting1[12] = {
    ENC_SET_TXT_1_1,
    ENC_SET_TXT_1_2,
    ENC_SET_TXT_1_3,
    ENC_SET_TXT_1_4,
    ENC_SET_TXT_2_1,
    ENC_SET_TXT_2_2,
    ENC_SET_TXT_2_3,
    ENC_SET_TXT_2_4,
    ENC_SET_TXT_3_1,
    ENC_SET_TXT_3_2,
    ENC_SET_TXT_3_3,
    ENC_SET_TXT_3_4,
};
const static int _ENC_SET_PIC_C2[] = {
    ENC_SET_PIC_C2_1,
    ENC_SET_PIC_C2_2,
};
const static int _ENC_SET_PIC_C3[] = {
    ENC_SET_PIC_C3_1,
    ENC_SET_PIC_C3_2,
    ENC_SET_PIC_C3_3,
};
const static int _ENC_SET_PIC_C4[] = {
    ENC_SET_PIC_C4_1,
    ENC_SET_PIC_C4_2,
    ENC_SET_PIC_C4_3,
    ENC_SET_PIC_C4_4,
};
const static int _ENC_SET_PIC_EXP[] = {
    ENC_SET_PIC_EXP_S01,
    ENC_SET_PIC_EXP_S02,
    ENC_SET_PIC_EXP_S03,
    ENC_SET_PIC_EXP_S04,
    ENC_SET_PIC_EXP_S05,
    ENC_SET_PIC_EXP_S06,
    ENC_SET_PIC_EXP_S07,
};
const static int _ENC_SET_PIC_C20[] = {
    ENC_SET_PIC_C20_1,
    ENC_SET_PIC_C20_2,
};

extern int storage_device_ready();
int sys_cur_mod;
u8 av_in_statu = 0;
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
    '9'
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
};


enum {
    PAGE_SHOW = 0,
    MODE_SW_EXIT,
    HOME_SW_EXIT,
};

struct car_num text_car_num_table[] = {
    {"province", ENC_PIC_CID_0, 0},// [> 京 <]
    {"town",     ENC_PIC_CID_1, 0},// [> A <]
    {"a",        ENC_PIC_CID_2, 0},// [> 1 <]
    {"b",        ENC_PIC_CID_3, 0},// [> 2 <]
    {"c",        ENC_PIC_CID_4, 0},// [> 3 <]
    {"d",        ENC_PIC_CID_5, 0},// [> 4 <]
    {"e",        ENC_PIC_CID_6, 0},// [> 5 <]
};



enum ENC_MENU {
    ENC_MENU_NULL = 0,
    ENC_MENU_RESOLUTION,
    ENC_MENU_DUALVIDEO,
    ENC_MENU_CYCLE,
    ENC_MENU_GUARD,
    //ENC_MENU_CID,
    //ENC_MENU_GAP,
    ENC_MENU_HDR,
    ENC_MENU_EXPOSURE,
    ENC_MENU_MOTION,
    ENC_MENU_LABEL,
    ENC_MENU_GSEN,
    ENC_MENU_SOUND,
   // ENC_MENU_GUARD,
    ENC_MENU_CID,
    ENC_MENU_GAP,
    ENC_MENU_HIDE = 100,
};


void enc_menu_show(enum ENC_MENU item)
{
    if (__this->enc_menu_status == ENC_MENU_HIDE) {
        __this->enc_menu_status = ENC_MENU_NULL;
        return;
    }
    switch (item) {
    case ENC_MENU_NULL:
        __this->enc_menu_status = ENC_MENU_NULL;
        break;
    case ENC_MENU_RESOLUTION:
        if (__this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_GUARD &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C3);
        ui_show(ENC_SET_PIC_SEL_1_1);
        ui_highlight_element_by_id(ENC_SET_PIC_1_1);
        ui_highlight_element_by_id(ENC_SET_TXT_1_1);
        ui_highlight_element_by_id(_ENC_SET_PIC_C3[__this->resolution]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_DUALVIDEO:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_CID &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_GUARD &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C2);
        ui_show(ENC_SET_PIC_SEL_1_2);
        ui_highlight_element_by_id(ENC_SET_PIC_1_2);
        ui_highlight_element_by_id(ENC_SET_TXT_1_2);
        ui_highlight_element_by_id(_ENC_SET_PIC_C2[__this->double_route]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_CYCLE:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_GUARD &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C4);
        ui_show(ENC_SET_PIC_SEL_1_3);
        ui_highlight_element_by_id(ENC_SET_PIC_1_3);
        ui_highlight_element_by_id(ENC_SET_TXT_1_3);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_2, 0);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_3, 0);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_4, 0);
        ui_highlight_element_by_id(_ENC_SET_PIC_C4[__this->cycle_rec]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_GAP:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_GUARD &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C4);
        ui_show(ENC_SET_PIC_SEL_1_4);
       // ui_highlight_element_by_id(ENC_SET_PIC_1_4);
       // ui_highlight_element_by_id(ENC_SET_TXT_1_4);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_2, 1);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_3, 1);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_4, 1);
        ui_highlight_element_by_id(_ENC_SET_PIC_C4[__this->gap_rec]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_HDR:
        if (__this->enc_menu_status != ENC_MENU_EXPOSURE &&
            __this->enc_menu_status != ENC_MENU_MOTION &&
            __this->enc_menu_status != ENC_MENU_LABEL) {
            ui_hide(ENC_SET_LAY_3);
            ui_show(ENC_SET_LAY_SET_2);
        }
        ui_show(ENC_SET_LAY_C20);
        ui_show(ENC_SET_PIC_SEL_2_1);
        ui_highlight_element_by_id(ENC_SET_PIC_2_1);
        ui_highlight_element_by_id(ENC_SET_TXT_2_1);
        ui_highlight_element_by_id(_ENC_SET_PIC_C20[__this->wdr]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_EXPOSURE:
        if (__this->enc_menu_status != ENC_MENU_HDR &&
            __this->enc_menu_status != ENC_MENU_MOTION &&
            __this->enc_menu_status != ENC_MENU_LABEL) {
            ui_hide(ENC_SET_LAY_3);
            ui_show(ENC_SET_LAY_SET_2);
        }
        ui_show(ENC_SET_LAY_EXP);
        ui_show(ENC_SET_PIC_SEL_2_2);
        ui_highlight_element_by_id(ENC_SET_PIC_2_2);
        ui_highlight_element_by_id(ENC_SET_TXT_2_2);
        ui_show(_ENC_SET_PIC_EXP[(6 - __this->exposure)]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_MOTION:
        if (__this->enc_menu_status != ENC_MENU_HDR &&
            __this->enc_menu_status != ENC_MENU_EXPOSURE &&
            __this->enc_menu_status != ENC_MENU_LABEL) {
            ui_hide(ENC_SET_LAY_3);
            ui_show(ENC_SET_LAY_SET_2);
        }
        ui_show(ENC_SET_LAY_C20);
        ui_show(ENC_SET_PIC_SEL_2_3);
        ui_highlight_element_by_id(ENC_SET_PIC_2_3);
        ui_highlight_element_by_id(ENC_SET_TXT_2_3);
        ui_highlight_element_by_id(_ENC_SET_PIC_C20[__this->motdet]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_LABEL:
        if (__this->enc_menu_status != ENC_MENU_HDR &&
            __this->enc_menu_status != ENC_MENU_EXPOSURE &&
            __this->enc_menu_status != ENC_MENU_MOTION) {
            ui_hide(ENC_SET_LAY_3);
            ui_show(ENC_SET_LAY_SET_2);
        }
        ui_show(ENC_SET_LAY_C20);
        ui_show(ENC_SET_PIC_SEL_2_4);
        ui_highlight_element_by_id(ENC_SET_PIC_2_4);
        ui_highlight_element_by_id(ENC_SET_TXT_2_4);
        ui_highlight_element_by_id(_ENC_SET_PIC_C20[__this->dat_label]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_GSEN:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_GUARD &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C4);
        ui_show(ENC_SET_PIC_SEL_3_1);
        ui_highlight_element_by_id(ENC_SET_PIC_3_1);
        ui_highlight_element_by_id(ENC_SET_TXT_3_1);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_2, 2);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_3, 2);
        ui_text_show_index_by_id(ENC_SET_TXT_C4_4, 2);
        ui_highlight_element_by_id(_ENC_SET_PIC_C4[__this->gravity]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_SOUND:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_GUARD &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C2);
        ui_show(ENC_SET_PIC_SEL_3_2);
        ui_highlight_element_by_id(ENC_SET_PIC_3_2);
        ui_highlight_element_by_id(ENC_SET_TXT_3_2);
        ui_highlight_element_by_id(_ENC_SET_PIC_C2[__this->mic]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_GUARD:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);
        }
        ui_show(ENC_SET_LAY_C2);
        ui_show(ENC_SET_PIC_SEL_1_4);
        ui_highlight_element_by_id(ENC_SET_PIC_1_4);
        ui_highlight_element_by_id(ENC_SET_TXT_1_4);
        ui_highlight_element_by_id(_ENC_SET_PIC_C2[__this->park_guard]);
        __this->enc_menu_status = item;
        break;
    case ENC_MENU_CID:
        if (__this->enc_menu_status != ENC_MENU_RESOLUTION &&
            __this->enc_menu_status != ENC_MENU_DUALVIDEO &&
            __this->enc_menu_status != ENC_MENU_CYCLE &&
            __this->enc_menu_status != ENC_MENU_GAP &&
            __this->enc_menu_status != ENC_MENU_GSEN &&
            __this->enc_menu_status != ENC_MENU_SOUND &&
            __this->enc_menu_status != ENC_MENU_GUARD) {
            ui_hide(ENC_SET_LAY_2);
            ui_show(ENC_SET_LAY_SET_1);

            //ui_show(ENC_SET_PIC_SEL_1_4);
        }
        ui_show(ENC_SET_LAY_C2);
        //ui_show(ENC_SET_PIC_SEL_3_4);

        ui_show(ENC_SET_PIC_SEL_3_3);
        ui_highlight_element_by_id(ENC_SET_PIC_3_3);
        ui_highlight_element_by_id(ENC_SET_TXT_3_3);

       // ui_highlight_element_by_id(ENC_SET_PIC_1_4);
       // ui_highlight_element_by_id(ENC_SET_TXT_1_4);

        ui_highlight_element_by_id(_ENC_SET_PIC_C2[__this->car_num]);
        __this->enc_menu_status = item;
        break;
    }
}


void enc_menu_hide(enum ENC_MENU item)
{
    switch (__this->enc_menu_status) {
    case ENC_MENU_NULL:
        break;
    case ENC_MENU_RESOLUTION:
        ui_no_highlight_element_by_id(ENC_SET_PIC_1_1);
        ui_no_highlight_element_by_id(ENC_SET_TXT_1_1);
        if (item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_GUARD &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_1_1);
            ui_hide(ENC_SET_LAY_C3);
        }
        break;
    case ENC_MENU_DUALVIDEO:
        ui_no_highlight_element_by_id(ENC_SET_PIC_1_2);
        ui_no_highlight_element_by_id(ENC_SET_TXT_1_2);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_GUARD &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_1_2);
            ui_hide(ENC_SET_LAY_C2);
        }
        break;
    case ENC_MENU_CYCLE:
        ui_no_highlight_element_by_id(ENC_SET_PIC_1_3);
        ui_no_highlight_element_by_id(ENC_SET_TXT_1_3);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_GUARD &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_1_3);
            ui_hide(ENC_SET_LAY_C4);
        }
        break;
    case ENC_MENU_GAP:
        ui_no_highlight_element_by_id(ENC_SET_PIC_1_4);
        ui_no_highlight_element_by_id(ENC_SET_TXT_1_4);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_GUARD &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_1_4);
            ui_hide(ENC_SET_LAY_C4);
        }
        break;
    case ENC_MENU_HDR:
        ui_no_highlight_element_by_id(ENC_SET_PIC_2_1);
        ui_no_highlight_element_by_id(ENC_SET_TXT_2_1);
        if (item != ENC_MENU_EXPOSURE &&
            item != ENC_MENU_MOTION &&
            item != ENC_MENU_LABEL) {
            ui_hide(ENC_SET_LAY_SET_2);
            ui_show(ENC_SET_LAY_3);
        } else {
            ui_hide(ENC_SET_PIC_SEL_2_1);
            ui_hide(ENC_SET_LAY_C20);
        }
        break;
    case ENC_MENU_EXPOSURE:
        ui_no_highlight_element_by_id(ENC_SET_PIC_2_2);
        ui_no_highlight_element_by_id(ENC_SET_TXT_2_2);
        if (item != ENC_MENU_HDR &&
            item != ENC_MENU_MOTION &&
            item != ENC_MENU_LABEL) {
            ui_hide(ENC_SET_LAY_SET_2);
            ui_show(ENC_SET_LAY_3);
        } else {
            ui_hide(ENC_SET_PIC_SEL_2_2);
            ui_hide(ENC_SET_LAY_EXP);
        }
        break;
    case ENC_MENU_MOTION:
        ui_no_highlight_element_by_id(ENC_SET_PIC_2_3);
        ui_no_highlight_element_by_id(ENC_SET_TXT_2_3);
        if (item != ENC_MENU_HDR &&
            item != ENC_MENU_EXPOSURE &&
            item != ENC_MENU_LABEL) {
            ui_hide(ENC_SET_LAY_SET_2);
            ui_show(ENC_SET_LAY_3);
        } else {
            ui_hide(ENC_SET_PIC_SEL_2_3);
            ui_hide(ENC_SET_LAY_C20);
        }
        break;
    case ENC_MENU_LABEL:
        ui_no_highlight_element_by_id(ENC_SET_PIC_2_4);
        ui_no_highlight_element_by_id(ENC_SET_TXT_2_4);
        if (item != ENC_MENU_HDR &&
            item != ENC_MENU_EXPOSURE &&
            item != ENC_MENU_MOTION) {
            ui_hide(ENC_SET_LAY_SET_2);
            ui_show(ENC_SET_LAY_3);
        } else {
            ui_hide(ENC_SET_PIC_SEL_2_4);
            ui_hide(ENC_SET_LAY_C20);
        }
        break;
    case ENC_MENU_GSEN:
        ui_no_highlight_element_by_id(ENC_SET_PIC_3_1);
        ui_no_highlight_element_by_id(ENC_SET_TXT_3_1);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_GUARD &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_3_1);
            ui_hide(ENC_SET_LAY_C4);
        }
        break;
    case ENC_MENU_SOUND:
        ui_no_highlight_element_by_id(ENC_SET_PIC_3_2);
        ui_no_highlight_element_by_id(ENC_SET_TXT_3_2);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_GUARD &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_3_2);
            ui_hide(ENC_SET_LAY_C2);
        }
        break;
    case ENC_MENU_GUARD:
        ui_no_highlight_element_by_id(ENC_SET_PIC_1_4);
        ui_no_highlight_element_by_id(ENC_SET_TXT_1_4);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_CID) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_1_4);
            ui_hide(ENC_SET_LAY_C2);
        }
        break;
    case ENC_MENU_CID:
        //ui_no_highlight_element_by_id(ENC_SET_PIC_3_4);
        //ui_no_highlight_element_by_id(ENC_SET_TXT_3_4);
        ui_no_highlight_element_by_id(ENC_SET_PIC_3_3);
        ui_no_highlight_element_by_id(ENC_SET_TXT_3_3);
        if (item != ENC_MENU_RESOLUTION &&
            item != ENC_MENU_DUALVIDEO &&
            item != ENC_MENU_CYCLE &&
            item != ENC_MENU_GAP &&
            item != ENC_MENU_GSEN &&
            item != ENC_MENU_SOUND &&
            item != ENC_MENU_GUARD) {
            ui_hide(ENC_SET_LAY_SET_1);
            ui_show(ENC_SET_LAY_2);
        } else {
            ui_hide(ENC_SET_PIC_SEL_1_4);
            ui_hide(ENC_SET_LAY_C2);
        }
        break;
    default:
        __this->enc_menu_status = ENC_MENU_NULL;
        break;
    }

    if (item == __this->enc_menu_status) {
        __this->onkey_mod = 2;
        __this->enc_menu_status = ENC_MENU_HIDE;
    }
}



int enc_menu(int item)
{
    enc_menu_hide(item);
    enc_menu_show(item);
    return 0;
}
/*
 * (begin)提示框显示接口 ********************************************
 */
enum box_msg {
    BOX_MSG_POWER_OFF = 1,
    BOX_MSG_NO_POWER,
    BOX_MSG_MEM_ERR,
    BOX_MSG_NO_MEM,
    BOX_MSG_NEED_FORMAT,
    BOX_MSG_INSERT_SD,
    BOX_MSG_GAG,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __rec_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(ENC_LAY_MESSAGEBOX);
        }
    } else if (id == 0) { /* 没有指定ID，强制隐藏 */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(ENC_LAY_MESSAGEBOX);
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
            ui_show(ENC_LAY_MESSAGEBOX);
            ui_text_show_index_by_id(ENC_TXT_MESSAGEBOX, msg - 1);
            /* ui_show(ENC_TXT_MESSAGEBOX_1); */
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
            ui_show(ENC_LAY_MESSAGEBOX);
        }
        ui_text_show_index_by_id(ENC_TXT_MESSAGEBOX, msg - 1);
        //        ui_show(ENC_TXT_MESSAGEBOX_1);//显示提示
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
        ASSERT(err == 0, ":rec opMENU fail! %d\n", err);
    }
    if (!strcmp(it.data, "opMENU:dis")) {
        puts("rec do not allow ui to open menu.\n");
        return -1;
    } else if (!strcmp(it.data, "opMENU:en")) {
        puts("rec allow ui to open menu.\n");
    } else {
        ASSERT(0, "opMENU err\n");
    }
    return 0; /* 返回0则打开菜单 */
}
/*
 * (begin)UI状态变更主动请求APP函数 ***********************************
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
        ASSERT(err == 0, ":rec exitMENU\n");
    }
}
static void menu_rec_set_res(int sel_item)
{
    struct intent it;
    __this->resolution = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "res";
    it.exdata = table_video_resolution[sel_item];
    start_app(&it);
    ui_pic_show_image_by_id(ENC_PIC_RESOLUTION, sel_item);
}
static void menu_rec_set_mic(int sel_item)
{
    struct intent it;
    __this->mic = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "mic";
    it.exdata = sel_item;
    start_app(&it);
    ui_pic_show_image_by_id(ENC_PIC_SOUND, sel_item);
}
static void menu_rec_set_mot(int sel_item)
{
    struct intent it;
    __this->motdet = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "mot";
    it.exdata = sel_item;
    start_app(&it);
    if (sel_item) {
        ui_show(ENC_PIC_MOVE);
    } else {
        ui_hide(ENC_PIC_MOVE);
    }

    rec_tell_app_exit_menu();//生效移动侦测选项
}
static void menu_rec_set_par(int sel_item)
{
    struct intent it;
    __this->park_guard = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "par";
    it.exdata = sel_item;
    start_app(&it);
    if (sel_item) {
        ui_show(ENC_PIC_GUARD);
    } else {
        ui_hide(ENC_PIC_GUARD);
    }
}
static void menu_rec_set_wdr(int sel_item)
{
    struct intent it;
    __this->wdr = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "wdr";
    it.exdata = sel_item;
    start_app(&it);
    if (sel_item) {
        ui_show(ENC_PIC_HDR);
    } else {
        ui_hide(ENC_PIC_HDR);
    }
}
static void menu_rec_set_num(int sel_item)
{
    struct intent it;
    __this->car_num = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "num";
    it.exdata = sel_item;
    start_app(&it);
    if (sel_item) {
        ui_show(ENC_LAY_CID);
    } else {
        ui_hide(ENC_LAY_CID);
    }
}
static void menu_rec_set_dat(int sel_item)
{
    struct intent it;
    __this->dat_label = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "dat";
    it.exdata = sel_item;
    start_app(&it);
}
static void menu_rec_set_double(int sel_item)
{
    struct intent it;
    __this->double_route = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "two";
    it.exdata = sel_item;
    start_app(&it);
}
static void menu_rec_set_gravity(int sel_item)
{
    struct intent it;
    __this->gravity = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "gra";
    it.exdata = table_video_gravity[sel_item];
    start_app(&it);
    if (sel_item == 0) {
        ui_hide(ENC_PIC_GSEN);
    } else {
        ui_pic_show_image_by_id(ENC_PIC_GSEN, sel_item - 1);
    }
}
static void menu_rec_set_cycle(int sel_item)
{
    struct intent it;
    __this->cycle_rec = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "cyc";
    it.exdata = table_video_cycle[sel_item];
    start_app(&it);
    ui_pic_show_image_by_id(ENC_PIC_CYCLE, sel_item);
}
static void menu_rec_set_exposure(int sel_item)
{
    struct intent it;
    __this->exposure = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "exp";
    it.exdata = table_video_exposure[sel_item];
    start_app(&it);
}
static void menu_rec_set_gap(int sel_item)
{
    struct intent it;
    __this->gap_rec = sel_item;
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_SET_CONFIG;
    it.data = "gap";
    it.exdata = table_video_gap[sel_item];
    start_app(&it);
    //ui_pic_show_image_by_id(ENC_PIC_DELAY, sel_item);
}

static void get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec);
    dev_close(fd);
}

static int disp_RecSetting_lay(u8 menu_status)
{
    if (menu_status) {
        /* if (if_in_rec == TRUE) { */
        /* puts("It is in rec,can't open menu.\n"); */
        /* return -1; */
        /* } */
        /* if (rec_ask_app_open_menu() == (int) - 1) { */
        /* return -1; */
        /* } */
        __this->menu_status = 1;
        __this->enc_menu_status = ENC_MENU_NULL;
        //ui_hide(ENC_LAY_REC);
        ui_show(ENC_SET_WIN);
        ui_highlight_element_by_id(ENC_PIC_SETTING);
    } else {
        ui_hide(ENC_SET_WIN);
        //ui_show(ENC_LAY_REC);
        if (av_in_statu) {
            ui_show(ENC_BTN_SWITCH);
        }
        if (if_in_rec == TRUE) {
            ui_show(ENC_ANI_REC_HL);
        }
        if (__this->onkey_mod == 0) {
            ui_no_highlight_element_by_id(ENC_PIC_SETTING);
        } else {
            ui_highlight_element_by_id(ENC_PIC_SETTING);
        }
        __this->menu_status = 0;
        __this->enc_menu_status = ENC_MENU_NULL;
    }
    return 0;
}
static void avin_event_handler(struct sys_event *event, void *priv)
{
    puts("avin event handler\n");
    if (!strcmp(event->arg, "video1") || !strncmp((char *)event->arg, "uvc", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            printf("__debug %s:%d\n", __FILE__, __LINE__);
            av_in_statu = 1;
            ui_show(ENC_BTN_SWITCH);
            break;
        case DEVICE_EVENT_OUT:
            printf("__debug %s:%d\n", __FILE__, __LINE__);
            av_in_statu = 0;
            ui_hide(ENC_BTN_SWITCH);
            break;
        }
    }
}
/*
 * sd卡事件处理函数
 */
static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        //case DEVICE_EVENT_ONLINE:
            ui_hide(ENC_PIC_SD);
            if (!if_in_rec) {
                ui_hide(ENC_TIM_REMAIN);
                ui_show(ENC_TIM_REMAIN);
            }
            else
            {
                ui_show(ENC_TIM_REC);
            }
            break;
        case DEVICE_EVENT_OUT:
        //case DEVICE_EVENT_OFFLINE:
            ui_show(ENC_PIC_SD);
            if (if_in_rec) {
                ui_hide(ENC_TIM_REC);
            }
            ui_hide(ENC_TIM_REMAIN);
            ui_show(ENC_TIM_REMAIN);
            break;
        default:
            break;
        }
    }
}

int rec_timer_handle = 0;
int rec_cnt = 0;
static void rec_timer_cnt(void *priv)
{
    /*
     * 录像计时的定时器,1s
     */
    struct utime time_r;
    rec_cnt++;
    time_r.hour = rec_cnt / 60 / 60;
    time_r.min = rec_cnt / 60 % 60;
    time_r.sec = rec_cnt % 60;
    ui_time_update_by_id(ENC_TIM_REC, &time_r);
}

/*
 * (begin)APP状态变更，UI响应回调 ***********************************
 */
static int rec_on_handler(const char *type, u32 arg)
{
    puts("\n***rec_on_handler.*rec**\n");
    if_in_rec = TRUE;
    ui_hide(ENC_TIM_REMAIN);
    ui_show(ENC_TIM_REC);
    ui_show(ENC_ANI_REC_HL);
    if (rec_timer_handle == 0) {
        rec_cnt = 0;
        rec_timer_handle = sys_timer_add(NULL, rec_timer_cnt, 1000);
    }
    return 0;
}
static int rec_off_handler(const char *type, u32 arg)
{
    puts("rec_off_handler.\n");
    if_in_rec = FALSE;
    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(ENC_PIC_LOCK);
    }
    ui_hide(ENC_TIM_REC);
    ui_show(ENC_TIM_REMAIN);
    ui_hide(ENC_ANI_REC_HL);

    sys_timer_del(rec_timer_handle);
    rec_timer_handle = 0;
    rec_cnt = 0;
    return 0;
}
static int rec_save_handler(const char *type, u32 arg)
{
    puts("rec_save_handler.\n");
    struct utime time_r;
    time_r.hour = 0;
    time_r.min = 0;
    time_r.sec = 0;
    ui_time_update_by_id(ENC_TIM_REC, &time_r);
    rec_cnt = 0;
    if (__this->lock_file_flag) {
        __this->lock_file_flag = 0;
        ui_hide(ENC_PIC_LOCK);
    }
    return 0;
}
static int rec_no_card_handler(const char *type, u32 arg)
{
    puts("rec_no_card_handler.\n");
    __rec_msg_show(BOX_MSG_INSERT_SD, 3000);
    return 0;
}
static int rec_fs_err_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_NEED_FORMAT, 3000);
    return 0;
}
/******add by cxy*******/
static int rec_no_mem_handler(const char *type, u32 arg)
{
    __rec_msg_show(BOX_MSG_NO_MEM, 3000);
    return 0;
}

static int rec_av_in_handler(const char *type, u32 arg)
{
    av_in_statu = 1;
    ui_show(ENC_BTN_SWITCH);
    return 0;
}
static int rec_av_off_handler(const char *type, u32 arg)
{
    av_in_statu = 0;
    ui_hide(ENC_BTN_SWITCH);
    return 0;
}

static int rec_lock_handler(const char *type, u32 arg)
{
    puts("rec lock handler\n");
    __this->lock_file_flag = 1;
    ui_show(ENC_PIC_LOCK);
    return 0;
}

extern void play_voice_file(const char *file_name);
static int rec_headlight_on_handler(const char *type, u32 arg)
{
    puts("rec_headlight_on_handler\n");
    if (__this->hlight_show_status == 0) {
        __this->hlight_show_status = 1;
        ui_show(ENC_LAY_FLIG);//show head light
        //play_voice_file("mnt/spiflash/audlogo/olight.adp");
    }
    return 0;
}
static int rec_headlight_off_handler(const char *type, u32 arg)
{
    puts("rec_headlight_off_handler\n");
    ui_hide(ENC_LAY_FLIG);//hide head light
    __this->hlight_show_status = 0;
    return 0;
}
#define maxsize  128

typedef struct SqQueue{  
    u8 data[maxsize];  
    int front;//???????  
    int rear;//??β???  
}spqueue; 

static spqueue queue;
void show_dog_info(const char *type, u32 arg);
void show_satellite(GPS_INFO gps_info);
void show_Angle(GPS_INFO gps_info);  //???????
void show_alarmType(GPS_INFO gps_info);
u8 init_qu(spqueue *qu);

void show_dog_pic(const char *type, u32 arg)
{
    int temp;
    
    ui_pic_show_image_by_id(ENC_PIC_DOG_STATE, gps_info.satellite);
    ui_show(DOG_ICON_ROAD);
    if(gps_info.curAngle > 0)
        ui_pic_show_image_by_id(ENC_PIC_DIRR,gps_info.curAngle);
    else
        ui_pic_show_image_by_id(ENC_PIC_DIRR,0);
    
    ui_show(DOG_TXT_1);
    temp = gps_info.curSpeed/100;
    ui_pic_show_image_by_id(ENC_PIC_SPEED_1,temp); 
    temp = gps_info.curSpeed/10%10;
    ui_pic_show_image_by_id(ENC_PIC_SPEED_2,temp); 
    temp = gps_info.curSpeed%10;
    ui_pic_show_image_by_id(ENC_PIC_SPEED_3,temp);
    ui_show(DOG_TXT_2);
    start_gps();
    init_qu(&queue);
    //ui_show(ENC_PIC_SPEED);
   
}

void hide_dog_pic(const char *type, u32 arg)
{
    ui_hide(ENC_PIC_DOG_STATE);   
    ui_hide(DOG_ICON_S);
    ui_hide(DOG_ICON_U);
    ui_hide(DOG_ICON_D);
    
    
    ui_hide(DOG_ICON_ROAD);
    ui_hide(ENC_PIC_DIRR);
    //ui_hide(ENC_PIC_SPEED);
    ui_hide(ENC_PIC_SPEED_1);
    ui_hide(ENC_PIC_SPEED_2);
    ui_hide(ENC_PIC_SPEED_3);
    ui_hide(DOG_TXT_1);
    ui_hide(DOG_TXT_2);
    clear_gps();
    init_qu(&queue);
}

/*
 * 录像模式的APP状态响应回调
 */
static const struct uimsg_handl rec_msg_handler[] = {
    { "lockREC",        rec_lock_handler     }, /* 锁文件 */
    { "onREC",          rec_on_handler       }, /* 开始录像 */
    { "offREC",         rec_off_handler      }, /* 停止录像 */
    { "saveREC",        rec_save_handler     }, /* 保存录像 */
    { "noCard",         rec_no_card_handler  }, /* 没有SD卡 */
    { "noMEM",          rec_no_mem_handler   }, /* 内存不足 */
    { "fsErr",          rec_fs_err_handler   },
    { "avin",           rec_av_in_handler    },
    { "avoff",          rec_av_off_handler   },
    { "HlightOn",    rec_headlight_on_handler},
    { "HlightOff",   rec_headlight_off_handler},
    { "doginfo",            show_dog_info},
    { "showdog",            show_dog_pic},
    { "hidedog",            hide_dog_pic},
    // { "onMIC",          rec_on_mic_handler   },
    // { "offMIC",         rec_off_mic_handler  },
    // { NULL, NULL},      /* 必须以此结尾！ */
};
/*
 * (end)
 */
/*****************************录像模式页面回调 ************************************/

extern void video_rec_post_msg(const char *msg, ...);
extern void video_home_post_msg(const char *msg, ...);



static int video_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("\n***rec mode onchange init***\n");
        ui_register_msg_handler(ID_WINDOW_VIDEO_REC, rec_msg_handler); /* 注册APP消息响应 */
        sys_cur_mod = 1;  /* 1:rec, 2:tph, 3:dec */
        memset(__this, 0, sizeof_this);
        __this->lock_file_flag = main_lock_flag;  //by peng
        #if 0
        if(__this->dog_timer == 0)
        {
            dog_det_init();
            __this->dog_timer = sys_timer_add((void *)0, dog_timer_hander, 500);
        }   
        #endif
        break;
    case ON_CHANGE_RELEASE:

        main_lock_flag = __this->lock_file_flag;  //by peng
        
        if (__this->menu_status) {
            ui_hide(ENC_SET_WIN);
        }
        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE);
        }
        #if 0
        if(__this->dog_timer)
        {
            sys_timer_del(__this->dog_timer);
            __this->dog_timer = 0;
        }
        #endif
        __rec_msg_hide(0);//强制隐藏消息框

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onchange = video_mode_onchange,
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
static int rec_cid_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct intent it;
    int err;
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        ui_pic_show_image_by_id(ENC_PIC_CID_0, index_of_table16(db_select("cna") >> 16, TABLE(province_gb2312)));
        ui_pic_show_image_by_id(ENC_PIC_CID_1, index_of_table8((db_select("cna") >> 8) & 0xff, TABLE(num_table)));
        ui_pic_show_image_by_id(ENC_PIC_CID_2, index_of_table8((db_select("cna") >> 0) & 0xff, TABLE(num_table)));
        ui_pic_show_image_by_id(ENC_PIC_CID_3, index_of_table8((db_select("cnb") >> 24) & 0xff, TABLE(num_table)));
        ui_pic_show_image_by_id(ENC_PIC_CID_4, index_of_table8((db_select("cnb") >> 16) & 0xff, TABLE(num_table)));
        ui_pic_show_image_by_id(ENC_PIC_CID_5, index_of_table8((db_select("cnb") >> 8) & 0xff, TABLE(num_table)));
        ui_pic_show_image_by_id(ENC_PIC_CID_6, index_of_table8((db_select("cnb") >> 0) & 0xff, TABLE(num_table)));

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY_CID)
.onchange = rec_cid_onchange,
};

/*****************************图标布局回调 ************************************/
static int rec_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    int index;
    int err;
    static int lock_event_flag = 0;

    switch (e) {
    case ON_CHANGE_INIT:
//        lock_event_id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, lock_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        //      unregister_sys_event_handler(lock_event_id);
        break;
    case ON_CHANGE_FIRST_SHOW: /* 在此获取默认隐藏的图标的状态并显示 */

        if (dev_online("uvc") || dev_online("video1")) {

            av_in_statu = 1;
            ui_show(ENC_BTN_SWITCH);
        }
        /* ui_show(ENC_TIM_REMAIN); */
        if (storage_device_ready() == 0) {
            ui_show(ENC_PIC_SD);
        }

        if(get_dog_status())
        { 
      
            int temp;
            
            ui_pic_show_image_by_id(ENC_PIC_DOG_STATE, gps_info.satellite);
            ui_show(DOG_ICON_ROAD);
            if(gps_info.curAngle > 0)
                ui_pic_show_image_by_id(ENC_PIC_DIRR,gps_info.curAngle);
            else
                ui_pic_show_image_by_id(ENC_PIC_DIRR,0);
            
            switch(gps_info.alarmType)
            {
                case 2:  //?????
                case 3:  //???????
                case 4:  //????????
                case 18:  //???????
                {   
                    if(gps_info.alarmType == 18)
                        ui_pic_show_image_by_id(DOG_ICON_U, 3);
                    else
                        ui_pic_show_image_by_id(DOG_ICON_U, gps_info.alarmType-2);
                        
                    ui_pic_show_image_by_id(DOG_ICON_D, (gps_info.limitSpeed/10)-2); 
                    break;    
                } 
                case 1:  //???
                case 5:  //??????
                case 6:  //???е?
                case 7:  //??·????
                case 8:  //????????
                case 9:  //?1??
                case 10:  //??????
                case 11:  //?У·??
                case 12:  //?????
                case 13:  //????
                case 14:  //??????
                case 15:  //???
                case 16:  //???????
                case 17:  //?????
                {
                    ui_pic_show_image_by_id(DOG_ICON_S, gps_info.alarmType-1);  
                    break; 
                }
                
            }
            
            ui_show(DOG_TXT_1);
            temp = gps_info.curSpeed/100;
            ui_pic_show_image_by_id(ENC_PIC_SPEED_1,temp); 
            temp = gps_info.curSpeed/10%10;
            ui_pic_show_image_by_id(ENC_PIC_SPEED_2,temp); 
            temp = gps_info.curSpeed%10;
            ui_pic_show_image_by_id(ENC_PIC_SPEED_3,temp);
            ui_show(DOG_TXT_2);
            //ui_show(ENC_PIC_SPEED);
   
        }

        /*从rec app获取当前录像加锁状态*/
        if (lock_event_flag) { //上电第一次不获取状态，防止start_app timeout
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_LOCK_FILE;
            it.data = "get_lock_statu";
            start_app(&it);
            if (it.exdata == 1) {
                //已加锁
                ui_show(ENC_PIC_LOCK);
                __this->lock_file_flag = 1;
            }
        }
        lock_event_flag = 1;

        if (db_select("mot")) {
            ui_show(ENC_PIC_MOVE);
        }

        index = db_select("gra");
        if (index != 0) {
            ui_pic_show_image_by_id(ENC_PIC_GSEN, index - 1);
        } else {
            ui_hide(ENC_PIC_GSEN);
        }

        if (db_select("par")) {
            ui_show(ENC_PIC_GUARD);
        }

        if (db_select("wdr")) {
            ui_show(ENC_PIC_HDR);
        }

        index = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));
        if (index != 0) {
            ui_pic_show_image_by_id(ENC_PIC_CYCLE, index);
        }

        index = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        if (index) {
            //ui_pic_show_image_by_id(ENC_PIC_DELAY, index);
        }

        index = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        if (index != 0) {
            ui_pic_show_image_by_id(ENC_PIC_RESOLUTION, index);
        }

        if (db_select("num")) {
            ui_show(ENC_LAY_CID);
        }

        if (if_in_rec == TRUE) {
            struct utime time_r;

            ui_hide(ENC_TIM_REMAIN);
            ui_show(ENC_TIM_REC);

            time_r.hour = rec_cnt / 60 / 60;
            time_r.min = rec_cnt / 60 % 60;
            time_r.sec = rec_cnt % 60;
            ui_time_update_by_id(ENC_TIM_REC, &time_r);

            ui_show(ENC_ANI_REC_HL);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_LAY)
.onchange = rec_layout_up_onchange,
};
static int rec_layout_button_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec layout button ontouch**");
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
        if (__this->menu_status) {
            if (if_in_rec) {
                __this->onkey_mod = 0;
                __this->onkey_sel = 0;
                disp_RecSetting_lay(0);
                break;
            }
            disp_RecSetting_lay(0);
            __this->onkey_mod = 1;
            __this->onkey_sel = 1;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_BASE)
.ontouch = rec_layout_button_ontouch,
};
static int ani_headlight_onchange(void *_ani, enum element_change_event e, void *arg)
{
    UI_ONTOUCH_DEBUG("ani_headlight_onchange: %d\n", e);
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        if (msg_show_f && __this->hlight_show_status) {
            //有提示框时隐藏前照灯
            ui_hide(ENC_LAY_FLIG);//hide head light
        }
        break;
    default:
        break;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ENC_ANI_FLIG)
.onchange = ani_headlight_onchange,
};
/***************************** 录像按钮显示 ************************************/
static int enc_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->onkey_mod == 0) {
            ui_highlight_element_by_id(ENC_PIC_REC);
        } else {
            ui_no_highlight_element_by_id(ENC_PIC_REC);
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(ENC_LAY_REC)
.onchange = enc_rec_onchange,
};

//static int tttttt;
static int to_home_ontouch(void *ctr, struct element_touch_event *e)  //adas
{    
    struct intent it;
    struct application *app;
    
    UI_ONTOUCH_DEBUG("**lane return ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        all_back = 0; 
        ui_hide(ID_WINDOW_ALL_BACK);
        ui_show(ID_WINDOW_MAIN_PAGE);
	
        if(if_in_rec)
        {
        printf("--------111-----\n");
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_TO_HOME;
            start_app(&it);  
            break;
        }
        
        init_intent(&it);
        app = get_current_app();
        if (app) {
	printf("--------222-----\n");
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }
        
        printf("--------3333-----\n");
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ALL_BACK_BTN)
.ontouch = to_home_ontouch,
};

/***************************** 录像设置显示 ************************************/
static int enc_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->onkey_sel) {
            ui_highlight_element_by_id(onkey_sel_setting[0]);
            ui_highlight_element_by_id(onkey_sel_setting1[0]);
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(ENC_SET_LAY)
.onchange = enc_set_onchange,
};
/***************************** MIC 图标动作 ************************************/
static int pic_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (db_select("mic")) {
            ui_pic_set_image_index(pic, 1);    /* 禁止录音 */
        } else {
            ui_pic_set_image_index(pic, 0);
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(ENC_PIC_SOUND)
.onchange = pic_mic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



static void get_sys_time_1(struct sys_time *time)
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
	get_sys_time_1(&sys_time);
	ui_time_update_by_id(ENC_TIM_TIME, &sys_time);	
}


/***************************** 系统时间控件动作 ************************************/
static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
#if 1
        get_sys_time(&sys_time);
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
#endif
	
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_TIM_TIME)
.onchange = timer_sys_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 录像时间控件动作 ************************************/
static int timer_rec_red_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        /*防止录像时从主界面回录像界面录像时间刷出0*/
        time->hour = rec_cnt / 60 / 60;
        time->min = rec_cnt / 60 % 60;
        time->sec = rec_cnt % 60;
        break;
    case ON_CHANGE_HIDE:
        time->hour = 0;
        time->min = 0;
        time->sec = 0;
        break;
    case ON_CHANGE_SHOW:
        break;
    case ON_CHANGE_SHOW_PROBE:

        break;
    case ON_CHANGE_SHOW_POST:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_TIM_REC)
.onchange = timer_rec_red_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int retime_buf_transform(char *retime_buf, struct ui_time *time)
{
    u32 cur_space;
    u32 one_pic_size;
    int err = 0;
    int hour, min, sec;
    int i, s;
    err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    if (err) {
        hour = 0;
        min = 0;
        sec = 0;
    } else {
        u32 res = db_select("res");
        if (res == VIDEO_RES_1080P) {
            one_pic_size = (0x21000 + 0xa000) / 1024;
        } else if (res == VIDEO_RES_720P) {
            one_pic_size = (0x13000 + 0xa000) / 1024;
        } else {
            one_pic_size = (0xa000 + 0xa000) / 1024;
        }
        hour = (cur_space / one_pic_size) / 30 / 60 / 60;
        min = (cur_space / one_pic_size) / 30 / 60 % 60;
        sec = (cur_space / one_pic_size) / 30 % 60;
    }
    sprintf(retime_buf, "%2d.%2d.%2d\0", hour, min, sec);
    printf("retime_buf: %s\n", retime_buf);
    i = 0;
    s = 10;
    time->hour = 0;
    while (retime_buf[i] != '.' && retime_buf[i] != '\0') {
        if (retime_buf[i] >= '0' && retime_buf[i] <= '9') {
            time->hour += ((retime_buf[i] - '0') * s);
        }
        i++;
        s = s / 10;
    }
    i++;
    s = 10;
    time->min = 0;
    while (retime_buf[i] != '.' && retime_buf[i] != '\0') {
        if (retime_buf[i] >= '0' && retime_buf[i] <= '9') {
            time->min += ((retime_buf[i] - '0') * s);
        }
        i++;
        s = s / 10;
    }
    i++;
    s = 10;
    time->sec = 0;
    while (retime_buf[i] != '.' && retime_buf[i] != '\0') {
        if (retime_buf[i] >= '0' && retime_buf[i] <= '9') {
            time->sec += ((retime_buf[i] - '0') * s);
        }
        i++;
        s = s / 10;
    }

    return err;
}
/***************************** 录像剩余时间控件动作 ************************************/
static int timer_rec_remain_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct intent it;
    int err;
    static char retime_buf[30];
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (storage_device_ready() == 0) {
            /*
             * 第一次显示需要判断一下SD卡是否在线
             */
            time->hour = 0;
            time->min = 0;
            time->sec = 0;
            break;
        }
        if (__this->lock_file_flag == 1) {
            ui_hide(ENC_TIM_REMAIN);
            break;
        }
        break;
    case ON_CHANGE_SHOW_PROBE:
        err = retime_buf_transform(retime_buf, time); //修改获取剩余时间方式，防止start_app timeout
        if (err) {
            puts("get retime err!\n");
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_TIM_REMAIN)
.onchange = timer_rec_remain_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static void no_power_msg_box_timer(void *priv)
{
    static u32 cnt = 0;
    if (__this->battery_val <= 20 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            puts("no power show.\n");
            __rec_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            puts("no power hide.\n");
            __rec_msg_hide(BOX_MSG_NO_POWER);
        }
	if(cnt>2)
	 send_key_long_msg();
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
    static u8 ten_sec_off = 0;
    if (event->type == SYS_KEY_EVENT || event->type == SYS_TOUCH_EVENT) {
        if (ten_sec_off) {
            ten_sec_off = 0;
            __rec_msg_hide(0);
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
                    __rec_msg_hide(0);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
               // __rec_msg_show(BOX_MSG_POWER_OFF, 0);
                ten_sec_off = 1;
            }
        }
        else if (!ASCII_StrCmp(event->arg, "disacc", 6))  //add by peng
        {
            if (event->u.dev.event == DEVICE_EVENT_IN) 
            {
                __rec_msg_hide(BOX_MSG_GAG);  //??? ??????????__sys_msg_hide????????????? BOX_MSG_NO_GAG?????????????????? 
            } 
            else if (event->u.dev.event == DEVICE_EVENT_OUT) 
            {
                __rec_msg_show(BOX_MSG_GAG, 10*1000);  //??? ??????????__sys_msg_hide????????????? BOX_MSG_NO_GAG??????????????????  
            }
        } 
    }
}
/*****************************主界面电池控件动作 ************************************/
static int battery_rec_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(ENC_BAT)
.onchange = battery_rec_onchange,
 .ontouch = NULL,
};

static int rec_set_lock_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec set lock ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (if_in_rec == TRUE) {
            init_intent(&it);
            __this->lock_file_flag = !__this->lock_file_flag;
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_LOCK_FILE;
            it.exdata = __this->lock_file_flag;
            start_app(&it);
            if (__this->lock_file_flag == 1) {
                puts("show lock\n");
                ui_show(ENC_PIC_LOCK);
            } else {
                puts("hide lock\n");
                ui_hide(ENC_PIC_LOCK);
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_LOCK)
.ontouch = rec_set_lock_ontouch,
};
/***************************** SD 卡图标动作 ************************************/
static int pic_sd_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u16 id = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, sd_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_PIC_SD)
.onchange = pic_sd_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** 录像设置按钮动作 ************************************/
static int rec_set_ontouch(void *ctr, struct element_touch_event *e)
{
    static u8 last_onkey_mod = 0;
    UI_ONTOUCH_DEBUG("**rec set ontouch**");
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
        if (__this->menu_status) {
            __this->onkey_mod = last_onkey_mod ? 1 : 0;
            __this->onkey_sel = 0;
            disp_RecSetting_lay(0);
        } else {
            last_onkey_mod = __this->onkey_mod;
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
            disp_RecSetting_lay(1);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_SETTING)
.ontouch = rec_set_ontouch,
};
/***************************** 录像开始结束动作 ************************************/
static void rec_control_ok(void *p, int err)
{
    if (err == 0) {
        puts("---rec control ok\n");
    } else {
        printf("---rec control faild: %d\n", err);
    }
    sys_touch_event_enable();
    __this->key_disable = 0;
}
static int rec_control_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec control ontouch**");
    struct intent it;
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
        if (__this->onkey_mod == 1) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(ENC_PIC_REC);
            __this->onkey_mod = 0;
            __this->onkey_sel = 0;
        }
        __this->key_disable = 1;
        sys_touch_event_disable();
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_CONTROL;
        //start_app_async(&it, rec_control_ok, NULL);  //????
        start_app(&it);
        sys_touch_event_enable();
        __this->key_disable = 0;

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_REC)
.ontouch = rec_control_ontouch,
};
/***************************** 切换镜头按钮动作 ************************************/
static int rec_switch_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static u16 id = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, avin_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}
static int rec_switch_win_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec switch win ontouch**");
    struct intent it;
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
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_SWITCH_WIN;
        start_app(&it);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_SWITCH)
.ontouch = rec_switch_win_ontouch,
 .onchange = rec_switch_onchange,
};


/***************************** 返回HOME按钮动作 ************************************/
static int rec_backhome_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec back to home ontouch**");
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
#if 1      
        if (1) {
			puts("\n++++++back home++++\n");
            //正在录像不退出rec app
#if REC_RUNNING_TO_HOME
            ui_hide(ui_get_current_window_id());
#endif
            break;
        }
#endif
        __this->page_exit = HOME_SW_EXIT;
        init_intent(&it);
        app = get_current_app();
        if (app) {
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_HOME)
.ontouch = rec_backhome_ontouch,
};

static int rec_to_tph_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec  to tph ontouch**");
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
        if (if_in_rec == TRUE) {
            puts("It is in rec,can't switch mode.\n");
            break;
        }
        init_intent(&it);
        app = get_current_app();
        if (app) {
            __this->page_exit = MODE_SW_EXIT;
            it.name = "video_rec";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app

            it.name = "video_photo";
            it.action = ACTION_PHOTO_TAKE_MAIN;
            start_app_async(&it, NULL, NULL); //不等待直接启动app

        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_BTN_VIDEO)
.ontouch = rec_to_tph_ontouch,
};

/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/
static int menu_res_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**res menu ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->resolution = index_of_table8(db_select("res"), TABLE(table_video_resolution));
        if (__this->enc_menu_status == ENC_MENU_RESOLUTION) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 1;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_RESOLUTION);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_1_1)
.ontouch = menu_res_ontouch,
};
static int menu_cyc_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**cyc video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->cycle_rec = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));
        if (__this->enc_menu_status == ENC_MENU_CYCLE) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 2;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_CYCLE);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_1_3)
.ontouch = menu_cyc_rec_ontouch,
};
static int menu_double_route_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**double video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->double_route = db_select("two");
        if (__this->enc_menu_status == ENC_MENU_DUALVIDEO) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 3;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_DUALVIDEO);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_1_2)
.ontouch = menu_double_route_ontouch,
};
#if 0
static int menu_gap_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**gap video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        //__this->gap_rec = index_of_table16(db_select("gap"), TABLE(table_video_gap));
        __this->car_num = db_select("num");
        
        if (__this->enc_menu_status == ENC_MENU_CID) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 4;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_CID);
        break;
        #if 0
        __this->car_num = db_select("num");
        if (__this->enc_menu_status == ENC_MENU_CID) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 12;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_CID);
        break;
        #endif
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_1_4)
.ontouch = menu_gap_rec_ontouch,
};

#endif
static int menu_hdr_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**hdr video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->wdr = db_select("wdr");
        if (__this->enc_menu_status == ENC_MENU_HDR) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 5;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_HDR);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_2_1)
.ontouch = menu_hdr_rec_ontouch,
};
static int menu_exposure_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**exp video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->exposure = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
        if (__this->enc_menu_status == ENC_MENU_EXPOSURE) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 6;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_EXPOSURE);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_2_2)
.ontouch = menu_exposure_rec_ontouch,
};
static int menu_movdet_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**movdet video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->motdet = db_select("mot");
        if (__this->enc_menu_status == ENC_MENU_MOTION) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 7;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_MOTION);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_2_3)
.ontouch = menu_movdet_rec_ontouch,
};
static int menu_date_label_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**date lable video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->dat_label = db_select("dat");
        if (__this->enc_menu_status == ENC_MENU_LABEL) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 8;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_LABEL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_2_4)
.ontouch = menu_date_label_rec_ontouch,
};
static int menu_gravity_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**gravity video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->gravity = index_of_table8(db_select("gra"), TABLE(table_video_gravity));
        if (__this->enc_menu_status == ENC_MENU_GSEN) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 9;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_GSEN);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_3_1)
.ontouch = menu_gravity_rec_ontouch,
};
static int menu_mic_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**mic video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->mic = db_select("mic");
        if (__this->enc_menu_status == ENC_MENU_SOUND) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 10;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_SOUND);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_3_2)
.ontouch = menu_mic_rec_ontouch,
};
static int menu_guard_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**guard video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->park_guard = db_select("par");
        if (__this->enc_menu_status == ENC_MENU_GUARD) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 11;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_GUARD);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_1_4)
.ontouch = menu_guard_rec_ontouch,
};
#if 1
static int menu_carnum_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**carnum video  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        __this->car_num = db_select("num");
        if (__this->enc_menu_status == ENC_MENU_CID) {
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
        } else {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = 2 + 12;
            __this->onkey_sel = 0;
        }
        ui_set_call(enc_menu, ENC_MENU_CID);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_3_3)
.ontouch = menu_carnum_rec_ontouch,
};
#endif
/*********************************************************************************
 *  		     				子菜单动作
 *********************************************************************************/
static int menu_enc_c2_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c2 ontouch**");
    struct intent it;
    int sel_item = 0;
    struct ui_pic *ctr = (struct ui_pic *)_ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        for (int i = 0; i < 2; i++) {
            if (ctr->elm.id == _ENC_SET_PIC_C2[i]) {
                sel_item = i;
                break;
            }
        }
#if ENC_MENU_HIDE_ENABLE
        ui_set_call(enc_menu, __this->enc_menu_status);
#endif
        switch (__this->enc_menu_status) {
        case ENC_MENU_DUALVIDEO:
            if (__this->double_route == sel_item) {
                return false;
            }
            menu_rec_set_double(sel_item);
            break;
        case ENC_MENU_SOUND:
            if (__this->mic == sel_item) {
                return false;
            }
            menu_rec_set_mic(sel_item);
            break;
        case ENC_MENU_GUARD:
            if (__this->park_guard == sel_item) {
                return false;
            }
            menu_rec_set_par(sel_item);
            break;
        case ENC_MENU_CID:
            if (__this->car_num == sel_item) {
                return false;
            }
            menu_rec_set_num(sel_item);
            break;
        default:
            return false;
            break;
        }
        ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[!sel_item]);
        ui_highlight_element_by_id(_ENC_SET_PIC_C2[sel_item]);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C2_1)
.ontouch = menu_enc_c2_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C2_2)
.ontouch = menu_enc_c2_ontouch,
};

static int menu_enc_c3_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c3 ontouch**");
    struct intent it;
    int sel_item = 0;
    struct ui_pic *ctr = (struct ui_pic *)_ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        for (int i = 0; i < 3; i++) {
            if (ctr->elm.id == _ENC_SET_PIC_C3[i]) {
                sel_item = i;
                break;
            }
        }
#if ENC_MENU_HIDE_ENABLE
        ui_set_call(enc_menu, __this->enc_menu_status);
#endif
        switch (__this->enc_menu_status) {
        case ENC_MENU_RESOLUTION:
            if (__this->resolution == sel_item) {
                break;
            }
            ui_no_highlight_element_by_id(_ENC_SET_PIC_C3[__this->resolution]);
            ui_highlight_element_by_id(_ENC_SET_PIC_C3[sel_item]);
            menu_rec_set_res(sel_item);
            break;
        default:
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C3_1)
.ontouch = menu_enc_c3_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C3_2)
.ontouch = menu_enc_c3_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C3_3)
.ontouch = menu_enc_c3_ontouch,
};

static int menu_enc_c4_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c4 ontouch**");
    const char *data;
    int sel_item = 0;
    struct ui_pic *ctr = (struct ui_pic *)_ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        for (int i = 0; i < 4; i++) {
            if (ctr->elm.id == _ENC_SET_PIC_C4[i]) {
                sel_item = i;
                break;
            }
        }
#if ENC_MENU_HIDE_ENABLE
        ui_set_call(enc_menu, __this->enc_menu_status);
#endif
        switch (__this->enc_menu_status) {
        case ENC_MENU_GAP:
            if (__this->gap_rec == sel_item) {
                return false;
            } else {
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[__this->gap_rec]);
            }
            menu_rec_set_gap(sel_item);
            break;
        case ENC_MENU_CYCLE:
            if (__this->cycle_rec == sel_item) {
                return false;
            } else {
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[__this->cycle_rec]);
            }
            menu_rec_set_cycle(sel_item);
            break;
        case ENC_MENU_GSEN:
            if (__this->gravity == sel_item) {
                return false;
            } else {
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[__this->gravity]);
            }
            menu_rec_set_gravity(sel_item);
            break;
        default:
            return false;
            break;
        }
        ui_highlight_element_by_id(_ENC_SET_PIC_C4[sel_item]);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C4_1)
.ontouch = menu_enc_c4_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C4_2)
.ontouch = menu_enc_c4_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C4_3)
.ontouch = menu_enc_c4_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C4_4)
.ontouch = menu_enc_c4_ontouch,
};

static int menu_enc_c20_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c20 ontouch**");
    const char *data;
    int sel_item = 0;
    struct ui_pic *ctr = (struct ui_pic *)_ctr;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        for (int i = 0; i < 2; i++) {
            if (ctr->elm.id == _ENC_SET_PIC_C20[i]) {
                sel_item = i;
                break;
            }
        }
#if ENC_MENU_HIDE_ENABLE
        ui_set_call(enc_menu, __this->enc_menu_status);
#endif
        switch (__this->enc_menu_status) {
        case ENC_MENU_HDR:
            if (__this->wdr == sel_item) {
                return false;
            }
            menu_rec_set_wdr(sel_item);
            break;
        case ENC_MENU_MOTION:
            if (__this->motdet == sel_item) {
                return false;
            }
            menu_rec_set_mot(sel_item);
            break;
        case ENC_MENU_LABEL:
            if (__this->dat_label == sel_item) {
                return false;
            }
            menu_rec_set_dat(sel_item);
            break;
        default:
            return false;
            break;
        }
        ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[!sel_item]);
        ui_highlight_element_by_id(_ENC_SET_PIC_C20[sel_item]);

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C20_1)
.ontouch = menu_enc_c20_ontouch,
};
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_C20_2)
.ontouch = menu_enc_c20_ontouch,
};




void show_Angle(GPS_INFO gps_info)  //???????
{
    u8 last_curAngle = 0xff;
    if(last_curAngle != gps_info.curAngle)
    {
        last_curAngle =  gps_info.curAngle;
        if(last_curAngle > 0)
            ui_pic_show_image_by_id(ENC_PIC_DIRR, (last_curAngle-1));
        else
            ui_pic_show_image_by_id(ENC_PIC_DIRR, 0);
    }
}


void show_satellite(GPS_INFO gps_info)
{
    static u8 last_sate = 0xff;
    if(last_sate != gps_info.satellite)
    {
        last_sate = gps_info.satellite;
        ui_pic_show_image_by_id(ENC_PIC_DOG_STATE, gps_info.satellite);
    }
   
}

void show_speed(int speed)   //??????
{
    static u32 last_speed;
    if(last_speed != speed)
    {
        int temp;
        temp = speed/100;
        ui_pic_show_image_by_id(ENC_PIC_SPEED_1,temp); 
        temp = speed/10%10;
        ui_pic_show_image_by_id(ENC_PIC_SPEED_2,temp); 
        temp = speed%10;
        ui_pic_show_image_by_id(ENC_PIC_SPEED_3,temp);
        last_speed = speed;
    }
         
}

#if 0
void show_latitude(GPS_INFO gps_info)  //???//?????  119.1234
{
    u8 buf[20];
    u32 temp;
    u8 i;
    
    memset(buf, 0, sizeof(buf));
    sprintf(buf,"%d%c%d%d%d%d",gps_info.curLongitude/10000, '.', (gps_info.curLongitude%10000/1000),(gps_info.curLongitude%10000%1000/100),(gps_info.curLongitude%10000%1000%100/10),(gps_info.curLongitude%10000%1000%100%10));  //????
    ui_text_set_str_by_id(DOG_TXT_LONG, "ascii", buf); 
    
    memset(buf, 0, sizeof(buf));  //γ??
    sprintf(buf,"%d%c%d%d%d%d",gps_info.curLattitude/10000, '.',(gps_info.curLattitude%10000/1000),(gps_info.curLattitude%10000%1000/100),(gps_info.curLattitude%10000%1000%100/10),(gps_info.curLattitude%10000%1000%100%10));
    ui_text_set_str_by_id(DOG_TXT_LATITUDE, "ascii", buf); 
    //printf("==%d==%d==\n", gps_info.curLongitude,gps_info.curLattitude);
}


void show_time(struct utime time)
{
    int temp;
    #if 0
    temp = time.year/1000;
    ui_pic_show_image_by_id(DOG_PIC_YEAE_1,temp); 
    temp = time.year/100%10;
    ui_pic_show_image_by_id(DOG_PIC_YEAE_2,temp);  
    temp = time.year/10%10;
    ui_pic_show_image_by_id(DOG_PIC_YEAE_3,temp);
    temp = time.year%10;
    ui_pic_show_image_by_id(DOG_PIC_YEAE_4,temp);
    
    temp = time.month/10;
    ui_pic_show_image_by_id(DOG_PIC_MON_1,temp); 
    temp = time.month%10;
    ui_pic_show_image_by_id(DOG_PIC_MON_2,temp);   

    temp = time.day/10;
    ui_pic_show_image_by_id(DOG_PIC_DAY_1,temp); 
    temp = time.day%10;
    ui_pic_show_image_by_id(DOG_PIC_DAY_2,temp); 
    #endif
}

void show_time_h_m(struct utime time)
{
    int temp;
    temp = time.hour/10;
    ui_pic_show_image_by_id(DOG_TIME_1,temp); 
    temp = time.hour%10;
    ui_pic_show_image_by_id(DOG_TIME_2,temp);  
    
    temp = time.min/10;
    ui_pic_show_image_by_id(DOG_TIME_3,temp); 
    temp = time.min%10;
    ui_pic_show_image_by_id(DOG_TIME_4,temp);     
}
#endif
static void UTC2BTC(struct utime *GPS)
{
//***************************************************
//?????????,??????????,?????????+1??
        GPS->sec++; //?????
        if(GPS->sec>59){
            GPS->sec=0;
            GPS->min++;
            if(GPS->min>59){
                GPS->min=0;
                GPS->hour++;
            }
        }

//***************************************************
        GPS->hour+=8;        //????????UTC??????8С?
        if(GPS->hour>23)
        {
            GPS->hour-=24;
            GPS->day+=1;
            if(GPS->month==2 ||GPS->month==4 ||GPS->month==6 ||GPS->month==9 ||GPS->month==11 ){
                if(GPS->day>30){          //?????????·???30????￡?2?·??????30
                    GPS->day=1;
                    GPS->month++;
                }
            }
            else{
                if(GPS->day>31){          //??μ?????·????31?????
                    GPS->day=1;
                    GPS->month++;
                }
            }
            if(GPS->year % 4 == 0 ){//
                if(GPS->day > 29 && GPS->month ==2){       //??????????29??
                    GPS->day=1;
                    GPS->month++;
                }
            }
            else{
                if(GPS->day>28 &&GPS->month ==2){      //???????????28?????
                    GPS->day=1;
                    GPS->month++;
                }
            }
            if(GPS->month>12){
                GPS->month-=12;
                GPS->year++;
            }
        }
}
/*****************************************************************************/
//001 ?м??        002 ?д????????        003?в???????       004 ??????????????
//005 ?е?????    006 ??????            007??·????         008????????
//009?1????     010??????               011?У·??         012?????
//013????         014????               015???             016???????
//017?????         018???????             019?????30         020?????40
//021?????50       022?????60             023?????70         024?????80
//025?????90       026?????100            027?????110        028?????120
//029???           030?????               031??????           032??????
//033?????         034?????               035??????           036?????
//037?????         038?????               039??????         040???????
//041?????
/*****************************************************************************/







u8 init_qu(spqueue *qu)
{
    qu->front = 0;
    qu->rear = 0; 
    memset(qu->data, 0, sizeof(qu->data));
    return 0;
}

u8 queue_is_empty(spqueue *qu)
{
    //printf("E  %d==%d\n", qu->front,qu->rear);
  
    if(qu->front == qu->rear)
    {
        return 1;
    }   
    else
    {
        return 0;  
    }
        
}

u8 queue_is_full(spqueue *qu)
{
    if((qu->rear+1)%maxsize == qu->front)
    {
        return 1;
    }   
    else
    {
        return 0;
    }     
}

u8 wirte_qu(spqueue *qu, u8 data)
{
    if(queue_is_full(qu) == 1)
    {
        init_qu(qu);
        return 0;

    }
       

    qu->data[qu->rear] = data;
    qu->rear = (qu->rear+1)%maxsize;
    //printf("qu->rear =%d\n", qu->rear);
    //printf("11qu->front =%d\n", qu->front);
    return 1;
}

u8 read_qu(spqueue *qu, u8 *data)
{
    
    if(queue_is_empty(qu) == 1)
    {
        return 1;
    }
    *data = qu->data[qu->front];
    qu->front  = (qu->front+1)%maxsize;
    //printf("qu->front =%d\n", qu->front);
    return 0;
}

/*****************************************************************************/
//001 ?м??        002 ?д????????        003?в???????       004 ??????????????
//005 ?е?????    006 ??????            007??·????         008????????
//009?1????     010??????               011?У·??         012?????
//013????         014????               015???             016???????
//017?????         018???????             019?????30         020?????40
//021?????50       022?????60             023?????70         024?????80
//025?????90       026?????100            027?????110        028?????120
//029???           030?????               031??????           032??????
//033?????         034?????               035??????           036?????
//037?????         038?????               039??????         040???????
//041?????
/*****************************************************************************/

static u8 last_alarmType;
static u8 limit = 0;

void show_alarmType(GPS_INFO gps_info)
{
    u16 distance = gps_info.distance;
    if(gps_info.alarmType > 0)
    {
        if(last_alarmType != gps_info.alarmType)
        {
            last_alarmType = gps_info.alarmType;

            if(distance > 900)  //???900??????
            {
                distance = 900;
            }
            //printf("=%d=%d=%d\n", gps_info.alarmType, gps_info.limitSpeed, gps_info.distance);
            wirte_qu(&queue,29);
            switch(last_alarmType)
            {
                
                case 1:  //???
                case 5:  //??????
                case 6:  //???е?
                case 7:  //??·????
                case 8:  //????????
                case 9:  //?1??
                case 10:  //??????
                case 11:  //?У·??
                case 12:  //?????
                case 13:  //????
                case 14:  //??????
                case 15:  //???
                case 16:  //???????
                case 17:  //?????
                {
                    if(distance >= 100)
                    {
                        if(distance/10%10 >= 5)
                        {
                            distance += 100;    
                        }
                        wirte_qu(&queue,(distance/100+29)); //????   
                    }
                    else
                    {
                        wirte_qu(&queue,30); //????     
                    }
                    
                    wirte_qu(&queue,last_alarmType);  //????
                    ui_hide(DOG_ICON_U);
                    ui_hide(DOG_ICON_D); 
                    ui_pic_show_image_by_id(DOG_ICON_S, last_alarmType-1);
                    break;
                }
                case 2:  //?????
                case 3:  //???????
                case 4:  //????????
                case 18:  //???????
                {   
                    if(distance >= 100)
                    {
                        if(distance/10%10 >= 5)
                        {
                            distance += 100;    
                        }
                        wirte_qu(&queue,(distance/100+29)); //????   
                    }
                    else
                    {
                        wirte_qu(&queue,30); //????     
                    }
                    wirte_qu(&queue,last_alarmType);   //????
                    //printf("%s\n", queue.data);
                    ui_hide(DOG_ICON_S);
                    if(gps_info.limitSpeed >= 30 && gps_info.limitSpeed <= 120)
                    {
                        limit = 1;
                        wirte_qu(&queue,(gps_info.limitSpeed/10+16));  //???????
                        if(gps_info.curSpeed > gps_info.limitSpeed)  //???????
                        {
                            wirte_qu(&queue,40);  //????    
                        }
                       
                        if(last_alarmType == 18)
                            ui_pic_show_image_by_id(DOG_ICON_U, 3);
                        else
                            ui_pic_show_image_by_id(DOG_ICON_U, last_alarmType-2);
                        
                        ui_pic_show_image_by_id(DOG_ICON_D, (gps_info.limitSpeed/10)-2);  //????20-120
                     
                    }
                    else
                    {
                        ui_pic_show_image_by_id(DOG_ICON_S, last_alarmType-1);    
                    }
                    break;
                } 
            }
            //show_speed(gps_info.curSpeed);
        }      
        else
        {
            //printf("g=%d\n", gps_info.distance);     
        }
        ui_text_show_index_by_id(DOG_TXT_1,1);
        ui_text_show_index_by_id(DOG_TXT_2,1);
        show_speed(gps_info.distance);  
    }
    else
    {
        ui_hide(DOG_ICON_U);
        ui_hide(DOG_ICON_D);
        ui_hide(DOG_ICON_S);
        if(limit == 1)   //??????
        {
            limit = 0;
            wirte_qu(&queue,41);  //????  
        }
        last_alarmType = 0; 
        ui_text_show_index_by_id(DOG_TXT_1,0);
        ui_text_show_index_by_id(DOG_TXT_2,0);
        show_speed(gps_info.curSpeed);
    }
}

void main_show_alarmType(GPS_INFO gps_info)
{
    u16 distance = gps_info.distance;
    if(gps_info.alarmType > 0)
    {
        if(last_alarmType != gps_info.alarmType)
        {
            last_alarmType = gps_info.alarmType;

            if(distance > 900)  //???900??????
            {
                distance = 900;
            }
            //printf("=%d=%d=%d\n", gps_info.alarmType, gps_info.limitSpeed, gps_info.distance);
            wirte_qu(&queue,29);
            switch(last_alarmType)
            {
                
                case 1:  //???
                case 5:  //??????
                case 6:  //???е?
                case 7:  //??·????
                case 8:  //????????
                case 9:  //?1??
                case 10:  //??????
                case 11:  //?У·??
                case 12:  //?????
                case 13:  //????
                case 14:  //??????
                case 15:  //???
                case 16:  //???????
                case 17:  //?????
                {
                    if(distance >= 100)
                    {
                        if(distance/10%10 >= 5)
                        {
                            distance += 100;    
                        }
                        wirte_qu(&queue,(distance/100+29)); //????   
                    }
                    else
                    {
                        wirte_qu(&queue,30); //????     
                    }
                    wirte_qu(&queue,last_alarmType);  //????
                    //ui_hide(DOG_ICON_U);
                    //ui_hide(DOG_ICON_D); 
                    //ui_pic_show_image_by_id(DOG_ICON_S, last_alarmType-1);
                    break;
                }
                case 2:  //?????
                case 3:  //???????
                case 4:  //????????
                case 18:  //???????
                {   
                    if(distance >= 100)
                    {
                        if(distance/10%10 >= 5)
                        {
                            distance += 100;    
                        }
                        wirte_qu(&queue,(distance/100+29)); //????   
                    }
                    else
                    {
                        wirte_qu(&queue,30); //????     
                    }
                    wirte_qu(&queue,last_alarmType);   //????
                    //printf("%s\n", queue.data);
                    //ui_hide(DOG_ICON_S);
                    if(gps_info.limitSpeed >= 30 && gps_info.limitSpeed <= 120)
                    {
                        limit = 1;
                        wirte_qu(&queue,(gps_info.limitSpeed/10+16));  //???????
                        if(gps_info.curSpeed > gps_info.limitSpeed)  //???????
                        {
                            wirte_qu(&queue,40);  //????    
                        }
                       
                        //if(last_alarmType == 18)
                        //    ui_pic_show_image_by_id(DOG_ICON_U, 3);
                        //else
                        //    ui_pic_show_image_by_id(DOG_ICON_U, last_alarmType-2);
                        
                        //ui_pic_show_image_by_id(DOG_ICON_D, (gps_info.limitSpeed/10)-2);  //????20-120
                     
                    }
                    else
                    {
                        //ui_pic_show_image_by_id(DOG_ICON_S, last_alarmType-1);    
                    }
                    break;
                } 
            }
            //show_speed(gps_info.curSpeed);
        }      
        else
        {
            //printf("g=%d\n", gps_info.distance);     
        }
        //ui_text_show_index_by_id(DOG_TXT_1,1);
        //ui_text_show_index_by_id(DOG_TXT_2,1);
        //show_speed(gps_info.distance);  
    }
    else
    {
        //ui_hide(DOG_ICON_U);
        //ui_hide(DOG_ICON_D);
        //ui_hide(DOG_ICON_S);
        if(limit == 1)   //??????
        {
            limit = 0;
            wirte_qu(&queue,41);  //????  
        }
        last_alarmType = 0; 
        //ui_text_show_index_by_id(DOG_TXT_1,0);
        //ui_text_show_index_by_id(DOG_TXT_2,0);
        //show_speed(gps_info.curSpeed);
    }
}


void play_dog_voice(u8 num)
{
    char dog_voice[30] = "mnt/spiflash/audlogo/001.adp";  
    dog_voice[21] = num/100+'0';
    dog_voice[22] = num/10%10+'0';
    dog_voice[23] = num%10+'0';
    play_voice_file(dog_voice); 
}   

void play_dog_voice_hander()
{
    //static u8 init = 0;
    u8 temp;
    //if(0 == init)
    //{
    //    init = 1;
    //    init_qu(&queue);
    //}
    if(get_play_dog_voice_busy() == 0)
    {
        //printf("1  %d=%d\n", queue.front, queue.rear);
        if(read_qu(&queue, &temp) == 0)
        {
            //printf("%d=%d\n", queue.front, queue.rear);
            play_dog_voice(temp);        
        }       
    }
             
}

#if 1

void show_dog_info(const char *type, u32 arg)
{
    struct intent it;
    void *fd;
    struct utime time;
    static u8 uptime = 0;
    static u8 dog_in_cnt = 0;
    u8 temp;
 
    //printf("a\n");
    
    //get_g_info(&gps_info);

    if(gps_info.sate_en == 0)  //??ж?λ
    {
        memset(&gps_info, 0, sizeof(gps_info));   

    }
    if(gps_info.err)
    {
        temp = 6 + gps_info.err;
        __rec_msg_show(temp, 0);    
    }

    show_satellite(gps_info);   //????????????
    show_Angle(gps_info);       //???????
    show_alarmType(gps_info);   //????????????

    if(queue_is_empty(&queue) == 0)
    {
        init_intent(&it);    
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_TO_DOG;
        start_app_async(&it, NULL, NULL);
    }
    

    if(uptime == 0 && gps_info.satellite > 0)
    {
        uptime = 1;
        memcpy(&time, &gps_info.time, sizeof(time));
        time.year += 2000;
        UTC2BTC(&time);
        
        fd = dev_open("rtc", NULL);  //gps?????μ??????
        if (!fd) 
        {
            return ;
        }
        dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&time);
        dev_close(fd);
        ui_time_update_by_id(ENC_TIM_TIME, &time);
    }

}
void main_show_dog_info(const char *type, u32 arg)
{
    struct intent it;
    void *fd;
    struct utime time;
    static u8 uptime = 0;
    static u8 dog_in_cnt = 0;
    u8 temp;
 
    //printf("b\n");
    
    //get_g_info(&gps_info);

    if(gps_info.sate_en == 0)  //??ж?λ
    {
        memset(&gps_info, 0, sizeof(gps_info));   

    }
    if(gps_info.err)
    {
        temp = 6 + gps_info.err;
        __rec_msg_show(temp, 0);    
    }

    //show_satellite(gps_info);   //????????????
    //show_Angle(gps_info);       //???????
    main_show_alarmType(gps_info);   //????????????

    if(queue_is_empty(&queue) == 0)
    {
        init_intent(&it);    
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_TO_DOG;
        start_app_async(&it, NULL, NULL);
    }
    

    if(uptime == 0 && gps_info.satellite > 0)
    {
        uptime = 1;
        memcpy(&time, &gps_info.time, sizeof(time));
        time.year += 2000;
        UTC2BTC(&time);
        
        fd = dev_open("rtc", NULL);  //gps?????μ??????
        if (!fd) 
        {
            return ;
        }
        dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&time);
        dev_close(fd);
        //ui_time_update_by_id(ENC_TIM_TIME, &time);
    }

}
#endif
/*************************************************************************************/
/**************************************曝光补偿控件动作***************************************/
#define SLID_X    820 //滑块x起始绝对坐标
#define SLID_GAP  42  //每一项的间隔(滑块长度/项目数)
#define SLID_ITEM 7  //项目数
static int menu_exp_ontouch(void *arg1, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("** exp video ontouch  ");
    static s16 x_pos_down = 0;
    static s16 old_exp = 0;
    s16 tmp;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        x_pos_down = e->pos.x;
        old_exp = __this->exposure;
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        s16 x_pos_now = e->pos.x;
        s16 x_pos_ch = x_pos_down - x_pos_now;
        tmp = __this->exposure;
        if (x_pos_ch < SLID_GAP && x_pos_ch > -SLID_GAP) {
            return false;
        }
        tmp = old_exp + x_pos_ch / SLID_GAP;
        if (tmp > SLID_ITEM - 1) {
            tmp = SLID_ITEM - 1;
            x_pos_down = x_pos_now;
            old_exp = SLID_ITEM - 1;
        } else if (tmp < 0) {
            tmp = 0;
            x_pos_down = x_pos_now;
            old_exp = 0;
        }
        printf("\n tmp_exp = %d \n", tmp);
        if (__this->exposure == tmp) {
            return false;
        }
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        /* printf("x_pos=%d y_pos=%d",e->pos.x,e->pos.y); */
#if ENC_MENU_HIDE_ENABLE
        ui_set_call(enc_menu, __this->enc_menu_status);
#endif
        int i;
        tmp = __this->exposure;
        for (i = 1; i <= SLID_ITEM; i++) {
            if (e->pos.x - SLID_X < SLID_GAP * i && e->pos.x > SLID_X + SLID_GAP * (i - 1)) {
                tmp = SLID_ITEM - i;
            }
        }
        if (__this->exposure == tmp) {
            return false;
        }
        break;
    }
    ui_hide(_ENC_SET_PIC_EXP[6 - __this->exposure]);
    ui_show(_ENC_SET_PIC_EXP[6 - tmp]);
    menu_rec_set_exposure(tmp);

    return false;
}
REGISTER_UI_EVENT_HANDLER(ENC_SET_PIC_EXP)
.ontouch = menu_exp_ontouch,
};








static int enc_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("enc_onchange\n");
        sys_key_event_takeover(true, false);
        __this->onkey_mod = 0;
        __this->onkey_sel = 0;
        ui_highlight_element_by_id(ENC_PIC_REC);
        break;
    default:
        return false;
    }

    return false;
}
static int enc_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    struct application *app;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }

    if (e->event != KEY_EVENT_CLICK || __this->key_disable) {
        return true;
    }
    if (__this->onkey_mod == 0) {
        switch (e->value) {
        case KEY_DOWN:
            if (if_in_rec == TRUE) {
                init_intent(&it);
                __this->lock_file_flag = !__this->lock_file_flag;
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_LOCK_FILE;
                it.exdata = __this->lock_file_flag;
                start_app(&it);
                if (__this->lock_file_flag == 1) {
                    puts("show lock\n");
                    ui_show(ENC_PIC_LOCK);
                } else {
                    puts("hide lock\n");
                    ui_hide(ENC_PIC_LOCK);
                }
            }
            break;

        case KEY_UP:
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_SWITCH_WIN;
            start_app(&it);
            break;
        case KEY_OK:
            __this->key_disable = 1;
            sys_touch_event_disable();
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_CONTROL;
            start_app_async(&it, rec_control_ok, NULL);
            break;
        case KEY_MODE:
            if (!if_in_rec) {
                __this->onkey_mod = 1;
                __this->onkey_sel = 1;
                ui_no_highlight_element_by_id(ENC_PIC_REC);
                ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            }
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod == 1) {
        switch (e->value) {
        case KEY_UP:
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            __this->onkey_sel --;
            if (__this->onkey_sel < 1) {
                __this->onkey_sel = 3;
            }
            ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            __this->onkey_sel ++;
            if (__this->onkey_sel > 3) {
                __this->onkey_sel = 1;
            }
            ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            switch (__this->onkey_sel) {
            case 1:
                if (!__this->menu_status) {
                    __this->onkey_mod = 2;
                    __this->onkey_sel = 1;
                    disp_RecSetting_lay(1);
                }
                break;
            case 2:
                if (if_in_rec == TRUE) {
#if REC_RUNNING_TO_HOME
                    ui_hide(ui_get_current_window_id());
#endif
                    break;
                }
                init_intent(&it);
                app = get_current_app();
                if (app) {
                    __this->page_exit = MODE_SW_EXIT;
                    it.name = "video_rec";
                    it.action = ACTION_BACK;
                    start_app_async(&it, NULL, NULL); //不等待直接启动app

                    it.name = "video_photo";
                    it.action = ACTION_PHOTO_TAKE_MAIN;
                    start_app_async(&it, NULL, NULL); //不等待直接启动app
                }
                break;
            case 3:
                if (if_in_rec) {
                    puts("It is in rec,can't back home.\n");
                    break;
                }
                __this->page_exit = HOME_SW_EXIT;
                init_intent(&it);
                app = get_current_app();
                if (app) {
                    it.name = "video_rec";
                    it.action = ACTION_BACK;
                    start_app_async(&it, NULL, NULL); //不等待直接启动app
                }
                break;
            }
            break;
        case KEY_MODE:
            ui_highlight_element_by_id(ENC_PIC_REC);
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            __this->onkey_mod = 0;
            __this->onkey_sel = 0;
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod == 2) {
        switch (e->value) {
        case KEY_UP:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_sel --;
            if (__this->onkey_sel < 1) {
                __this->onkey_sel = 11;
            }
            ui_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_sel ++;
            if (__this->onkey_sel > 11) {
                __this->onkey_sel = 1;
            }
            ui_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            if (__this->onkey_sel) {
                __this->onkey_mod = 2 + __this->onkey_sel;
                switch (__this->onkey_sel) {
                case 1:
                    __this->resolution = index_of_table8(db_select("res"), TABLE(table_video_resolution));
                    ui_set_call(enc_menu, ENC_MENU_RESOLUTION);
                    break;
                case 2:
                    __this->double_route = db_select("two");
                    __this->onkey_sel = __this->double_route;
                    ui_set_call(enc_menu, ENC_MENU_DUALVIDEO);
                    break;
                case 3:
                    __this->cycle_rec = index_of_table8(db_select("cyc"), TABLE(table_video_cycle));
                    __this->onkey_sel = __this->cycle_rec;
                    ui_set_call(enc_menu, ENC_MENU_CYCLE);
                    break;
                case 4:
                    //__this->gap_rec = index_of_table16(db_select("gap"), TABLE(table_video_gap));
                    //__this->onkey_sel = __this->gap_rec;
                    //ui_set_call(enc_menu, ENC_MENU_GAP);
                    __this->car_num = db_select("num");
                    __this->onkey_sel = __this->car_num;
                    ui_set_call(enc_menu, ENC_MENU_CID);
                    break;
                case 5:
                    __this->wdr = db_select("wdr");
                    __this->onkey_sel = __this->wdr;
                    ui_set_call(enc_menu, ENC_MENU_HDR);
                    break;
                case 6:
                    __this->exposure = index_of_table8(db_select("exp"), TABLE(table_video_exposure));
                    __this->onkey_sel = __this->exposure;
                    ui_set_call(enc_menu, ENC_MENU_EXPOSURE);
                    break;
                case 7:
                    __this->motdet = db_select("mot");
                    __this->onkey_sel = __this->motdet;
                    ui_set_call(enc_menu, ENC_MENU_MOTION);
                    break;
                case 8:
                    __this->dat_label = db_select("dat");
                    __this->onkey_sel = __this->dat_label;
                    ui_set_call(enc_menu, ENC_MENU_LABEL);
                    break;
                case 9:
                    __this->gravity = index_of_table8(db_select("gra"), TABLE(table_video_gravity));
                    __this->onkey_sel = __this->gravity;
                    ui_set_call(enc_menu, ENC_MENU_GSEN);
                    break;
                case 10:
                    __this->mic = db_select("mic");
                    __this->onkey_sel = __this->mic;
                    ui_set_call(enc_menu, ENC_MENU_SOUND);
                    break;
                case 11:
                    __this->park_guard = db_select("par");
                    __this->onkey_sel = __this->park_guard;
                    ui_set_call(enc_menu, ENC_MENU_GUARD);
                    break;
                case 12:
                    __this->car_num = db_select("num");
                    __this->onkey_sel = __this->car_num;
                    ui_set_call(enc_menu, ENC_MENU_CID);
                    break;
                }
            }
            break;
        case KEY_MODE:
            __this->onkey_mod = 1;
            __this->onkey_sel = 1;
            disp_RecSetting_lay(0);
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod > 2 && __this->onkey_mod < 3 + 12) {
        s8 tmp;
        switch (e->value) {
        case KEY_UP:
            switch (__this->enc_menu_status) {
            case ENC_MENU_RESOLUTION:
                tmp = __this->resolution;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C3[tmp]);
                tmp = tmp == 0 ? 2 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C3[tmp]);
                menu_rec_set_res(tmp);
                break;
            case ENC_MENU_DUALVIDEO:
                tmp = __this->double_route;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_double(tmp);
                break;
            case ENC_MENU_CYCLE:
                tmp = __this->cycle_rec;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                tmp = tmp == 0 ? 3 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                menu_rec_set_cycle(tmp);
                break;
            case ENC_MENU_GAP:
                tmp = __this->gap_rec;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                tmp = tmp == 0 ? 3 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                menu_rec_set_gap(tmp);
                break;
            case ENC_MENU_HDR:
                tmp = __this->wdr;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                menu_rec_set_wdr(tmp);
                break;
            case ENC_MENU_EXPOSURE:
                if (__this->exposure == 6) {
                    break;
                }
                tmp = __this->exposure;
                tmp ++;
                ui_hide(_ENC_SET_PIC_EXP[6 - __this->exposure]);
                ui_show(_ENC_SET_PIC_EXP[6 - tmp]);
                menu_rec_set_exposure(tmp);
                break;
            case ENC_MENU_MOTION:
                tmp = __this->motdet;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                menu_rec_set_mot(tmp);
                break;
            case ENC_MENU_LABEL:
                tmp = __this->dat_label;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                menu_rec_set_dat(tmp);
                break;
            case ENC_MENU_GSEN:
                tmp = __this->gravity;
                if (tmp == 0) {
                    ui_show(ENC_PIC_GSEN);
                }
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                tmp = tmp == 0 ? 3 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                menu_rec_set_gravity(tmp);
                break;
            case ENC_MENU_SOUND:
                tmp = __this->mic;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_mic(tmp);
                break;
            case ENC_MENU_GUARD:
                tmp = __this->park_guard;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_par(tmp);
                break;
            case ENC_MENU_CID:
                tmp = __this->car_num;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_num(tmp);
                break;
            }
            break;
        case KEY_DOWN:
            switch (__this->enc_menu_status) {
            case ENC_MENU_RESOLUTION:
                tmp = __this->resolution;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C3[tmp]);
                tmp = tmp == 2 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C3[tmp]);
                menu_rec_set_res(tmp);
                break;
            case ENC_MENU_DUALVIDEO:
                tmp = __this->double_route;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_double(tmp);
                break;
            case ENC_MENU_CYCLE:
                tmp = __this->cycle_rec;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                tmp = tmp == 3 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                menu_rec_set_cycle(tmp);
                break;
            case ENC_MENU_GAP:
                tmp = __this->gap_rec;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                tmp = tmp == 3 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                menu_rec_set_gap(tmp);
                break;
            case ENC_MENU_HDR:
                tmp = __this->wdr;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                menu_rec_set_wdr(tmp);
                break;
            case ENC_MENU_EXPOSURE:
                if (__this->exposure == 0) {
                    break;
                }
                tmp = __this->exposure;
                tmp --;
                ui_hide(_ENC_SET_PIC_EXP[6 - __this->exposure]);
                ui_show(_ENC_SET_PIC_EXP[6 - tmp]);
                menu_rec_set_exposure(tmp);
                break;
            case ENC_MENU_MOTION:
                tmp = __this->motdet;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                menu_rec_set_mot(tmp);
                break;
            case ENC_MENU_LABEL:
                tmp = __this->dat_label;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C20[tmp]);
                menu_rec_set_dat(tmp);
                break;
            case ENC_MENU_GSEN:
                tmp = __this->gravity;
                if (tmp == 0) {
                    ui_show(ENC_PIC_GSEN);
                }
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                tmp = tmp == 3 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C4[tmp]);
                menu_rec_set_gravity(tmp);
                break;
            case ENC_MENU_SOUND:
                tmp = __this->mic;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_mic(tmp);
                break;
            case ENC_MENU_GUARD:
                tmp = __this->park_guard;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_par(tmp);
                break;
            case ENC_MENU_CID:
                tmp = __this->car_num;
                ui_no_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_ENC_SET_PIC_C2[tmp]);
                menu_rec_set_num(tmp);
                break;
            }
            break;
        case KEY_OK:
        case KEY_MODE:
            __this->onkey_sel = __this->onkey_mod - 2;
            __this->onkey_mod = 2;
            enc_menu(__this->enc_menu_status);
            ui_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            break;
        default:
            return false;
        }
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(ENC_WIN)
.onchange = enc_onchange,
 .onkey = enc_onkey,
};
#endif
