/*****************************************************************
>file name : app/car_camera/parking_detect.c
>author : lichao
>create time : Mon 19 Jun 2017 04:43:26 PM HKT
*****************************************************************/

#include "system/includes.h"
#include "device/device.h"
#include "app_config.h"

struct parking_detect_info {
    u8  dev_online;
    int state;
    int count;
    int timer;
    int timer_accin;
    int count_accin;
    int state_accin;
    int display_accin;
#ifdef CONFIG_VIDEO3_ENABLE
    int uvc_id;
    u16 product_id;
    void *fd;
    OS_SEM sem;
#endif
};

#if ((defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO1_ENABLE) || (defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO3_ENABLE))

static struct parking_detect_info parking_detect;
#define __this  (&parking_detect)
#ifdef CONFIG_VIDEO1_ENABLE
#define FILT_PAR    5
#define DETECT_TIME     100
#elif (defined CONFIG_VIDEO3_ENABLE)
#define FILT_PAR    1
#define DETECT_TIME     500

#define DEV_TYPE_DA7267     0x3233
#define DEV_TYPE_2082A      0x8301
#define DEV_TYPE_7100       0x3822
#endif

//倒车检测 扫描关闭
void parking_timer_close()
{
    if (__this->timer) {
        __this->count = 0;
        __this->state = 0;
        sys_timer_del(__this->timer);
        __this->timer = 0;
    }
}

//获取倒车检测状态
int get_parking_status()
{
    return (__this->state);
}

#ifdef CONFIG_VIDEO1_ENABLE
static int avin_parking_det()
{
    if (av_parking_det_status()) {
        return 1;
    }
    return 0;
}
#endif

//倒车状态消息发送
void parking_det_notify(u8 on_off)
{
    struct sys_event eve;
    eve.arg = "parking";
    eve.type = SYS_DEVICE_EVENT;
    if (on_off) {
        sys_key_event_disable();
        sys_touch_event_disable();
        eve.u.dev.event = DEVICE_EVENT_IN;
    } else {
        sys_key_event_enable();
        sys_touch_event_enable();
        eve.u.dev.event = DEVICE_EVENT_OUT;
    }

    sys_event_notify(&eve);//MSG FOR parking ui DISP
}

//倒车检测扫描函数

static void parking_dectect_ctrl(void *par)
{
    int err = 0;
    int state = 0;
#ifdef CONFIG_VIDEO1_ENABLE
    if (!__this->dev_online) {
        __this->count = 0;
        return;
    }

    state = avin_parking_det();
#elif (defined CONFIG_VIDEO3_ENABLE)
    struct uvc_processing_unit uvc_pu;
    int pk_state = 0;
    os_sem_pend(&__this->sem, 0);

    memset(&uvc_pu, 0x0, sizeof(uvc_pu));
    if (__this->product_id == DEV_TYPE_DA7267) {
        uvc_pu.request = 0x81;
        uvc_pu.type = 0xA1;
        uvc_pu.value = 0x0100;
        uvc_pu.index = 0x0300;
        uvc_pu.len = 4;
        pk_state = 0xfd1005;//
    } else if (__this->product_id == DEV_TYPE_2082A) {
        u32 set_cmd = 0xff001005;
        uvc_pu.request = 0x01;
        uvc_pu.type = 0x21;
        uvc_pu.value = 0x0100;
        uvc_pu.index = 0x0300;
        memcpy(uvc_pu.buf, &set_cmd, sizeof(set_cmd));
        uvc_pu.len = 4;
        err = dev_ioctl(__this->fd, UVCIOC_REQ_PROCESSING_UNIT, (unsigned int)&uvc_pu);

        uvc_pu.request = 0x85;
        uvc_pu.type = 0xA1;
        uvc_pu.len = 2;
        err = dev_ioctl(__this->fd, UVCIOC_REQ_PROCESSING_UNIT, (unsigned int)&uvc_pu);

        uvc_pu.request = 0x81;
        uvc_pu.type = 0xA1;
        uvc_pu.len = 4;
        pk_state = 0x9f1005;
    } else {
        uvc_pu.request = 0x81;
        uvc_pu.type = 0xA1;
        uvc_pu.value = 0x0100;
        uvc_pu.index = 0x0200;
        uvc_pu.len = 2;
        pk_state = 0x79;
    }
    if (__this->fd) {
        err = dev_ioctl(__this->fd, UVCIOC_REQ_PROCESSING_UNIT, (unsigned int)&uvc_pu);
    } else {
        __this->count = 0;
    }
    memcpy(&state, uvc_pu.buf, uvc_pu.len);
    /*printf("state : 0x%x.\n", state);*/
    if (err || (state != pk_state)) {
        state = 0;
    }
    os_sem_post(&__this->sem);
#endif
    if (state) {
        if (__this->count <= FILT_PAR + 1) {
            __this->count++;
        }
    } else {
        if (__this->count) {
            __this->count = 0;
        }
    }

    if ((__this->count > FILT_PAR) && (__this->state == 0)) { //500ms
        __this->state = 1;
        puts("\n msg parking on \n") ;
        parking_det_notify(1);
    }

    if ((!__this->count) && (__this->state == 1)) {
        __this->state = 0;
        puts("\n msg parking off\n") ;
        parking_det_notify(0);
    }

}
//倒车检测扫描注册,由系统调用
static int parking_detect_init()
{
    memset(__this, 0x0, sizeof(*__this));

#ifdef CONFIG_VIDEO1_ENABLE
    av_parking_det_init();
    __this->dev_online = dev_online("video1");
    if (!__this->timer) {
        __this->timer =  sys_timer_add(NULL, parking_dectect_ctrl, DETECT_TIME);//100ms
    }
#elif (defined CONFIG_VIDEO3_ENABLE)
    __this->uvc_id = -1;
    os_sem_create(&__this->sem, 1);
#endif

    return 0;
}
late_initcall(parking_detect_init);


/***************????acc in?????   by peng*************************************/

static int accin_det_status()
{
    if(get_accin_det_status())
    {
        return 1;
    }
    return 0;
}

void accin_det_notify(u8 on_off)
{
    struct sys_event eve;
    eve.arg = "accin";
    eve.type = SYS_DEVICE_EVENT;
    if (on_off) {
        
        sys_key_event_enable();
        sys_touch_event_enable();
        eve.u.dev.event = DEVICE_EVENT_IN;
    } else {
        sys_key_event_disable();
        sys_touch_event_disable();
        eve.u.dev.event = DEVICE_EVENT_OUT;
    }

    sys_event_notify(&eve);//MSG FOR parking ui DISP
}

static u8 acc_cnt = 0;

static void accin_disp_tip_timer(void *par)
{
    struct sys_event eve;
    

    if(acc_cnt == 0)
    {
        eve.arg = "disacc";
        eve.type = SYS_DEVICE_EVENT;
        eve.u.dev.event = DEVICE_EVENT_OUT;
        sys_event_notify(&eve);
    }
    else if(acc_cnt >= 10)
    {
        accin_det_notify(0);
        sys_timer_del(__this->display_accin);
        __this->display_accin = 0;
        acc_cnt = 0;
        return;
    }
    acc_cnt++;
}

static void accin_dectect_ctrl(void *par)
{
    int err = 0;
    int state = 0;
    struct sys_event eve;
    if(!usb_is_charging())
		{
			__this->count_accin = 0;
			return;	
		}
    state = accin_det_status();

    if (state) {
        if (__this->count_accin <= FILT_PAR + 1) {
            __this->count_accin++;
        }
    } else {
        if (__this->count_accin) {
            __this->count_accin = 0;
        }
    }

    if ((__this->count_accin > FILT_PAR) && (__this->state_accin == 0)) { //500ms
        __this->state_accin = 1;

        if(__this->display_accin)
        {
            eve.arg = "disacc";
            eve.type = SYS_DEVICE_EVENT;
            eve.u.dev.event = DEVICE_EVENT_IN;
            sys_event_notify(&eve);
            
            sys_timer_del(__this->display_accin);
            acc_cnt = 0;
            return;
        }
        accin_det_notify(1);
    }

    if ((!__this->count_accin) && (__this->state_accin == 1)) {
        __this->state_accin = 0;
        __this->display_accin = sys_timer_add(NULL, accin_disp_tip_timer, 1000);
        
        //accin_det_notify(0);
    }

}



static int accin_detect_init()
{
    memset(__this, 0x0, sizeof(*__this));

    accin_det_init();
  
    if (!__this->timer_accin) {
        __this->timer_accin =  sys_timer_add(NULL, accin_dectect_ctrl, DETECT_TIME);//100ms
    }

    return 0;
}
late_initcall(accin_detect_init);

/********************************************************************************/

static void video_device_event_handler(struct sys_event *event)
{
    int id = 0;

#ifdef CONFIG_VIDEO1_ENABLE
    if (!strcmp(event->arg, "video1")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            if (!__this->dev_online) {
                __this->dev_online = 1;
            }
            break;
        case DEVICE_EVENT_OUT:
            if (__this->dev_online) {
                __this->dev_online = 0;
            }
            break;
        }
    }

#elif (defined CONFIG_VIDEO3_ENABLE)
    if (strncmp((char *)event->arg, "uvc", 3) == 0) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            id = ((char *)event->arg)[3] - '0';
            __this->uvc_id = id;
            if (!__this->fd) {
                __this->fd = dev_open("uvc", (void *)__this->uvc_id);
            }

            if (!__this->dev_online) {
                __this->dev_online = 1;
                if (!__this->timer) {
                    __this->timer =  sys_timer_add(NULL, parking_dectect_ctrl, DETECT_TIME);//100ms
                }
            }

            if (__this->fd) {
                struct usb_device_id id;
                dev_ioctl(__this->fd, UVCIOC_GET_DEVICE_ID, (u32)(&id));
                __this->product_id = id.product;
            }
            break;
        case DEVICE_EVENT_OUT:
            if (__this->dev_online) {
                __this->dev_online = 0;
                __this->uvc_id = -1;
                parking_timer_close();
                os_sem_pend(&__this->sem, 0);
                if (__this->fd) {
                    dev_close(__this->fd);
                    __this->fd = NULL;
                }
                os_sem_post(&__this->sem);
            }
            break;
        }
    }

#endif
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, video_device_event_handler, 0);
#else

//获取倒车检测状态
int get_parking_status()
{
    return 0;

}
#endif
