#include "ui/includes.h"
#include "system/includes.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "storage_device.h"
#include "res.h"
#include "app_database.h"

#ifdef CONFIG_UI_STYLE_LY_ENABLE


#define STYLE_NAME  LY

REGISTER_UI_STYLE(STYLE_NAME)

int sys_cur_mod = 0;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */
extern void rec_exit_menu_post(void);
extern void tph_exit_menu_post(void);
extern void rep_exit_menu_post(void);



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
    1,
    3,
 //   5,
 //   10,
};


/*
 * system屏幕保护设置
 */
static const u16 table_system_lcd_protect[] = {
    0,
 //   30,
    60,
    180,
};

/*
 * system语言设置
 */
static const u8 table_system_language[] = {
    Chinese_Simplified,  /* 简体中文 */
    Chinese_Traditional, /* 繁体中文 */
    German,
    English,            /* 日文 */
    French,             /* 英文 */
    Italian,              /* 韩文 */
    Spanish,              /* 法文 */
    Portuguese,
    Russian,
    Japanese,
};

/*
 * system光源频率设置
 */
static const u8 table_system_tv_mod[] = {
    TVM_PAL,
    TVM_NTSC,
};




/*
 * (begin)提示框显示接口
 */
enum box_msg {
    BOX_MSG_INSERT_SD,
    BOX_MSG_DELETING_FILE,
    BOX_MSG_DEFAULTING,
    BOX_MSG_FORMATTING,
    BOX_MSG_DELETING_ALL_FILE,
    BOX_MSG_FORMAT_ERR,
};
static u8 msg_show_f = 0;
static enum box_msg g_msg_id = 0;
static void __sys_msg_hide(void)
{
    if (msg_show_f) {
        msg_show_f = 0;
        ui_hide(LAYOUT_SYS_MSG);
    }
}
static void __sys_msg_timeout_func(void *priv)
{
    __sys_msg_hide();
}
static void __sys_msg_show(enum box_msg msg, u32 timeout_msec)
{

    if (msg_show_f == 0) {
        ui_show(LAYOUT_SYS_MSG);
        ui_text_show_index_by_id(TEXT_SYS_MSG, msg);
        g_msg_id = msg;
        msg_show_f = 1;
        if (timeout_msec > 0) {
            sys_timeout_add(NULL, __sys_msg_timeout_func, timeout_msec);
        }
    } else {
        ui_text_show_index_by_id(TEXT_SYS_MSG, msg);
        g_msg_id = msg;
        if (timeout_msec > 0) {
            sys_timeout_add(NULL, __sys_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */


/*
 * (begin)系统设置各项操作请求
 */
static int sys_app_back(void)
{

    struct intent it;
    int err;

    init_intent(&it);
    it.name	= "video_system";
    it.action = ACTION_BACK;
    it.data = NULL;
    err = start_app(&it);
    if (err) {
        printf("sys app back err! %d\n", err);
        /* ASSERT(err == 0, "sys back app fail.\n"); */
        return -1;
    }
    return 0;
}


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
    sys_app_back();
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
    sys_app_back();
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
    sys_app_back();
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
    sys_app_back();
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
    sys_app_back();
    return 0;
}

static int __time_get(struct sys_time *t);

int menu_sys_date_set(u8 sel_item)
{
    struct sys_time t;

    __time_get(&t);

    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        return -EFAULT;
    }
    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&t);
    dev_close(fd);

    return 0;
}
#if 0
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
    sys_app_back();
    return 0;
}
#endif
void format_callback(void *priv, int err)
{
    sys_app_back();
    __sys_msg_hide();
    if (err) {
        /*
         * 格式化出错
         */
        printf("format_callback err!!\n");
        __sys_msg_show(BOX_MSG_FORMAT_ERR, 2000);
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
        __sys_msg_show(BOX_MSG_FORMATTING, 0);
    }
    return 0;
}

int menu_sys_version_set(u8 sel_item)
{

    sys_app_back();
    return 0;
}
/*
 * (end)
 */


/***************************** 菜单模式图标动作 ************************************/
static int pic_sys_mn_mod_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (sys_cur_mod) {
            ui_pic_set_image_index(pic, sys_cur_mod - 1);
        }
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(PIC_SYS_MN_MOD_LOGO)
.onchange = pic_sys_mn_mod_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};




void sys_exit_menu_post(void)
{
    ui_hide(ID_WINDOW_VIDEO_SYS);

    switch (sys_cur_mod) {
    case 1:
        rec_exit_menu_post();
        break;
    case 2:
        tph_exit_menu_post();
        break;
    case 3:
        rep_exit_menu_post();
        break;
    default:
        break;
    }
    sys_key_event_takeover(false, false);
}

static int sys_page_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->event) {
    case KEY_EVENT_LONG:
        if (e->value == KEY_POWER) {
            sys_exit_menu_post();
            sys_key_event_takeover(false, true);
        }
        break;
    }
    return false;
}

static void video_sys_ui_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "parking")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            break;
        case DEVICE_EVENT_OUT:
        case DEVICE_EVENT_OFFLINE:
            /* if(__this->menu_status) { */
            sys_key_event_takeover(false, false);
            /* } */
            break;
        default:
            break;
        }
    }
}
/***************************** 系统菜单页面回调 ************************************/
static int sys_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static int id = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, video_sys_ui_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(id);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_SYS)
.onchange = sys_page_onchange,
 .onkey = sys_page_onkey,
  .ontouch = NULL,
};

/*********************************************************************************
 *								系统设置菜单
*********************************************************************************/


static const int menu_sys_items[] = {
	LAYOUT_MN_TIME_SYS,
	LAYOUT_MN_AUTOF_SYS,
	LAYOUT_MN_KEYV_SYS,
	LAYOUT_MN_LANG_SYS,
	LAYOUT_MN_HZ_SYS,
    LAYOUT_MN_LCD_SYS,

	LAYOUT_MN_NUM_REC,
	
 	LAYOUT_MN_FORMAT_SYS,
    LAYOUT_MN_DEFAULT_SYS,
    LAYOUT_MN_VERSION_SYS,   
    //LAYOUT_MN_TV_SYS,  
};

static int menu_sys_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        ASSERT(sel_item < (sizeof(menu_sys_items) / sizeof(int)));
        if (menu_sys_items[sel_item] == LAYOUT_MN_FORMAT_SYS) {
            if (dev_online(SDX_DEV) == false && storage_device_ready() == 0) {
                printf("format sd offline !\n");
                __sys_msg_show(BOX_MSG_INSERT_SD, 2000);

                return TRUE;
            }
        }
        ui_show(menu_sys_items[sel_item]);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MENU:
        sys_exit_menu_post();
        break;
    case KEY_MODE:
        sys_exit_menu_post();

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_MENU_SYS)
.onkey = menu_sys_onkey,
 .ontouch = NULL,
};






/***************************** 屏幕保护设置 ************************************/

static int menu_lcd_pro_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_lcd_pro_onkey(void *_grid, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_lcd_pro_set(sel_item);

        ui_hide(LAYOUT_MN_LCD_SYS);

        break;
    case KEY_DOWN:
        return FALSE;
        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LCD_SYS);
        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_LCD1_SYS)
	.onchange = menu_lcd_pro_onchange,
 	.onkey = menu_lcd_pro_onkey,
  	.ontouch = NULL,
};




/***************************** 自动关机设置 ************************************/

static int menu_auto_off_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("aff"), TABLE(table_system_auto_close));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_auto_off_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_auto_off_set(sel_item);

        ui_hide(LAYOUT_MN_AUTOF_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_AUTOF_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_AUTOF_SYS)
.onchange = menu_auto_off_onchange,
 .onkey = menu_auto_off_onkey,
  .ontouch = NULL,
};


/***************************** 前照灯提醒设置 ************************************/
static int menu_headlight_onchange(void *ctr, enum element_change_event e, void *arg)
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




/***************************** 光源频率设置 ************************************/

static int menu_led_hz_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("fre"), TABLE(table_system_led_fre));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_led_hz_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_led_fre_set(sel_item);

        ui_hide(LAYOUT_MN_HZ_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_HZ_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(VLIST_HZ_SYS)
.onchange = menu_led_hz_onchange,
 .onkey = menu_led_hz_onkey,
  .ontouch = NULL,
};



/***************************** 按键声音设置 ************************************/

static int menu_key_voice_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("kvo"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_key_voice_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_key_voice_set(sel_item);

        ui_hide(LAYOUT_MN_KEYV_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_KEYV_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_KEYV_SYS)
.onchange = menu_key_voice_onchange,
 .onkey = menu_key_voice_onkey,
  .ontouch = NULL,
};




/***************************** 语言设置设置 ************************************/

static int menu_language_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("lag"), TABLE(table_system_language));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_language_onkey(void *ctr, struct element_key_event *e)
{
    int sel_item = 0;
    struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_language_set(sel_item);
        extern int menu_rec_car_num_set_lang(void);  ///peng
        menu_rec_car_num_set_lang();
        ui_hide(LAYOUT_MN_LANG_SYS);
        ui_show(VLIST_MENU_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_LANG_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_LANG_SYS)
.onchange = menu_language_onchange,
 .onkey = menu_language_onkey,
  .ontouch = NULL,
};


/***************************** 日期时间设置 ************************************/
struct time_setting {
    const char *mark;
    u32 time_id;
    u16 time_value;
};
struct time_setting time_set_table[] = {
    {"year",  TIMER_YEAR_SYS,   0},
    {"month", TIMER_MONTH_SYS,  0},
    {"day",   TIMER_DAY_SYS,    0},
    {"hour",  TIMER_HOUR_SYS,   0},
    {"min",   TIMER_MINUTE_SYS, 0},
    {"sec",   TIMER_SECOND_SYS, 0},
};
enum sw_dir { /* 切换方向 */
    DIR_NEXT = 1,
    DIR_PREV,
    DIR_SET,
};
enum set_mod { /* 加减时间方向 */
    MOD_ADD = 1,
    MOD_DEC,
    MOD_SET,
};
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
static u16 time_set_p = 0xff; /* time_set_table的当前设置指针 */
static u8 __time_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(time_set_table) / sizeof(struct time_setting);
    while (p < table_sum) {
        if (!strcmp(mark, time_set_table[p].mark)) {
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __time_set_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = time_set_p;
    u8 p;
    printf("__time_set_switch dir: %d\n", dir);
    table_sum = sizeof(time_set_table) / sizeof(struct time_setting);
    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (time_set_p >= (table_sum - 1)) {
            time_set_p = 0;
        } else {
            time_set_p++;
        }
        break;
    case DIR_PREV:
        if (time_set_p == 0) {
            time_set_p = (table_sum - 1);
        } else {
            time_set_p--;
        }
        break;
    case DIR_SET:
        p = __time_search_by_mark(mark);
        if (p == (u8) - 1) {
            return -1;
        }
        time_set_p = p;

        break;
    }
    if (prev_set_p != 0xff) {
        ui_no_highlight_element_by_id(time_set_table[prev_set_p].time_id);
    }
    ui_highlight_element_by_id(time_set_table[time_set_p].time_id);
    printf("__time_set_switch ok.\n");
    return 0;
}
static u8 __time_set_reset(void)
{
    struct sys_time time;

    time_set_p = 0xff;
    /*
     * 此处应该读取系统RTC时间
     */
    get_sys_time(&time);

    time_set_table[0].time_value = time.year;
    time_set_table[1].time_value = time.month;
    time_set_table[2].time_value = time.day;
    time_set_table[3].time_value = time.hour;
    time_set_table[4].time_value = time.min;
    time_set_table[5].time_value = time.sec;

    return 0;
}
u16 leap_month_table[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static u8 __is_leap_year(u16 year)
{
    return (((year % 4) == 0 && (year % 100) != 0) || ((year % 400) == 0));
}
static u8 __time_update_show(u8 pos) /* 更新单个时间控件的时间 */
{
    struct utime t;
    t.year  = time_set_table[0].time_value;
    t.month = time_set_table[1].time_value;
    t.day   = time_set_table[2].time_value;
    t.hour  = time_set_table[3].time_value;
    t.min   = time_set_table[4].time_value;
    t.sec   = time_set_table[5].time_value;
    ui_time_update_by_id(time_set_table[pos].time_id, &t);
    return 0;
}
static u8 __time_set_value(enum set_mod mod, u16 value)
{

    enum set_mod need_check = 0;
    u16 year, month, day;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    switch (mod) {
    case MOD_ADD:
        switch (time_set_p) {
        case 0: /* year */
            value = time_set_table[time_set_p].time_value + 1;
            if (value >= 2100) {
                value = 2099;
            }
            need_check  = MOD_ADD;
            break;
        case 1: /* month */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 12) {
                value = 1;
            }
            need_check  = MOD_ADD;
            break;
        case 2: /* day */
            month = time_set_table[1].time_value;
            ASSERT(month >= 1 && month <= 12);
            value = time_set_table[time_set_p].time_value + 1;
            if (value > leap_month_table[month - 1]) {
                value = 1;
            }
            need_check  = MOD_ADD;
            break;
        case 3: /* hour */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 23) {
                value = 0;
            }
            break;
        case 4: /* min */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        case 5: /* sec */
            value = time_set_table[time_set_p].time_value + 1;
            if (value > 59) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add time_set_p:%d err!", time_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (time_set_p) {
        case 0: /* year */
            value = time_set_table[time_set_p].time_value - 1;
            if (value <= 2000) {
                value = 2001;
            }
            need_check  = MOD_DEC;
            break;
        case 1: /* month */
            value = time_set_table[time_set_p].time_value;
            if (value == 1) {
                value = 12;
            } else {
                value--;
            }
            need_check  = MOD_DEC;
            break;
        case 2: /* day */
            value = time_set_table[time_set_p].time_value;
            if (value == 1) {
                month = time_set_table[1].time_value;
                ASSERT(month >= 1 && month <= 12);
                value = leap_month_table[month - 1];
            } else {
                value--;
            }
            need_check = MOD_DEC;
            break;
        case 3: /* hour */
            value = time_set_table[time_set_p].time_value;
            if (value == 0) {
                value = 23;
            } else {
                value--;
            }
            break;
        case 4: /* min */
            value = time_set_table[time_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        case 5: /* sec */
            value = time_set_table[time_set_p].time_value;
            if (value == 0) {
                value = 59;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec time_set_p:%d err!", time_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (time_set_p) {
        case 0: /* year */
            need_check = MOD_SET;
            break;
        case 1: /* month */
            ASSERT(value >= 1 && value <= 12);
            need_check = MOD_SET;
            break;
        case 2: /* day */
            need_check = MOD_SET;
            break;
        case 3: /* hour */
            ASSERT(value >= 0 && value <= 23);
            break;
        case 4: /* min */
            ASSERT(value >= 0 && value <= 59);
            break;
        case 5: /* sec */
            ASSERT(value >= 0 && value <= 59);
            break;
        default:
            ASSERT(0, "mod_set time_set_p:%d err!", time_set_p);
            break;
        }
        break;
    }
    time_set_table[time_set_p].time_value = value;

    if (need_check) {
        year = time_set_table[0].time_value;
        month = time_set_table[1].time_value;
        day = time_set_table[2].time_value;
        if (month == 2 && !__is_leap_year(year)) {
            if (day >= 29) {
                if (need_check == MOD_ADD) {
                    day = 1;
                } else if (need_check == MOD_DEC) {
                    day = 28;
                } else {
                    day = 28;
                }
                time_set_table[2].time_value = day;
                __time_update_show(2); /* 调整day数值显示 */
            }

        } else {
            if (day > leap_month_table[month - 1]) {
                day = leap_month_table[month - 1];
                time_set_table[2].time_value = day;
                __time_update_show(2); /* 调整day数值显示 */
            }
        }
    }
    __time_update_show(time_set_p); /* 更新当前位数值显示 */

    return 0;
}
static int __time_get(struct sys_time *t)
{
    t->year  = time_set_table[0].time_value;
    t->month = time_set_table[1].time_value;
    t->day   = time_set_table[2].time_value;
    t->hour  = time_set_table[3].time_value;
    t->min   = time_set_table[4].time_value;
    t->sec   = time_set_table[5].time_value;
    return 0;
}
static int time_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("time_layout_onchange onfocus.\n");
        layout_on_focus(layout);
        __time_set_reset();
        break;
    case ON_CHANGE_RELEASE:
        puts("time_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        puts("time_layout_onchange ON_CHANGE_FIRST_SHOW.\n");
        __time_set_switch(DIR_SET, "year");

        break;
    default:
        break;
    }
    return FALSE;
}
static int time_layout_onkey(void *ctr, struct element_key_event *e)
{

    switch (e->value) {
    case KEY_OK:
        puts("time_layout_onkey KEY_OK.\n");
        __time_set_switch(DIR_NEXT, NULL);

        break;
    case KEY_DOWN:
        __time_set_value(MOD_ADD, 0);

        break;
    case KEY_UP:
        __time_set_value(MOD_DEC, 0);

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        menu_sys_date_set(0);
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TIME_SYS);
        menu_sys_date_set(0);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_TIME_SYS)
.onchange = time_layout_onchange,
 .onkey = time_layout_onkey,
  .ontouch = NULL,
};


#if 0
/***************************** 电视制式设置 ************************************/

static int menu_tv_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item;
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        item = index_of_table8(db_select("tvm"), TABLE(table_system_tv_mod));
        ui_grid_set_item(grid, item);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_tv_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_tv_mod_set(sel_item);

        ui_hide(LAYOUT_MN_TV_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_TV_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_TV_SYS)
.onchange = menu_tv_onchange,
 .onkey = menu_tv_onkey,
  .ontouch = NULL,
};
#endif


/***************************** 格式化设置 ************************************/

static int menu_format_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        menu_sys_format_set(sel_item);

        ui_hide(LAYOUT_MN_FORMAT_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_FORMAT_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_FORMAT_SYS)
.onkey = menu_format_onkey,
 .ontouch = NULL,
};



/***************************** 默认设置设置 ************************************/

static int menu_default_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        if (sel_item) {
            it.name	= "video_system";
            it.action = ACTION_SYSTEM_SET_CONFIG;
            it.data = "def";
            __sys_msg_show(BOX_MSG_DEFAULTING, 1000);
            start_app_async(&it, NULL, NULL);
        }

        ui_hide(LAYOUT_MN_DEFAULT_SYS);

        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_DEFAULT_SYS);

        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_DEFAULT_SYS)
.onkey = menu_default_onkey,
 .ontouch = NULL,
};



/***************************** 版本 ************************************/

static int menu_version_onchange(void *ctr, enum element_change_event e, void *arg)
{

    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        /*
         * 布局key消息接管
         */
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        /*
         * 布局key消息取消接管
         */
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
static int menu_version_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_OK:
        ui_hide(LAYOUT_MN_VERSION_SYS);
        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
#ifdef KEY_MODE_EXIT_MENU
        sys_exit_menu_post();
        break;
#endif
    case KEY_MENU:
        ui_hide(LAYOUT_MN_VERSION_SYS);
        break;
    default:
        break;
    }

    return FALSE;
}


REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_VERSION_SYS)
.onchange = menu_version_onchange,
 .onkey = menu_version_onkey,
  .ontouch = NULL,
};


/***************************** 版本字符串 ************************************/
static int text_sys_version_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        //text->str = "APEMAN-C5";
        text->str = "APEMAN-C450";
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_VERSION_SYS)
.onchange = text_sys_version_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_sys_version1_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
		#ifdef GSENSOR_UP
        text->str = "V2.1.1.4.T-A";   //英文版本号
        //text->str = "V2.2.1.4.T-A";  //日文版本号
        #else
		//text->str = "V2.1.1.4-A";   //英文版本号
        text->str = "V2.2.1.4-A";  //日文版本号
		#endif
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_VERSION_SYS1)
.onchange = text_sys_version1_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


/***************************** 系统时间各个数字动作 ************************************/
static int timer_sys_year_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("timer_sys_year_onchange .\n");
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_YEAR_SYS)
.onchange = timer_sys_year_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_month_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_MONTH_SYS)
.onchange = timer_sys_month_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_day_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_DAY_SYS)
.onchange = timer_sys_day_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_hour_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_HOUR_SYS)
.onchange = timer_sys_hour_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TIMER_MINUTE_SYS)
.onchange = timer_sys_min_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int timer_sys_sec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = time_set_table[0].time_value;
        time->month = time_set_table[1].time_value;
        time->day = time_set_table[2].time_value;
        time->hour = time_set_table[3].time_value;
        time->min = time_set_table[4].time_value;
        time->sec = time_set_table[5].time_value;
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_SECOND_SYS)
.onchange = timer_sys_sec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_sys_msg_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, g_msg_id);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_SYS_MSG)
.onchange = text_sys_msg_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif





