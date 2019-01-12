#include "ui/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "system/includes.h"
#include "menu_parm_api.h"
#include "video_rec.h"
#include "asm/dsi.h"
#include "storage_device.h"


#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

REGISTER_UI_STYLE(STYLE_NAME)


struct main_page_info {
    u8 battery_val;
    u8 battery_char;
    s8 onkey_sel;
};
static int onkey_sel_item[3] = {
    HOME_BTN_RECORDER,
    HOME_BTN_FILE,
    HOME_BTN_SETTINGS,
};
static int onkey_sel_item1[3] = {
    //HOME_TXT_RECORDER,
    //HOME_TXT_FILES,
    //HOME_TXT_SETTINGS,
};

extern u8 all_back;
extern volatile u8 main_lock_flag;
extern int lane_entry_flag;
extern volatile char if_in_rec; /* 是否正在录像 */
extern int rec_timer_handle;
extern int rec_cnt;
static struct main_page_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct main_page_info))

/*
 * (begin)提示框显示接口 ********************************************
 */
enum box_msg {
    BOX_MSG_POWER_OFF = 1,
    BOX_MSG_NO_POWER,
};
static u8 msg_show_f = 0;
static enum box_msg msg_show_id = 0;
int main_rec_flag = 0;

static void __main_msg_hide(enum box_msg id)
{
    if (msg_show_id == id) {
        if (msg_show_f) {
            msg_show_f = 0;
            //ui_hide(HOME_LAY_MESSAGEBOX);
        }
    } else if (id == 0) { /* 没有指定ID，强制隐藏 */
        if (msg_show_f) {
            msg_show_f = 0;
            //ui_hide(HOME_LAY_MESSAGEBOX);
        }
    }
}

static void home_rec_timer_cnt(void *priv)
{
    /*
     * ???????????,1s
     */
    struct utime time_r;
    rec_cnt++;//??????
    time_r.hour = rec_cnt / 60 / 60;
    time_r.min = rec_cnt / 60 % 60;
    time_r.sec = rec_cnt % 60;

}

static int rec_on_handler(const char *type, u32 arg)
{
    puts("\n***home rec_on_handler.***\n");
    if_in_rec = TRUE;
    
    //ui_hide(MAIN_REC_RRTIME);   
    //ui_show(MAIN_REC_TIME); //peng
    //ui_show(MAIN_MOVE_1);
    //ui_hide(MAIN_PIC_BREC);
    //ui_pic_show_image_by_id(MAIN_PIC_BREC, 1); 
    //ui_show(HOME_LAY_1);
    //printf("111111111111111111111111111111111111111\n");
    if (rec_timer_handle == 0) {
        rec_cnt = 0;
        rec_timer_handle = sys_timer_add(NULL, home_rec_timer_cnt, 1000);
    }
    return 0;
}

static void __main_msg_timeout_func(void *priv)
{
    __main_msg_hide((enum box_msg)priv);
}
static void __main_msg_show(enum box_msg msg, u32 timeout_msec)
{
    static int t_id = 0;
    if (msg == 0) {
        ASSERT(0, "__main_msg_show msg 0!\n");
        return;
    }

    if (msg == msg_show_id) {
        if (msg_show_f == 0) {
            msg_show_f = 1;
            //ui_show(HOME_LAY_MESSAGEBOX);
            //ui_text_show_index_by_id(HOME_TXT_MESSAGEBOX, msg - 1);
            if (t_id) {
                sys_timeout_del(t_id);
                t_id = 0;
            }
            if (timeout_msec > 0) {
                t_id = sys_timeout_add((void *)msg, __main_msg_timeout_func, timeout_msec);
            }
        }
    } else {
        msg_show_id = msg;
        if (msg_show_f == 0) {
            msg_show_f = 1;
            //ui_show(HOME_LAY_MESSAGEBOX);
        }
        //ui_text_show_index_by_id(HOME_TXT_MESSAGEBOX, msg - 1);
        if (t_id) {
            sys_timeout_del(t_id);
            t_id = 0;
        }
        if (timeout_msec > 0) {
            t_id = sys_timeout_add((void *)msg, __main_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */
static void get_system_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    /* printf("get_sys_time : %d-%d-%d,%d:%d:%d\n", time->year, time->month, time->day, time->hour, time->min, time->sec); */
    dev_close(fd);
}
int ReturnWeekDay(unsigned int iYear, unsigned int iMonth, unsigned int iDay)
{
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    if (iMonth == 1 || iMonth == 2) {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    } else {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }

    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);        //iWeek为负时取模
    if (iWeek == 0) {     //星期日不作为一周的第一天
        iWeek = 7;
    }

    return iWeek;
}
//
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
	ui_time_update_by_id(HOME_TIM_DATE, &sys_time);	
}

static void no_power_msg_box_timer(void *priv)
{
    static u32 cnt = 0;
    if (__this->battery_val <= 20 && __this->battery_char == 0) {
        cnt++;
        if ((cnt % 2) == 0) {
            puts("no power show.\n");
            __main_msg_show(BOX_MSG_NO_POWER, 0);
        } else {
            puts("no power hide.\n");
            __main_msg_hide(BOX_MSG_NO_POWER);
        }
        if(cnt>2)
	 send_key_long_msg();
    } else {
        __main_msg_hide(BOX_MSG_NO_POWER);
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
            __main_msg_hide(0);
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
                    __main_msg_hide(0);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                ui_battery_level_change(__this->battery_val, 0);
                __this->battery_char = 0;
                //__main_msg_show(BOX_MSG_POWER_OFF, 0);
                ten_sec_off = 1;
            }
        }
    }
}
extern u8 home_rec_touch;
static int rec_off_handler(const char *type, u32 arg)
{
    puts("rec_off_handler.\n");
    struct intent it;
    if_in_rec = FALSE;

    sys_timer_del(rec_timer_handle);
    rec_timer_handle = 0;
    rec_cnt = 0;
    if(home_rec_touch){
        
        home_rec_touch = 0;
        //ui_hide(MAIN_REC_TIME);  //peng
        
        //ui_hide(MAIN_MOVE_1);
        //ui_show(MAIN_REC_RRTIME);  
        //ui_hide(MAIN_PIC_BREC);
        
        //ui_pic_show_image_by_id(MAIN_PIC_BREC, 0); 
        //ui_show(HOME_LAY_1);
        ui_pic_show_image_by_id(MAIN_PIC_LOCK, 1);
        main_lock_flag = 0;
        init_intent(&it);
        it.name = "video_rec";
        it.action = ACTION_BACK;
        start_app_async(&it,NULL,NULL);
        
    }
    return 0;
}

static int rec_save_handler(const char *type, u32 arg)                                                                         
{   
   //?????????????????0
    puts("rec_save_handler.++++++++\n");		//***cxy****add
    struct utime time_r;						//rec to home
    time_r.hour = 0;							//循环录影时
    time_r.min = 0;								//录完一段视频
    time_r.sec = 0;								//时间不会归零
    ui_time_update_by_id(MAIN_REC_TIME, &time_r);//***cxy***add
    rec_cnt = 0;

    return 0;
} 

/*
 * 主界面响应回调
 */
 extern void main_show_dog_info(const char *type, u32 arg);
static const struct uimsg_handl main_msg_handler[] = {
	{ "onREC",          rec_on_handler       }, /* ?aê????? */ 
    { "offREC",         rec_off_handler      }, /* í￡?1???? */
    { "saveREC",        rec_save_handler     }, /* ±￡′????? */
    { "doginfo",        main_show_dog_info     },
    
    // { NULL, NULL},      /* ±?D?ò?′??á?2￡? */
};


#if 0
static rec_time_int(void *ctr, enum element_change_event e, void *arg)
{
    struct utime time_r;
    switch (e) {
    case ON_CHANGE_INIT:
        
        //rec_cnt++;//??????
        time_r.hour = rec_cnt / 60 / 60;
        time_r.min = rec_cnt / 60 % 60;
        time_r.sec = rec_cnt % 60;
        ui_time_update_by_id(MAIN_TIME, &time_r);
        break;
    default:
        return false;
    }

    return false;    
}
REGISTER_UI_EVENT_HANDLER(MAIN_TIME)
.onchange = rec_time_int,
};
#endif

#if 0
static int main_pic_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (db_select("mic")) {
            ui_pic_set_image_index(pic, 0);    /* 禁止录音 */
        } else {
            ui_pic_set_image_index(pic, 1);
        }
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

REGISTER_UI_EVENT_HANDLER(MAIN_PIC_VIEDO)
.onchange = main_pic_mic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif

static void sd_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            ui_pic_show_image_by_id(MAIN_PIC_CARK, 0);
		if(main_rec_flag == 1)
		{
			 ui_show(MAIN_MOVE_1); 
			 ui_hide(MAIN_REC_RRTIME);
	     		ui_show(MAIN_REC_TIME);
            		
            		//ui_show(MAIN_REC_RRTIME);
		}
	    //ui_hide(MAIN_REC_RRTIME);
            //ui_show(MAIN_REC_RRTIME);
            break;
        case DEVICE_EVENT_OUT:
            ui_pic_show_image_by_id(MAIN_PIC_CARK, 1);
		 if (if_in_rec) {
                ui_hide(MAIN_REC_TIME);
            }
	      ui_hide(MAIN_MOVE_1); 
	     ui_hide(MAIN_REC_TIME);
            ui_hide(MAIN_REC_RRTIME);
            ui_hide(MAIN_REC_RRTIME);
            //ui_hide(MAIN_REC_TIME);  
            //ui_show(MAIN_REC_RRTIME);  
            //ui_hide(MAIN_MOVE_1);
            ui_pic_show_image_by_id(MAIN_PIC_LOCK, 1);
            //ui_pic_show_image_by_id(MAIN_PIC_BREC, 0); 
            main_lock_flag = 0;
            break;
        default:
            break;
        }
    }
}
static int main_pic_sd_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(MAIN_PIC_CARK)
.onchange = main_pic_sd_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};


static int main_retime_buf_transform(char *retime_buf, struct ui_time *time)
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
        if (res == 0) {
            one_pic_size = (0x21000 + 0xa000) / 1024;
        } else if (res == 1) {
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
#if 0
static int main_timer_rec_remain_onchange(void *ctr, enum element_change_event e, void *arg)
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
        if (main_lock_flag == 1) 
        {
            //ui_hide(MAIN_REC_RRTIME);
            break;
        }
        break;
    case ON_CHANGE_SHOW_PROBE:
        err = main_retime_buf_transform(retime_buf, time); //修改获取剩余时间方式，防止start_app timeout
        if (err) {
            puts("get retime err!\n");
        }
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MAIN_REC_RRTIME)
.onchange = main_timer_rec_remain_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif

static int samll_pic_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        if (db_select("mic")) {
            ui_pic_set_image_index(pic, 0);    /* ?|??-￠???é?3 */
        } else {
            ui_pic_set_image_index(pic, 1);
        }
        return false;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(MAIN_PIC_V)
.onchange = samll_pic_mic_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int main_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    const char *str = NULL;
    struct intent it;
    struct application *app;
    int ret;
    /* UI_ONTOUCH_DEBUG("\nmain page onchange \n"); */
    switch (e) {
    case ON_CHANGE_INIT:
        ui_register_msg_handler(ID_WINDOW_MAIN_PAGE, main_msg_handler);
        malloc_stats();
        break;
    case ON_CHANGE_FIRST_SHOW:
        #if 1
        if(if_in_rec == TRUE)  //by peng
        {
            //ui_pic_show_image_by_id(MAIN_PIC_BREC, 1);  
            ui_show(MAIN_REC_TIME);      
            ui_show(MAIN_MOVE_1);
	    main_rec_flag = 1;
        }
        else
        {
            //ui_pic_show_image_by_id(MAIN_PIC_BREC, 0); 
            //ui_show(MAIN_REC_RRTIME);   
            main_rec_flag = 0;
        }
        #endif
        
        if (db_select("mic")) {
            ui_pic_show_image_by_id(MAIN_PIC_VIEDO, 0);    /* 禁止录音 */
        } else {
            ui_pic_show_image_by_id(MAIN_PIC_VIEDO, 1);
        }
        
        if (storage_device_ready() == 1)
        {
            ui_pic_show_image_by_id(MAIN_PIC_CARK, 0);    
        }
        else
        {
            ui_pic_show_image_by_id(MAIN_PIC_CARK, 1);    
        }
        if (main_lock_flag == 1) 
        {
            ui_pic_show_image_by_id(MAIN_PIC_LOCK, 0);
        } 
        else 
        {
            ui_pic_show_image_by_id(MAIN_PIC_LOCK, 1);
        }
        break;
    default:
        return false;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ID_WINDOW_MAIN_PAGE)
.onchange = main_page_onchange,
};

extern int video_rec_aud_mute();
#if 1
static int car_viedo_ontouch(void *ctr, struct element_touch_event *e)  
{
    UI_ONTOUCH_DEBUG("**car recorder ontouch**");
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
         struct intent it;
        //__this->mic = !__this->mic;
       mic_set_toggle();
       video_rec_aud_mute();

       if (db_select("mic")) {
            ui_pic_show_image_by_id(MAIN_PIC_V, 0);   
            ui_pic_show_image_by_id(MAIN_PIC_VIEDO, 0); 
        } else {
            ui_pic_show_image_by_id(MAIN_PIC_V, 1);
            ui_pic_show_image_by_id(MAIN_PIC_VIEDO, 1); 
        }
        
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_VOIDE)
.ontouch = car_viedo_ontouch,
};
#endif

#if 0
/*******************************?????????**********************************/
static int main_set_lock_ontouch(void *ctr, struct element_touch_event *e)
{
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (if_in_rec == TRUE) {
            init_intent(&it);
            main_lock_flag = !main_lock_flag;
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_LOCK_FILE;
            it.exdata = main_lock_flag;
            start_app(&it);
            if (main_lock_flag == 1) {
                ui_pic_show_image_by_id(MAIN_PIC_LOCK, 0);
            } else {
                ui_pic_show_image_by_id(MAIN_PIC_LOCK, 1);
            }
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_LOCK)
.ontouch = main_set_lock_ontouch,
};
#endif
/***************************** 星期文字动作 ************************************/
static int text_week_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    struct sys_time sys_time;
    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
        //printf("\nit is week %d\n", ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day));
        ui_text_set_index(text, ReturnWeekDay(sys_time.year, sys_time.month, sys_time.day) - 1);
        return true;
    default:
        return false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(HOME_TXT_WEEKDAY)
.onchange = text_week_onchange,
 .ontouch = NULL,
};

/*****************************主界面电池控件动作 ************************************/
static int battery_main_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(HOME_BAT)
.onchange = battery_main_onchange,
 .ontouch = NULL,
};
#if 0
/****************************主界面时间控件动作 ************************************/
static int timer_sys_main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
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
REGISTER_UI_EVENT_HANDLER(HOME_TIM_TIME)
.onchange = timer_sys_main_onchange,
 .ontouch = NULL,
};
#endif

static int timer_main_red_onchange(void *ctr, enum element_change_event e, void *arg)
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
REGISTER_UI_EVENT_HANDLER(MAIN_REC_TIME)
.onchange = timer_main_red_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

/*****************************主界面系统日期控件动作 ************************************/
static int timer_sys_date_main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_system_time(&sys_time);
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
REGISTER_UI_EVENT_HANDLER(HOME_TIM_DATE)
.onchange = timer_sys_date_main_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#if 1
static int car_recorder_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**car recorder ontouch**");
    struct intent it;
    struct application *app;
   // union uireq req;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        if (__this->onkey_sel) {
            //ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            //ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        //ui_hide(MAIN_PIC_1);
        //ui_highlight_element_by_id(MAIN_PIC_1);
        //ui_hide(MAIN_PIC_1);
        //ui_show(MAIN_PIC_2);
        
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
	//show_main_ui();
        app = get_current_app();
        
        pmsg("app = %d\n", app);
		printf("\n----app = %s---\n",app->name);
        
        if (app) {
            if (!strcmp(app->name, "video_rec")) {
                ui_hide(ui_get_current_window_id());
                ui_show(ID_WINDOW_VIDEO_REC);
            }
            else
            {
                pmsg("11111\n");
            }
        } 
        else 
        {
            ui_hide(ui_get_current_window_id());
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_MAIN;
            start_app_async(&it, NULL, NULL);
        }
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_RECORDER)
.ontouch = car_recorder_ontouch,
};
#endif


u8 home_rec_touch = 0;
#if 0
static int home_rec_control_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**rec control ontouch**");
    struct intent it;
	struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
		if(!storage_device_ready()){
			//ui_set_call(main_show_msg,BOX_MSG_XXX);//?????sd??
			return false;
		}
		app = get_current_app();
        
		home_rec_touch = 1;
		if(!app){
			it.name = "video_rec";
			it.action = ACTION_VIDEO_REC_MAIN;
			start_app(&it);
		}
		 
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_CONTROL;
        start_app_async(&it, NULL, NULL);  
        
        break;
    }
    
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_REC)
.ontouch = home_rec_control_ontouch,
};
#endif
#if 0
extern int video_rec_take_photo(void);

static int car_photo_ontouch(void *ctr, struct element_touch_event *e)  //????
{
    UI_ONTOUCH_DEBUG("**car recorder ontouch**");
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

        if(!storage_device_ready()){
			//ui_set_call(main_show_msg,BOX_MSG_XXX);//?????sd??
			return false;
		}
        
        if(if_in_rec)
        {
            video_rec_take_photo();
        }
        else
        {
            app = get_current_app();
        
    		home_rec_touch = 1;  //by peng

    		if(!app){
                init_intent(&it);
                it.name = "video_photo";
                it.action = ACTION_PHOTO_TAKE_MAIN;
                start_app_async(&it, NULL, NULL);
    		}
            else
            {
                
            }

            it.name = "video_photo";
            it.action = ACTION_PHOTO_TAKE_CONTROL;
            start_app_async(&it, NULL, NULL);

            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app_async(&it, NULL, NULL); 
        }
		
            
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_PHOTO)
.ontouch = car_photo_ontouch,
};
#endif
static void app_action_back_ok(void *p, int err)
{
    if (err == 0) {
        puts("---app action back ok\n");
        ui_hide(ui_get_current_window_id());
    } else {
        printf("---app action back faild: %d\n", err);
    }
}


extern u8 av_in_statu;

static int main_lane_ontouch(void *ctr, struct element_touch_event *e)  //adas
#if 1
{    
    struct intent it;
    struct application *app;
    
    UI_ONTOUCH_DEBUG("**lane return ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        app = get_current_app();

        if(if_in_rec == TRUE)
        {
            return false;	     
        }
	#if 1
       	 init_intent(&it);
        app = get_current_app();	
        if (app) 
        {
		it.name = "video_rec";
		it.action = ACTION_BACK;
		start_app_async(&it, NULL, NULL);//不等待直接启动app
        } 
	#endif	
	#if 1	
        {
                struct intent it;
                lane_entry_flag = 1;
		sys_in_lane_flag = 1;
                ui_hide(ui_get_current_window_id());

                init_intent(&it);
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                it.data   = "lan_setting";
                //start_app(&it);  
                 start_app_async(&it, NULL, NULL);


        } 
	#endif	
        break;
    }
    return false;
}
#else
{    
    struct intent it;
    struct application *app;
    
    UI_ONTOUCH_DEBUG("**lane return ontouch**");
    switch (e->event) {
    case ELM_EVENT_TOUCH_UP:
        if (!av_in_statu)
        {
            break;        
        }
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");

        app = get_current_app();
        all_back = 1;
        if (app) {
            if (!strcmp(app->name, "video_rec")) {
                ui_hide(ui_get_current_window_id());
                ui_show(ID_WINDOW_ALL_BACK);
                
                init_intent(&it);
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_SWITCH;
                start_app(&it);    
            }
            else
            {
                pmsg("11111\n");
            }
        } 
        else 
        {
            ui_hide(ui_get_current_window_id());
            init_intent(&it);
            it.name = "video_rec";
            it.action = ACTION_VIDEO_REC_ALL_BACK;
            start_app_async(&it, NULL, NULL);
        }
        
        //app = get_current_app();
        //it.name = "video_rec";
        //it.action = ACTION_VIDEO_REC_ALL_BACK;
        //start_app(&it);
        #if 0
        if(if_in_rec == TRUE)
        {
            return false;	     
        }
        
        {
                struct intent it;
                lane_entry_flag = 1;
                ui_hide(ui_get_current_window_id());

                init_intent(&it);
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                it.data   = "lan_setting";
                start_app(&it);    
        } 
        #endif
        break;
    }
    return false;
}
#endif
REGISTER_UI_EVENT_HANDLER(HOME_BTN_ADAS)
.ontouch = main_lane_ontouch,
};




static int main_show_msg(int p)
{
	__main_msg_show(p,2000);
	return 0;
}

static int system_setting_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**sys setting ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        //ui_highlight_element_by_id(HOME_TXT_SETTINGS);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(if_in_rec == TRUE)
        {
            return false;	     
        }
        app = get_current_app();
        if (app) {
		//????????????????rec_app ?????????????
		#if 1
            init_intent(&it);
            it.name = app->name;
            it.action = ACTION_BACK;
            start_app_async(&it, app_action_back_ok, NULL);
		#else
			//?????????????,??????????,???????
			ui_set_call(main_show_msg,BOX_MSG_XXX);
			return false;	
		#endif
        } else {
            ui_hide(ui_get_current_window_id());
        }
        init_intent(&it);
        it.name = "video_system";
        it.action = ACTION_SYSTEM_MAIN;
        start_app_async(&it, NULL, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_SETTINGS)
.ontouch = system_setting_ontouch,
};
static void file_browse_ok(void *p, int err)
{
    puts("open file browser ok!!!\n");
    sys_touch_event_enable();
}
static int file_browse_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**file_browse  ontouch**");
    struct intent it;
    struct application *app;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_DOWN\n");
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        //ui_highlight_element_by_id(HOME_TXT_FILES);
        break;
    case ELM_EVENT_TOUCH_HOLD:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_HOLD\n");
        break;
    case ELM_EVENT_TOUCH_MOVE:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_MOVE\n");
        break;
    case ELM_EVENT_TOUCH_UP:
        UI_ONTOUCH_DEBUG("ELM_EVENT_TOUCH_UP\n");
        if(if_in_rec == TRUE)
        {
            return false;	     
        }
        
        sys_touch_event_disable();
        app = get_current_app();
        if (app) {
		//????????????????rec_app ?????????б?
		#if 1
            init_intent(&it);
            it.name = app->name;
            it.action = ACTION_BACK;
            start_app_async(&it, app_action_back_ok, NULL);
		#else
			//?????????????,??????????,???????
			ui_set_call(main_show_msg,BOX_MSG_XXX);
			sys_touch_event_enable();
			return false;
		#endif
        } else {
            ui_hide(ui_get_current_window_id());
        }
        init_intent(&it);
        it.name = "video_dec";
        it.action = ACTION_VIDEO_DEC_MAIN;
        start_app_async(&it, file_browse_ok, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(HOME_BTN_FILE)
.ontouch = file_browse_ontouch,
};







static int main_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("main_onchange\n");
        sys_key_event_takeover(true, false);
        __this->onkey_sel = 0;
        break;
    case ON_CHANGE_RELEASE:
        break;
    default:
        return false;
    }

    return false;
}
static int main_onkey(void *ctr, struct element_key_event *e)
{
    struct intent it;
    if (e->event == KEY_EVENT_LONG && e->value == KEY_POWER) {
        ui_hide(ui_get_current_window_id());
        sys_key_event_takeover(false, true);
        return true;
    }
    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    switch (e->value) {
    case KEY_UP:
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        __this->onkey_sel --;
        if (__this->onkey_sel < 1) {
            __this->onkey_sel = 3;
        }
        ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
        ui_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        break;
    case KEY_DOWN:
        if (__this->onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        }
        __this->onkey_sel ++;
        if (__this->onkey_sel > 3) {
            __this->onkey_sel = 1;
        }
        ui_highlight_element_by_id(onkey_sel_item[__this->onkey_sel - 1]);
        ui_highlight_element_by_id(onkey_sel_item1[__this->onkey_sel - 1]);
        break;
    case KEY_OK:
        if (__this->onkey_sel) {
            struct intent it;
            switch (__this->onkey_sel) {
            case 1:
                ui_hide(ui_get_current_window_id());
                init_intent(&it);
                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app_async(&it, NULL, NULL);
                break;
            case 2:
                ui_hide(ui_get_current_window_id());
                sys_touch_event_disable();
                init_intent(&it);
                it.name = "video_dec";
                it.action = ACTION_VIDEO_DEC_MAIN;
                start_app_async(&it, file_browse_ok, NULL);
                break;
            case 3:
                ui_hide(ui_get_current_window_id());
                init_intent(&it);
                it.name = "video_system";
                it.action = ACTION_SYSTEM_MAIN;
                start_app(&it);
                break;
            }
            return true;
        }
        break;
    case KEY_MODE:
        return true;
        break;
    default:
        return false;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(HOME_WIN)
.onchange = main_onchange,
 .onkey = main_onkey,
};

#endif
