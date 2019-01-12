#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "system/includes.h"
#include "menu_parm_api.h"
#include "app_database.h"

#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

#define TPH_REMAIN_DISCAL 1
#define TPH_MENU_HIDE_ENABLE    1  //选定菜单后子菜单收起

struct tph_menu_sta {
    char tph_delay;
    char resolution;
    char repeat;
    char quality;
    char acutance;
    char white_balance;
    char iso;
    char exposure;
    char hand_shake;
    char dat_label;
    char color;
    char quick_scan;
};

struct tph_menu_info {
    u8 cur_camera_id;
    struct tph_menu_sta camera[2];

    u8 menu_status;
    u8 battery_val; /* 电池电量 0-100 */
    u8 battery_char; /* 是否充电中 1充电 0没充电 */
    u8 take_photo_busy; /* 是否正在拍照 */
    u8 tph_menu_status;
    u8 page_exit;   /*页面退出方式  1切换模式退出  2返回home退出  */

    u8 onkey_mod;
    s8 onkey_sel;

    u8 hlight_show_status;  /* 前照灯显示状态 */

#if TPH_REMAIN_DISCAL
    u32 remain_num;
#endif
};

static struct tph_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct tph_menu_info))

extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec */
extern u8 av_in_statu;
extern int storage_device_ready();

const static int onkey_sel_item[3] = {
    TPH_PIC_SETTING,
    TPH_BTN_PHOTO,
    TPH_BTN_HOME,
};

const static int onkey_sel_setting[12] = {
    TPH_SET_PIC_1_1,
    TPH_SET_PIC_1_2,
    TPH_SET_PIC_1_3,
    TPH_SET_PIC_1_4,
    TPH_SET_PIC_2_1,
    TPH_SET_PIC_2_2,
    TPH_SET_PIC_2_3,
    TPH_SET_PIC_2_4,
    TPH_SET_PIC_3_1,
    TPH_SET_PIC_3_2,
    TPH_SET_PIC_3_3,
    TPH_SET_PIC_3_4,
};
const static int onkey_sel_setting1[12] = {
    TPH_SET_TXT_1_1,
    TPH_SET_TXT_1_2,
    TPH_SET_TXT_1_3,
    TPH_SET_TXT_1_4,
    TPH_SET_TXT_2_1,
    TPH_SET_TXT_2_2,
    TPH_SET_TXT_2_3,
    TPH_SET_TXT_2_4,
    TPH_SET_TXT_3_1,
    TPH_SET_TXT_3_2,
    TPH_SET_TXT_3_3,
    TPH_SET_TXT_3_4,
};
const static int _TPH_SET_PIC_C2[] = {
    TPH_SET_PIC_C2_1,
    TPH_SET_PIC_C2_2,
};
const static int _TPH_SET_PIC_C3[] = {
    TPH_SET_PIC_C3_1,
    TPH_SET_PIC_C3_2,
    TPH_SET_PIC_C3_3,
};
const static int _TPH_SET_PIC_C4[] = {
    TPH_SET_PIC_C4_1,
    TPH_SET_PIC_C4_2,
    TPH_SET_PIC_C4_3,
    TPH_SET_PIC_C4_4,
};
const static int _TPH_SET_PIC_RES[] = {
    TPH_SET_PIC_RES_S1,
    TPH_SET_PIC_RES_S2,
    TPH_SET_PIC_RES_S3,
    TPH_SET_PIC_RES_S4,
    TPH_SET_PIC_RES_S5,
    TPH_SET_PIC_RES_S6,
    TPH_SET_PIC_RES_S7,
};
const static int _TPH_SET_PIC_EXP[] = {
    TPH_SET_PIC_EXP_S01,
    TPH_SET_PIC_EXP_S02,
    TPH_SET_PIC_EXP_S03,
    TPH_SET_PIC_EXP_S04,
    TPH_SET_PIC_EXP_S05,
    TPH_SET_PIC_EXP_S06,
    TPH_SET_PIC_EXP_S07,
};
const static int _TPH_SET_PIC_C30[] = {
    TPH_SET_PIC_C30_1,
    TPH_SET_PIC_C30_2,
    TPH_SET_PIC_C30_3,
};
const static int _TPH_SET_PIC_C40[] = {
    TPH_SET_PIC_C40_1,
    TPH_SET_PIC_C40_2,
    TPH_SET_PIC_C40_3,
    TPH_SET_PIC_C40_4,
};
const static int _TPH_SET_PIC_C50[] = {
    TPH_SET_PIC_C50_1,
    TPH_SET_PIC_C50_2,
    TPH_SET_PIC_C50_3,
    TPH_SET_PIC_C50_4,
    TPH_SET_PIC_C50_5,
};

/************************************************************
					  拍照模式设置
************************************************************/
/*
 *  * photo拍摄延时方式设置
 *   */
static const u8 table_photo_delay_mode_camera0[] = {
    0,
    2,
    5,
    10,
};


/*
 *  * photo拍摄分辨率设置
 *   */
static const u8 table_photo_res_camera0[] = {
    PHOTO_RES_VGA,      /* 640*480 */
    PHOTO_RES_1D3M,     /* 1280*960 */
    PHOTO_RES_2M,       /* 1920*1080 */
    PHOTO_RES_3M,       /* 2048*1536 */
    PHOTO_RES_5M,       /* 2592*1944 */
    PHOTO_RES_8M,       /* 3264*2448 */
    PHOTO_RES_10M,      /* 3648*2736 */
    PHOTO_RES_12M,      /* 4032*3024 */
};


/*
 *  * photo图像质量设置
 *   */
static const u8 table_photo_quality_camera0[] = {
    PHOTO_QUA_HI,
    PHOTO_QUA_MD,
    PHOTO_QUA_LO,
};

/*
 *  * photo图像锐度设置
 *   */
static const u8 table_photo_acutance_camera0[] = {
    PHOTO_ACU_HI,
    PHOTO_ACU_MD,
    PHOTO_ACU_LO,
};

/*
 *  * photo图像白平衡设置
 *   */
static const u8 table_photo_white_balance_camera0[] = {
    PHOTO_WBL_AUTO,     /* 自动 */
    PHOTO_WBL_SUN,      /* 日光 */
    PHOTO_WBL_CLDY,     /* 阴天 */
    PHOTO_WBL_TSL,      /* 钨丝灯 */
    PHOTO_WBL_FLUL,     /* 荧光灯 */
};

/*
 *  * photo图像ISO设置
 *   */
static const u16 table_photo_iso_camera0[] = {
    0,
    100,
    200,
    400,
};

/*
 *  * photo曝光补偿设置
 *   */
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
 *  * photo快速预览设置
 *   */
static const u8 table_photo_quick_scan_camera0[] = {
    0,
    2,
    5,
};


/*
 *  * photo色彩设置
 *   */
static const u8 table_photo_color_camera0[] = {
    PHOTO_COLOR_NORMAL,
    PHOTO_COLOR_WB,
    PHOTO_COLOR_OLD,
};


/*
 *  * (begin)拍照设置各项操作请求
 *   */

int menu_tph_set_sok(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "sok";
    it.exdata = sel_item;
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":hand shake set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_pdat(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "pdat";
    it.exdata = sel_item;
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":date label set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_cyt(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "cyt";
    it.exdata = sel_item;
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":repeat set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_qua(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "qua";
    it.exdata = table_photo_quality_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":quality set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_acu(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "acu";
    it.exdata = table_photo_acutance_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":acutance set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_phm(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "phm";
    it.exdata = table_photo_delay_mode_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":tph delay set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_pres(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "pres";
    it.exdata = table_photo_res_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":photo res set fail!\n");
        return -1;
    }
    ui_hide(TPH_TXT_REMAIN);
    ui_show(TPH_TXT_REMAIN);
    return 0;
}
int menu_tph_set_wbl(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "wbl";
    it.exdata = table_photo_white_balance_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":photo white_balance set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_col(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "col";
    it.exdata = table_photo_color_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":tph color set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_sca(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "sca";
    it.exdata = table_photo_quick_scan_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":photo quick_scan set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_pexp(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "pexp";
    it.exdata = table_photo_exposure_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":photo exposure set fail!\n");
        return -1;
    }
    return 0;
}
int menu_tph_set_iso(u8 sel_item)
{
    struct intent it;
    int err;

    init_intent(&it);
    it.name = "video_photo";
    it.action = ACTION_PHOTO_TAKE_SET_CONFIG;
    it.data = "iso";
    it.exdata = table_photo_iso_camera0[sel_item];
    err = start_app(&it);
    if (err) {
        ASSERT(err == 0, ":photo iso set fail!\n");
        return -1;
    }
    return 0;
}

/*
 *  * (end)拍照设置各项操作请求
 *   */
enum {
    PAGE_SHOW = 0,
    MODE_SW_EXIT,
    HOME_SW_EXIT,
};

enum TPH_MENU {
    TPH_MENU_NULL = 0,
    TPH_MENU_RESOLUTION,
    TPH_MENU_DELAY,
    TPH_MENU_CONTINUOUS,
    TPH_MENU_QUALITY,
    TPH_MENU_ACUITY,
    TPH_MENU_AWB,
    TPH_MENU_COLOR,
    TPH_MENU_ISO,
    TPH_MENU_EXPOSURE,
    TPH_MENU_TREMOR,
    TPH_MENU_PREVIEW,
    TPH_MENU_LABEL,
    TPH_MENU_HIDE,
};


void tph_menu_show(enum TPH_MENU item)
{
    if (__this->tph_menu_status == TPH_MENU_HIDE) {
        __this->tph_menu_status = TPH_MENU_NULL;
        return;
    }
    switch (item) {
    case TPH_MENU_NULL:
        __this->tph_menu_status = TPH_MENU_NULL;
        break;
    case TPH_MENU_RESOLUTION:
        if (__this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_PREVIEW &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_RES);
        ui_show(TPH_SET_PIC_SEL_1_1);
        ui_highlight_element_by_id(TPH_SET_PIC_1_1);
        ui_highlight_element_by_id(TPH_SET_TXT_1_1);
        ui_show(_TPH_SET_PIC_RES[__this->camera[0].resolution]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_DELAY:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_LABEL &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_PREVIEW &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_C4);
        ui_show(TPH_SET_PIC_SEL_1_2);
        ui_highlight_element_by_id(TPH_SET_PIC_1_2);
        ui_highlight_element_by_id(TPH_SET_TXT_1_2);
        ui_highlight_element_by_id(_TPH_SET_PIC_C4[__this->camera[0].tph_delay]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_CONTINUOUS:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_PREVIEW &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_C2);
        ui_show(TPH_SET_PIC_SEL_1_3);
        ui_highlight_element_by_id(TPH_SET_PIC_1_3);
        ui_highlight_element_by_id(TPH_SET_TXT_1_3);
        ui_highlight_element_by_id(_TPH_SET_PIC_C2[__this->camera[0].repeat]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_QUALITY:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_PREVIEW &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_C3);
        ui_show(TPH_SET_PIC_SEL_1_4);
        ui_highlight_element_by_id(TPH_SET_PIC_1_4);
        ui_highlight_element_by_id(TPH_SET_TXT_1_4);
        ui_text_show_index_by_id(TPH_SET_TXT_C3_1, 0);
        ui_text_show_index_by_id(TPH_SET_TXT_C3_2, 0);
        ui_text_show_index_by_id(TPH_SET_TXT_C3_3, 0);
        ui_highlight_element_by_id(_TPH_SET_PIC_C3[__this->camera[0].quality]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_ACUITY:
        if (__this->tph_menu_status != TPH_MENU_AWB &&
            __this->tph_menu_status != TPH_MENU_COLOR &&
            __this->tph_menu_status != TPH_MENU_ISO) {
            ui_hide(TPH_SET_LAY_3);
            ui_show(TPH_SET_LAY_SET_2);
        }
        ui_show(TPH_SET_LAY_C30);
        ui_show(TPH_SET_PIC_SEL_2_1);
        ui_highlight_element_by_id(TPH_SET_PIC_2_1);
        ui_highlight_element_by_id(TPH_SET_TXT_2_1);
        ui_text_show_index_by_id(TPH_SET_TXT_C30_1, 0);
        ui_text_show_index_by_id(TPH_SET_TXT_C30_2, 0);
        ui_text_show_index_by_id(TPH_SET_TXT_C30_3, 0);
        ui_highlight_element_by_id(_TPH_SET_PIC_C30[__this->camera[0].acutance]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_AWB:
        if (__this->tph_menu_status != TPH_MENU_ACUITY &&
            __this->tph_menu_status != TPH_MENU_COLOR &&
            __this->tph_menu_status != TPH_MENU_ISO) {
            ui_hide(TPH_SET_LAY_3);
            ui_show(TPH_SET_LAY_SET_2);
        }
        ui_show(TPH_SET_LAY_C50);
        ui_show(TPH_SET_PIC_SEL_2_2);
        ui_highlight_element_by_id(TPH_SET_PIC_2_2);
        ui_highlight_element_by_id(TPH_SET_TXT_2_2);
        ui_highlight_element_by_id(_TPH_SET_PIC_C50[__this->camera[0].white_balance]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_COLOR:
        if (__this->tph_menu_status != TPH_MENU_ACUITY &&
            __this->tph_menu_status != TPH_MENU_AWB &&
            __this->tph_menu_status != TPH_MENU_ISO) {
            ui_hide(TPH_SET_LAY_3);
            ui_show(TPH_SET_LAY_SET_2);
        }
        ui_show(TPH_SET_LAY_C30);
        ui_show(TPH_SET_PIC_SEL_2_3);
        ui_highlight_element_by_id(TPH_SET_PIC_2_3);
        ui_highlight_element_by_id(TPH_SET_TXT_2_3);
        ui_text_show_index_by_id(TPH_SET_TXT_C30_1, 1);
        ui_text_show_index_by_id(TPH_SET_TXT_C30_2, 1);
        ui_text_show_index_by_id(TPH_SET_TXT_C30_3, 1);
        ui_highlight_element_by_id(_TPH_SET_PIC_C30[__this->camera[0].color]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_ISO:
        if (__this->tph_menu_status != TPH_MENU_ACUITY &&
            __this->tph_menu_status != TPH_MENU_AWB &&
            __this->tph_menu_status != TPH_MENU_COLOR) {
            ui_hide(TPH_SET_LAY_3);
            ui_show(TPH_SET_LAY_SET_2);
        }
        ui_show(TPH_SET_LAY_C40);
        ui_show(TPH_SET_PIC_SEL_2_4);
        ui_highlight_element_by_id(TPH_SET_PIC_2_4);
        ui_highlight_element_by_id(TPH_SET_TXT_2_4);
        ui_highlight_element_by_id(_TPH_SET_PIC_C40[__this->camera[0].iso]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_EXPOSURE:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_PREVIEW &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_EXP);
        ui_show(TPH_SET_PIC_SEL_3_1);
        ui_highlight_element_by_id(TPH_SET_PIC_3_1);
        ui_highlight_element_by_id(TPH_SET_TXT_3_1);
        ui_show(_TPH_SET_PIC_EXP[(6 - __this->camera[0].exposure)]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_TREMOR:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_PREVIEW &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_C2);
        ui_show(TPH_SET_PIC_SEL_3_2);
        ui_highlight_element_by_id(TPH_SET_PIC_3_2);
        ui_highlight_element_by_id(TPH_SET_TXT_3_2);
        ui_highlight_element_by_id(_TPH_SET_PIC_C2[__this->camera[0].hand_shake]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_PREVIEW:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_C3);
        ui_show(TPH_SET_PIC_SEL_3_3);
        ui_highlight_element_by_id(TPH_SET_PIC_3_3);
        ui_highlight_element_by_id(TPH_SET_TXT_3_3);
        ui_text_show_index_by_id(TPH_SET_TXT_C3_1, 1);
        ui_text_show_index_by_id(TPH_SET_TXT_C3_2, 1);
        ui_text_show_index_by_id(TPH_SET_TXT_C3_3, 1);
        ui_highlight_element_by_id(_TPH_SET_PIC_C3[__this->camera[0].quick_scan]);
        __this->tph_menu_status = item;
        break;
    case TPH_MENU_LABEL:
        if (__this->tph_menu_status != TPH_MENU_RESOLUTION &&
            __this->tph_menu_status != TPH_MENU_DELAY &&
            __this->tph_menu_status != TPH_MENU_CONTINUOUS &&
            __this->tph_menu_status != TPH_MENU_QUALITY &&
            __this->tph_menu_status != TPH_MENU_EXPOSURE &&
            __this->tph_menu_status != TPH_MENU_TREMOR &&
            __this->tph_menu_status != TPH_MENU_PREVIEW) {
            ui_hide(TPH_SET_LAY_2);
            ui_show(TPH_SET_LAY_SET_1);
        }
        ui_show(TPH_SET_LAY_C2);
        ui_show(TPH_SET_PIC_SEL_3_4);
        ui_highlight_element_by_id(TPH_SET_PIC_3_4);
        ui_highlight_element_by_id(TPH_SET_TXT_3_4);
        ui_highlight_element_by_id(_TPH_SET_PIC_C2[__this->camera[0].dat_label]);
        __this->tph_menu_status = item;
        break;
    default:
        break;
    }
}


void tph_menu_hide(enum TPH_MENU item)
{
    switch (__this->tph_menu_status) {
    case TPH_MENU_NULL:
        break;
    case TPH_MENU_RESOLUTION:
        ui_no_highlight_element_by_id(TPH_SET_PIC_1_1);
        ui_no_highlight_element_by_id(TPH_SET_TXT_1_1);
        if (item != TPH_MENU_DELAY &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_PREVIEW &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_1_1);
            ui_hide(TPH_SET_LAY_RES);
        }
        break;
    case TPH_MENU_DELAY:
        ui_no_highlight_element_by_id(TPH_SET_PIC_1_2);
        ui_no_highlight_element_by_id(TPH_SET_TXT_1_2);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_PREVIEW &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_1_2);
            ui_hide(TPH_SET_LAY_C4);
        }
        break;
    case TPH_MENU_CONTINUOUS:
        ui_no_highlight_element_by_id(TPH_SET_PIC_1_3);
        ui_no_highlight_element_by_id(TPH_SET_TXT_1_3);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_DELAY &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_PREVIEW &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_1_3);
            ui_hide(TPH_SET_LAY_C2);
        }
        break;
    case TPH_MENU_QUALITY:
        ui_no_highlight_element_by_id(TPH_SET_PIC_1_4);
        ui_no_highlight_element_by_id(TPH_SET_TXT_1_4);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_DELAY &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_PREVIEW &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_1_4);
            ui_hide(TPH_SET_LAY_C3);
        }
        break;
    case TPH_MENU_ACUITY:
        ui_no_highlight_element_by_id(TPH_SET_PIC_2_1);
        ui_no_highlight_element_by_id(TPH_SET_TXT_2_1);
        if (item != TPH_MENU_AWB &&
            item != TPH_MENU_COLOR &&
            item != TPH_MENU_ISO) {
            ui_hide(TPH_SET_LAY_SET_2);
            ui_show(TPH_SET_LAY_3);
        } else {
            ui_hide(TPH_SET_PIC_SEL_2_1);
            ui_hide(TPH_SET_LAY_C30);
        }
        break;
    case TPH_MENU_AWB:
        ui_no_highlight_element_by_id(TPH_SET_PIC_2_2);
        ui_no_highlight_element_by_id(TPH_SET_TXT_2_2);
        if (item != TPH_MENU_ACUITY &&
            item != TPH_MENU_COLOR &&
            item != TPH_MENU_ISO) {
            ui_hide(TPH_SET_LAY_SET_2);
            ui_show(TPH_SET_LAY_3);
        } else {
            ui_hide(TPH_SET_PIC_SEL_2_2);
            ui_hide(TPH_SET_LAY_C50);
        }
        break;
    case TPH_MENU_COLOR:
        ui_no_highlight_element_by_id(TPH_SET_PIC_2_3);
        ui_no_highlight_element_by_id(TPH_SET_TXT_2_3);
        if (item != TPH_MENU_ACUITY &&
            item != TPH_MENU_AWB &&
            item != TPH_MENU_ISO) {
            ui_hide(TPH_SET_LAY_SET_2);
            ui_show(TPH_SET_LAY_3);
        } else {
            ui_hide(TPH_SET_PIC_SEL_2_3);
            ui_hide(TPH_SET_LAY_C30);
        }
        break;
    case TPH_MENU_ISO:
        ui_no_highlight_element_by_id(TPH_SET_PIC_2_4);
        ui_no_highlight_element_by_id(TPH_SET_TXT_2_4);
        if (item != TPH_MENU_ACUITY &&
            item != TPH_MENU_AWB &&
            item != TPH_MENU_COLOR) {
            ui_hide(TPH_SET_LAY_SET_2);
            ui_show(TPH_SET_LAY_3);
        } else {
            ui_hide(TPH_SET_PIC_SEL_2_4);
            ui_hide(TPH_SET_LAY_C40);
        }
        break;
    case TPH_MENU_EXPOSURE:
        ui_no_highlight_element_by_id(TPH_SET_PIC_3_1);
        ui_no_highlight_element_by_id(TPH_SET_TXT_3_1);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_DELAY &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_PREVIEW &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_3_1);
            ui_hide(TPH_SET_LAY_EXP);
        }
        break;
    case TPH_MENU_TREMOR:
        ui_no_highlight_element_by_id(TPH_SET_PIC_3_2);
        ui_no_highlight_element_by_id(TPH_SET_TXT_3_2);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_DELAY &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_PREVIEW &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_3_2);
            ui_hide(TPH_SET_LAY_C2);
        }
        break;
    case TPH_MENU_PREVIEW:
        ui_no_highlight_element_by_id(TPH_SET_PIC_3_3);
        ui_no_highlight_element_by_id(TPH_SET_TXT_3_3);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_DELAY &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_LABEL) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_3_3);
            ui_hide(TPH_SET_LAY_C3);
        }
        break;
    case TPH_MENU_LABEL:
        ui_no_highlight_element_by_id(TPH_SET_PIC_3_4);
        ui_no_highlight_element_by_id(TPH_SET_TXT_3_4);
        if (item != TPH_MENU_RESOLUTION &&
            item != TPH_MENU_DELAY &&
            item != TPH_MENU_CONTINUOUS &&
            item != TPH_MENU_QUALITY &&
            item != TPH_MENU_EXPOSURE &&
            item != TPH_MENU_TREMOR &&
            item != TPH_MENU_PREVIEW) {
            ui_hide(TPH_SET_LAY_SET_1);
            ui_show(TPH_SET_LAY_2);
        } else {
            ui_hide(TPH_SET_PIC_SEL_3_4);
            ui_hide(TPH_SET_LAY_C2);
        }
        break;
    default:
        __this->onkey_mod = 2;
        __this->tph_menu_status = TPH_MENU_NULL;
        break;
    }

    if (item == __this->tph_menu_status) {
        __this->tph_menu_status = TPH_MENU_HIDE;
    }
}

int tph_menu(int item)
{
    tph_menu_hide(item);
    tph_menu_show(item);
    return 0;
}
/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_POWER_OFF = 1,
    BOX_MSG_NO_POWER,
    BOX_MSG_MEM_ERR,
    BOX_MSG_NO_MEM,
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
            ui_hide(TPH_LAY_MESSAGEBOX);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(TPH_LAY_MESSAGEBOX);
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
            ui_show(TPH_LAY_MESSAGEBOX);
            ui_text_show_index_by_id(TPH_TXT_MESSAGEBOX, msg - 1);
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
            ui_show(TPH_LAY_MESSAGEBOX);
        }
        ui_text_show_index_by_id(TPH_TXT_MESSAGEBOX, msg - 1);
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
        puts("tph do not allow ui to open menu.\n");
        return -1;
    } else if (!strcmp(it.data, "opMENU:en")) {
        puts("tph allow ui to open menu.\n");
    } else {
        ASSERT(0, "opMENU err\n");
    }
    return 0;
}

static int disp_TphSetting_lay(u8 menu_status)
{
    if (menu_status) {
        __this->menu_status = 1;
        __this->tph_menu_status = TPH_MENU_NULL;
        ui_hide(TPH_LAY_TPH);
        ui_show(TPH_SET_WIN);
        ui_highlight_element_by_id(TPH_PIC_SETTING);
    } else {
        ui_hide(TPH_SET_WIN);
        ui_show(TPH_LAY_TPH);
        if (av_in_statu) {
            ui_show(TPH_BTN_SWITCH);
        }
        if (__this->onkey_mod == 0) {
            ui_no_highlight_element_by_id(TPH_PIC_SETTING);
        } else {
            ui_highlight_element_by_id(TPH_PIC_SETTING);
        }
        __this->menu_status = 0;
        __this->tph_menu_status = TPH_MENU_NULL;
    }
    return 0;
}

static void open_screen(void *p)
{
    lcd_backlight_ctrl(true);
}
static int tph_take_photo_in_handler(const char *type, u32 arg)
{
    if (__this->camera[0].tph_delay == 0 || __this->camera[0].repeat == 1) {
        sys_touch_event_disable();
        sys_key_event_disable();
    }
    __this->take_photo_busy = 1;
    __tph_msg_hide(0);
    return 0;
}
static int tph_take_photo_out_handler(const char *type, u32 arg)
{
    /* os_time_dly(50); */
    ui_hide(TPH_TXT_REMAIN);
    ui_show(TPH_TXT_REMAIN);
    __this->take_photo_busy = 0;
//    if (__this->camera[0].tph_delay == 0 || __this->camera[0].repeat == 1) {   //��ʱ���հ�������Ӧ
        sys_touch_event_enable();
        sys_key_event_enable();
//    }

    return 0;
}

static int tph_take_photo_end_handler(const char *type, u32 arg)
{
    //闪动
    //lcd_backlight_ctrl(false);
    //sys_timeout_add(NULL, open_screen, 50);

#if TPH_REMAIN_DISCAL
    if (__this->remain_num) {
        __this->remain_num--;
    }
#endif

    return 0;
}
static int tph_camera_sw_handler(const char *type, u32 args)
{

    ASSERT(type != NULL, "tph_camera_sw_handler err.");
    printf("tph_camera_sw type: %s = %d.\n", type, args);

    if (!strcmp(type, "id")) {
        __this->cur_camera_id = args;
        ui_hide(TPH_TXT_REMAIN);
        ui_show(TPH_TXT_REMAIN);
        __this->camera[1].resolution = 0; /* VGA */
        ui_pic_show_image_by_id(TPH_PIC_RESOLUTION, __this->camera[__this->cur_camera_id].resolution);
    }
    return 0;
}
static int tph_delay_take_handler(const char *type, u32 arg)
{
    puts("tph_delay_take_handler.\n");
    __this->take_photo_busy = 1;
    __tph_msg_hide(0);
    if (!strcmp(type, "ms")) {
        printf("ms:%d\n", arg);
        ui_show(TPH_LAY_CONUTDOWN);
        switch (arg) {
        case 2000:
            ui_show(TPH_ANI_COUNTDOWN_2);
            break;
        case 5000:
            ui_show(TPH_ANI_COUNTDOWN_5);
            break;
        case 10000:
            ui_show(TPH_ANI_COUNTDOWN_10);
            break;
        }
    }
    sys_touch_event_disable();
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
static int tph_av_in_handler(const char *type, u32 arg)
{
    av_in_statu = 1;
    ui_show(TPH_BTN_SWITCH);
    return 0;
}
static int tph_av_off_handler(const char *type, u32 arg)
{
    av_in_statu = 0;
    ui_hide(TPH_BTN_SWITCH);
    return 0;
}

extern void play_voice_file(const char *file_name);
static int tph_headlight_on_handler(const char *type, u32 arg)
{
    puts("tph_headlight_on_handler\n");
    if (__this->hlight_show_status == 0) {
        __this->hlight_show_status = 1;

        ui_show(TPH_LAY_FLIG);//show head light
        play_voice_file("mnt/spiflash/audlogo/olight.adp");
    }
    return 0;
}
static int tph_headlight_off_handler(const char *type, u32 arg)
{
    puts("tph_headlight_off_handler\n");
    ui_hide(TPH_LAY_FLIG);//hide head light
    __this->hlight_show_status = 0;
    return 0;
}

/*
 * 拍照模式的APP状态响应回调
 */
static const struct uimsg_handl tph_msg_handler[] = {

    { "swCAM",      tph_camera_sw_handler             },
    { "fsErr",      tph_fs_err_handler                },
    { "noCard",     tph_no_card_handler               },
    { "tphin",      tph_take_photo_in_handler         },
    { "tphout",     tph_take_photo_out_handler        },
    { "tphend",     tph_take_photo_end_handler        },
    { "dlyTPH",     tph_delay_take_handler            },
    { "avin",       tph_av_in_handler                 },
    { "avoff",      tph_av_off_handler                },
    { "HlightOn",   tph_headlight_on_handler          },
    { "HlightOff",  tph_headlight_off_handler         },
    /* { NULL, NULL } */
};

static void avin_event_handler(struct sys_event *event, void *priv)
{
    puts("avin event handler\n");
    if (!strcmp(event->arg, "video1") || !strncmp((char *)event->arg, "uvc", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            av_in_statu = 1;
            ui_show(TPH_BTN_SWITCH);
            break;
        case DEVICE_EVENT_OUT:
            av_in_statu = 0;
            ui_hide(TPH_BTN_SWITCH);
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
            ui_hide(TPH_PIC_SD);
            ui_hide(TPH_TXT_REMAIN);
            ui_show(TPH_TXT_REMAIN);
            break;
        case DEVICE_EVENT_OUT:
        //case DEVICE_EVENT_OFFLINE:
            ui_show(TPH_PIC_SD);
            ui_show(TPH_TXT_REMAIN);
            break;
        default:
            break;
        }
    }
}
/***************************** 拍照按钮显示 ************************************/
static int tph_tph_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->onkey_mod == 0) {
            ui_highlight_element_by_id(TPH_PIC_TPH);
        } else {
            ui_no_highlight_element_by_id(TPH_PIC_TPH);
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(TPH_LAY_TPH)
.onchange = tph_tph_onchange,
};
/***************************** 拍照设置显示 ************************************/
static int tph_set_onchange(void *ctr, enum element_change_event e, void *arg)
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

REGISTER_UI_EVENT_HANDLER(TPH_SET_LAY)
.onchange = tph_set_onchange,
};
/***************************** SD 卡图标动作 ************************************/
static int pic_sd_tph_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(TPH_PIC_SD)
.onchange = pic_sd_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static char remain_num_str[10];

static int text_tph_remain_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    struct intent it;
    int err;
    switch (e) {
    case ON_CHANGE_INIT:
        if (storage_device_ready() == 0) {
            text->str = "00000";
#if TPH_REMAIN_DISCAL
            __this->remain_num = 0;
#endif
            return TRUE;
        }
        break;
    case ON_CHANGE_SHOW_PROBE:
        puts("tph_remain_num show probe.\n");

        if (storage_device_ready() == 0) {
            text->str = "00000";
#if TPH_REMAIN_DISCAL
            __this->remain_num = 0;
#endif
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

#if TPH_REMAIN_DISCAL
            if (!__this->remain_num) {
                ASCII_StrToInt(remain_num_str, &__this->remain_num, strlen(remain_num_str));
            } else {
                sprintf(remain_num_str, "%05d", __this->remain_num);
            }
#endif

            printf("remain_num_str: %s\n", remain_num_str);
            text->str = remain_num_str;
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(TPH_TXT_REMAIN)
.onchange = text_tph_remain_num_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
/***************************** 拍照系统日期控件动作 ************************************/
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
REGISTER_UI_EVENT_HANDLER(TPH_TIM_TIME)
.onchange = timer_sys_date_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int ani_delay_2s_onchange(void *_ani, enum element_change_event e, void *arg)
{
    UI_ONTOUCH_DEBUG("ani_delay_2s_onchange: %d\n", e);
    if (e == ON_CHANGE_ANIMATION_END || (e == ON_CHANGE_SHOW_PROBE && msg_show_f)) {
        //动画播完或有提示框时隐藏倒计时动画
        ui_hide(TPH_ANI_COUNTDOWN_2);
        ui_hide(TPH_LAY_CONUTDOWN);
        sys_touch_event_enable();
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(TPH_ANI_COUNTDOWN_2)
.onchange = ani_delay_2s_onchange,
};
static int ani_delay_5s_onchange(void *_ani, enum element_change_event e, void *arg)
{
    UI_ONTOUCH_DEBUG("ani_delay_5s_onchange: %d\n", e);
    if (e == ON_CHANGE_ANIMATION_END || (e == ON_CHANGE_SHOW_PROBE && msg_show_f)) {
        //动画播完或有提示框时隐藏倒计时动画
        ui_hide(TPH_ANI_COUNTDOWN_5);
        ui_hide(TPH_LAY_CONUTDOWN);
        sys_touch_event_enable();
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(TPH_ANI_COUNTDOWN_5)
.onchange = ani_delay_5s_onchange,
};
static int ani_delay_10s_onchange(void *_ani, enum element_change_event e, void *arg)
{
    UI_ONTOUCH_DEBUG("ani_delay_10s_onchange: %d\n", e);
    if (e == ON_CHANGE_ANIMATION_END || (e == ON_CHANGE_SHOW_PROBE && msg_show_f)) {
        //动画播完或有提示框时隐藏倒计时动画
        ui_hide(TPH_ANI_COUNTDOWN_10);
        ui_hide(TPH_LAY_CONUTDOWN);
        sys_touch_event_enable();
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(TPH_ANI_COUNTDOWN_10)
.onchange = ani_delay_10s_onchange,
};


static void battery_event_handler(struct sys_event *event, void *priv)
{
    static u8 ten_sec_off = 0;
    if (event->type == SYS_KEY_EVENT || event->type == SYS_TOUCH_EVENT) {
        if (ten_sec_off) {
            ten_sec_off = 0;
            __tph_msg_hide(0);
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
                    __tph_msg_hide(0);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
                __tph_msg_show(BOX_MSG_POWER_OFF, 0);
                ten_sec_off = 1;
            }
        }
    }
}
static void no_power_msg_box_timer(void *priv)
{
    static u32 cnt = 0;
    if (__this->battery_val <= 20
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

/***************************** 电池控件动作 ************************************/
static int battery_tph_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(TPH_BAT)
.onchange = battery_tph_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/*****************************拍照模式页面回调 ************************************/
int tph_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    int ret;
    /* printf("^^^ tph_mode_onchange %d^^^\r\n", e); */

    switch (e) {
    case ON_CHANGE_INIT:
        /* 注册APP消息响应 */
        ui_register_msg_handler(ID_WINDOW_VIDEO_TPH, tph_msg_handler);
        __this->cur_camera_id = 0;
        sys_cur_mod = 2;  /* 1:rec, 2:tph, 3:dec */
        memset(__this, 0, sizeof_this);
        __this->camera[0].hand_shake = db_select("sok");
        __this->camera[0].dat_label = db_select("pdat");
        __this->camera[0].repeat = db_select("cyt");
        __this->camera[0].quality = index_of_table8(db_select("qua"), TABLE(table_photo_quality_camera0));
        __this->camera[0].tph_delay = index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0));
        __this->camera[0].acutance = index_of_table8(db_select("acu"), TABLE(table_photo_acutance_camera0));
        __this->camera[0].resolution = index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0));
        __this->camera[0].white_balance = index_of_table8(db_select("wbl"), TABLE(table_photo_white_balance_camera0));
        __this->camera[0].color = index_of_table8(db_select("col"), TABLE(table_photo_color_camera0));
        __this->camera[0].quick_scan = index_of_table8(db_select("sca"), TABLE(table_photo_quick_scan_camera0));
        __this->camera[0].exposure = index_of_table8(db_select("pexp"), TABLE(table_photo_exposure_camera0));
        __this->camera[0].iso = index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0));
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    case ON_CHANGE_RELEASE:
        /* ui_hide(ID_WINDOW_VIDEO_SYS); */
        if (__this->menu_status) {
            ui_hide(TPH_SET_WIN);
        }
        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE);
        }
        __tph_msg_hide(0);//强制隐藏消息框
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_TPH)
.onchange = tph_mode_onchange,
 .ontouch = NULL,
};
/*****************************布局up回调 ************************************/
static int tph_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW: /* 在此获取默认隐藏的图标的状态并显示 */

        if (dev_online("uvc") || dev_online("video1")) {
            av_in_statu = 1;
            ui_show(TPH_BTN_SWITCH);
        }
        if (__this->camera[0].tph_delay != 0) {
            ui_pic_show_image_by_id(TPH_PIC_DELAY, __this->camera[0].tph_delay - 1);
        }
        printf("\n__this->camera[0].res===%d\n", __this->camera[0].resolution);
        ui_pic_show_image_by_id(TPH_PIC_RESOLUTION, __this->camera[0].resolution);
        ui_pic_show_image_by_id(TPH_PIC_QUALITY, 2 - __this->camera[0].quality);
        ui_pic_show_image_by_id(TPH_PIC_WB, __this->camera[0].white_balance);
        ui_pic_show_image_by_id(TPH_PIC_ISO, __this->camera[0].iso);
        if (__this->camera[0].hand_shake != 0) {
            ui_show(TPH_PIC_HS);
        }
        if (__this->camera[0].repeat != 0) {
            ui_show(TPH_PIC_TRIPLE);
        }

        if (storage_device_ready() == 0) {
            ui_show(TPH_PIC_SD);
        }

        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TPH_LAY)
.onchange = tph_layout_up_onchange,
};
static int tph_layout_button_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**tph layout button ontouch**");
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
            disp_TphSetting_lay(0);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_BTN_BASE)
.ontouch = tph_layout_button_ontouch,
};
static int ani_headlight_onchange(void *_ani, enum element_change_event e, void *arg)
{
    UI_ONTOUCH_DEBUG("ani_headlight_onchange: %d\n", e);
    switch (e) {
    case ON_CHANGE_SHOW_PROBE:
        if (msg_show_f && __this->hlight_show_status) {
            //有提示框时隐藏前照灯
            ui_hide(TPH_LAY_FLIG);//hide head light
        }
        break;
    default:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_ANI_FLIG)
.onchange = ani_headlight_onchange,
};
/***************************** 拍照设置按钮动作 ************************************/
static int tph_set_ontouch(void *ctr, struct element_touch_event *e)
{
    static u8 last_onkey_mod = 0;
    UI_ONTOUCH_DEBUG("**tph set ontouch**");
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
            disp_TphSetting_lay(0);
            if (__this->onkey_mod == 0) {
                tph_tell_app_exit_menu();
            }
        } else {
            if (tph_ask_app_open_menu() == (int) - 1) {
                return -1;
            }
            last_onkey_mod = __this->onkey_mod;
            __this->onkey_mod = 2;
            __this->onkey_sel = 0;
            disp_TphSetting_lay(1);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_BTN_SETTING)
.ontouch = tph_set_ontouch,
};
/***************************** 拍照开始结束动作 ************************************/
static void tph_control_ok(void *p, int err)
{
    if (err == 0) {
        puts("---tph control ok\n");
    } else {
        printf("---tph control faild: %d\n", err);
    }
}
static int tph_control_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**tph control ontouch**");
    struct intent it;
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
        if (__this->onkey_mod == 1) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(TPH_PIC_TPH);
            tph_tell_app_exit_menu();
            __this->onkey_mod = 0;
            __this->onkey_sel = 0;
        }
        it.name = "video_photo";
        it.action = ACTION_PHOTO_TAKE_CONTROL;
        start_app_async(&it, tph_control_ok, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_PIC_TPH)
.ontouch = tph_control_ontouch,
};
/***************************** 切换镜头按钮动作 ************************************/
static int tph_switch_onchange(void *ctr, enum element_change_event e, void *arg)
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
static int tph_switch_cam_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**tph switch camera ontouch**");
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
        it.name = "video_photo";
        it.action = ACTION_PHOTO_TAKE_SWITCH_WIN;
        start_app(&it);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_BTN_SWITCH)
.ontouch = tph_switch_cam_ontouch,
 .onchange = tph_switch_onchange,
};
/***************************** 返回HOME按钮动作 ************************************/
static int tph_backhome_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**tph back to home ontouch**");
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
            __this->page_exit = HOME_SW_EXIT;
            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_BTN_HOME)
.ontouch = tph_backhome_ontouch,
};
static int tph_to_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**tph  to rec ontouch**");
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
        __this->page_exit = MODE_SW_EXIT;
        init_intent(&it);
        app = get_current_app();
        if (app) {
            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); //不等待直接启动app

            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_MAIN;
            start_app_async(&it, NULL, NULL); //不等待直接启动app
        }
        malloc_stats();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_BTN_PHOTO)
.ontouch = tph_to_rec_ontouch,
};

/*********************************************************************************
 *  *                           菜单动作
 *********************************************************************************/
static int menu_photo_res_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**res menu ontouch**");
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
        __this->camera[0].resolution = index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0));
        if (__this->tph_menu_status == TPH_MENU_RESOLUTION) {
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
        ui_set_call(tph_menu, TPH_MENU_RESOLUTION);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_1_1)
.ontouch = menu_photo_res_ontouch,
};

static int menu_photo_mode_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo mode  ontouch**");
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
        __this->camera[0].tph_delay = index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0));
        if (__this->tph_menu_status == TPH_MENU_DELAY) {
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
        ui_set_call(tph_menu, TPH_MENU_DELAY);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_1_2)
.ontouch = menu_photo_mode_ontouch,
};
static int menu_photo_continue_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo continue ontouch**");
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
        __this->camera[0].repeat = db_select("cyt");
        if (__this->tph_menu_status == TPH_MENU_CONTINUOUS) {
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
        ui_set_call(tph_menu, TPH_MENU_CONTINUOUS);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_1_3)
.ontouch = menu_photo_continue_ontouch,
};

static int menu_photo_quality_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo quality ontouch**");
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
        __this->camera[0].quality = index_of_table8(db_select("qua"), TABLE(table_photo_quality_camera0));
        if (__this->tph_menu_status == TPH_MENU_QUALITY) {
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
        ui_set_call(tph_menu, TPH_MENU_QUALITY);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_1_4)
.ontouch = menu_photo_quality_ontouch,
};
static int menu_photo_acu_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo acutance ontouch**");
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
        __this->camera[0].acutance = index_of_table8(db_select("acu"), TABLE(table_photo_acutance_camera0));
        if (__this->tph_menu_status == TPH_MENU_ACUITY) {
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
        ui_set_call(tph_menu, TPH_MENU_ACUITY);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_2_1)
.ontouch = menu_photo_acu_ontouch,
};
static int menu_photo_wbl_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo white_balance ontouch**");
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
        __this->camera[0].white_balance = index_of_table8(db_select("wbl"), TABLE(table_photo_white_balance_camera0));
        if (__this->tph_menu_status == TPH_MENU_AWB) {
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
        ui_set_call(tph_menu, TPH_MENU_AWB);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_2_2)
.ontouch = menu_photo_wbl_ontouch,
};
static int menu_photo_color_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo color ontouch**");
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
        __this->camera[0].color = index_of_table8(db_select("col"), TABLE(table_photo_color_camera0));
        if (__this->tph_menu_status == TPH_MENU_COLOR) {
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
        ui_set_call(tph_menu, TPH_MENU_COLOR);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_2_3)
.ontouch = menu_photo_color_ontouch,
};

static int menu_photo_iso_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo iso  ontouch**");
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
        __this->camera[0].iso = index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0));
        if (__this->tph_menu_status == TPH_MENU_ISO) {
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
        ui_set_call(tph_menu, TPH_MENU_ISO);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_2_4)
.ontouch = menu_photo_iso_ontouch,
};
static int menu_photo_exp_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo exp  ontouch**");
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
        __this->camera[0].exposure = index_of_table8(db_select("pexp"), TABLE(table_photo_exposure_camera0));
        if (__this->tph_menu_status == TPH_MENU_EXPOSURE) {
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
        ui_set_call(tph_menu, TPH_MENU_EXPOSURE);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_3_1)
.ontouch = menu_photo_exp_ontouch,
};
static int menu_photo_sok_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo shake  ontouch**");
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
        __this->camera[0].hand_shake = db_select("sok");
        if (__this->tph_menu_status == TPH_MENU_TREMOR) {
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
        ui_set_call(tph_menu, TPH_MENU_TREMOR);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_3_2)
.ontouch = menu_photo_sok_ontouch,
};
static int menu_photo_preview_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo preview  ontouch**");
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
        __this->camera[0].quick_scan = index_of_table8(db_select("sca"), TABLE(table_photo_quick_scan_camera0));
        if (__this->tph_menu_status == TPH_MENU_PREVIEW) {
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
        ui_set_call(tph_menu, TPH_MENU_PREVIEW);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_3_3)
.ontouch = menu_photo_preview_ontouch,
};
static int menu_photo_datlable_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**photo date lable  ontouch**");
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
        __this->camera[0].dat_label = db_select("pdat");
        if (__this->tph_menu_status == TPH_MENU_LABEL) {
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
        ui_set_call(tph_menu, TPH_MENU_LABEL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_3_4)
.ontouch = menu_photo_datlable_ontouch,
};

/*********************************************************************************
 *  *                          子菜单动作
* *********************************************************************************/
/* *******************************图像大小控件动作*********************************/
#define SLID_RES_X    820 //滑块x起始绝对坐标
#define SLID_RES_GAP  40  //每一项的间隔
#define SLID_RES_ITEM 7   //项目数
int menu_tph_res_ontouch(void *arg1, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("** res ontouch  ");
    static s16 x_pos_down = 0;
    static s16 old_res = 0;
    s16 tmp;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        x_pos_down = e->pos.x;
        old_res = __this->camera[0].resolution;
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        s16 x_pos_now = e->pos.x;
        s16 x_pos_ch = x_pos_now -  x_pos_down;
        tmp = __this->camera[0].resolution;
        if (x_pos_ch < SLID_RES_GAP && x_pos_ch > -SLID_RES_GAP) {
            return false;
        }
        tmp = old_res + x_pos_ch / SLID_RES_GAP;
        if (tmp > SLID_RES_ITEM - 1) {
            tmp = SLID_RES_ITEM - 1;
            x_pos_down = x_pos_now;
            old_res = SLID_RES_ITEM - 1;
        } else if (tmp < 0) {
            tmp = 0;
            x_pos_down = x_pos_now;
            old_res = 0;
        }
        printf("\n tmp_res = %d \n", tmp);
        if (__this->camera[0].resolution == tmp) {
            return false;
        }
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        int i;
        tmp = __this->camera[0].resolution;
        for (i = 1; i <= SLID_RES_ITEM; i++) {
            if (e->pos.x - SLID_RES_X < SLID_RES_GAP * i && e->pos.x > SLID_RES_X + SLID_RES_GAP * (i - 1)) {
                tmp = i - 1;
            }
        }
        if (__this->camera[0].resolution == tmp) {
            return false;
        }
        break;
    }
    ui_hide(_TPH_SET_PIC_RES[__this->camera[0].resolution]);
    ui_show(_TPH_SET_PIC_RES[tmp]);
    menu_tph_set_pres(tmp);
    __this->camera[0].resolution = tmp;
#if TPH_REMAIN_DISCAL
    __this->remain_num = 0;
#endif
    if (__this->cur_camera_id == 0) {
        ui_pic_show_image_by_id(TPH_PIC_RESOLUTION, __this->camera[0].resolution);
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_RES)
.ontouch = menu_tph_res_ontouch,
};

static int menu_tph_c4_ontouch(void *_ctr, struct element_touch_event *e)
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
            if (ctr->elm.id == _TPH_SET_PIC_C4[i]) {
                sel_item = i;
                break;
            }
        }
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        switch (__this->tph_menu_status) {
        case TPH_MENU_DELAY:
            if (__this->camera[0].tph_delay == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C4[__this->camera[0].tph_delay]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C4[sel_item]);
            if (__this->camera[0].tph_delay == 0) {
                ui_show(TPH_PIC_DELAY);
            }
            menu_tph_set_phm(sel_item);
            __this->camera[0].tph_delay = sel_item;
            if (sel_item == 0) {
                ui_hide(TPH_PIC_DELAY);
            } else {
                ui_pic_show_image_by_id(TPH_PIC_DELAY, sel_item - 1);
            }
            break;
        default:
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C4_1)
.ontouch = menu_tph_c4_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C4_2)
.ontouch = menu_tph_c4_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C4_3)
.ontouch = menu_tph_c4_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C4_4)
.ontouch = menu_tph_c4_ontouch,
};


static int menu_tph_c2_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c2 ontouch**");
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
            if (ctr->elm.id == _TPH_SET_PIC_C2[i]) {
                sel_item = i;
                break;
            }
        }
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        switch (__this->tph_menu_status) {
        case TPH_MENU_CONTINUOUS:
            if (__this->camera[0].repeat == sel_item) {
                return false;
            }
            menu_tph_set_cyt(sel_item);
            __this->camera[0].repeat = sel_item;
            if (sel_item) {
                ui_show(TPH_PIC_TRIPLE);
            } else {
                ui_hide(TPH_PIC_TRIPLE);
            }
            break;
        case TPH_MENU_TREMOR:
            if (__this->camera[0].hand_shake == sel_item) {
                return false;
            }
            menu_tph_set_sok(sel_item);
            __this->camera[0].hand_shake = sel_item;
            if (sel_item) {
                ui_show(TPH_PIC_HS);
            } else {
                ui_hide(TPH_PIC_HS);
            }
            break;
        case TPH_MENU_LABEL:
            if (__this->camera[0].dat_label == sel_item) {
                return false;
            }
            menu_tph_set_pdat(sel_item);
            __this->camera[0].dat_label = sel_item;
            break;
        default:
            return false;
            break;
        }
        ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[!sel_item]);
        ui_highlight_element_by_id(_TPH_SET_PIC_C2[sel_item]);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C2_1)
.ontouch = menu_tph_c2_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C2_2)
.ontouch = menu_tph_c2_ontouch,
};

static int menu_tph_c3_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c3 ontouch**");
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
        for (int i = 0; i < 3; i++) {
            if (ctr->elm.id == _TPH_SET_PIC_C3[i]) {
                sel_item = i;
                break;
            }
        }
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        switch (__this->tph_menu_status) {
        case TPH_MENU_QUALITY:
            if (__this->camera[0].quality == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C3[__this->camera[0].quality]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C3[sel_item]);
            menu_tph_set_qua(sel_item);
            __this->camera[0].quality = sel_item;
            ui_pic_show_image_by_id(TPH_PIC_QUALITY, 2 - __this->camera[0].quality);
            break;
        case TPH_MENU_PREVIEW:
            if (__this->camera[0].quick_scan == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C3[__this->camera[0].quick_scan]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C3[sel_item]);
            menu_tph_set_sca(sel_item);
            __this->camera[0].quick_scan = sel_item;
            break;
        default:
            return false;
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C3_1)
.ontouch = menu_tph_c3_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C3_2)
.ontouch = menu_tph_c3_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C3_3)
.ontouch = menu_tph_c3_ontouch,
};

static int menu_tph_c30_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c30 ontouch**");
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
        for (int i = 0; i < 3; i++) {
            if (ctr->elm.id == _TPH_SET_PIC_C30[i]) {
                sel_item = i;
                break;
            }
        }
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        switch (__this->tph_menu_status) {
        case TPH_MENU_ACUITY:
            if (__this->camera[0].acutance == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C30[__this->camera[0].acutance]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C30[sel_item]);
            menu_tph_set_acu(sel_item);
            __this->camera[0].acutance = sel_item;
            break;
        case TPH_MENU_COLOR:
            if (__this->camera[0].color == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C30[__this->camera[0].color]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C30[sel_item]);
            menu_tph_set_col(sel_item);
            __this->camera[0].color = sel_item;
            break;
        default:
            return false;
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C30_1)
.ontouch = menu_tph_c30_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C30_2)
.ontouch = menu_tph_c30_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C30_3)
.ontouch = menu_tph_c30_ontouch,
};

static int menu_tph_c50_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c50 ontouch**");
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
        for (int i = 0; i < 5; i++) {
            if (ctr->elm.id == _TPH_SET_PIC_C50[i]) {
                sel_item = i;
                break;
            }
        }
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        switch (__this->tph_menu_status) {
        case TPH_MENU_AWB:
            if (__this->camera[0].white_balance == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C50[__this->camera[0].white_balance]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C50[sel_item]);
            menu_tph_set_wbl(sel_item);
            __this->camera[0].white_balance = sel_item;
            ui_pic_show_image_by_id(TPH_PIC_WB, __this->camera[0].white_balance);
            break;
        default:
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C50_1)
.ontouch = menu_tph_c50_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C50_2)
.ontouch = menu_tph_c50_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C50_3)
.ontouch = menu_tph_c50_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C50_4)
.ontouch = menu_tph_c50_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C50_5)
.ontouch = menu_tph_c50_ontouch,
};

static int menu_tph_c40_ontouch(void *_ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**c40 ontouch**");
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
            if (ctr->elm.id == _TPH_SET_PIC_C40[i]) {
                sel_item = i;
                break;
            }
        }
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        switch (__this->tph_menu_status) {
        case TPH_MENU_ISO:
            if (__this->camera[0].iso == sel_item) {
                return false;
            }
            ui_no_highlight_element_by_id(_TPH_SET_PIC_C40[__this->camera[0].iso]);
            ui_highlight_element_by_id(_TPH_SET_PIC_C40[sel_item]);
            menu_tph_set_iso(sel_item);
            __this->camera[0].iso = sel_item;
            ui_pic_show_image_by_id(TPH_PIC_ISO, __this->camera[0].iso);
            break;
        default:
            break;
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C40_1)
.ontouch = menu_tph_c40_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C40_2)
.ontouch = menu_tph_c40_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C40_3)
.ontouch = menu_tph_c40_ontouch,
};
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_C40_4)
.ontouch = menu_tph_c40_ontouch,
};

/**************************************曝光补偿控件动作***************************************/
#define SLID_X    820 //滑块x起始绝对坐标
#define SLID_GAP  42  //每一项的间隔(滑块长度/项目数)
#define SLID_ITEM 7  //项目数
static int menu_tph_exp_ontouch(void *arg1, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("** exp photo ontouch  ");
    static s16 x_pos_down = 0;
    static s16 old_exp = 0;
    s16 tmp;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        x_pos_down = e->pos.x;
        old_exp = __this->camera[0].exposure;
        return true;
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        s16 x_pos_now = e->pos.x;
        s16 x_pos_ch = x_pos_down - x_pos_now;
        tmp = __this->camera[0].exposure;
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
        if (__this->camera[0].exposure == tmp) {
            return false;
        }
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        /* printf("x_pos=%d y_pos=%d",e->pos.x,e->pos.y); */
#if TPH_MENU_HIDE_ENABLE
        ui_set_call(tph_menu, __this->tph_menu_status);
#endif
        int i;
        tmp = __this->camera[0].exposure;
        for (i = 1; i <= SLID_ITEM; i++) {
            if (e->pos.x - SLID_X < SLID_GAP * i && e->pos.x > SLID_X + SLID_GAP * (i - 1)) {
                tmp = SLID_ITEM - i;
            }
        }
        if (__this->camera[0].exposure == tmp) {
            return false;
        }
        break;
    }
    ui_hide(_TPH_SET_PIC_EXP[(6 - __this->camera[0].exposure)]);
    ui_show(_TPH_SET_PIC_EXP[(6 - tmp)]);
    /* printf("tmp=%d\n",tmp); */
    menu_tph_set_pexp(tmp);
    __this->camera[0].exposure = tmp;

    return false;
}
REGISTER_UI_EVENT_HANDLER(TPH_SET_PIC_EXP)
.ontouch = menu_tph_exp_ontouch,
};




static int tph_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("tph_onchange\n");
        sys_key_event_takeover(true, false);
        __this->onkey_mod = 0;
        __this->onkey_sel = 0;
        ui_highlight_element_by_id(TPH_PIC_TPH);
        break;
    default:
        return false;
    }

    return false;
}
static int tph_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    struct application *app;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }
    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    if (__this->onkey_mod == 0) {
        //拍照模式
        switch (e->value) {
        case KEY_UP:
        case KEY_DOWN:
            //上下键切前后视
            init_intent(&it);
            it.name = "video_photo";
            it.action = ACTION_PHOTO_TAKE_SWITCH_WIN;
            start_app(&it);
            break;
        case KEY_PHOTO:
            //OK键拍照
            it.name = "video_photo";
            it.action = ACTION_PHOTO_TAKE_CONTROL;
            start_app_async(&it, tph_control_ok, NULL);
            break;
        case KEY_MODE:
            //MODE键切换到工具栏
            if (tph_ask_app_open_menu() == (int) - 1) {
                return -1;
            }
            __this->onkey_mod = 1;
            __this->onkey_sel = 1;
            ui_no_highlight_element_by_id(TPH_PIC_TPH);
            ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod == 1) {
        //工具栏
        switch (e->value) {
        case KEY_UP:
            //上键向上
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            __this->onkey_sel --;
            if (__this->onkey_sel < 1) {
                __this->onkey_sel = 3;
            }
            ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            //下键向下
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            __this->onkey_sel ++;
            if (__this->onkey_sel > 3) {
                __this->onkey_sel = 1;
            }
            ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            //OK键对应动作
            switch (__this->onkey_sel) {
            case 1:
                //进入设置模式
                if (!__this->menu_status) {
                    __this->onkey_mod = 2;
                    __this->onkey_sel = 1;
                    disp_TphSetting_lay(1);
                }
                break;
            case 2:
                //切换到录像模式
                __this->page_exit = MODE_SW_EXIT;
                init_intent(&it);
                app = get_current_app();
                if (app) {
                    it.name = "video_photo";
                    it.action = ACTION_BACK;
                    start_app_async(&it, NULL, NULL); //不等待直接启动app

                    it.name = "video_rec";
                    it.action = ACTION_VIDEO_REC_MAIN;
                    start_app_async(&it, NULL, NULL); //不等待直接启动app
                }
                break;
            case 3:
                //退出到主界面
                init_intent(&it);
                app = get_current_app();
                if (app) {
                    __this->page_exit = HOME_SW_EXIT;
                    it.name = "video_photo";
                    it.action = ACTION_BACK;
                    start_app_async(&it, NULL, NULL); //不等待直接启动app
                }
                break;
            }
            break;
        case KEY_MODE:
            //MODE键返回拍照模式
            ui_highlight_element_by_id(TPH_PIC_TPH);
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            tph_tell_app_exit_menu();
            __this->onkey_mod = 0;
            __this->onkey_sel = 0;
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod == 2) {
        //设置模式
        switch (e->value) {
        case KEY_UP:
            //上键向左
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_sel --;
            if (__this->onkey_sel < 1) {
                __this->onkey_sel = 12;
            }
            ui_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            //下键向右
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
                ui_no_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            }
            __this->onkey_sel ++;
            if (__this->onkey_sel > 12) {
                __this->onkey_sel = 1;
            }
            ui_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            //OK键进入对应设置项
            if (__this->onkey_sel) {
                __this->onkey_mod = 2 + __this->onkey_sel;
                switch (__this->onkey_sel) {
                case 1:
                    __this->camera[0].resolution = index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0));
                    __this->onkey_sel = __this->camera[0].resolution;
                    ui_set_call(tph_menu, TPH_MENU_RESOLUTION);
                    break;
                case 2:
                    __this->camera[0].tph_delay = index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0));
                    __this->onkey_sel = __this->camera->tph_delay;
                    ui_set_call(tph_menu, TPH_MENU_DELAY);
                    break;
                case 3:
                    __this->camera[0].repeat = db_select("cyt");
                    __this->onkey_sel =	__this->camera[0].repeat;
                    ui_set_call(tph_menu, TPH_MENU_CONTINUOUS);
                    break;
                case 4:
                    __this->camera[0].quality = index_of_table8(db_select("qua"), TABLE(table_photo_quality_camera0));
                    __this->onkey_sel = __this->camera[0].quality;
                    ui_set_call(tph_menu, TPH_MENU_QUALITY);
                    break;
                case 5:
                    __this->camera[0].acutance = index_of_table8(db_select("acu"), TABLE(table_photo_acutance_camera0));
                    __this->onkey_sel = __this->camera[0].acutance;
                    ui_set_call(tph_menu, TPH_MENU_ACUITY);
                    break;
                case 6:
                    __this->camera[0].white_balance = index_of_table8(db_select("wbl"), TABLE(table_photo_white_balance_camera0));
                    __this->onkey_sel = __this->camera[0].white_balance;
                    ui_set_call(tph_menu, TPH_MENU_AWB);
                    break;
                case 7:
                    __this->camera[0].color = index_of_table8(db_select("col"), TABLE(table_photo_color_camera0));
                    __this->onkey_sel = __this->camera[0].color;
                    ui_set_call(tph_menu, TPH_MENU_COLOR);
                    break;
                case 8:
                    __this->camera[0].iso = index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0));
                    __this->onkey_sel = __this->camera[0].iso;
                    ui_set_call(tph_menu, TPH_MENU_ISO);
                    break;
                case 9:
                    __this->camera[0].exposure = index_of_table8(db_select("pexp"), TABLE(table_photo_exposure_camera0));
                    __this->onkey_sel = __this->camera[0].exposure;
                    ui_set_call(tph_menu, TPH_MENU_EXPOSURE);
                    break;
                case 10:
                    __this->camera[0].hand_shake = db_select("sok");
                    __this->onkey_sel = __this->camera[0].hand_shake;
                    ui_set_call(tph_menu, TPH_MENU_TREMOR);
                    break;
                case 11:
                    __this->camera[0].quick_scan = index_of_table8(db_select("sca"), TABLE(table_photo_quick_scan_camera0));
                    __this->onkey_sel = __this->camera[0].quick_scan;
                    ui_set_call(tph_menu, TPH_MENU_PREVIEW);
                    break;
                case 12:
                    __this->camera[0].dat_label = db_select("pdat");
                    __this->onkey_sel = __this->camera[0].dat_label;
                    ui_set_call(tph_menu, TPH_MENU_LABEL);
                    break;
                }
            }
            break;
        case KEY_MODE:
            //MODE键退出到工具栏
            __this->onkey_mod = 1;
            __this->onkey_sel = 1;
            disp_TphSetting_lay(0);
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod > 2 && __this->onkey_mod < 3 + 12) {
        s8 tmp;
        switch (e->value) {
        case KEY_UP:
            switch (__this->tph_menu_status) {
            case TPH_MENU_RESOLUTION:
                tmp = __this->camera[0].resolution;
                ui_hide(_TPH_SET_PIC_RES[tmp]);
                tmp = tmp == 0 ? 0 : tmp - 1;
                ui_show(_TPH_SET_PIC_RES[tmp]);
                menu_tph_set_pres(tmp);
                __this->camera[0].resolution = tmp;
                if (__this->cur_camera_id == 0) {
                    ui_pic_show_image_by_id(TPH_PIC_RESOLUTION, __this->camera[0].resolution);
                }
                break;
            case TPH_MENU_DELAY:
                tmp = __this->camera[0].tph_delay;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C4[tmp]);
                tmp = tmp == 0 ? 3 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C4[tmp]);
                if (__this->camera[0].tph_delay == 0) {
                    ui_show(TPH_PIC_DELAY);
                }
                menu_tph_set_phm(tmp);
                __this->camera[0].tph_delay = tmp;
                if (tmp == 0) {
                    ui_hide(TPH_PIC_DELAY);
                } else {
                    ui_pic_show_image_by_id(TPH_PIC_DELAY, tmp - 1);
                }
                break;
            case TPH_MENU_CONTINUOUS:
                tmp = __this->camera[0].repeat ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                menu_tph_set_cyt(tmp);
                __this->camera[0].repeat = tmp;
                if (tmp) {
                    ui_show(TPH_PIC_TRIPLE);
                } else {
                    ui_hide(TPH_PIC_TRIPLE);
                }
                break;
            case TPH_MENU_QUALITY:
                tmp = __this->camera[0].quality ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                tmp = tmp == 0 ? 2 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                menu_tph_set_qua(tmp);
                __this->camera[0].quality = tmp;
                ui_pic_show_image_by_id(TPH_PIC_QUALITY, 2 - __this->camera[0].quality);
                break;
            case TPH_MENU_ACUITY:
                tmp = __this->camera[0].acutance  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                tmp = tmp == 0 ? 2 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                menu_tph_set_acu(tmp);
                __this->camera[0].acutance = tmp;
                break;
            case TPH_MENU_AWB:
                tmp = __this->camera[0].white_balance  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C50[tmp]);
                tmp = tmp == 0 ? 4 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C50[tmp]);
                menu_tph_set_wbl(tmp);
                __this->camera[0].white_balance = tmp;
                ui_pic_show_image_by_id(TPH_PIC_WB, __this->camera[0].white_balance);
                break;
            case TPH_MENU_COLOR:
                tmp = __this->camera[0].color  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                tmp = tmp == 0 ? 2 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                menu_tph_set_col(tmp);
                __this->camera[0].color = tmp;
                break;
            case TPH_MENU_ISO:
                tmp = __this->camera[0].iso  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C40[tmp]);
                tmp = tmp == 0 ? 3 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C40[tmp]);
                menu_tph_set_iso(tmp);
                __this->camera[0].iso = tmp;
                ui_pic_show_image_by_id(TPH_PIC_ISO, __this->camera[0].iso);
                break;
            case TPH_MENU_EXPOSURE:
                tmp = __this->camera[0].exposure;
                ui_hide(_TPH_SET_PIC_EXP[6 - tmp]);
                tmp = tmp == 6 ? 6 : tmp + 1;
                ui_show(_TPH_SET_PIC_EXP[6 - tmp]);
                menu_tph_set_pexp(tmp);
                __this->camera[0].exposure = tmp;
                break;
            case TPH_MENU_TREMOR:
                tmp = __this->camera[0].hand_shake ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                menu_tph_set_sok(tmp);
                __this->camera[0].hand_shake = tmp;
                if (tmp) {
                    ui_show(TPH_PIC_HS);
                } else {
                    ui_hide(TPH_PIC_HS);
                }
                break;
            case TPH_MENU_PREVIEW:
                tmp = __this->camera[0].quick_scan  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                tmp = tmp == 0 ? 2 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                menu_tph_set_sca(tmp);
                __this->camera[0].quick_scan = tmp;
                break;
            case TPH_MENU_LABEL:
                tmp = __this->camera[0].dat_label ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                tmp = tmp == 0 ? 1 : tmp - 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                menu_tph_set_pdat(tmp);
                __this->camera[0].dat_label = tmp;
                break;
            }
            break;
        case KEY_DOWN:
            switch (__this->tph_menu_status) {
            case TPH_MENU_RESOLUTION:
                tmp = __this->camera[0].resolution;
                ui_hide(_TPH_SET_PIC_RES[tmp]);
                tmp = tmp == 6 ? 6 : tmp + 1;
                ui_show(_TPH_SET_PIC_RES[tmp]);
                menu_tph_set_pres(tmp);
                __this->camera[0].resolution = tmp;
                if (__this->cur_camera_id == 0) {
                    ui_pic_show_image_by_id(TPH_PIC_RESOLUTION, __this->camera[0].resolution);
                }
                break;
            case TPH_MENU_DELAY:
                tmp = __this->camera[0].tph_delay;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C4[tmp]);
                tmp = tmp == 3 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C4[tmp]);
                if (__this->camera[0].tph_delay == 0) {
                    ui_show(TPH_PIC_DELAY);
                }
                menu_tph_set_phm(tmp);
                __this->camera[0].tph_delay = tmp;
                if (tmp == 0) {
                    ui_hide(TPH_PIC_DELAY);
                } else {
                    ui_pic_show_image_by_id(TPH_PIC_DELAY, tmp - 1);
                }
                break;
            case TPH_MENU_CONTINUOUS:
                tmp = __this->camera[0].repeat ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                menu_tph_set_cyt(tmp);
                __this->camera[0].repeat = tmp;
                if (tmp) {
                    ui_show(TPH_PIC_TRIPLE);
                } else {
                    ui_hide(TPH_PIC_TRIPLE);
                }
                break;
            case TPH_MENU_QUALITY:
                tmp = __this->camera[0].quality ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                tmp = tmp == 2 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                menu_tph_set_qua(tmp);
                __this->camera[0].quality = tmp;
                ui_pic_show_image_by_id(TPH_PIC_QUALITY, 2 - __this->camera[0].quality);
                break;
            case TPH_MENU_ACUITY:
                tmp = __this->camera[0].acutance  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                tmp = tmp == 2 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                menu_tph_set_acu(tmp);
                __this->camera[0].acutance = tmp;
                break;
            case TPH_MENU_AWB:
                tmp = __this->camera[0].white_balance  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C50[tmp]);
                tmp = tmp == 4 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C50[tmp]);
                menu_tph_set_wbl(tmp);
                __this->camera[0].white_balance = tmp;
                ui_pic_show_image_by_id(TPH_PIC_WB, __this->camera[0].white_balance);
                break;
            case TPH_MENU_COLOR:
                tmp = __this->camera[0].color  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                tmp = tmp == 2 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C30[tmp]);
                menu_tph_set_col(tmp);
                __this->camera[0].color = tmp;
                break;
            case TPH_MENU_ISO:
                tmp = __this->camera[0].iso  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C40[tmp]);
                tmp = tmp == 3 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C40[tmp]);
                menu_tph_set_iso(tmp);
                __this->camera[0].iso = tmp;
                ui_pic_show_image_by_id(TPH_PIC_ISO, __this->camera[0].iso);
                break;
            case TPH_MENU_EXPOSURE:
                tmp = __this->camera[0].exposure;
                ui_hide(_TPH_SET_PIC_EXP[6 - tmp]);
                tmp = tmp == 0 ? 0 : tmp - 1;
                ui_show(_TPH_SET_PIC_EXP[6 - tmp]);
                menu_tph_set_pexp(tmp);
                __this->camera[0].exposure = tmp;
                break;
            case TPH_MENU_TREMOR:
                tmp = __this->camera[0].hand_shake ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                menu_tph_set_sok(tmp);
                __this->camera[0].hand_shake = tmp;
                if (tmp) {
                    ui_show(TPH_PIC_HS);
                } else {
                    ui_hide(TPH_PIC_HS);
                }
                break;
            case TPH_MENU_PREVIEW:
                tmp = __this->camera[0].quick_scan  ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                tmp = tmp == 2 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C3[tmp]);
                menu_tph_set_sca(tmp);
                __this->camera[0].quick_scan = tmp;
                break;
            case TPH_MENU_LABEL:
                tmp = __this->camera[0].dat_label ;
                ui_no_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                tmp = tmp == 1 ? 0 : tmp + 1;
                ui_highlight_element_by_id(_TPH_SET_PIC_C2[tmp]);
                menu_tph_set_pdat(tmp);
                __this->camera[0].dat_label = tmp;
                break;
            }
            break;
        case KEY_OK:
        case KEY_MODE:
            __this->onkey_sel = __this->onkey_mod - 2;
            __this->onkey_mod = 2;
            tph_menu(__this->tph_menu_status);
            ui_highlight_element_by_id(onkey_sel_setting[__this->onkey_sel - 1]);
            ui_highlight_element_by_id(onkey_sel_setting1[__this->onkey_sel - 1]);
            break;
        default:
            return false;
        }
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(TPH_WIN)
.onchange = tph_onchange,
 .onkey = tph_onkey,
};













#endif
