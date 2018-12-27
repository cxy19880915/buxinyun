/*************************************************************************
	> File Name: app/car_camera/upgrade.c
	> Author:
	> Mail:
	> Created Time: Fri 26 May 2017 03:45:26 PM HKT
 ************************************************************************/
#include "system/includes.h"
#include "app_config.h"
#include "server/ui_server.h"
#include "storage_device.h"
#include "upgrade/upgrade_detect.h"
#include "upgrade_style.h"
#include "action.h"
#include "ui/ui_slider.h"
#include "gSensor_manage.h"

#define STYLE_NAME UPGRADE

REGISTER_UI_STYLE(STYLE_NAME)

#define USE_CONFIRM_UI      0//升级确认打开/关闭

/*#define UPGRADE_UBOOT_FOR_V0_5_7*/

#define STORAGE_READY_TIMEOUT   (5)

#ifdef UPGRADE_UBOOT_FOR_V0_5_7
#define UPGRADE_UI_PATH         CONFIG_ROOT_PATH
#else
#define UPGRADE_UI_PATH         "ram/bfu/C/"
#endif
#define UPGRADE_PATH        CONFIG_ROOT_PATH

#if USE_CONFIRM_UI
#define FORCE_UPD_FILE      UPGRADE_PATH"RestoreA.bin"

#define UPD_CONFIRM         0x1
#define UPD_CANCEL          0x0
#endif

struct upgrade_ui_info {
    u8 ui_enable;
    u8 success;
    u8 upgrade_out;
#if USE_CONFIRM_UI
    u8 choice;
    u8 force;
    OS_SEM  sem_choose;
#endif
    OS_SEM  sem_wait;
    OS_SEM  sem_end;
    struct server *ui;
    int prev_id;
    char spercent[8];
};

static struct upgrade_ui_info ui_info;
#define __this  (&ui_info)

#if USE_CONFIRM_UI

static void remove_restore_file(const char *path)
{
    fdelete_by_name(path);
}

static bool need_force_upgrade(const char *path)
{
    FILE *fp;

    if (!path) {
        return false;
    }

    fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }

    return false;
}

#endif
/*
 *升级过程进度条
 */
static int upgrade_show_progress(int percent)
{
    printf("%d\n", percent);
    if (!__this->ui || !__this->ui_enable) {
        return -EFAULT;
    }

    ui_slider_set_persent_by_id(UPD_ID_SLIDER, percent);

    if (percent < 100) {
        sprintf(__this->spercent, "%02d %%", percent);
    } else {
        sprintf(__this->spercent, "%d %%", percent);
    }
    printf("%s\n", __this->spercent);


    ui_text_set_str_by_id(UPD_ID_TEXT_PERCENT, "ascii", __this->spercent);

    return 0;
}

/*
 *升级信息显示
 */
static int upgrade_show_message(int msg)
{
    int id;
    union uireq ui_req;

    if (!__this->ui || !__this->ui_enable) {
        return -EINVAL;
    }
    switch (msg) {
    case UPGRADE_START:
        if (__this->ui) {
            ui_req.show.id = UPD_ID_PAGE_UPGRADE;
            server_request(__this->ui, UI_REQ_SHOW, &ui_req);
        }
        id = UPD_ID_MESSAGE_0;
        break;
    case UPGRADE_ERR_FILE:
        id = UPD_ID_MESSAGE_1;
        break;
    case UPGRADE_ERR_KEY:
        id = UPD_ID_MESSAGE_2;
        break;
    case UPGRADE_ERR_ADDR:
        id = UPD_ID_MESSAGE_3;
        break;
    case UPGRADE_ERR_FAILED:
        id = UPD_ID_MESSAGE_4;
        break;
    case UPGRADE_CALC_ECC:
        id = UPD_ID_MESSAGE_5;
        break;
    case UPGRADE_SUCCESS:
        id = UPD_ID_MESSAGE_6;
        puts("upgrade success.\n");
        break;
    default:
        return -EINVAL;
    }

    if (__this->prev_id) {
        ui_req.hide.id = __this->prev_id;
        server_request(__this->ui, UI_REQ_HIDE, &ui_req);
    }
    ui_req.show.id = id;
    server_request(__this->ui, UI_REQ_SHOW, &ui_req);
    __this->prev_id = id;
    return 0;
}

static int upgrade_complete(u8 success)
{
    union uireq ui_req;

    __this->success = success;
#if USE_CONFIRM_UI
    if (success && __this->force) {
        remove_restore_file(FORCE_UPD_FILE);
    }
#endif

#ifdef UPGRADE_UBOOT_FOR_V0_5_7
    int msg[32];
    while (dev_online(SDX_DEV)) {
        os_time_dly(2);
        mount_sd_to_fs(SDX_DEV);
        os_taskq_accept(ARRAY_SIZE(msg), msg);
    }
#endif
    if (!__this->ui || !__this->ui_enable) {
        __this->upgrade_out = 1;
        return -EINVAL;
    }

    ui_req.hide.id = UPD_ID_PAGE_UPGRADE;
    server_request(__this->ui, UI_REQ_HIDE, &ui_req);
    /*
     *关闭UI
     */
#if !USE_CONFIRM_UI
    if (__this->ui) {
        server_close(__this->ui);
        __this->ui = NULL;
    }
#endif
    __this->upgrade_out = 1;
    return 0;
}

static bool upgrade_confirm(void)
{
    struct ui_style style;
    /*
     *打开UI服务
     */
#ifdef CONFIG_UI_ENABLE
    style.file = UPGRADE_UI_PATH"upgrade.sty";
    style.version = UPD_UI_VERSION;
    __this->ui = server_open("ui_server", &style);

    if (!__this->ui) {
        return true;
    }

    os_sem_post(&__this->sem_wait);
#if USE_CONFIRM_UI
    union uireq req;
    __this->success = 1;
    if (__this->force) {
        return true;
    }

    __this->upgrade_out = 1;
    sys_key_event_enable();

    req.show.id = UPD_ID_PAGE_CONFIRM;
    server_request(__this->ui, UI_REQ_SHOW, &req);

    os_sem_pend(&__this->sem_choose, 0);

    req.hide.id = UPD_ID_PAGE_CONFIRM;
    server_request(__this->ui, UI_REQ_HIDE, &req);

    if (__this->choice != UPD_CONFIRM) {
        return false;
    }
#endif

#endif

    return true;
}

static int mount_ui_fs(void)
{
    struct imount *mt;

    mt = mount(NULL, "ram/bfu", "ramfs", NULL);
    if (!mt) {
        puts("mount fail\n");
        return -EFAULT;
    }

    return 0;
}

/*
 *SD卡升级检测
 */
int start_upgrade_detect(const char *sdcard_name)
{
    int timeout = 0;
    int err;
	u32 temp;
    struct upgrade_req req = {0};

    if (timeout <= STORAGE_READY_TIMEOUT) {
        /*
         *升级类型（文件或buffer)
         *升级路径配置，路径可自选，最好UI文件和bfu放在同一目录
         *升级的ui配置
         *path
         */
        req.type = UPGRADE_FROM_FILE;
#if USE_CONFIRM_UI
        req.force = need_force_upgrade(FORCE_UPD_FILE);;
        __this->force = req.force;
        if (__this->force) {
            req.path = FORCE_UPD_FILE;
            req.file = fopen(FORCE_UPD_FILE, "r");
        } else {
            req.path = UPGRADE_PATH;
            req.file = fopen(UPGRADE_PATH"JL_AC54.bfu", "r");
        }

        /*
         * 自己配置的file，可以根据需要来处理未打开的情况
         */
        if (!req.file) {
            upgrade_complete(0);
            return -EINVAL;
        }
#else
        req.path = UPGRADE_PATH;

#ifdef UPGRADE_UBOOT_FOR_V0_5_7
        req.file = fopen(UPGRADE_PATH"jl_isd.bin", "r");
        if (!req.file) {
            log_e("open jl isd bin file error");
            if (!req.file) {
                upgrade_complete(0);
                return -EINVAL;
            }
        }
#else
        req.file = fopen(UPGRADE_PATH"JL_AC54.bfu", "r");
		#if 1
		if(req.file)
		{
				struct sys_time t;

			    memset(&t, 0, sizeof(t));
			    t.year = 2018;
			    t.day = 1;
			    t.month = 1;
				
			    
			    void *fd = dev_open("rtc", NULL);
			    if (!fd) {
			        return -EFAULT;
			    }
			    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&t);
			    dev_close(fd);	
			printf("=====ss======up time=============++++++++++++++++++++\n");	
		}
		#endif
#endif

#endif

		
        req.complete = upgrade_complete;
        req.confirm = upgrade_confirm;
        req.ui.step_msecs = 200;
        req.ui.show_progress = upgrade_show_progress;
        req.ui.show_message = upgrade_show_message;
        req.ui.path = UPGRADE_UI_PATH;
#ifdef UPGRADE_UBOOT_FOR_V0_5_7
        req.type = 0x65;
        req.force = 0x12;
#endif

        err = mount_ui_fs();
        if (!err) {
            __this->ui_enable = 1;
        }

        err = upgrade_file_detect(&req);

        if (req.file) {
            fclose(req.file);
        }

        return err;
    }

    return -EINVAL;
}


static void switch_to_other_app(void)
{
    struct intent it;
    struct application *app;

    init_intent(&it);

    app = get_current_app();
    if (app) {
        it.action = ACTION_BACK;
        start_app(&it);
    }
    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

#ifdef CONFIG_UI_ENABLE
    if (dev_online("usb0")) {
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_MAIN;
#ifdef CONFIG_GSENSOR_ENABLE
        usb_online_clear_park_wkup();
#endif
        start_app(&it);
        return;
    }
#endif
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}

static void wait_upgrade_stop(void)
{
    os_sem_pend(&__this->sem_end, 0);

    task_kill("upgrade_core");
}

#if USE_CONFIRM_UI
static int set_upgrade_confirm(int confirm)
{
    if (confirm) {
        __this->choice = UPD_CONFIRM;
    } else {
        __this->choice = UPD_CANCEL;
    }

    os_sem_post(&__this->sem_choose);

    wait_upgrade_stop();

    if (!confirm) {
        switch_to_other_app();
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
    }
    return 0;
}
#endif

static void upgrade_task(void *arg)
{
    os_sem_create(&__this->sem_wait, 0);
    os_sem_create(&__this->sem_end, 0);
#if USE_CONFIRM_UI
    os_sem_create(&__this->sem_choose, 0);
#endif

    start_upgrade_detect((char *)arg);
    os_sem_post(&__this->sem_end);
    while (1) {
        os_sem_pend(&__this->sem_wait, 0);
    }
}


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    struct intent mit;

    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_UPGRADE_MAIN:
            task_create(upgrade_task, (void *)it->data, "upgrade_core");
            break;
#if USE_CONFIRM_UI
        case ACTION_UPGRADE_SET_CONFIRM:
            set_upgrade_confirm((int)it->data);
            break;
#endif
        }
        break;
    case APP_STA_PAUSE:
        return -EFAULT;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
        break;
    }

    return 0;

}
static int upgrade_key_event_handler(struct key_event *key)
{
    struct intent it;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            break;
        case KEY_UP:

            break;
        case KEY_DOWN:

            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

static int upgrade_device_event_handler(struct sys_event *event)
{
    int err;

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        //upgrade_pause();
        break;
    }
    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_KEY_EVENT:
        return upgrade_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return upgrade_device_event_handler(event);
    default:
        return false;
    }
    return false;
}


static const struct application_operation upgrade_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_upgrade) = {
    .name 	= "sdcard_upgrade",
    .action	= ACTION_UPGRADE_MAIN,
    .ops 	= &upgrade_ops,
    .state  = APP_STA_DESTROY,
};

int upgrade_detect(const char *sdcard_name)
{
    int err;
    int msg[32];
    memset(__this, 0x0, sizeof(*__this));

    int mount_count = 0;

#ifdef UPGRADE_UBOOT_FOR_V0_5_7

    while (!storage_device_ready()) {
        os_taskq_accept(ARRAY_SIZE(msg), msg);
        os_time_dly(2);
        if (++mount_count > 200) {
            log_e("wait sdcard timeout");
            alarm_wkup_ctrl(1);
        }
    }
#else
    puts("upgrade_normal_mode\n");
    mount_sd_to_fs(SDX_DEV);
    if (!storage_device_ready()) {
        return -ENODEV;
    }
#endif

    struct intent it;

    init_intent(&it);
    it.name = "sdcard_upgrade";
    it.action = ACTION_UPGRADE_MAIN;
    it.data = SDX_DEV;
    start_app(&it);
    while (1) {
        os_time_dly(2);
        os_taskq_accept(ARRAY_SIZE(msg), msg);
        if (__this->upgrade_out) {
            break;
        }
    }

    if (!__this->success) {
        it.action = ACTION_BACK;
        start_app(&it);
        return -EFAULT;
    }
    return 0;

    /*return start_upgrade_detect(sdcard_name);*/
}
