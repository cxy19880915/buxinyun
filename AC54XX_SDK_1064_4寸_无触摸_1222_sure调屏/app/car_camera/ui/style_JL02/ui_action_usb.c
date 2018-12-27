#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "menu_parm_api.h"
#include "app_config.h"

#ifdef CONFIG_UI_STYLE_JL02_ENABLE

#define STYLE_NAME  JL02

static s8 onkey_sel = 0;

static int onkey_sel_item[3] = {
    USB_BTN_MASS,
    USB_BTN_CAM,
    USB_BTN_REC,
};
static int onkey_sel_item1[3] = {
    USB_TXT_MASS,
    USB_TXT_CAM,
    USB_TXT_REC,
};
/*
 * USB菜单
 */
static const char *table_usb_menu[] = {
    "usb:msd",
    "usb:uvc",
    "usb:rec",
    "\0"
};

/*****************************USB页面回调 ************************************/
static int usb_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct window *window = (struct window *)ctr;
    int err, item, id;
    struct intent it;
    int ret;
    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        ui_hide(ID_WINDOW_MAIN_PAGE);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_USB_SLAVE)
.onchange = usb_page_onchange,
};

/***************************** USB 存储器模式按钮动作 ************************************/
static int usb_mass_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**usb mass ontouch**");
    struct intent it;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        if (onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        }
        ui_highlight_element_by_id(USB_TXT_MASS);
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
        /*
         * U盘模式
         */
        ui_hide(USB_LAY_B);
        // ui_show(USB_LAY_MODE);
        ui_pic_show_image_by_id(USB_PIC_MODE, 0);

        init_intent(&it);
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_SET_CONFIG;
        it.data = table_usb_menu[0];
        start_app_async(&it, NULL, NULL);

        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USB_BTN_MASS)
.ontouch = usb_mass_ontouch,
};
/***************************** USB_PC模式按钮动作 ************************************/
static int usb_cam_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**usb cameraPc ontouch**");
    struct intent it;

    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        if (onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        }
        ui_highlight_element_by_id(USB_TXT_CAM);
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
        /*
         * USB摄像头模式
         */
        ui_hide(USB_LAY_B);
        // ui_show(USB_LAY_MODE);
        ui_pic_show_image_by_id(USB_PIC_MODE, 1);

        init_intent(&it);
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_SET_CONFIG;
        it.data = table_usb_menu[1];
        start_app_async(&it, NULL, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USB_BTN_CAM)
.ontouch = usb_cam_ontouch,
};
/***************************** USB 录影模式按钮动作 ************************************/
static int usb_rec_ontouch(void *ctr, struct element_touch_event *e)
{
    UI_ONTOUCH_DEBUG("**usb to rec ontouch**");
    struct intent it;
    switch (e->event) {
    case ELM_EVENT_TOUCH_DOWN:
        if (onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        }
        ui_highlight_element_by_id(USB_TXT_REC);
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
        init_intent(&it);
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_SET_CONFIG;
        it.data = table_usb_menu[2];
        start_app_async(&it, NULL, NULL);
        break;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(USB_BTN_REC)
.ontouch = usb_rec_ontouch,
};





static int usb_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        printf("usb_onchange\n");
        /* sys_key_event_takeover(true, false); */
        onkey_sel = 0;
        break;
    default:
        return false;
    }

    return false;
}
static int usb_onkey(void *ctr, struct element_key_event *e)
{
    if (e->event != KEY_EVENT_CLICK) {
        return false;
    }
    switch (e->value) {
    case KEY_UP:
        if (onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        }
        onkey_sel --;
        if (onkey_sel < 1) {
            onkey_sel = 3;
        }
        ui_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
        ui_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        break;
    case KEY_DOWN:
        if (onkey_sel) {
            ui_no_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
            ui_no_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        }
        onkey_sel ++;
        if (onkey_sel > 3) {
            onkey_sel = 1;
        }
        ui_highlight_element_by_id(onkey_sel_item[onkey_sel - 1]);
        ui_highlight_element_by_id(onkey_sel_item1[onkey_sel - 1]);
        break;
    case KEY_OK:
        if (onkey_sel) {
            struct intent it;
            switch (onkey_sel) {
            case 1:
                /*
                 * U盘模式
                 */
                ui_hide(USB_LAY_B);
                ui_pic_show_image_by_id(USB_PIC_MODE, 0);

                init_intent(&it);
                it.name	= "usb_app";
                it.action = ACTION_USB_SLAVE_SET_CONFIG;
                it.data = table_usb_menu[0];
                start_app_async(&it, NULL, NULL);
                break;
            case 2:
                /*
                 * USB摄像头模式
                 */
                ui_hide(USB_LAY_B);
                ui_pic_show_image_by_id(USB_PIC_MODE, 1);

                init_intent(&it);
                it.name	= "usb_app";
                it.action = ACTION_USB_SLAVE_SET_CONFIG;
                it.data = table_usb_menu[1];
                start_app_async(&it, NULL, NULL);
                break;
            case 3:
                /*
                 * 录像模式
                 */
                init_intent(&it);
                it.name	= "usb_app";
                it.action = ACTION_USB_SLAVE_SET_CONFIG;
                it.data = table_usb_menu[2];
                start_app_async(&it, NULL, NULL);
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
REGISTER_UI_EVENT_HANDLER(USB_WIN)
.onchange = usb_onchange,
 .onkey = usb_onkey,
};


#endif
