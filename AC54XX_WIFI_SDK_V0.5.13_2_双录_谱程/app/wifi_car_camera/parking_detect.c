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
    int prev_state;
    int state;
    int count;
    int on_cnt;
    int off_cnt;
    int timer;
#if ((VIDEO_PARK_DECT == 3) || (VIDEO_PARK_DECT == 4))
    int uvc_id;
    u16 product_id;
    char manufacturer[32];
    char product[32];
    void *fd;
    OS_SEM sem;
#endif
};
#define USB_US_ENGLISH      0x0409

/* #if ((defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO1_ENABLE) || (defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO3_ENABLE)) */
#if (VIDEO_PARK_DECT != 0)

static struct parking_detect_info parking_detect;
#define __this  (&parking_detect)
#define DETECT_TIME     100
#define PARKING_STA_ON  1
#define PARKING_STA_OFF 0
#ifdef CONFIG_VIDEO1_ENABLE
#define FILT_PAR    5
#define PARKING_ON_CNT  5
#define PARKING_OFF_CNT 4
#elif (VIDEO_PARK_DECT == 3)
#define FILT_PAR    1
#define PARKING_ON_CNT  5
#define PARKING_OFF_CNT 4

#define DEV_TYPE_DA7267     0x3233
#define DEV_TYPE_2082A      0x8301
#define DEV_TYPE_7100       0x3822

#elif (VIDEO_PARK_DECT == 4)
#define FILT_PAR    1
#define PARKING_ON_CNT  5
#define PARKING_OFF_CNT 4


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
    if (__this->state == PARKING_STA_ON) {
        return 1;
    }
    return 0;
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

static void parking_detect_ctrl(void *par)
{
    int err = 0;
    int state = 0;

    if (__this->dev_online) {
        /* #ifdef CONFIG_VIDEO1_ENABLE */
#if (VIDEO_PARK_DECT == 1)
        state = avin_parking_det();
        /* #elif (defined CONFIG_VIDEO3_ENABLE) */
#elif (VIDEO_PARK_DECT == 3)
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
#else


        //video4 倒车处理



#endif

    } else {
        __this->count = 0;
        __this->state = PARKING_STA_OFF;
        __this->on_cnt = 0;
        __this->off_cnt = 0;
    }
#if 0
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
        puts("\nmsg parking on\n") ;
        parking_det_notify(1);
    }

    if ((!__this->count) && (__this->state == 1)) {
        __this->state = 0;
        puts("\nmsg parking off\n") ;
        parking_det_notify(0);
    }
#else
    if (__this->state == PARKING_STA_OFF) {
        if (state) {
            __this->on_cnt++;
            if (__this->on_cnt >= PARKING_ON_CNT) {
                __this->on_cnt = 0;
                __this->state = PARKING_STA_ON;
            }
        } else {
            __this->on_cnt = 0;
        }
    } else {
        if (!state) {
            __this->off_cnt++;
            if (__this->off_cnt >= PARKING_OFF_CNT) {
                __this->off_cnt = 0;
                __this->state = PARKING_STA_OFF;
            }
        } else {
            __this->off_cnt = 0;
        }
    }

    if (__this->state != __this->prev_state) {
        __this->prev_state = __this->state;
        if (__this->state == PARKING_STA_ON) {
            puts("\nmsg parking on\n");
            parking_det_notify(1);
        } else {
            puts("\nmsg parking off\n");
            parking_det_notify(0);
        }
    }
#endif
}
//倒车检测扫描注册,由系统调用
static int parking_detect_init()
{
    memset(__this, 0x0, sizeof(*__this));

    __this->state = PARKING_STA_OFF;
    __this->prev_state = __this->state;
    /* #ifdef CONFIG_VIDEO1_ENABLE */
#if (VIDEO_PARK_DECT == 1)
    av_parking_det_init();
    __this->dev_online = dev_online("video1");
    /* #elif (defined CONFIG_VIDEO3_ENABLE) */
#elif (VIDEO_PARK_DECT == 3)
    __this->uvc_id = -1;
    os_sem_create(&__this->sem, 1);

#elif (VIDEO_PARK_DECT == 4)
    //video4倒车处理

    os_sem_create(&__this->sem, 1);

#endif
    __this->timer = sys_timer_add(NULL, parking_detect_ctrl, DETECT_TIME);
    if (!__this->timer) {
        log_e("register parking detect fail\n");
        return -1;
    }

    return 0;
}
late_initcall(parking_detect_init);

static int pick_out_string(char *str, struct usb_string *us)
{
    int i = 0;
    if (us->code_type == 0x3) {
        if (us->language == USB_US_ENGLISH) {
            for (i = 0; i < us->len; i += 2, str++) {
                *str = (char)us->string[i];
            }
            *str = '\0';
        }
    }
    return 0;
}

static void video_device_event_handler(struct sys_event *event)
{
    int id = 0;

    /* #ifdef CONFIG_VIDEO1_ENABLE */
#if (VIDEO_PARK_DECT == 1)
    if (!strcmp(event->arg, "video1")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            /* case DEVICE_EVENT_ONLINE: */
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

    /* #elif (defined CONFIG_VIDEO3_ENABLE) */
#elif (VIDEO_PARK_DECT == 3)
    if (strncmp((char *)event->arg, "uvc", 3) == 0) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            /* case DEVICE_EVENT_ONLINE: */
            id = ((char *)event->arg)[3] - '0';
            os_sem_pend(&__this->sem, 0);
            __this->uvc_id = id;
            if (!__this->fd) {
                __this->fd = dev_open("uvc", (void *)__this->uvc_id);
            }

            if (!__this->dev_online) {
                __this->dev_online = 1;
            }

            if (__this->fd) {
                struct usb_device_id id;
                struct usb_string us;
                dev_ioctl(__this->fd, UVCIOC_GET_DEVICE_ID, (u32)(&id));
                __this->product_id = id.product;
                dev_ioctl(__this->fd, USBIOC_GET_MANUFACTURER, (u32)&us);
                pick_out_string(__this->manufacturer, &us);
                dev_ioctl(__this->fd, USBIOC_GET_PRODUCT_NAME, (u32)&us);
                pick_out_string(__this->product, &us);
            }
            os_sem_post(&__this->sem);
            break;
        case DEVICE_EVENT_OUT:
            if (__this->dev_online) {
                os_sem_pend(&__this->sem, 0);
                __this->dev_online = 0;
                __this->uvc_id = -1;
                /* parking_timer_close(); */
                if (__this->fd) {
                    dev_close(__this->fd);
                    __this->fd = NULL;
                }
                os_sem_post(&__this->sem);
            }
            break;
        }
    }
#else


    //video4倒车处理


#endif
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, video_device_event_handler, 0);

#else  // (video0 && video1) || (video0 && video3)
//获取倒车检测状态
int get_parking_status()
{
    return 0;
}
#endif
