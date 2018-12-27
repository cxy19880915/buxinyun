#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"


#ifdef CONFIG_UI_STYLE_LY_ENABLE

#define STYLE_NAME  LY


/*
 * USB菜单
 */
static const char *table_usb_menu[] = {
    "usb:msd",
    "usb:uvc",
    "usb:rec",
    "\0"
};


/***************************** USB菜单设置 ************************************/

static int usb_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        sys_key_event_takeover(true, false);
        /*
         * USB页面打开后，UI任务接管key事件，APP不接收key事件
         */

        break;
    case ON_CHANGE_RELEASE:
        sys_key_event_takeover(false, false);
        /*
         * USB页面关闭后，恢复APP接收下一个key事件
         */

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_USB_SLAVE)
.onchange = usb_page_onchange,
 .ontouch = NULL,
};
static int menu_usb_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        switch (sel_item) {
        case 0:
            /*
             * U盘模式
             */
            ui_hide(VLIST_USB);
            ui_show(PIC_MSD_USB);
            break;
        case 1:
            /*
             * USB摄像头模式
             */
            ui_hide(VLIST_USB);
            ui_show(PIC_UVC_USB);
            break;
        case 2:
            /*
             * 回到录像模式
             */
            break;
        default:
            break;
        }

        init_intent(&it);
        it.name	= "usb_app";
        it.action = ACTION_USB_SLAVE_SET_CONFIG;
        it.data = table_usb_menu[sel_item];
        start_app_async(&it, NULL, NULL);
        break;
    case KEY_DOWN:
        return false;

        break;
    case KEY_UP:
        return false;

        break;
    case KEY_MENU:

        break;
    case KEY_MODE:
        break;
    default:
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_USB)
.onkey = menu_usb_onkey,
 .ontouch = NULL,
};


#endif
