#include "system/includes.h"
#include "server/ui_server.h"
#include "server/ctp_server.h"
#include "server/net_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "asm/rtc.h"
#include "power_ctrl.h"
#include "storage_device.h"

#define LOG_TAG "app_main"
#include "generic/log.h"


u32 spin_lock_cnt[2] = {0};

int upgrade_detect(const char *sdcard_name);


/*任务列表 */
const struct task_info task_info_table[] = {
    {"imb_task",          30,     1024,   1024  },
    {"ui_server",           29,     1024,   1024  },
    {"ui_task",             30,     4096,   512   },
    {"init",                30,     4096,   128   },
    {"app_core",            15,     0x4000,   2048  },
    {"sys_event",           30,     1024,   0     },
    {"video_server",        16,     4096,   128   },
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     1024,   0     },
    {"video_dec_server",    27,     4096,   1024  },
    {"video0_rec0",         20,     2048,   256   },
    {"video0_rec1",         21,     2048,   256   },
    {"audio_rec0",          24,     2048,   256   },
    {"audio_rec1",          24,     2048,   256   },
    {"audio_rec2",          24,     2048,   256   },
    {"video1_rec0",         22,     2048,   256   },
    {"video1_rec1",         22,     2048,   256   },
    {"video3_rec0",         19,     2048,   256   },
    {"video3_rec2",         18,     2048,   256   },
    {"video4_rec3",         21,     2048,   256   },
    {"isp_update",          28,     1024,   0     },
    {"vpkg_server",         24,     4096,   512   },
    {"vunpkg_server",       23,     1024,   128   },
    {"avi0",                28,     2048,   64    },
    {"avi1",                29,     2048,   64    },
    {"avi2",                29,     2048,   64    },
    {"avi3",                28,     2048,   64    },
    {"mov0",                27,     2048,   64    },
    {"mov1",                27,     2048,   64    },
    {"mov2",                27,     2048,   64    },
    {"mov3",                28,     2048,   64    },
    {"video_engine_server", 14,     1024,   1024  },
    {"video_engine_task",   15,     2048,   0     },
    {"usb_server",          20,     1024,   128   },
    {"khubd",               26,     1024,   32    },

    {"uvc_transceiver",     28,     2048,   32    },
    {"upgrade_ui",          16,     1024,   32    },
    {"upgrade_core",        20,     1024,   32    },
    {"dynamic_huffman0",    15,		1024,	32    },
    {"ext_scale",           28,     1024,   32    },
    {"jpeg_encoder",        27,     1024,   32    },
    {"auto_test",           15,		1024,	1024  },
    {"ctp_server",          25,     0x2000,   1024    },
    {"net_avi0",            22,     2048,   64    },
    {"net_avi1",            22,     2048,   64    },
    {"net_mov0",            29,     2048,   128    },
    {"net_mov1",            29,     2048,   128    },
    {"mov_unpkg0",          27,     0x400,   128    },
    {"stream_avi0",         29,     2048,   256   },
    {"stream_avi1",         28,     2048,   256   },
    {"stream_mov0",         29,     2048,   256   },
    {"stream_mov1",         28,     2048,   256   },
    {"audio_rec2",          20,     2048,   256   },
    {"audio_rec3",          20,     2048,   256   },
    {"rf_task",             27,     0x4000,   1024 },
    {"rf_data_process",     26,     0x400,   256 },
    {"rf_video_task",       18,     2048,    64    },

    {"vcam_transceiver",    16,     2048,   32    },
    {"vcam_audio_transceiver",    17,     2048,   32    },
    {"net_audio_transceiver",    15,     2048,   32    },
    {"vimc2_task",          28,     2048,   32    },
    {"ipc_task",            16,     4096,   32    },
    {"vcam_stream_task",    14,     4096,   32    },
    {"g729_encode",         16,     4096,   128   },

    {0, 0},
};



/*
 * 开机动画播放完毕
 */
static void animation_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

    server_close(ui);

    /*
     * 显示完开机画面后更新配置文件,避免效果调节过度导致开机图片偏色
     */
    void *imd = dev_open("imd", NULL);
    if (imd) {
        dev_ioctl(imd, IMD_SET_COLOR_CFG, (u32)"scr_auto.bin"); /* 更新配置文件  */
        dev_close(imd);
    }

    /*
     *按键消息使能
     */
    if (!get_parking_status()) {
        sys_key_event_enable();
        sys_touch_event_enable();//使能触摸事件
    }
}

/*
 * 关机动画播放完毕, 关闭电源
 */
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

    if (ui) {
        server_close(ui);
    }

    system_shutdown(0);
}
static int main_net_event_hander(struct sys_event *event)
{
    struct ctp_arg *__ctp_arg = (struct ctp_arg *)event->arg;
    struct net_event *net = &event->u.net;

    switch (net->event) {
    case NET_EVENT_CMD:
        printf("IN NET_EVENT_CMD\n\n\n\n");

        ctp_cmd_analysis(__ctp_arg->topic, __ctp_arg->content, __ctp_arg->cli);
        if (__ctp_arg->content) {
            free(__ctp_arg->content);
        }
        __ctp_arg->content = NULL;
        if (__ctp_arg) {
            free(__ctp_arg);
        }
        __ctp_arg = NULL;

        return true;
        break;
    case NET_EVENT_DATA:
        /* printf("IN NET_EVENT_DATA\n"); */
        break;
    }

    return false;
}
static int main_key_event_handler(struct key_event *key)
{
    struct intent it;
    struct application *app;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_MODE:
            init_intent(&it);
            app = get_current_app();

            if (app) {
                if (!strcmp(app->name, "usb_app")) {
                    break;
                }
                it.action = ACTION_BACK;
                start_app(&it);

                if (!strcmp(app->name, "video_rec")) {
                    it.name = "video_photo";
                    it.action = ACTION_PHOTO_TAKE_MAIN;
                } else if (!strcmp(app->name, "video_photo")) {
                    it.name = "video_dec";
                    it.action = ACTION_VIDEO_DEC_MAIN;
                } else if (!strcmp(app->name, "video_dec")) { //by peng
                    it.name = "sys_set";
                    it.action = ACTION_SYS_SET_MAIN;
                }
				else if (!strcmp(app->name, "sys_set")) {  
                    it.name = "video_rec";
                    it.action = ACTION_VIDEO_REC_MAIN;
                }
                start_app(&it);
            }
            break;
        default:
            return false;
        }
        break;
    case KEY_EVENT_LONG:
        if (key->value == KEY_POWER) {
            puts("---key_power\n");
            static u8 power_fi = 0;
            if (power_fi) {
                puts("re enter power off\n");
                break;
            }
            power_fi = 1;
            sys_key_event_disable();

            init_intent(&it);
            app = get_current_app();
            if (app) {
                it.action = ACTION_BACK;
                start_app(&it);
            }

#ifdef CONFIG_UI_ENABLE
            struct ui_style style;
            style.file = "mnt/spiflash/audlogo/ani.sty";
            style.version = ANI_UI_VERSION;
            struct server *ui = server_open("ui_server", &style);
            if (ui) {
                union uireq req;
                req.show.id = ANI_ID_PAGE_POWER_OFF;
                server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                                     power_off_play_end, ui);
            }
#else
            system_shutdown(0);
#endif
        } else if (key->value == KEY_UP) {
            /* void wifi_on(void); */
            /* wifi_on(); */
        } else if (key->value == KEY_DOWN) {
            /* void wifi_off(void); */
            /* wifi_off(); */
        }
        break;
    default:
        return false;
    }

    return true;
}

extern u8 get_usb_in_status();
static int main_dev_event_handler(struct sys_event *event)
{
    struct intent it;
    struct application *app;
    char buf[64];
    init_intent(&it);
    app = get_current_app();
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
#ifdef CONFIG_UI_ENABLE
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && strcmp(app->name, "usb_app") && strcmp(app->name, "sdcard_upgrade")) {
                it.action = ACTION_BACK;
                start_app(&it);

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }
#ifdef CONFIG_TOUCH_UI_ENABLE
            else if (!app) { //主界面进入usb界面
                union uireq req;
                struct server *ui;
                ui = server_open("ui_server", NULL);
                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "usb_app";
                it.action = ACTION_USB_SLAVE_MAIN;
                start_app(&it);
            }
#endif
            if (get_parking_status()) {
                sys_key_event_enable();
                sys_touch_event_enable();
            }
        }
#endif

        if (!ASCII_StrCmp(event->arg, "parking", 7)) {
            if (app) {
                if (!strcmp(app->name, "video_rec")) {
                    break;
                }
                if ((!strcmp(app->name, "video_photo"))
                    || (!strcmp(app->name, "video_dec"))
                    || (!strcmp(app->name, "video_system"))
                    || (!strcmp(app->name, "usb_app"))) {

                    if (!strcmp(app->name, "usb_app")) {
                        if (get_usb_in_status()) {
                            puts("usb in status\n");
                            break;
                        }
                    }

                    it.action = ACTION_BACK;
                    start_app(&it);
                }
                puts("\n =============parking on eee video rec=========\n");
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }
#ifdef CONFIG_TOUCH_UI_ENABLE
            else if (!app) { //主界面进入倒车界面
                union uireq req;
                struct server *ui;
                ui = server_open("ui_server", NULL);
                req.hide.id = ID_WINDOW_MAIN_PAGE;
                server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app(&it);
            }
#endif

        }

        if (!ASCII_StrCmp(event->arg, "sd", 2)) {
            /* extern int thread_fork(char *thread_name, int prio, int stk_size, u32 q_size, int *pid, void (*func)(void *), void *parm); */
            /* thread_fork("test_task",20,0x1000,0,0,test,NULL); */
            /* printf("===========================00000000\n\n\n\n\n"); */
#if defined CONFIG_ENABLE_VLIST
            FILE_LIST_IN_MEM(1);
#endif
            /* FILE *fd = NULL; */
            /* fd = fopen(CONFIG_REC_PATH_1"forward_list.txt", "w+"); */
            /* fclose(fd); */
            /*  */
            /* struct vfs_partition *part; */
            /* int err; */
            /* part = fget_partition(CONFIG_ROOT_PATH); */
            /* printf("\n  part->clust_size ------ %d\n",part->clust_size); */
            /* if(part->clust_size  < 32){ */
            /* 	puts("\n format SD\n"); */
            /* 	err = f_format(CONFIG_ROOT_PATH, "fat", 32 * 1024); */
            /* } */
            strcpy(buf, "online:1");
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);
#if 1
            u32 space = 0;
            struct vfs_partition *part = NULL;
            if (storage_device_ready() == 0) {
                CTP_CMD_COMBINED(NULL, CTP_SD_OFFLINE, "TF_CAP", "NOTIFY", CTP_SD_OFFLINE_MSG);
            } else {
                part = fget_partition(CONFIG_ROOT_PATH);
                fget_free_space(CONFIG_ROOT_PATH, &space);
                sprintf(buf, "left:%d,total:%d", space / 1024, part->total_size / 1024);
                CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "TF_CAP", "NOTIFY", buf);

            }
#endif


        }

        break;
    case DEVICE_EVENT_OUT:
        if (!ASCII_StrCmp(event->arg, "usb0", 4)) {
            if (app && !strcmp(app->name, "usb_app")) {
                clear_usb_charg_off_sec_cnt();
                it.action = ACTION_BACK;
                start_app(&it);

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;

                start_app(&it);
                if (get_parking_status()) {
                    sys_key_event_disable();
                    sys_touch_event_disable();
                }
            }
        }

        if (!ASCII_StrCmp(event->arg, "sd", 2)) {

#if defined CONFIG_ENABLE_VLIST
            FILE_REMOVE_ALL();
#endif
            strcpy(buf, "online:0");
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);
        }

        break;
    case DEVICE_EVENT_CHANGE:
#ifdef CONFIG_GSENSOR_ENABLE
        if (!strcmp(event->arg, "gsen_lock")) {
            if (get_gsen_active_flag()) {
                clear_gsen_active_flag();
            }
        }
#endif

        break;
    }
    return 0;
}
/*
 * 默认的系统事件处理函数
 * 当所有活动的app的事件处理函数都返回false时此函数会被调用
 */
void app_default_event_handler(struct sys_event *event)
{
    char *dev = (char *)event->arg;

    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
        break;
    case SYS_NET_EVENT:
        main_net_event_hander(event);
        break;
    default:
        ASSERT(0, "unknow event type: %s\n", __func__);
        break;
    }
}


/*
 * 应用程序主函数
 */
void app_main()
{
    struct intent it;
    int err;
    char buf[64];
    if (!fdir_exist("mnt/spiflash")) {
        mount("spiflash", "mnt/spiflash", "sdfile", NULL);
    }

    mount_sd_to_fs(SDX_DEV);

    err = upgrade_detect(SDX_DEV);
    if (!err) {
        return;
    }


    /*
     * 播放开机动画
     */
#ifdef CONFIG_UI_ENABLE
    struct ui_style style;

    style.file = "mnt/spiflash/audlogo/ani.sty";
    style.version = ANI_UI_VERSION;

    struct server *ui = server_open("ui_server", &style);
    if (ui) {
        union uireq req;

        req.show.id = ANI_ID_PAGE_POWER_ON;
        server_request_async(ui, UI_REQ_SHOW_SYNC | REQ_COMPLETE_CALLBACK, &req,
                             animation_play_end, ui);
    }
#else
    sys_key_event_enable();
    sys_touch_event_enable();
#endif

    /*生成文件列表*/
    if (dev_online(SDX_DEV)) {
#if defined CONFIG_ENABLE_VLIST
        FILE_LIST_IN_MEM(1);
#endif
        /* FILE *fd = NULL; */
        /* fd = fopen(CONFIG_REC_PATH_1"forward_list.txt", "w+"); */
        /* fclose(fd); */
        /*  */
        /* struct vfs_partition *part; */
        /* int err; */
        /* part = fget_partition(CONFIG_ROOT_PATH); */
        /* printf("\n  part->clust_size ------ %d\n",part->clust_size); */
        /* if(part->clust_size  < 32){ */
        /* 	puts("\n format SD\n"); */
        /* 	err = f_format(CONFIG_ROOT_PATH, "fat", 32 * 1024); */
        /* } */
        strcpy(buf, "online:1");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "SD_STATUS", "NOTIFY", buf);

    }




    /* void vm_eraser(void); */
    /* vm_eraser(); */
    init_intent(&it);


    it.name = "video_system";
    it.action = ACTION_SYSTEM_MAIN;
    start_app(&it);
    it.action = ACTION_BACK;
    start_app(&it);

#if 0
    int flow_init(void);
    flow_init();
    os_time_dly(250);
    printf("video start ...\n\n");
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
    return;
#endif


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
    printf("sssssssssssssssss\n\n\n");
#else
#if 0
    if (dev_online("usb0")) {
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_SET_CONFIG;
        it.data = "usb:uvc";
        start_app(&it);
        return;
    }
#endif
#endif

    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}





