/*************************************************************************
	> File Name: usb_app.c
	> Author:
	> Mail:
	> Created Time: Tue 14 Feb 2017 09:14:45 AM HKT
 ************************************************************************/

#include "system/includes.h"
#include "server/usb_server.h"
#include "app_config.h"
#include "action.h"
#include "server/ui_server.h"
#include "ui_ctr_def.h"
#include "style.h"

enum {
    USB_SLAVE_MASS_STORAGE,
    USB_SLAVE_CAMERA,
    USB_SLAVE_ISP_TOOL,
    USB_SLAVE_MAX,
};


#define USB_STATE_NO_DEV         0x0
#define USB_STATE_DEVICE_MOUNT   0x1

struct usb_app_handle {
    u8 state;
    u8 mode;
    struct server *usb_slave;
    struct server *usb_host;
    struct server *ui;
    u8 *buf;
    u32 buf_size;
};

static struct usb_app_handle handler = {
    .state = USB_STATE_NO_DEV,
};

#define __this  (&handler)

extern int usb_app_set_config(struct intent *it);
extern int usb_app_get_config(struct intent *it);
/*
static int usb_app_mode_start(void)
{
    int err = 0;

    if (!__this->usb_slave){

        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave){
            return -EFAULT;
        }
    }

    return err;
}
*/
#define USB_CAMERA_BUF_SIZE     (3 * 1024 * 1024) // + 512 * 1024)

/*
 *mass storage挂载的设备列表
 */
const static char *mass_storage_dev_list[] = {
    SDX_DEV,
};


static int show_slave_list(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.show.id = ID_WINDOW_USB_SLAVE;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
    return 0;
}

static int hide_slave_list(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -EINVAL;
    }
    req.hide.id = ID_WINDOW_USB_SLAVE;
    server_request(__this->ui, UI_REQ_HIDE, &req);

#endif
    return 0;

}
static int usb_slave_start(void)
{
    show_slave_list();

    //usb_isp_tool_test();

    return 0;
}

static int usb_slave_stop(void)
{
    /*
     *关闭服务、释放资源
     */
    if (__this->usb_slave) {
        server_close(__this->usb_slave);
        __this->usb_slave = NULL;
    }

    if (__this->buf) {
        free(__this->buf);
        __this->buf = NULL;
    }


    __this->state = USB_STATE_NO_DEV;

    hide_slave_list();
    return 0;
}


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{

    switch (state) {
    case APP_STA_CREATE:
        memset(__this, 0x0, sizeof(*__this));
        server_load(usb_server);
#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EFAULT;
        }
#endif
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_USB_SLAVE_MAIN:
            usb_slave_start();
            break;
        case ACTION_USB_SLAVE_SET_CONFIG:
            usb_app_set_config(it);
            break;
        case ACTION_USB_SLAVE_GET_CONFIG:
            usb_app_get_config(it);
            break;
        }
        break;
    case APP_STA_PAUSE:
        return -EFAULT;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        usb_slave_stop();
        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
        }
        break;
    }

    return 0;

}
static int usb_app_key_event_handler(struct key_event *key)
{

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            //usb_app_select();
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

static int usb_app_device_event_handler(struct sys_event *event)
{

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        //usb_app_pause();
        break;
    }
    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_KEY_EVENT:
        return usb_app_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return usb_app_device_event_handler(event);
    default:
        return false;
    }
    return false;
}


static const struct application_operation usb_app_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_usb) = {
    .name 	= "usb_app",
    .action	= ACTION_USB_SLAVE_MAIN,
    .ops 	= &usb_app_ops,
    .state  = APP_STA_DESTROY,
};


/*
 *设置mass storage
 */
int set_usb_mass_storage(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        //printf("state mass storage is opened, state : 0x%x.\n", __this->state);
        return 0;
    }
    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            puts("usb server open failed.\n");
            return -EFAULT;
        }
    }

    //printf("debug isd download.\n");
    /*
     *设置请求参数：
     *类型：USB_MASS_STORAGE
     *挂载设备数量
     *挂载设备列表
     *状态：从机连接
     */
    req.type = USB_MASS_STORAGE;
    req.storage.dev_num = 1;
    req.storage.dev = mass_storage_dev_list;
    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    __this->state = USB_STATE_DEVICE_MOUNT;
    return 0;
}

/*
 *设置PC Camera
 */
int set_usb_camera(void)
{
    struct usb_req req = {0};
    int err = 0;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }

    if (!__this->usb_slave) {
        __this->usb_slave = server_open("usb_server", "slave");
        if (!__this->usb_slave) {
            return -EFAULT;
        }
    }

    /*
     *设置请求参数
     *类型：USB_CAMERA (可增加isp工具，调屏工具选项)
     *设置camera的编码所需buffer、buffer长度
     *quality -- 设置图像质量
     */
    req.type = USB_CAMERA | USB_ISP_TOOL | USB_SCREEN_TOOL;
    req.camera.name = "video0";
    if (!__this->buf) {
        __this->buf = (u8 *)malloc(USB_CAMERA_BUF_SIZE);
        if (!__this->buf) {
            return -ENOMEM;
        }
    }
    __this->buf_size = USB_CAMERA_BUF_SIZE;
    req.camera.buf = __this->buf;
    req.camera.buf_size = __this->buf_size;
    req.camera.info = NULL;
    req.camera.quality = 1;

    req.state = USB_STATE_SLAVE_CONNECT;
    err = server_request(__this->usb_slave, USB_REQ_SLAVE_MODE, &req);
    if (err != 0) {
        puts("usb slave request err.\n");
        return -EFAULT;
    }

    __this->state = USB_STATE_DEVICE_MOUNT;
    return 0;
}

/*
 *返回录像模式
 */
int back_to_video_rec(void)
{
    struct intent it;
    struct application *app;

    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 0;
    }
    init_intent(&it);
    app = get_current_app();
    if (app) {
        it.action = ACTION_BACK;
        start_app(&it);

        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_MAIN;
        start_app(&it);
    }

    return 0;
}

u8 get_usb_in_status()
{
    if (__this->state == USB_STATE_DEVICE_MOUNT) {
        return 1;
    }
    return 0;
}

