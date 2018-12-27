#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "ui/ui_slider.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "menu_parm_api.h"
#include "action.h"
#include "app_config.h"
#include "stdlib.h"
#include "video_rec.h"

#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

struct replay_info {
    u8 onkey_mod;			/*按键选中组*/
    s8 onkey_sel;			/*按键选中项*/

    u8 type;				/*当前过滤类型*/
    u8 edit;				/*编辑模式*/
    u8 no_file;             /*没有文件 */

    u8 dec_player;			/*播放器显示状态*/
    u8 is_lock;				/*加锁文件*/
    u8 err_file;			/*损坏文件*/
    u8 file_res;    		/*文件分辨率*/
    u8 if_in_rep;           /*正在播放*/
    u8 file_type;           /*文件类型*/
    int dec_player_timer;	/*播放器隐藏计时器*/

    u8 file_dialog;			/*预览对话框内容*/
    u8 file_msg;			/*预览弹窗提示内容*/
    u8 file_timerout_msg;	/*自动隐藏的预览弹窗内容*/
    int file_msg_timer;		/*预览弹窗隐藏计时器*/

    u8 dec_msg;				/*播放弹窗提示内容*/
    u8 dec_timerout_msg;	/*自动隐藏的播放弹窗内容*/
    int dec_msg_timer;		/*播放弹窗隐藏计时器*/

    u8 dec_show_status;     /*界面状态 0:文件列表 1:解码界面  */
    u8 page_exit;           /* 退出页面方式 */

    u8 battery_val;			/*电池电量*/
    u8 battery_charging;	/*电池充电状态*/

    int file_num;			/*当前页文件数*/
    char page_cur[5];		/*当前页码值*/
    char page_tol[5];		/*总页码值*/

    char cur_path[128];		/*当前目录路径*/

    u8 edit_sel[FILE_SHOW_NUM];		/*编辑选中*/

    struct utime sum_time;  /*当前播放的视频的总时间 */
    struct utime cur_time;  /*当前播放的视频的当前时间 */

    struct ui_browser *browser;		/*文件列表控件指针*/
};

static struct replay_info handler;

#define __this 	(&handler)
#define sizeof_this     (sizeof(struct replay_info))
extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec */

extern int storage_device_ready();

const static char *cPATH[] = {
    CONFIG_DEC_PATH_1,
    CONFIG_DEC_PATH_2,
};
const static char *cTYPE[] = {
    "-tMOVAVI -sn -d",
    "-tJPG -sn -d",
    "-tMOVJPGAVI -sn -ar -d",//只读文件
};
static int file_tool[] = {
    FILE_PIC_EDIT,
    FILE_BTN_PHOTO,
    FILE_BTN_HOME,
};
const static int file_tool_type[] = {
    FILE_BTN_PHOTO,
    FILE_BTN_LOCK,
    FILE_BTN_VIDEO,
};
const static int file_tool_dir[] = {
    FILE_PIC_BACK,
    FILE_BTN_PREV,
    FILE_BTN_NEXT,
};
static int file_edit_tool[] = {
    FILE_PIC_EDIT,
    FILE_BTN_UNLOCK,
    FILE_BTN_DELETE,
};
const static int dec_tool[] = {
    DEC_PIC_LOCK,
    DEC_BTN_DELETE,
    DEC_BTN_RETURN,
};
enum {
    PAGE_SHOW = 0,
    MODE_SW_EXIT,
    HOME_SW_EXIT,
};
enum eFILE_DIALOG {
    NONE = 0,
    DEL_ALL,
    DEL_CUR,
    DEL_DIR,
    UNLOCK_ALL,
};
enum eONKEY_MOD {
    ONKEY_MOD_NORMAL = 0,
    ONKEY_MOD_NORMAL_TOOL,
    ONKEY_MOD_NORMAL_DIR,
    ONKEY_MOD_EDIT,
    ONKEY_MOD_EDIT_TOOL,
    ONKEY_MOD_EDIT_DIALOG,
    ONKEY_MOD_PLAY,
    ONKEY_MOD_PLAY_TOOL,
    ONKEY_MOD_PLAY_DIALOG,
};
enum eFILE_MSG {
    //数字越小，优先级越高，高优先级的消息能替换低优先级消息的提示(需UI布局工具修改对应的顺序)
    FILE_MSG_NONE = 0,
    FILE_MSG_POWER_OFF,
    FILE_MSG_NO_POWER,
    FILE_MSG_NO_FILE,
    FILE_MSG_DEL_FILE,
    FILE_MSG_UNLOCK_FILE,
    FILE_MSG_LOCK_FILE,
    FILE_MSG_LOCKED_FILE,
};
enum eDEC_MSG {
    //数字越小，优先级越高，高优先级的消息能替换低优先级消息的提示
    DEC_MSG_NONE = 0,
    DEC_MSG_POWER_OFF,
    DEC_MSG_NO_POWER,
    DEC_MSG_ERR_FILE,
    DEC_MSG_LOCKED_FILE,
    DEC_MSG_DEL_FILE,
};
enum eDIR {
    eDIR_FRONT = 0,
    eDIR_BACK,
};
enum eTYPE {
    eTYPE_VIDEO = 0,
    eTYPE_PHOTO,
    eTYPE_LOCK,
};





static void file_msg_show(int msg)
{
    if (__this->file_msg > msg) {
        __this->file_msg = msg;
        ui_text_show_index_by_id(FILE_TXT_MESSAGEBOX, __this->file_msg - 1);
    } else if (__this->file_msg == FILE_MSG_NONE) {
        __this->file_msg = msg;
        ui_show(FILE_LAY_MESSAGEBOX);
    }
}
static void file_msg_hide(int msg)
{
    if (__this->file_msg == msg) {
        __this->file_msg = FILE_MSG_NONE;
        ui_hide(FILE_LAY_MESSAGEBOX);
    }
}
static void file_msg_timeout()
{
    file_msg_hide(__this->file_timerout_msg);
    __this->file_msg_timer = 0;
    __this->file_timerout_msg = FILE_MSG_NONE;
}
static void file_msg_timeout_start(int msg, int ms)
{
    if (!__this->file_timerout_msg) {
        file_msg_show(msg);
        __this->file_timerout_msg = msg;
        __this->file_msg_timer = sys_timeout_add(NULL, file_msg_timeout, ms);
    } else if (__this->file_timerout_msg == msg) {
        sys_timeout_del(__this->file_msg_timer);
        __this->file_msg_timer = sys_timeout_add(NULL, file_msg_timeout, ms);
    }
}
static void dec_msg_show(int msg)
{
    if (__this->dec_msg > msg) {
        __this->dec_msg = msg;
        ui_text_show_index_by_id(DEC_TXT_MESSAGEBOX, __this->dec_msg - 1);
    } else if (__this->dec_msg == DEC_MSG_NONE) {
        __this->dec_msg = msg;
        ui_show(DEC_LAY_MESSAGEBOX);
    }
}
static void dec_msg_hide(int msg)
{
    if (__this->dec_msg == msg) {
        __this->dec_msg = DEC_MSG_NONE;
        ui_hide(DEC_LAY_MESSAGEBOX);
    }
}
static void dec_msg_timeout()
{
    dec_msg_hide(__this->dec_timerout_msg);
    __this->dec_msg_timer = 0;
    __this->dec_timerout_msg = DEC_MSG_NONE;
}
static void dec_msg_timeout_start(int msg, int ms)
{
    if (!__this->dec_timerout_msg) {
        dec_msg_show(msg);
        __this->dec_timerout_msg = msg;
        __this->dec_msg_timer = sys_timeout_add(NULL, dec_msg_timeout, ms);
    } else if (__this->dec_timerout_msg == msg) {
        sys_timeout_del(__this->dec_msg_timer);
        __this->dec_msg_timer = sys_timeout_add(NULL, dec_msg_timeout, ms);
    }
}
static int show_file_dialog(const char *cmd)
{
    __this->onkey_mod = ONKEY_MOD_EDIT_DIALOG;
    __this->onkey_sel = 0;
    printf("cmd===%s\n", cmd);
    if (!strcmp(cmd, "unlock_all")) {
        ui_text_show_index_by_id(FILE_TXT_DELETE, 2);
        ui_text_show_index_by_id(FILE_TXT_DELETE_DELETE, 1);
    } else if (!strcmp(cmd, "del_all")) {
        ui_text_show_index_by_id(FILE_TXT_DELETE, 1);
        ui_text_show_index_by_id(FILE_TXT_DELETE_DELETE, 0);
    } else if (!strcmp(cmd, "del_cur")) {
        ui_text_show_index_by_id(FILE_TXT_DELETE, 0);
        ui_text_show_index_by_id(FILE_TXT_DELETE_DELETE, 0);
    } else if (!strcmp(cmd, "del_dir")) {
        ui_text_show_index_by_id(FILE_TXT_DELETE, 3);
        ui_text_show_index_by_id(FILE_TXT_DELETE_DELETE, 0);
    }
    ui_highlight_element_by_id(FILE_BTN_DELETE_CANCEL);
    return 0;
}
static int hide_file_dialog(void)
{
    puts("hide file dialog\n");
    ui_hide(FILE_LAY_DELETE);
    __this->file_dialog  = 0;
    __this->onkey_mod = ONKEY_MOD_EDIT_TOOL;
    __this->onkey_sel = 1;
    return 0;
}
static int browser_set_dir(int p)
{
    ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
    return 0;
}
static void back_to_normal_mode(void)
{
    int i;
    ui_no_highlight_element_by_id(FILE_PIC_EDIT);
    ui_hide(file_edit_tool[1]);
    ui_show(file_tool_type[__this->type]);
    ui_hide(FILE_BTN_DELETE);
    ui_show(FILE_BTN_HOME);
    __this->edit = 0;
    __this->onkey_sel = 0;
    __this->onkey_mod = ONKEY_MOD_NORMAL;

    if (__this->browser) {
        for (int i = 0; i < __this->file_num; i++) {
            ui_core_hide((struct element *)ui_file_browser_get_child_by_id(__this->browser, i, FILE_FORM_PIC_SEL));
        }
    }
}
static void return_prev_path(char *path)
{
    int i, len;
    if (!path) {
        return;
    }
    if (!strcmp(path, CONFIG_ROOT_PATH)) {
        printf("it's root dir!\n");
        return;
    }
    len = strlen(path) - 1;
    do {
        len--;
    } while (path[len] != '/');
    for (i = len + 1; i < strlen(path) - 1; i++) {
        path[i] = 0;
    }

}
static void goto_next_path(char *path, char *name)
{
    int len;
    if (!path || !name) {
        return;
    }
    strcat(path, name);
    len = strlen(path);
    path[len] = '/';
    path[len + 1] = '\0';

}

int open_file(int p)
{

    struct intent it;
    FILE *fp = ui_file_browser_open_file(__this->browser, p);
    if (fp) {
        printf("sel=%d\n", __this->onkey_sel - 1);
        ui_hide(FILE_WIN);
        init_intent(&it);
        it.name = "video_dec";
        it.action = ACTION_VIDEO_DEC_OPEN_FILE;
        it.data = (const char *)fp;
        start_app_async(&it, NULL, NULL);
        __this->onkey_mod = ONKEY_MOD_PLAY;
        ui_show(DEC_WIN);
    }
    return 0;
}
static u8 is_dir_protect(u8 *dir_path)
{
    u8 cur_path[128];
    u8 fname[MAX_FILE_NAME_LEN];
    struct vfscan *fs;
    int attr;
    FILE *file;
    printf("dir_path=%s\n", dir_path);
    fs = fscan((char *)dir_path, "-tMOVJPG -sn -ar -d");

    if (!fs) {
        puts("folder open failed!\n");
        return 0;
    }

    file = fselect(fs, FSEL_FIRST_FILE, 0);
    while (file) {
        fget_attr(file, &attr);
        if (attr & F_ATTR_RO) {
            fclose(file);
            file = NULL;
            fscan_release(fs);
            return 1;
        } else if (attr & F_ATTR_DIR) {
            strcpy((char *)cur_path, (char *)dir_path);
            fget_name(file, fname, MAX_FILE_NAME_LEN);
            goto_next_path((char *)cur_path, (char *)fname);
            if (is_dir_protect(cur_path) == 1) {
                fclose(file);
                file = NULL;
                fscan_release(fs);
                return 1;
            }
        } else {
            fclose(file);
            file = NULL;
        }

        file = fselect(fs, FSEL_NEXT_FILE, 0);

    }
    fscan_release(fs);
    return 0;

}
static int del_sel_file(int del_dir)
{
    puts("del cur file \n");
    struct ui_file_attrs attrs;
    struct ui_text *fname_text;
    struct intent it;
    u8 cur_path[128];
    char itdata[128];
    u8 del_unlocked_file = 0;
    u8 del_cnt = 0;
    back_to_normal_mode();

    for (int i = __this->file_num - 1; i >= 0; i--) {
        if (__this->edit_sel[i]) {
            ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
            if (!(attrs.attr.attr & F_ATTR_RO)) {
                del_unlocked_file = 1;
                file_msg_show(FILE_MSG_DEL_FILE);
                break;
            }
        }
    }
    if (del_unlocked_file == 0) {
        //删除单个加锁文件显示文件已加锁提示
        file_msg_show(FILE_MSG_LOCKED_FILE);
    }

    for (int i = __this->file_num - 1; i >= 0; i--) {
        if (__this->edit_sel[i]) {
            //删除文件
            ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
            if (attrs.ftype == UI_FTYPE_DIR) {
                strcpy((char *)cur_path, __this->cur_path);
                fname_text = (struct ui_text *)ui_file_browser_get_child_by_id(__this->browser, i, FILE_FORM_TXT_NAME);
                goto_next_path((char *)cur_path, (char *)fname_text->str);
                if (is_dir_protect(cur_path) == 1 && del_dir == 0) {
                    //提示
                    file_msg_hide(FILE_MSG_DEL_FILE);
                    ui_show(FILE_LAY_DELETE);
                    show_file_dialog("del_dir");
                    __this->file_dialog = DEL_DIR;
                    sys_touch_event_enable();
                    return 0;
                }
                puts("del dir\n");
                del_cnt++;
                ui_file_browser_del_file(__this->browser, i);
                ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);//删除文件夹后，手动刷新列表
                continue;
            }
            del_cnt++;
            ui_file_browser_del_file(__this->browser, i);
        }
    }
    if (del_unlocked_file) {
        file_msg_hide(FILE_MSG_DEL_FILE);
    } else {
        os_time_dly(50);
        file_msg_hide(FILE_MSG_LOCKED_FILE);
    }

    if (del_cnt == __this->file_num) {
        //当前页删到无文件,手动刷新列表
        ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
    }

    //刷新页码
    if (ui_file_browser_page_num(__this->browser)) {
        sprintf(__this->page_cur, "%d", ui_file_browser_cur_page(__this->browser, &__this->file_num) + 1);
        sprintf(__this->page_tol, "%d", ui_file_browser_page_num(__this->browser));
        ui_text_set_str_by_id(FILE_TXT_PAGE_CUR, "ascii", __this->page_cur);
        ui_text_set_str_by_id(FILE_TXT_PAGE_TOL, "ascii", __this->page_tol);
        __this->no_file = 0;
    } else {
        __this->no_file = 1;
        __this->file_num = 0;
        strcpy(__this->page_cur, "0");
        strcpy(__this->page_tol, "0");
        ui_hide(FILE_FORM_BRO);
        file_msg_show(FILE_MSG_NO_FILE);
    }
    sys_touch_event_enable();
    return 0;
}
static void del_all_file_ok(void *p, int err)
{
    int i;
    if (err == 0) {
        puts("---del_all_file_ok\n");
    } else {
        printf("---del_file_faild: %d\n", err);
    }
    back_to_normal_mode();
    file_msg_hide(FILE_MSG_DEL_FILE);
    ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
    sys_touch_event_enable();
}
static void unlock_all_file_ok(void *p, int err)
{
    int i, file_num;
    struct ui_file_attrs attrs;
    struct intent it;
    u8 tol_page;
    u8 cur_page;
    if (err == 0) {
        puts("---unlock_all_file_ok\n");
        back_to_normal_mode();
        if (__this->type == eTYPE_LOCK) {
            //刷新加锁文件
            browser_set_dir(0);
            file_msg_hide(FILE_MSG_UNLOCK_FILE);
            /* ui_file_browser_set_dir_by_id(FILE_FORM_BRO, cPATH[__this->dir], cTYPE[__this->type]); */
            return;
        }
        //刷新第一页文件图标
        for (i = 0; i < __this->file_num; i++) {
            ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
            attrs.attr.attr &= ~F_ATTR_RO;
            ui_file_browser_set_file_attrs(__this->browser, i, &attrs);
        }
    } else {
        printf("---unlock_file_faild: %d\n", err);
    }
    file_msg_hide(FILE_MSG_UNLOCK_FILE);
    sys_touch_event_enable();
}
static int rep_current_time_handler(const char *type, u32 args)
{
    int sec;
    u32 sum_sec;
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
    ui_time_update_by_id(DEC_TIM_CUR, &t);
    sum_sec = __this->sum_time.sec + __this->sum_time.min * 60 + __this->sum_time.hour * 60 * 60;
    ui_slider_set_persent_by_id(DEC_SLI, sec * 100 / sum_sec);

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
            ui_time_update_by_id(DEC_TIM_TOL, &t);
        } else {
            t.sec = 1;
            t.min = 0;
            t.hour = 0;
            __this->sum_time.sec = 1;
            __this->sum_time.min = 0;
            __this->sum_time.hour = 0;
            ui_time_update_by_id(DEC_TIM_TOL, &t);
        }
    }
    printf("rep_film_length_handler: %d.\n", args);

    return 0;
}
static void dec_slider_timeout_func(void *priv)
{
    if (__this->if_in_rep) {
        ui_hide(DEC_BTN_PAUSE);
        ui_hide(DEC_LAY_PLAYER);
        __this->dec_player_timer = 0;
        __this->dec_player = 0;
    }
}
static int rep_play_handler(const char *type, u32 args)
{
    puts("rep_play!\n");
    __this->if_in_rep = 1;
    ui_show(DEC_BTN_PAUSE);
    ui_hide(DEC_BTN_PLAY);
    ui_pic_show_image_by_id(DEC_PIC_STATUS, 1);
    if (__this->dec_player_timer) {
        sys_timer_del(__this->dec_player_timer);
        __this->dec_player_timer = 0;
    }
    __this->dec_player_timer = sys_timeout_add(NULL, dec_slider_timeout_func, 4000);

    return 0;
}
static int rep_pause_handler(const char *type, u32 args)
{
    puts("rep_pause!\n");
    struct utime t;
    struct utime t_tol;

    __this->if_in_rep = 0;
    ui_show(DEC_BTN_PLAY);
    ui_show(DEC_LAY_PLAYER);

    t.sec = __this->cur_time.sec;
    t.min = __this->cur_time.min;
    t.hour = __this->cur_time.hour;
    ui_time_update_by_id(DEC_TIM_CUR, &t);
    t_tol.sec = __this->sum_time.sec;
    t_tol.min = __this->sum_time.min;
    t_tol.hour = __this->sum_time.hour;
    ui_time_update_by_id(DEC_TIM_TOL, &t_tol);
    ui_show(DEC_BTN_PLAY);
    ui_hide(DEC_BTN_PAUSE);
    ui_pic_show_image_by_id(DEC_PIC_STATUS, 0);

    return 0;
}
static int rep_end_handler(const char *type, u32 args)
{
    puts("rep_end!\n");
    __this->if_in_rep = 0;
    ui_show(DEC_BTN_PLAY);
    ui_hide(DEC_BTN_PAUSE);
    ui_pic_show_image_by_id(DEC_PIC_STATUS, 0);

    return 0;
}
static int rep_file_name_handler(const char *type, u32 args)
{
    const char *fname = (const char *)args;
    const char *str_p;

    puts("rep_file_name_handler\n");

    if (!strcmp(type, "ascii")) {
        str_p = (const char *)(args + strlen((char *)args) - 3);
        if (!strcmp(str_p, "JPG") || !strcmp(str_p, "jpg")) {
            __this->file_type = 0;
            ui_hide(DEC_LAY_PLAYER);
            ui_hide(DEC_BTN_PLAY);
        } else {
            __this->file_type = 1;
            ui_show(DEC_LAY_PLAYER);
            ui_show(DEC_BTN_PLAY);

        }
        __this->dec_player = 1;
        __this->if_in_rep = 0;
        __this->no_file = 0;
        __this->err_file = 0;
        ui_text_set_str_by_id(DEC_TXT_FILENAME, "ascii", (const char *)args);
    } else if (!strcmp(type, "utf16")) {

    }
    return 0;
}
static int rep_file_attribute_handler(const char *type, u32 read_only)
{
    puts("rep_file_attribute_handler \n");
    if (read_only) {
        __this->is_lock = 1;
        ui_pic_show_image_by_id(DEC_PIC_LOCK, 1);
    } else {
        __this->is_lock = 0;
        ui_pic_show_image_by_id(DEC_PIC_LOCK, 0);
    }

    return 0;
}
static int rep_file_res_handler(const char *type, u32 args)
{
    if (*type == 'w') {
        printf("rep_file_res_handler w: %d.\n", args);
        if (__this->file_type == 1) {
            if (args >= 1920) {
                /* 1080p */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "1080P 1920x1080");
                __this->file_res = 0;
            } else if (args >= 1080) {
                /* 720p */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "720P 1280x720");
                __this->file_res = 1;
            } else {
                /* vga */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "VGA 640x480");
                __this->file_res = 2;
            }
        } else if (__this->file_type == 0) {
            if (args >= 4032) {
                /* 12M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "12M 4032x3024");
                __this->file_res = 10;
            } else if (args >= 3648) {
                /* 10M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "10M 3648x2736");
                __this->file_res = 9;
            } else if (args >= 3264) {
                /* 8M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "8M 3264x2448");
                __this->file_res = 8;
            } else if (args >= 2592) {
                /* 5M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "5M 2599x1944");
                __this->file_res = 7;
            } else if (args >= 2048) {
                /* 3M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "3M 2048x1536");
                __this->file_res = 6;
            } else if (args >= 1920) {
                /* 2M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "2M 1920x1080");
                __this->file_res = 5;
            } else if (args >= 1280) {
                /* 1.3M */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "1M 1280x960");
                __this->file_res = 4;
            } else {
                /* vga */
                ui_text_set_str_by_id(DEC_TXT_RESOLUTION, "ascii", "VGA 640x480");
                __this->file_res = 3;
            }
        }
    }

    return 0;
}
static int rep_no_file_handler(const char *type, u32 args)
{
    struct intent it;
    int cur_page;
    printf("rep_no_file_handler.\n");
    __this->no_file = 1;
    ui_hide(DEC_WIN);
    ui_show(FILE_WIN);

    return 0;
}
static int rep_file_err_handler(const char *type, u32 args)
{
    puts("rep_file_err_handler \n");
    dec_msg_show(DEC_MSG_ERR_FILE);
    __this->err_file = 1;
    return 0;
}
static void sd_event_handler(struct sys_event *event, void *priv)
{
    puts("sd_event_handler\n");
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("dev event online\n");
            file_msg_hide(FILE_MSG_NO_FILE);
            strcpy(__this->cur_path, CONFIG_ROOT_PATH);
            ui_show(FILE_FORM_LAY);
            break;
        case DEVICE_EVENT_OUT:
            puts("dev event offline\n");
            ui_hide(FILE_FORM_LAY);
            if (__this->file_dialog) {
                hide_file_dialog();
            }
            file_msg_show(FILE_MSG_NO_FILE);
            //页数清0
            strcpy(__this->page_cur, "0");
            strcpy(__this->page_tol, "0");
            ui_text_set_str_by_id(FILE_TXT_PAGE_CUR, "ascii", __this->page_cur);
            ui_text_set_str_by_id(FILE_TXT_PAGE_TOL, "ascii", __this->page_tol);
            if (__this->edit) {
                back_to_normal_mode();
            }
            break;
        default:
            break;
        }
    }
}
static void del_file_callback(void *priv, int err)
{
    /* printf("del_file_callback: err=%d\n", err); */
    if (err == 0) {
        puts("---del_file_ok\n");
        ui_hide(DEC_WIN);
        ui_show(FILE_WIN);
    } else {
        printf("---del_file_faild: %d\n", err);
    }
}
static const struct uimsg_handl rep_msg_handler[] = {
    { "fname",     rep_file_name_handler     },
    /* { "fnum",      rep_file_number_handler   }, */
    { "res",       rep_file_res_handler      },
    { "filmLen",   rep_film_length_handler   },
    { "plyTime",   rep_current_time_handler  },
    { "fattr",     rep_file_attribute_handler},
    { "noFile",    rep_no_file_handler       },
    { "fileErr",   rep_file_err_handler      },
    { "play",      rep_play_handler          },
    { "pause",     rep_pause_handler         },
    { "end",       rep_end_handler           },
    /* { "ff",        rep_ff_handler            }, */
    /* { "fr",        rep_fr_handler            }, */
    { NULL, NULL},
};







static void cfun_table_normal(int p)
{
    struct ui_file_attrs attrs;
    ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, p, FILE_FORM_VID));

    ui_file_browser_get_file_attrs(__this->browser, p, &attrs); //获取文件属性
    if (attrs.ftype == UI_FTYPE_DIR) {
        goto_next_path(__this->cur_path, attrs.fname);
        ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
        return;
    }

    __this->onkey_mod = ONKEY_MOD_PLAY;
    __this->onkey_sel = p + 1;

    if (attrs.attr.attr & F_ATTR_RO) {
        __this->is_lock = 1;
    } else {
        __this->is_lock = 0;
    }
    ui_set_call(open_file, p);
}
static void cfun_table_edit(int p)
{
    int i;
    struct ui_file_attrs attrs;

    ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, p, FILE_FORM_VID));
    __this->onkey_mod = ONKEY_MOD_EDIT;
    __this->onkey_sel = p + 1;

    if (__this->edit_sel[p]) {
        __this->edit_sel[p] = 0;
        ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, p, FILE_FORM_PIC_SEL));
    } else {
        __this->edit_sel[p] = 1;
        ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, p, FILE_FORM_PIC_SEL));
    }
    if (file_edit_tool[1] != FILE_BTN_LOCK) {
        for (i = 0; i < __this->file_num; i++) {
            if (__this->edit_sel[i]) {
                ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
                if ((attrs.attr.attr & F_ATTR_RO) == 0) {
                    ui_hide(FILE_BTN_UNLOCK);
                    ui_show(FILE_BTN_LOCK);
                    file_edit_tool[1] = FILE_BTN_LOCK;
                    break;
                }
            }
        }
    } else {
        for (i = 0; i < __this->file_num; i++) {
            if (__this->edit_sel[i]) {
                ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
                if ((attrs.attr.attr & F_ATTR_RO) == 0) {
                    break;
                }
            }
        }
        if (i >= __this->file_num) {
            ui_hide(FILE_BTN_LOCK);
            ui_show(FILE_BTN_UNLOCK);
            file_edit_tool[1] = FILE_BTN_UNLOCK;
        }
    }
}
static void cfun_file_home()
{
    struct intent it;
    struct application *app;
    init_intent(&it);
    app = get_current_app();
    if (app) {
        it.name = "video_dec";
        it.action = ACTION_BACK;
        start_app_async(&it, NULL, NULL); //不等待直接启动app
    }
    __this->page_exit = HOME_SW_EXIT;
}
static void cfun_file_back()
{
    if (!strcmp(__this->cur_path, CONFIG_ROOT_PATH)) {
        cfun_file_home();
        return;
    }
    if (__this->no_file) {
        file_msg_hide(FILE_MSG_NO_FILE);
        ui_show(FILE_FORM_BRO);
        __this->no_file = 0;
    }
    return_prev_path(__this->cur_path);
    ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
}
static void cfun_file_next()
{
    if (__this->edit) {
        return;
    }
    if (strcmp(__this->page_cur, __this->page_tol)) {
        /* ui_set_call(ui_file_browser_next_page_by_id,FILE_FORM_BRO); */
        ui_file_browser_next_page_by_id(FILE_FORM_BRO);
        if (__this->onkey_mod == ONKEY_MOD_NORMAL) {
            __this->onkey_sel = 0;
        }
    }
}
static void cfun_file_prev()
{
    if (__this->edit) {
        return;
    }
    if (strcmp(__this->page_cur, "1") && strcmp(__this->page_cur, "0")) {
        /* ui_set_call(ui_file_browser_prev_page_by_id,FILE_FORM_BRO); */
        ui_file_browser_prev_page_by_id(FILE_FORM_BRO);
        if (__this->onkey_mod == ONKEY_MOD_NORMAL) {
            __this->onkey_sel = 0;
        }
    }
}
static void cfun_file_edit()
{
    if (__this->no_file) {
        return;
    }
    if (__this->edit) {
        __this->onkey_mod = ONKEY_MOD_NORMAL;
        if (__this->onkey_sel) {
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
        }
        back_to_normal_mode();
    } else {
        __this->onkey_mod = ONKEY_MOD_EDIT;
        if (__this->onkey_sel) {
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
        }
        __this->edit = 1;
        ui_highlight_element_by_id(FILE_PIC_EDIT);
        ui_hide(file_tool_type[__this->type]);
        ui_hide(FILE_BTN_HOME);
        ui_show(FILE_BTN_UNLOCK);
        ui_show(FILE_BTN_DELETE);
        file_edit_tool[1] = FILE_BTN_UNLOCK;
        for (int i = 0; i < __this->file_num; i++) {
            ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, i, FILE_FORM_PIC_SEL));
            __this->edit_sel[i] = 0;
            puts("show sel\n");
            ui_core_show((struct element *)ui_file_browser_get_child_by_id(__this->browser, i, FILE_FORM_PIC_SEL), false);
        }
    }
}
static void cfun_file_type()
{
    ui_no_highlight_element_by_id(file_tool[1]);
    ui_hide(file_tool_type[__this->type]);
    __this->type = __this->type >= 2 ? 0 : __this->type + 1;
    file_tool[1] = file_tool_type[__this->type];
    ui_show(file_tool_type[__this->type]);
    if (storage_device_ready()) {
        if (__this->no_file) {
            ui_show(FILE_FORM_BRO);
        }
        /* ui_set_call(browser_set_dir,0); */
        ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
        if (__this->onkey_mod == ONKEY_MOD_NORMAL) {
            __this->onkey_sel = 0;
        }
    }
}
static void cfun_file_dialog()
{
    u8 dir;
    struct intent it;
    __this->onkey_mod = ONKEY_MOD_EDIT_TOOL;
    if (__this->file_dialog == UNLOCK_ALL) {
        hide_file_dialog();
        file_msg_show(FILE_MSG_UNLOCK_FILE);
        init_intent(&it);
        it.name	= "video_dec";
        it.action = ACTION_VIDEO_DEC_SET_CONFIG;
        it.data = "unlock:all";
        if (!strcmp(__this->cur_path, CONFIG_DEC_PATH_1)) {
            dir = 0;
        } else {
            dir = 1;
        }
        it.exdata = dir * 10 + __this->type;
        /* start_app_async(&it, unlock_all_file_ok, NULL); */
        start_app(&it);
        unlock_all_file_ok(NULL, 0);
    } else if (__this->file_dialog == DEL_CUR) {
        sys_touch_event_disable();
        hide_file_dialog();
        ui_set_call(del_sel_file, 0);
    } else if (__this->file_dialog == DEL_DIR) {
        sys_touch_event_disable();
        hide_file_dialog();
        ui_set_call(del_sel_file, 1);
    } else if (__this->file_dialog == DEL_ALL) {
        hide_file_dialog();
        file_msg_show(FILE_MSG_DEL_FILE);
        init_intent(&it);
        it.name	= "video_dec";
        it.action = ACTION_VIDEO_DEC_SET_CONFIG;
        /* it.data = "del:all"; */
        /* if (!strcmp(__this->cur_path, CONFIG_DEC_PATH_1)) { */
        /* dir = 0; */
        /* } else { */
        /* dir = 1; */
        /* } */
        /* it.exdata = dir * 10 + __this->type; */
        static char itdata[128];
        sprintf(itdata, "delall:%s", __this->cur_path);
        it.data = itdata;
        it.exdata = __this->type;
        sys_touch_event_disable();
        start_app_async(&it, del_all_file_ok, NULL);
    }
}
static void cfun_file_lock()
{
    struct ui_file_attrs attrs;
    for (int i = 0; i < __this->file_num; i++) {
        if (__this->edit_sel[i]) {
            __this->edit_sel[i] = 0;
            ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, i, FILE_FORM_PIC_SEL));
            ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
            if (attrs.ftype == UI_FTYPE_DIR) {
                continue;
            }
            if ((attrs.attr.attr & F_ATTR_RO) == 0) {
                //加锁文件
                attrs.attr.attr |= F_ATTR_RO;
                ui_file_browser_set_file_attrs(__this->browser, i, &attrs);
            }
        }
    }
    ui_no_highlight_element_by_id(file_edit_tool[1]);
    ui_hide(file_edit_tool[1]);
    file_edit_tool[1] = FILE_BTN_UNLOCK;
    ui_show(file_edit_tool[1]);
    if (__this->onkey_mod == ONKEY_MOD_EDIT_TOOL) {
        ui_highlight_element_by_id(file_edit_tool[1]);
    }
}
static void cfun_file_unlock()
{
    struct ui_file_attrs attrs;
    u8 unlock_all_file_mark = 1;
    if (__this->edit) {
        for (int i = 0; i < __this->file_num; i++) {
            if (__this->edit_sel[i]) {
                unlock_all_file_mark = 0;
                ui_file_browser_get_file_attrs(__this->browser, i, &attrs);
                if (attrs.attr.attr & F_ATTR_RO) {
                    //解锁文件
                    __this->edit_sel[i] = 0;
                    ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, i, FILE_FORM_PIC_SEL));
                    attrs.attr.attr &= ~F_ATTR_RO;
                    ui_file_browser_set_file_attrs(__this->browser, i, &attrs);
                }
            }
        }
        if (unlock_all_file_mark == 0) {
            if (__this->type == eTYPE_LOCK) {
                //加锁文件刷新
                back_to_normal_mode();
                /* ui_set_call(browser_set_dir,0); */
                ui_file_browser_set_dir_by_id(FILE_FORM_BRO, __this->cur_path, cTYPE[__this->type]);
            }
        }
        if (unlock_all_file_mark) {
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
                __this->onkey_sel = 0;
            }
            puts("\n unlock_all_file \n");
            ui_no_highlight_element_by_id(FILE_BTN_UNLOCK);
            ui_show(FILE_LAY_DELETE);
            ui_set_call((int (*)(int))show_file_dialog, (int)"unlock_all");
            __this->file_dialog = UNLOCK_ALL;
        }
    }
}
static void cfun_file_delete()
{
    ui_no_highlight_element_by_id(FILE_BTN_DELETE);
    if (__this->edit) {
        if (__this->type == eTYPE_LOCK) {
            //加锁文件类型不删除
            //弹窗
            return;
        }
        for (int i = 0; i < __this->file_num; i++) {
            if (__this->edit_sel[i]) {
                ui_show(FILE_LAY_DELETE);
                ui_set_call((int (*)(int))show_file_dialog, (int)"del_cur");
                __this->file_dialog = DEL_CUR;
                return;
            }
        }

        puts("\n del all file  \n");
        ui_show(FILE_LAY_DELETE);
        ui_set_call((int (*)(int))show_file_dialog, (int)"del_all");
        __this->file_dialog = DEL_ALL;
    }
}


static void cfun_dec_ok()
{
    struct intent it;
    init_intent(&it);
    it.name = "video_dec";
    it.action = ACTION_VIDEO_DEC_CONTROL;
    start_app(&it);
    if (__this->dec_player) {
        if (__this->dec_player_timer) {
            sys_timeout_del(__this->dec_player_timer);
            __this->dec_player_timer = sys_timer_add(NULL, dec_slider_timeout_func, 4000);
        }
    }
}
static void cfun_dec_lock()
{
    struct intent it;
    if (__this->if_in_rep == 0) {
        if (__this->is_lock) {
            init_intent(&it);
            it.name	= "video_dec";
            it.data = "unlock:cur";
            it.action = ACTION_VIDEO_DEC_SET_CONFIG;
            start_app(&it);
        } else {
            init_intent(&it);
            it.name	= "video_dec";
            it.data = "lock:cur";
            it.action = ACTION_VIDEO_DEC_SET_CONFIG;
            start_app(&it);
        }
    }
}
static void cfun_dec_delete()
{
    if (__this->if_in_rep == 0) {
        if (__this->is_lock) {
            //是加锁文件
            puts("is lock file\n");
            dec_msg_timeout_start(DEC_MSG_LOCKED_FILE, 1000);
            return;
        }
        //弹窗
        __this->onkey_mod = ONKEY_MOD_PLAY_DIALOG;
        __this->onkey_sel = 0;
        ui_show(DEC_LAY_DELETE);
        ui_highlight_element_by_id(DEC_BTN_DELETE_CANCEL);
    }
}
static void cfun_dec_return()
{
    struct intent it;
    if (__this->if_in_rep == 0) {
        ui_hide(DEC_WIN);
        init_intent(&it);
        it.name = "video_dec";
        it.action = ACTION_VIDEO_DEC_CUR_PAGE;
        start_app(&it);
        ui_show(FILE_WIN);
    }
}
static void cfun_dec_dialog()
{
    struct intent it;
    ui_hide(DEC_LAY_DELETE);
    it.name	= "video_dec";
    it.action = ACTION_VIDEO_DEC_SET_CONFIG;
    it.data = "del:cur";
    start_app_async(&it, del_file_callback, NULL);
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
            if (__this->dec_show_status == 0) {
                file_msg_hide(FILE_MSG_POWER_OFF);
            } else {
                dec_msg_hide(DEC_MSG_POWER_OFF);
            }
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
                if (__this->battery_charging == 0) {
                    /* ui_battery_level_change(__this->battery_val, 0); */
                }
            }
        } else if (!ASCII_StrCmp(event->arg, "charger", 7)) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                /* ui_battery_level_change(100, 1); */
                __this->battery_charging = 1;
                if (ten_sec_off) {
                    ten_sec_off = 0;
                    if (__this->dec_show_status == 0) {
                        file_msg_hide(FILE_MSG_POWER_OFF);
                    } else {
                        dec_msg_hide(DEC_MSG_POWER_OFF);
                    }
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                /* ui_battery_level_change(__this->battery_val, 0); */
                __this->battery_charging = 0;
                if (__this->dec_show_status == 0) {
                    file_msg_show(FILE_MSG_POWER_OFF);
                } else {
                    dec_msg_show(DEC_MSG_POWER_OFF);
                }
                ten_sec_off = 1;
            }
        }
    }
}
static void dec_no_power_msg_box_timer(void *priv)
{
    static u32 cnt = 0;
    if (__this->battery_val <= 20  && __this->battery_charging == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            puts("no power show.\n");
            if (__this->dec_show_status == 0) {
                file_msg_show(FILE_MSG_NO_POWER);
            } else {
                dec_msg_show(DEC_MSG_NO_POWER);
            }
        } else {
            puts("no power hide.\n");
            if (__this->dec_show_status == 0) {
                file_msg_hide(FILE_MSG_NO_POWER);
            } else {
                dec_msg_hide(DEC_MSG_NO_POWER);
            }
        }
    } else {
        if (__this->dec_show_status == 0) {
            file_msg_hide(FILE_MSG_NO_POWER);
        } else {
            dec_msg_hide(DEC_MSG_NO_POWER);
        }
        cnt = 0;
    }
}


static int replay_mode_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    static u32 timer_handle = 0;
    static u16 id = 0;
    static u16 id_bat = 0;
    static void *fd = NULL;
    switch (e) {
    case ON_CHANGE_INIT:
        memset(__this, 0, sizeof_this);
        strcpy(__this->cur_path, CONFIG_ROOT_PATH);
        file_tool[1] = file_tool_type[0];

        ui_register_msg_handler(ID_WINDOW_VIDEO_REP, rep_msg_handler);
        if (id == 0) {
            id = register_sys_event_handler(SYS_DEVICE_EVENT, 0, 0, sd_event_handler);
        }
        id_bat = register_sys_event_handler(SYS_DEVICE_EVENT | SYS_KEY_EVENT | SYS_TOUCH_EVENT, 200, fd, battery_event_handler);
        sys_cur_mod = 3;
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
        __this->battery_charging = (usb_is_charging() ? 1 : 0);

        timer_handle = sys_timer_add(NULL, dec_no_power_msg_box_timer, 1000);
        break;
    case ON_CHANGE_RELEASE:
        puts("replay page release\n");
        if (id || id_bat) {
            unregister_sys_event_handler(id);
            unregister_sys_event_handler(id_bat);
            id = 0;
            id_bat = 0;
        }
        if (timer_handle) {
            sys_timer_del(timer_handle);
            timer_handle = 0;
        }
        if (get_parking_status()) {
            break;
        }

        if (__this->page_exit == HOME_SW_EXIT) {
            ui_show(ID_WINDOW_MAIN_PAGE);
        }

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REP)
.onchange = replay_mode_onchange,
};


static int file_msg_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->file_msg) {
            ui_text_show_index_by_id(FILE_TXT_MESSAGEBOX, __this->file_msg - 1);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_LAY_MESSAGEBOX)
.onchange = file_msg_onchange,
};
static int dec_msg_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->dec_msg) {
            ui_text_show_index_by_id(DEC_TXT_MESSAGEBOX, __this->dec_msg - 1);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_LAY_MESSAGEBOX)
.onchange = dec_msg_onchange,
};
static int dec_right_tool_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->type == eTYPE_PHOTO) {
            ui_hide(FILE_BTN_PHOTO);
            ui_show(FILE_BTN_LOCK);
        } else if (__this->type == eTYPE_LOCK) {
            ui_hide(FILE_BTN_PHOTO);
            ui_show(FILE_BTN_VIDEO);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_LAY_TOOL)
.onchange = dec_right_tool_onchange,
};


static int file_browse_table_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u32 cur_page;
    switch (e) {
    case ON_CHANGE_INIT:
        __this->browser = (struct ui_browser *)ctr;
        __this->browser->order = 1;
        __this->browser->path = __this->cur_path;
        /* __this->browser->path = cPATH[__this->dir]; */
        __this->browser->ftype = cTYPE[__this->type];
        ASCII_StrToInt(__this->page_cur, &cur_page, strlen(__this->page_cur));
        if (cur_page) {
            ui_file_browser_set_page(__this->browser, cur_page - 1);
        }
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        if (ui_file_browser_page_num(__this->browser)) {
            sprintf(__this->page_cur, "%d", ui_file_browser_cur_page(__this->browser, NULL) + 1);
            sprintf(__this->page_tol, "%d", ui_file_browser_page_num(__this->browser));
            __this->no_file = 0;
            file_msg_hide(FILE_MSG_NO_FILE);
        } else {
            __this->no_file = 1;
            ui_hide(FILE_FORM_BRO);
            strcpy(__this->page_cur, "0");
            strcpy(__this->page_tol, "0");
        }
        ui_text_set_str_by_id(FILE_TXT_PAGE_CUR, "ascii", __this->page_cur);
        ui_text_set_str_by_id(FILE_TXT_PAGE_TOL, "ascii", __this->page_tol);

        if (__this->no_file) {
            __this->file_num = 0;
            file_msg_show(FILE_MSG_NO_FILE);
        } else {
            ui_file_browser_cur_page(__this->browser, &__this->file_num);
            if (__this->onkey_sel && __this->onkey_sel < __this->file_num) {
                ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
        }
        printf("file_num =%d\n", __this->file_num);
        break;
    case ON_CHANGE_RELEASE:
        __this->browser = NULL;
        break;
    default:
        return false;
    }
    return false;
}
static int file_browse_table_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file_browse table ontouch**");
    u8 select_grid = ui_grid_cur_item(__this->browser->grid);
    int i;
    switch (e->event) {
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->no_file || select_grid >= __this->file_num) {
            break;
        }
        if (__this->edit) {
            if (__this->onkey_mod == ONKEY_MOD_EDIT) {
                if (__this->onkey_sel) {
                    ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
                }
            } else if (__this->onkey_mod == ONKEY_MOD_EDIT_TOOL) {
                if (__this->onkey_sel > 1) {
                    ui_no_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
                }
            }
            __this->onkey_sel = 0;
            cfun_table_edit(select_grid);
        } else {

            if (__this->onkey_mod == ONKEY_MOD_NORMAL) {
                if (__this->onkey_sel) {
                    ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
                }
            } else if (__this->onkey_mod == ONKEY_MOD_NORMAL_TOOL) {
                if (__this->onkey_sel > 1) {
                    ui_no_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
                }
            } else if (__this->onkey_mod == ONKEY_MOD_NORMAL_DIR) {
                if (__this->onkey_sel) {
                    ui_no_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
                }
            }
            __this->onkey_sel = 0;
            cfun_table_normal(select_grid);
        }
        break;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(FILE_FORM_BRO)
.onchange = file_browse_table_onchange,
 .ontouch = file_browse_table_ontouch,
};

static int file_browse_down_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file_browse down ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_next();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_NEXT)
.ontouch = file_browse_down_ontouch,
};

static int file_browse_prev_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file_browse up ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_prev();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_PREV)
.ontouch = file_browse_prev_ontouch,
};



static int dec_dir_return_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_dir_return**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->edit == 0) {
            cfun_file_back();
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_PIC_BACK)
.ontouch = dec_dir_return_ontouch,
};
static int dec_edit_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_edit_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        return true;
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (!dev_online(SDX_DEV)) {
            break;
        }

        if (__this->onkey_mod == ONKEY_MOD_NORMAL || __this->onkey_mod == ONKEY_MOD_EDIT) {
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
        } else if (__this->onkey_mod == ONKEY_MOD_NORMAL_TOOL) {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            }
        } else if (__this->onkey_mod == ONKEY_MOD_NORMAL_DIR) {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            }
        } else if (__this->onkey_mod == ONKEY_MOD_EDIT_TOOL) {
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            }
        }
        __this->onkey_sel = 0;
        cfun_file_edit();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_EDIT)
.ontouch = dec_edit_ontouch,
};

static int dec_home_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_home_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_home();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_HOME)
.ontouch = dec_home_ontouch,
};

static int dec_photo_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_photo_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_type();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_PHOTO)
.ontouch = dec_photo_ontouch,
};

static int file_dialog_cancal_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file dialog cancal ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        hide_file_dialog();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_DELETE_CANCEL)
.ontouch = file_dialog_cancal_ontouch,
};
static int file_dialog_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file dialog confirm ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_dialog();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_DELETE_DELETE)
.ontouch = file_dialog_confirm_ontouch,
};

static int dec_lock_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_lock_ontouch**");
    u8 lock_all_file_mark = 1;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->edit) {
            cfun_file_lock();
        } else {
            cfun_file_type();
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_LOCK)
.ontouch = dec_lock_ontouch,
};

static int dec_unlock_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_unlock_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_unlock();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_UNLOCK)
.ontouch = dec_unlock_ontouch,
};

static int dec_delete_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_delete_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_delete();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_DELETE)
.ontouch = dec_delete_ontouch,
};

static int file_dialog_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_SHOW:
        ui_ontouch_lock(layout); /* 对话框聚焦 */
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_LAY_DELETE)
.onchange = file_dialog_onchange,
};
static int dec_video_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec_video_ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_file_type();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(FILE_BTN_VIDEO)
.ontouch = dec_video_ontouch,
};

/************************ 解码界面控件动作************************************/
static int dec_dialog_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_SHOW:
        ui_ontouch_lock(layout);
        break;
    case ON_CHANGE_RELEASE:
        ui_ontouch_unlock(layout);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_LAY_DELETE)
.onchange = dec_dialog_onchange,
};

static int dec_lay_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->is_lock) {
            ui_pic_show_image_by_id(DEC_PIC_LOCK, 1);
        } else {
            ui_pic_show_image_by_id(DEC_PIC_LOCK, 0);
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_LAY_TOOL)
.onchange = dec_lay_onchange,
};
static int dec_play_button_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec play button ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->file_type) {
            if (__this->if_in_rep == 0) {
                if (__this->dec_player) {
                    cfun_dec_ok();
                }
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_PLAY)
.ontouch = dec_play_button_ontouch,
};
static int dec_pause_button_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec pause button ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->file_type) {
            if (__this->if_in_rep == 1) {
                if (__this->dec_player) {
                    cfun_dec_ok();
                }
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_PAUSE)
.ontouch = dec_pause_button_ontouch,
};
static void show_cur_page_file(void *p, int err)
{
    puts("show cur page file.\n");
    ui_show(FILE_WIN);
}
static int dec_return_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec return ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_dec_return();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_RETURN)
.ontouch = dec_return_ontouch,
};

static int dec_player_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int item, id;
    const char *str = NULL;
    struct intent it;
    struct utime t;
    struct utime t_tol;
    int ret;
    int err;
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        if (__this->if_in_rep) {
            ui_pic_show_image_by_id(DEC_PIC_STATUS, 1);
        } else {
            ui_pic_show_image_by_id(DEC_PIC_STATUS, 0);
        }
        t.sec = __this->cur_time.sec;
        t.min = __this->cur_time.min;
        t.hour = __this->cur_time.hour;
        ui_time_update_by_id(DEC_TIM_CUR, &t);
        t_tol.sec = __this->sum_time.sec;
        t_tol.min = __this->sum_time.min;
        t_tol.hour = __this->sum_time.hour;
        ui_time_update_by_id(DEC_TIM_TOL, &t_tol);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_LAY_PLAYER)
.onchange = dec_player_layout_onchange,
};
static int dec_layout_button_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec layout button ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if (__this->file_type) {
            if (__this->if_in_rep == 0) {
                if (__this->dec_player) {
                    ui_hide(DEC_LAY_PLAYER);
                    ui_hide(DEC_BTN_PLAY);
                    if (__this->dec_player_timer) {
                        sys_timer_del(__this->dec_player_timer);
                        __this->dec_player_timer = 0;
                    }
                    __this->dec_player = 0;
                } else {
                    ui_show(DEC_LAY_PLAYER);
                    ui_show(DEC_BTN_PLAY);
                    __this->dec_player = 1;
                }
            } else {
                if (__this->dec_player) {
                    ui_hide(DEC_LAY_PLAYER);
                    ui_hide(DEC_BTN_PAUSE);
                    if (__this->dec_player_timer) {
                        sys_timer_del(__this->dec_player_timer);
                        __this->dec_player_timer = 0;
                    }
                    __this->dec_player = 0;
                } else {
                    ui_show(DEC_LAY_PLAYER);
                    ui_show(DEC_BTN_PAUSE);
                    if (__this->dec_player_timer) {
                        sys_timeout_del(__this->dec_player_timer);
                        __this->dec_player_timer = 0;
                    }
                    __this->dec_player_timer = sys_timeout_add(NULL, dec_slider_timeout_func, 4000);
                    __this->dec_player = 1;
                }
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_SHOW)
.ontouch = dec_layout_button_ontouch,
};
static int dec_lock_unlockfile_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec lock or unlock file ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_dec_lock();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_LOCK)
.ontouch = dec_lock_unlockfile_ontouch,
};
static int dec_delfile_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec del file ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_dec_delete();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_DELETE)
.ontouch = dec_delfile_ontouch,
};
static int dec_del_cancal_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**dec del cancal ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        ui_hide(DEC_LAY_DELETE);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_DELETE_CANCEL)
.ontouch = dec_del_cancal_ontouch,
};

static int dec_del_confirm_ontouch(void *ctr, struct element_touch_event *e)
{
    struct intent it;
    UI_ONTOUCH_DEBUG("**dec del confirm ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        cfun_dec_dialog();
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_BTN_DELETE_DELETE)
.ontouch = dec_del_confirm_ontouch,
};












static int file_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        sys_key_event_takeover(true, false);
        __this->onkey_mod = ONKEY_MOD_NORMAL;
        __this->file_msg = FILE_MSG_NONE;
        __this->dec_show_status = 0;
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        ui_core_element_on_focus(ui_core_get_element_by_id(FILE_WIN), true);
        if (!storage_device_ready()) {
            puts("no sd card!\n");
            ui_hide(FILE_FORM_LAY);
        }
        break;
    case ON_CHANGE_RELEASE:
    case ON_CHANGE_HIDE:
        if (__this->file_msg) {
            if (__this->file_msg_timer) {
                sys_timer_del(__this->file_msg_timer);
                __this->file_msg_timer = 0;
                __this->file_timerout_msg = FILE_MSG_NONE;
            }
            file_msg_hide(__this->file_msg);
        }
        ui_core_element_on_focus(ui_core_get_element_by_id(FILE_WIN), false);
        break;
    default:
        return false;
    }

    return false;
}
static int file_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    struct application *app;
    struct ui_file_attrs attrs;
    FILE *fp;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }

    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    if (__this->onkey_mod == ONKEY_MOD_NORMAL) {
        //预览模式中间窗口
        switch (e->value) {
        case KEY_UP:
        case KEY_LEFT:
            if (__this->file_num == 0) {
                __this->onkey_sel = 0;
                break;
            }
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
            __this->onkey_sel = __this->onkey_sel <= 1 ? __this->file_num : __this->onkey_sel - 1;
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            break;
        case KEY_DOWN:
        case KEY_RIGHT:
            if (__this->file_num == 0) {
                __this->onkey_sel = 0;
                break;
            }
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
            __this->onkey_sel = __this->onkey_sel >= __this->file_num ? 1 : __this->onkey_sel + 1;
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            break;
        case KEY_OK:
            if (__this->file_num == 0 || __this->onkey_sel == 0) {
                __this->onkey_sel = 0;
                break;
            }
            cfun_table_normal(__this->onkey_sel - 1);
            break;
        case KEY_MODE:
            //MODE键切换到工具栏
            if (__this->onkey_sel && !__this->no_file) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
            __this->onkey_mod = ONKEY_MOD_NORMAL_TOOL;
            __this->onkey_sel = 1;
            ui_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_NORMAL_TOOL) {
        switch (e->value) {
        case KEY_UP:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel <= 1 ? 3 : __this->onkey_sel - 1;
            ui_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel >= 3 ? 1 : __this->onkey_sel + 1;
            ui_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            switch (__this->onkey_sel) {
            case 1:
                __this->onkey_sel = 1;
                cfun_file_edit();
                break;
            case 2:
                cfun_file_type();
                break;
            case 3:
                cfun_file_home();
                break;
            }
            break;
        case KEY_MODE:
            //MODE键切换到左侧栏
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = ONKEY_MOD_NORMAL_DIR;
            __this->onkey_sel = 1;
            ui_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            break;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_NORMAL_DIR) {
        switch (e->value) {
        case KEY_UP:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel <= 1 ? 3 : __this->onkey_sel - 1;
            ui_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel >= 3 ? 1 : __this->onkey_sel + 1;
            ui_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            if (__this->edit) {
                break;
            }
            switch (__this->onkey_sel) {
            case 1:
                cfun_file_back();
                break;
            case 2:
                cfun_file_prev();
                break;
            case 3:
                cfun_file_next();
                break;
            }
            break;
        case KEY_MODE:
            //MODE键切换到预览区
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(file_tool_dir[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = ONKEY_MOD_NORMAL;
            __this->onkey_sel = 1;
            if (!__this->no_file) {
                ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
                __this->onkey_sel = 0;
            }
            break;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_EDIT) {
        switch (e->value) {
        case KEY_UP:
        case KEY_LEFT:
            if (__this->file_num == 0) {
                __this->onkey_sel = 0;
                break;
            }
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
            __this->onkey_sel = __this->onkey_sel <= 1 ? __this->file_num : __this->onkey_sel - 1;
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            break;
        case KEY_DOWN:
        case KEY_RIGHT:
            if (__this->file_num == 0) {
                __this->onkey_sel = 0;
                break;
            }
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
            __this->onkey_sel = __this->onkey_sel >= __this->file_num ? 1 : __this->onkey_sel + 1;
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            break;
        case KEY_OK:
            cfun_table_edit(__this->onkey_sel - 1);
            break;
        case KEY_MODE:
            if (__this->onkey_sel) {
                ui_no_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            }
            __this->onkey_mod = ONKEY_MOD_EDIT_TOOL;
            __this->onkey_sel = 1;
            ui_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            break;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_EDIT_TOOL) {
        switch (e->value) {
        case KEY_UP:
            if (__this->onkey_sel > 1) {
                ui_no_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel <= 1 ? 3 : __this->onkey_sel - 1;
            ui_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            if (__this->onkey_sel > 1) {
                ui_no_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel >= 3 ? 1 : __this->onkey_sel + 1;
            ui_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            switch (__this->onkey_sel) {
            case 1:
                __this->onkey_sel = 1;
                cfun_file_edit();
                ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
                break;
            case 2:
                if (file_edit_tool[1] == FILE_BTN_LOCK) {
                    cfun_file_lock();
                } else {
                    cfun_file_unlock();
                }
                break;
            case 3:
                cfun_file_delete();
                break;
            }
            break;
        case KEY_MODE:
            __this->onkey_mod = ONKEY_MOD_EDIT;
            if (__this->onkey_sel > 1) {
                ui_no_highlight_element_by_id(file_edit_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = 1;
            ui_highlight_element((struct element *)ui_file_browser_get_child_by_id(__this->browser, __this->onkey_sel - 1, FILE_FORM_VID));
            break;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_EDIT_DIALOG) {
        switch (e->value) {
        case KEY_UP:
        case KEY_DOWN:
            if (__this->onkey_sel) {
                ui_highlight_element_by_id(FILE_BTN_DELETE_CANCEL);
                ui_no_highlight_element_by_id(FILE_BTN_DELETE_DELETE);
                __this->onkey_sel = 0;
            } else {
                ui_highlight_element_by_id(FILE_BTN_DELETE_DELETE);
                ui_no_highlight_element_by_id(FILE_BTN_DELETE_CANCEL);
                __this->onkey_sel = 1;
            }
            break;
        case KEY_OK:
            if (__this->onkey_sel) {
                cfun_file_dialog();
            } else {
                hide_file_dialog();
            }
            break;
        case KEY_MODE:
            hide_file_dialog();
            break;
        }
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(FILE_WIN)
.onchange = file_onchange,
 .onkey = file_onkey,
};


static int dec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    static u8 file_num;
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("dec_onchange\n");
        sys_key_event_takeover(true, false);
        file_num = __this->onkey_sel;
        __this->onkey_mod = ONKEY_MOD_PLAY;
        __this->onkey_sel = 0;
        __this->dec_msg = DEC_MSG_NONE;
        __this->dec_show_status = 1;
        __this->cur_time.sec = 0;
        __this->cur_time.min = 0;
        __this->cur_time.hour = 0;
        break;
    case ON_CHANGE_SHOW_COMPLETED:
        ui_core_element_on_focus(ui_core_get_element_by_id(DEC_WIN), true);
        break;
    case ON_CHANGE_HIDE:
        if (__this->dec_msg) {
            if (__this->dec_msg_timer) {
                sys_timeout_del(__this->dec_msg_timer);
                __this->dec_msg_timer = 0;
                __this->dec_timerout_msg = DEC_MSG_NONE;
            }
            dec_msg_hide(__this->dec_msg);
        }
        ui_core_element_on_focus(ui_core_get_element_by_id(DEC_WIN), false);
        __this->onkey_sel = file_num;
        __this->onkey_mod = ONKEY_MOD_NORMAL;
        break;
    default:
        return false;
    }
    return false;
}
static int dec_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    struct application *app;
    struct ui_file_attrs attrs;
    FILE *fp;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }
    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    if (__this->onkey_mod == ONKEY_MOD_PLAY) {
        //预览模式中间窗口
        switch (e->value) {
        case KEY_UP:
        case KEY_LEFT:
        case KEY_DOWN:
        case KEY_RIGHT:
            break;
        case KEY_OK:
            if (__this->file_type) {
                if (__this->dec_player) {
                    ui_hide(DEC_LAY_PLAYER);
                    ui_hide(DEC_BTN_PLAY);
                    if (__this->dec_player_timer) {
                        sys_timeout_del(__this->dec_player_timer);
                        __this->dec_player_timer = 0;
                    }
                    __this->dec_player = 0;
                }
                cfun_dec_ok();
            }
            break;
        case KEY_MODE:
            if (__this->if_in_rep == 0) {
                __this->onkey_mod = ONKEY_MOD_PLAY_TOOL;
                __this->onkey_sel = 1;
                ui_highlight_element_by_id(dec_tool[__this->onkey_sel - 1]);
            }
            break;
        default:
            return false;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_PLAY_TOOL) {
        switch (e->value) {
        case KEY_UP:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(dec_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel <= 1 ? 3 : __this->onkey_sel - 1;
            ui_highlight_element_by_id(dec_tool[__this->onkey_sel - 1]);
            break;
        case KEY_DOWN:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(dec_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_sel = __this->onkey_sel >= 3 ? 1 : __this->onkey_sel + 1;
            ui_highlight_element_by_id(dec_tool[__this->onkey_sel - 1]);
            break;
        case KEY_OK:
            switch (__this->onkey_sel) {
            case 1:
                cfun_dec_lock();
                break;
            case 2:
                cfun_dec_delete();
                break;
            case 3:
                cfun_dec_return();
                break;
            }
            break;
        case KEY_MODE:
            if (__this->onkey_sel) {
                ui_no_highlight_element_by_id(dec_tool[__this->onkey_sel - 1]);
            }
            __this->onkey_mod = ONKEY_MOD_PLAY;
            __this->onkey_sel = 0;
            break;
        }
    } else if (__this->onkey_mod == ONKEY_MOD_PLAY_DIALOG) {
        switch (e->value) {
        case KEY_UP:
        case KEY_DOWN:
            if (__this->onkey_sel) {
                ui_highlight_element_by_id(DEC_BTN_DELETE_CANCEL);
                ui_no_highlight_element_by_id(DEC_BTN_DELETE_DELETE);
                __this->onkey_sel = 0;
            } else {
                ui_highlight_element_by_id(DEC_BTN_DELETE_DELETE);
                ui_no_highlight_element_by_id(DEC_BTN_DELETE_CANCEL);
                __this->onkey_sel = 1;
            }
            break;
        case KEY_OK:
            if (__this->onkey_sel) {
                cfun_dec_dialog();
            } else {
                ui_hide(DEC_LAY_DELETE);
                __this->onkey_mod = ONKEY_MOD_PLAY_TOOL;
                __this->onkey_sel = 2;
            }
            break;
        case KEY_MODE:
            ui_hide(DEC_LAY_DELETE);
            __this->onkey_mod = ONKEY_MOD_PLAY_TOOL;
            __this->onkey_sel = 2;
            break;
        }
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(DEC_WIN)
.onchange = dec_onchange,
 .onkey = dec_onkey,
};









#endif
