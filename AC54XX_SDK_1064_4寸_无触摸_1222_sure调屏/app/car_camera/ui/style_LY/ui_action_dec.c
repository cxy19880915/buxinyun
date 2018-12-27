#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "video_system.h"

#ifdef CONFIG_UI_STYLE_LY_ENABLE


extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */
u8 home_rec_touch = 0;


extern int menu_sys_lcd_pro_set(u8 sel_item);
extern int menu_sys_auto_off_set(u8 sel_item);
extern int menu_sys_led_fre_set(u8 sel_item);
extern int menu_sys_key_voice_set(u8 sel_item);
extern int menu_sys_language_set(u8 sel_item);
extern int menu_sys_date_set(u8 sel_item);
extern int menu_sys_tv_mod_set(u8 sel_item);
extern int menu_sys_format_set(u8 sel_item);
extern int menu_sys_default_set(u8 sel_item);

void rep_exit_menu_post(void);

#define STYLE_NAME  LY


struct replay_menu_info {

    u8 if_in_rep;           /* 是否正在播放 */
    s8 ff_fr_flag;          /* 是否正在快进快退, 0 否，1 快进， -1快退 */
    u8 no_file;
    u8 file_type;           /* 文件是JPG还是MOV,0 jpg, 1 mov */
    u8 file_type_index;     /* 文件分辨率的索引号，根据UI */
    u8 ppt;
    u8 is_lock;
    u8 err_file;
    const char *fname;      /* 文件名 */
    struct utime sum_time;  /* 视频的总时间 */
    struct utime cur_time;  /* 视频的当前时间 */

    u8 menu_status;
    u8 init;
    u8 battery_val;
    u8 battery_char;

};

static struct replay_menu_info handler;

#define __this 	(&handler)

/************************************************************
						回放模式
************************************************************/


/*
 * replay的ppt播放间隔
 */
static const char *table_ppt_sec[] = {
    "ppt:2s",
    "ppt:5s",
    "ppt:8s",

};


/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_NO_POWER = 1,
    BOX_MSG_NO_FILE,
    BOX_MSG_MEM_ERR,
    BOX_MSG_INSERT_SD,
    BOX_MSG_DEL_CUR,
    BOX_MSG_DEFAULT_SET,
    BOX_MSG_FORMATTING,
    BOX_MSG_DEL_ALL,
    BOX_MSG_ERR_FILE,
    BOX_MSG_LOCKED_FILE,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
static void __dec_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REP);
        }
    } else if (id == 0) {
        /*
         * 没有指定ID，强制隐藏
         */
        if (msg_show_f) {
            msg_show_f = 0;
            ui_hide(LAYER_MSG_REP);
        }
    }
}
static void __dec_msg_timeout_func(void *priv)
{
    __dec_msg_hide((enum box_msg)priv);
}
static void __dec_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__dec_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REP);
            ui_text_show_index_by_id(TEXT_MSG_REP, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __dec_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            ui_show(LAYER_MSG_REP);
        }
        ui_text_show_index_by_id(TEXT_MSG_REP, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __dec_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */

static int rep_file_name_handler(const char *type, u32 args)
{
    const char *fname = (const char *)args;
    const char *str_p;

    /* puts("rep_file_name_handler\n"); */

    if (!strcmp(type, "ascii")) {
        str_p = (const char *)(args + strlen((char *)args) - 3);
        if (!strcmp(str_p, "JPG") || !strcmp(str_p, "jpg")) {
            if (__this->file_type != 0) {
                __this->file_type = 0;

                ui_hide(TIMER_SUM_REP);
                ui_hide(TIMER_RUN_REP);

                ui_hide(PIC_START_REP);
                ui_hide(PIC_SS_REP);
                ui_hide(PIC_PREV_REP);
                ui_hide(PIC_NEXT_REP);

                ui_pic_show_image_by_id(PIC_LOGO_REP, 0);
            }
        } else {
            if (__this->file_type != 1) {
                __this->file_type = 1;
            } else {

            }
            ui_hide(TIMER_RUN_REP);
            ui_show(TIMER_SUM_REP);
            ui_pic_show_image_by_id(PIC_START_REP, 1);
            ui_show(PIC_SS_REP);
            ui_show(PIC_PREV_REP);
            ui_show(PIC_NEXT_REP);
            ui_pic_show_image_by_id(PIC_LOGO_REP, 1);
            ui_pic_show_image_by_id(PIC_PREV_REP, 0);
            ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
        }
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
        __this->ff_fr_flag = 0;
        __this->if_in_rep = 0;
        __this->no_file = 0;
        __this->err_file = 0;
        __this->fname = (const char *)args;
        ui_text_set_str_by_id(TEXT_FNAME_REP, "ascii", (const char *)args);
        ui_show(TEXT_FNAME_REP);
    } else if (!strcmp(type, "utf16")) {

    }

    __dec_msg_hide(0);

    return 0;
}

static int rep_file_res_handler(const char *type, u32 args)
{
    if (*type == 'w') {
        /* printf("rep_file_res_handler w: %d.\n", args); */
        if (__this->file_type == 1) {
            if (args >= 1920) {
                /*
                 * 1080p
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 0);
                __this->file_type_index = 0;
            } else if (args >= 1080) {
                /*
                 * 720p
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 1);
                __this->file_type_index = 1;
            } else {
                /*
                 * vga
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 2);
                __this->file_type_index = 2;
            }
        } else if (__this->file_type == 0) {
            if (args >= 4032) {
                /*
                 * 12M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 10);
                __this->file_type_index = 10;
            } else if (args >= 3648) {
                /*
                 * 10M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 9);
                __this->file_type_index = 9;
            } else if (args >= 3264) {
                /*
                 * 8M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 8);
                __this->file_type_index = 8;
            } else if (args >= 2592) {
                /*
                 * 5M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 7);
                __this->file_type_index = 7;
            } else if (args >= 2048) {
                /*
                 * 3M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 6);
                __this->file_type_index = 6;
            } else if (args >= 1920) {
                /*
                 * 2M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 5);
                __this->file_type_index = 5;
            } else if (args >= 1280) {
                /*
                 * 1.3M
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 4);
                __this->file_type_index = 4;
            } else {
                /*
                 * vga
                 */
                ui_text_show_index_by_id(TEXT_RES_REP, 3);
                __this->file_type_index = 3;
            }
        }
    }

    return 0;
}

static int rep_film_length_handler(const char *type, u32 args)
{
    int sec;
    struct utime t;

    if (*type == 's') {
        sec = args;
    } else {
        return 0;
    }

    if (__this->file_type == 1) {
        if (sec) {
            t.sec = sec % 60;
            t.min = sec / 60 % 60;
            t.hour = sec / 60 / 60;
            __this->sum_time.sec = t.sec;
            __this->sum_time.min = t.min;
            __this->sum_time.hour = t.hour;
            ui_time_update_by_id(TIMER_SUM_REP, &t);
        } else {
            t.sec = 1;
            t.min = 0;
            t.hour = 0;
            __this->sum_time.sec = 1;
            __this->sum_time.min = 0;
            __this->sum_time.hour = 0;
            ui_time_update_by_id(TIMER_SUM_REP, &t);
        }
    }
    /* printf("rep_film_length_handler: %d.\n", args); */

    return 0;
}

static int rep_current_time_handler(const char *type, u32 args)
{
    int sec;
    struct utime t;

    if (*type == 's') {
        sec = args;
    } else {
        return 0;
    }

    t.sec = sec % 60;
    t.min = sec / 60 % 60;
    t.hour = sec / 60 / 60;

    __this->cur_time.sec = t.sec;
    __this->cur_time.min = t.min;
    __this->cur_time.hour = t.hour;
    ui_time_update_by_id(TIMER_RUN_REP, &t);
    /* printf("rep_curtime: %d.\n", args); */

    return 0;
}
static int rep_no_file_handler(const char *type, u32 args)
{
    struct intent it;

    if (__this->ppt) {
        /*
         * 退出幻灯片浏览模式
         */
        __this->ppt = 0;

        init_intent(&it);
        it.name	= "video_dec";
        it.data = "ppt:0s";
        it.action = ACTION_VIDEO_DEC_SET_CONFIG;
        start_app_async(&it, NULL, NULL);
    }

    __this->file_type = 0xff;
    __this->no_file = 1;
    if (__this->menu_status == 0) {
        ui_hide(PIC_LOCK_REP);
        ui_hide(TIMER_SUM_REP);
        ui_hide(TIMER_RUN_REP);
        ui_hide(TEXT_RES_REP);
        ui_hide(TEXT_FNAME_REP);
        ui_hide(PIC_START_REP);
        ui_hide(PIC_SS_REP);
        ui_hide(PIC_PREV_REP);
        ui_hide(PIC_NEXT_REP);
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
        __dec_msg_show(BOX_MSG_NO_FILE, 0);
    } else {
        if (__this->menu_status == 1) {
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
        } else if (__this->menu_status == 2) {
            sys_exit_menu_post();
        }
        __this->menu_status = 0;
    }
    return 0;
}

static int rep_file_err_handler(const char *type, u32 args)
{
    /* puts("rep_file_err_handler \n"); */

    if (__this->menu_status == 0) {
        ui_hide(PIC_LOCK_REP);
        ui_hide(TIMER_SUM_REP);
        ui_hide(TIMER_RUN_REP);
        ui_hide(TEXT_RES_REP);
        ui_hide(TEXT_FNAME_REP);
        ui_hide(PIC_START_REP);
        ui_hide(PIC_SS_REP);
        ui_hide(PIC_PREV_REP);
        ui_hide(PIC_NEXT_REP);
        ui_hide(PIC_FF_NUM_REP);
        ui_hide(PIC_FR_NUM_REP);
        __dec_msg_show(BOX_MSG_ERR_FILE, 0);
    } else {

    }
    __this->err_file = 1;
    return 0;
}


static int rep_file_attribute_handler(const char *type, u32 read_only)
{
    /* puts("rep_file_attribute_handler \n"); */
    if (read_only) {
        __this->is_lock = 1;
        ui_show(PIC_LOCK_REP);
    } else {
        __this->is_lock = 0;
        ui_hide(PIC_LOCK_REP);
    }

    return 0;
}

static int rep_play_handler(const char *type, u32 args)
{
    /* puts("rep_play!\n"); */
    __this->if_in_rep = 1;
    if (__this->ff_fr_flag != 0) {
        if (__this->ff_fr_flag >= 1) {

            ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
            ui_hide(PIC_FF_NUM_REP);
        } else if (__this->ff_fr_flag <= -1) {

            ui_pic_show_image_by_id(PIC_PREV_REP, 0);
            ui_hide(PIC_FR_NUM_REP);
        }
        __this->ff_fr_flag = 0;
    } else {
        ui_pic_show_image_by_id(PIC_START_REP, 0);
        ui_hide(TIMER_SUM_REP);
        ui_show(TIMER_RUN_REP);
    }


    return 0;
}

static int rep_pause_handler(const char *type, u32 args)
{
    /* puts("rep_pause!\n"); */
    __this->if_in_rep = 0;
    ui_pic_show_image_by_id(PIC_START_REP, 1);

    return 0;
}

static int rep_ff_handler(const char *type, u32 args)
{
    /* printf("rep_ff: %d\n", args); */

    ui_pic_show_image_by_id(PIC_NEXT_REP, 1);
    if (__this->ff_fr_flag <= -1) {
        ui_pic_show_image_by_id(PIC_PREV_REP, 0);
        ui_hide(PIC_FR_NUM_REP);
    }
    __this->ff_fr_flag = 1;
    switch (args) {
    case 2:
        __this->ff_fr_flag = 1;
        ui_pic_show_image_by_id(PIC_FF_NUM_REP, 0);
        break;
    case 4:
        __this->ff_fr_flag = 2;
        ui_pic_show_image_by_id(PIC_FF_NUM_REP, 1);
        break;
    case 8:
        __this->ff_fr_flag = 3;
        ui_pic_show_image_by_id(PIC_FF_NUM_REP, 2);
        break;
    }
    return 0;
}

static int rep_fr_handler(const char *type, u32 args)
{
    /* printf("rep_fr: %d\n", args); */

    ui_pic_show_image_by_id(PIC_PREV_REP, 1);
    if (__this->ff_fr_flag >= 1) {
        ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
        ui_hide(PIC_FF_NUM_REP);
    }
    __this->ff_fr_flag = -1;
    switch (args) {
    case 2:
        __this->ff_fr_flag = -1;
        ui_pic_show_image_by_id(PIC_FR_NUM_REP, 0);
        break;
    case 4:
        __this->ff_fr_flag = -2;
        ui_pic_show_image_by_id(PIC_FR_NUM_REP, 1);
        break;
    case 8:
        __this->ff_fr_flag = -3;
        ui_pic_show_image_by_id(PIC_FR_NUM_REP, 2);
        break;
    }
    return 0;
}
/*
 * 回放模式的APP状态响应回调
 */
static const struct uimsg_handl rep_msg_handler[] = {
    { "fname",     rep_file_name_handler     },
    { "res",       rep_file_res_handler      },
    { "filmLen",   rep_film_length_handler   },
    { "plyTime",   rep_current_time_handler  },
    { "fattr",     rep_file_attribute_handler},
    { "noFile",    rep_no_file_handler       },
    { "fileErr",   rep_file_err_handler      },
    { "play",      rep_play_handler          },
    { "pause",     rep_pause_handler         },
    { "ff",        rep_ff_handler            },
    { "fr",        rep_fr_handler            },
    { NULL, NULL},
};



static void rep_tell_app_exit_menu(void)
{
    struct intent it;

    init_intent(&it);

    it.name	= "video_dec";
    it.action = ACTION_VIDEO_DEC_CHANGE_STATUS;
    it.data = "exitMenu";
    start_app(&it);
}

static int rep_ask_app_open_menu(void)
{
    int err;
    struct intent it;

    init_intent(&it);
    it.name	= "video_dec";
    it.action = ACTION_VIDEO_DEC_CHANGE_STATUS;
    it.data = "openMenu";
    err = start_app(&it);
    if (err) {
        return -1;
    }
    if (!strcmp(it.data, "dis")) {
        return -1;
    }

    return 0;
}

void rep_exit_menu_post(void)
{
    ui_hide(LAYER_MENU_REP);
    ui_show(LAYER_UP_REP);
    ui_show(LAYER_DOWN_REP);
    rep_tell_app_exit_menu();
    __this->menu_status = 0;
}


/*****************************回放模式页面回调 ************************************/
static int replay_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:

        puts("\n***dec mode onchange init***\n");
        memset(__this, 0, sizeof(struct replay_menu_info));

        __this->file_type = 0xff;
        ui_register_msg_handler(ID_WINDOW_VIDEO_REP, rep_msg_handler);
        sys_cur_mod = 3;
        break;
    case ON_CHANGE_RELEASE:
        __dec_msg_hide(0);
        ui_hide(ID_WINDOW_VIDEO_SYS);
        /*
         * 要隐藏一下系统菜单页面，防止在系统菜单插入USB进入USB页面
         */
        break;

        break;
    default:
        return false;
    }
    return false;
}

static int replay_mode_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            if (__this->ppt) {
                /*
                 * 在幻灯片浏览模式，按menu键退出
                 */
                __this->ppt = 0;

                init_intent(&it);
                it.name	= "video_dec";
                it.data = "ppt:0s";
                it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                start_app_async(&it, NULL, NULL);

                ui_show(LAYER_UP_REP);
                ui_show(LAYER_DOWN_REP);
                sys_key_event_takeover(false, false);
            } else {
                if (rep_ask_app_open_menu()) {
                    break;
                }
                __dec_msg_hide(0);
                ui_hide(LAYER_UP_REP);
                ui_hide(LAYER_DOWN_REP);
                ui_show(LAYER_MENU_REP);
                ui_show(LAYOUT_MN_REP_REP);
                sys_key_event_takeover(true, false);
            }

            __this->menu_status = 1;
            break;
        default:
            return false;
        }
        break;
    default:
        if (__this->menu_status) {


            if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
                rep_exit_menu_post();
                sys_key_event_takeover(false, true);
                /* puts("\n take off msg power rep\n"); */
                return false;
            }
        }
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REP)
.onchange = replay_mode_onchange,
 .onkey = replay_mode_onkey,
};






/*********************************************************************************
 *  		     				菜单动作
 *********************************************************************************/


static const int menu_rep_list_items[] = {
    LAYOUT_MN_DEL_REP,
    LAYOUT_MN_LOCK_REP,
    LAYOUT_MN_PPT_REP,
};
/***************************** 回放模式菜单1 ************************************/
static int menu_rep_rep_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    /* puts("\n ===menu_rep_rep_onkey===\n"); */
    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            ASSERT(sel_item < (sizeof(menu_rep_list_items) / sizeof(int)));
            ui_show(menu_rep_list_items[sel_item]);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_show(ID_WINDOW_VIDEO_SYS);
            ui_hide(LAYER_MENU_REP);
            __this->menu_status = 2;
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_REP_REP)
.onkey = menu_rep_rep_onkey,
};


/***************************** 删除选择菜单 ************************************/
static const int menu_del_list_items[] = {
    LAYOUT_MN_DEL_CUR_REP,
    LAYOUT_MN_DEL_ALL_REP,
};

static int menu_del_rep_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            ASSERT(sel_item < (sizeof(menu_del_list_items) / sizeof(int)));
            ui_show(menu_del_list_items[sel_item]);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_DEL_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_DEL_REP)
.onkey = menu_del_rep_onkey,
};


/***************************** 锁文件菜单 ************************************/
static int menu_lock_rep_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;
    int err;

    init_intent(&it);

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            if (sel_item == 0) {
                it.data = "lock:cur";
            } else if (sel_item == 1) {
                it.data = "unlock:cur";
            } else if (sel_item == 2) {
                it.data = "lock:all";
                __this->is_lock = 1;
            } else if (sel_item == 3) {
                it.data = "unlock:all";
                __this->is_lock = 0;
            }
            it.name	= "video_dec";
            it.action = ACTION_VIDEO_DEC_SET_CONFIG;
            start_app(&it);
            ui_hide(LAYOUT_MN_LOCK_REP);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_LOCK_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_LOCK_REP)
.onkey = menu_lock_rep_onkey,
};


/***************************** ppt菜单 ************************************/
static int menu_ppt_rep_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;
    int err;

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);

            ASSERT(sel_item < ARRAY_SIZE(table_ppt_sec));

            init_intent(&it);
            it.name	= "video_dec";
            it.action = ACTION_VIDEO_DEC_SET_CONFIG;
            it.data = table_ppt_sec[sel_item];
            err = start_app(&it);
            if (err) {
                break;
            }
            ui_hide(LAYER_MENU_REP);
            ui_hide(LAYER_UP_REP);
            ui_hide(LAYER_DOWN_REP);
            __this->ppt = true;
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_PPT_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_PPT_REP)
.onkey = menu_ppt_rep_onkey,
};


void del_file_callback(void *priv, int err)
{
    /* printf("del_file_callback: err=%d\n", err); */
    if (err == (int) - EPERM) {
        puts("del file -EPERM, locked file.\n");
        __dec_msg_show(BOX_MSG_LOCKED_FILE, 2000);
    } else {
        if (__this->no_file == 1) {
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else {
            __dec_msg_hide(BOX_MSG_LOCKED_FILE);
        }
    }
}
/***************************** 删除当前菜单 ************************************/
static int menu_delcur_rep_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;

    init_intent(&it);

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            if (sel_item == 1) {
                it.name	= "video_dec";
                it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                it.data = "del:cur";
                start_app_async(&it, del_file_callback, NULL);
                __dec_msg_show(BOX_MSG_DEL_CUR, 0);
            }
            ui_hide(LAYOUT_MN_DEL_CUR_REP);
            ui_hide(LAYOUT_MN_DEL_REP);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_DEL_CUR_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_DEL_CUR_REP)
.onkey = menu_delcur_rep_onkey,
};


static void del_all_file_ok(void *p, int err)
{
    sys_key_event_enable();
    if (err == 0) {
        puts("---del_file_ok\n");
    } else {
        printf("---del_file_faild: %d\n", err);
    }
}



/***************************** 删除所有菜单 ************************************/
static int menu_delall_rep_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    u8 sel_item = 0;
    struct intent it;

    init_intent(&it);

    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_OK:
            sel_item = ui_grid_cur_item(grid);
            if (sel_item == 1) {
                it.name	= "video_dec";
                it.action = ACTION_VIDEO_DEC_SET_CONFIG;
                it.data = "del:all";
                sys_key_event_disable();
                start_app_async(&it, del_all_file_ok, NULL);
                __dec_msg_show(BOX_MSG_DEL_ALL, 0);
            }
            ui_hide(LAYOUT_MN_DEL_ALL_REP);
            ui_hide(LAYOUT_MN_DEL_REP);
            break;
        case KEY_DOWN:
            return FALSE;
            break;
        case KEY_UP:
            return FALSE;
            break;
        case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
            rep_exit_menu_post();
            sys_key_event_takeover(false, false);
            break;
#endif
        case KEY_MENU:
            ui_hide(LAYOUT_MN_DEL_ALL_REP);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_DEL_ALL_REP)
.onkey = menu_delall_rep_onkey,
};



/*****************************布局上部回调 ************************************/
static int rep_layout_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */

        printf("rep_layout_up_onchange first show\n");
        if (__this->no_file == 0) {
            if (__this->file_type != 0xff) {
                ui_text_show_index_by_id(TEXT_RES_REP, __this->file_type_index);
            }
            if (__this->is_lock) {
                ui_show(PIC_LOCK_REP);
            }
            if (__this->fname) {
                ui_text_set_str_by_id(TEXT_FNAME_REP, "ascii", __this->fname);
            }
            if (__this->err_file) {
                __dec_msg_show(BOX_MSG_ERR_FILE, 0);
            }
            if (__this->file_type == 1) {
                if (__this->cur_time.hour || __this->cur_time.min || __this->cur_time.sec) {
                    ui_time_update_by_id(TIMER_RUN_REP, &__this->cur_time);
                    ui_show(TIMER_RUN_REP);
                } else {
                    ui_time_update_by_id(TIMER_SUM_REP, &__this->sum_time);
                    ui_show(TIMER_SUM_REP);
                }

                /* ui_show(PIC_START_REP); */
                /* ui_show(PIC_SS_REP); */
                /* ui_show(PIC_PREV_REP); */
                /* ui_show(PIC_NEXT_REP); */
                /* printf("__this->ff_fr_flag = %d\n", __this->ff_fr_flag); */
                /* if(__this->ff_fr_flag > 0){ */
                /*     ui_show(PIC_FF_NUM_REP); */
                /*     ui_pic_show_image_by_id(PIC_FF_NUM_REP, __this->ff_fr_flag-1); */
                /*     ui_pic_show_image_by_id(PIC_NEXT_REP, 1); */
                /* } */
                /* else if(__this->ff_fr_flag < 0){ */
                /*     ui_show(PIC_FR_NUM_REP); */
                /*     ui_pic_show_image_by_id(PIC_FR_NUM_REP, __this->ff_fr_flag*(-1)-1); */
                /*     ui_pic_show_image_by_id(PIC_PREV_REP, 1); */
                /* } */
            }
        } else {
            if (__this->init == 1) {
                __dec_msg_show(BOX_MSG_NO_FILE, 0);
                ui_hide(TEXT_FNAME_REP);
                ui_hide(TEXT_RES_REP);
            }
        }
        __this->init = 1;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_UP_REP)
.onchange = rep_layout_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




/*****************************布局下部回调 ************************************/
static int rep_layout_down_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    int ret;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        /*
         * 在此获取默认隐藏的图标的状态并显示
         */

        printf("rep_layout_down_onchange first show\n");
        if (__this->no_file == 0) {

            /*
             * 视频文件
             */
            if (__this->file_type == 1) {

                ui_show(PIC_SS_REP);
                printf("__this->ff_fr_flag = %d\n", __this->ff_fr_flag);

                /*
                 * 快进图标
                 */
                if (__this->ff_fr_flag > 0) {
                    ui_pic_show_image_by_id(PIC_FF_NUM_REP, __this->ff_fr_flag - 1);
                    ui_pic_show_image_by_id(PIC_NEXT_REP, 1);
                    ui_pic_show_image_by_id(PIC_PREV_REP, 0);
                } else if (__this->ff_fr_flag < 0) { /* 快退图标 */
                    ui_pic_show_image_by_id(PIC_FR_NUM_REP, __this->ff_fr_flag * (-1) - 1);
                    ui_pic_show_image_by_id(PIC_PREV_REP, 1);
                    ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
                } else {
                    ui_pic_show_image_by_id(PIC_NEXT_REP, 0);
                    ui_pic_show_image_by_id(PIC_PREV_REP, 0);
                }

                /*
                 * 正在播放
                 */
                if (__this->if_in_rep == 1) {
                    ui_pic_show_image_by_id(PIC_START_REP, 0);
                } else {
                    ui_pic_show_image_by_id(PIC_START_REP, 1);
                }
            }
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DOWN_REP)
.onchange = rep_layout_down_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static void no_power_msg_box_timer(void *priv)
{
    static u8 cnt = 0;
    if (__this->battery_val <= 20 && __this->menu_status == 0 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            __dec_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            __dec_msg_hide(BOX_MSG_NO_POWER);
        }
    } else {
        __dec_msg_hide(BOX_MSG_NO_POWER);
        cnt = 0;
        if (__this->no_file == 1 && __this->err_file == 0) {
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else if (__this->err_file == 1) {
            __dec_msg_show(BOX_MSG_ERR_FILE, 0);
        }
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
static int battery_rep_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(BATTERY_REP)
.onchange = battery_rep_onchange,
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
            ui_pic_show_image_by_id(PIC_SD_REP, 1);
            __dec_msg_hide(BOX_MSG_NO_FILE);
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            ui_pic_show_image_by_id(PIC_SD_REP, 0);
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
            break;
        default:
            break;
        }
    }
}

extern int storage_device_ready();
/***************************** SD 卡图标动作 ************************************/
int pic_sd_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    static u16 id = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        if (storage_device_ready() == 0) {
            /*
             * 第一次显示需要判断一下,SD不在线
             */
            ui_pic_set_image_index(pic, 0);
            __dec_msg_show(BOX_MSG_NO_FILE, 0);
        } else {
            /*
             * SD卡在线
             */
            ui_pic_set_image_index(pic, 1);
        }
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

REGISTER_UI_EVENT_HANDLER(PIC_SD_REP)
.onchange = pic_sd_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


char *path = "B:/VIDEO/video.avi\0";

/***************************** 文件路径文本动作 ************************************/
int text_path_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        /*txt->str = path;*/
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_FNAME_REP)
.onchange = text_path_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 文件格式文本动作 ************************************/
int text_res_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    int index = 0;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(txt, __this->file_type_index);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(TEXT_RES_REP)
.onchange = text_res_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/***************************** MSG BOX文本动作 ************************************/
int text_msg_box_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *txt = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        return TRUE;
    case ON_CHANGE_SHOW_PROBE:
        ui_text_set_index(txt, msg_show_id - 1);
        return TRUE;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(TEXT_MSG_REP)
.onchange = text_msg_box_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 回放模式图标动作 ************************************/
int pic_logo_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, __this->file_type);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_LOGO_REP)
.onchange = pic_logo_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};



/***************************** 播放图标动作 ************************************/
int pic_play_rep_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    int index = 1;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, index);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_START_REP)
.onchange = pic_play_rep_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


#endif
