#include "system/includes.h"
#include "server/ui_server.h"
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
#include "asm/lcd_config.h"

#define LOG_TAG "app_main"
#include "generic/log.h"
#include "touch_panel_manager.h"

u32 spin_lock_cnt[2] = {0};


/*任务列表 */
const struct task_info task_info_table[] = {
    {"ui_server",           29,     1024,   1024  },
    {"ui_task",             30,     4096,   512   },
    {"init",                30,     1024,   128   },
    {"app_core",            15,     2048,   2048  },
    {"sys_event",           30,     1024,   0     },
    {"video_server",        16,     1024,   128   },
    {"audio_server",        16,     1024,   256   },
    {"audio_decoder",       17,     1024,   0     },
    {"video_dec_server",    27,     1024,   1024  },
    {"video0_rec0",         20,     2048,   256   },
    {"video0_rec1",         21,     2048,   256   },
    {"audio_rec0",          20,     2048,   256   },
    {"audio_rec1",          20,     2048,   256   },
    {"video1_rec0",         19,     2048,   256   },
    {"video1_rec1",         18,     2048,   256   },
    {"video3_rec0",         19,     2048,   256   },
    {"video3_rec2",         18,     2048,   256   },
    {"isp_update",          28,     1024,   0     },
    {"vpkg_server",         24,     1024,   512   },
    {"vunpkg_server",       23,     1024,   128   },
    {"avi0",                28,     2048,   64    },
    {"avi1",                29,     2048,   64    },
    {"mov0",                27,     2048,   64    },
    {"mov1",                26,     2048,   64    },
    {"mov2",                26,     2048,   64    },
    {"video_engine_server", 14,     1024,   1024  },
    {"video_engine_task",   15,     2048,   0     },
    {"usb_server",          20,     1024,   128   },
    {"khubd",               26,     1024,   32    },

    {"uvc_transceiver",     28,     2048,   32    },
    {"upgrade_ui",          16,     1024,   32    },
    {"upgrade_core",        15,     1024,   32    },

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
#ifdef FLASE_POWER_OFF
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;
    u16 power_key_cnt = 0;
    
    if (ui) {
        server_close(ui);
    }

    int park_en = db_select("par");
#ifdef CONFIG_GSENSOR_ENABLE
    set_parking_guard_wkpu(park_en);//gsensor parking guard
#endif
    ui_lcd_light_off();
    if(usb_is_charging())
    {
        printf("usb_is_charging()\n");
        
        if(get_power_off_state())  //�����USB����ػ��ģ���ʱ�Ѿ������ϵ磬���¿���
        {
            cpu_reset();
        }
        WDT_CON &= ~BIT(4); //�رտ��Ź�
        
        gpio_direction_input(IO_PORT_PR_01);
		gpio_set_pull_up(IO_PORT_PR_01, 0);
		gpio_set_pull_down(IO_PORT_PR_01, 0);
		gpio_set_die(IO_PORT_PR_01, 1);
		delay(10);
                
        while(usb_is_charging())
        {
            if (read_power_key())
            {
                power_key_cnt++;
                if(power_key_cnt > 10)
                {
                    cpu_reset();
                }
            }
            else
            {
                power_key_cnt = 0;    
            }
            #if 1
            if(gpio_read(IO_PORT_PR_01))
            {
                printf("gsensor wkup\n");
                cpu_reset();    
            }
            #endif
            os_time_dly(10);
        }
    }
    port_wkup_ctrl(park_en);//power off
    while(1)
    {
        os_time_dly(10);
        PWR_CTL(0);
    }
}
#else
static void power_off_play_end(void *_ui)
{
    struct server *ui = (struct server *)_ui;

    if (ui) {
        server_close(ui);
    }

    int park_en = db_select("par");
#ifdef CONFIG_GSENSOR_ENABLE
    set_parking_guard_wkpu(park_en);//gsensor parking guard
#endif
    port_wkup_ctrl(park_en);//power off
}
#endif


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
                } else if (!strcmp(app->name, "video_dec")) {
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
            sys_touch_event_disable(); //����
            
            dev_close_touch();  //����
            
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
            int park_en = db_select("par");
            set_parking_guard_wkpu(park_en);//gsensor parking guard
            port_wkup_ctrl(park_en);//power off
#endif
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
            }
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
    switch (event->type) {
    case SYS_KEY_EVENT:
        main_key_event_handler(&event->u.key);
        break;
    case SYS_TOUCH_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        main_dev_event_handler(event);
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
#endif
    init_intent(&it);

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





