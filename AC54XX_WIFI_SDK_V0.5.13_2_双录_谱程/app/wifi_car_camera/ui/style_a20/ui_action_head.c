#include "ui/ui.h"
#include "sys/event.h"
#include "sys/app_core.h"

#include "app/main.h"
#include "app/action.h"


#define STYLE_NAME  JL01

REGISTER_UI_STYLE(STYLE_NAME)


static int dev_event_handler(struct sys_event *event, void *priv)
{
    if (!strcmp(event->arg, "sd0") || !strcmp(event->arg, "sd1") || !strcmp(event->arg, "sd2")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            ui_pic_show_image_by_id(ID_SD_PIC, 0);
            break;
        case DEVICE_EVENT_OUT:
            ui_pic_show_image_by_id(ID_SD_PIC, 1);
            break;
        }
    }

    return false;
}

static int sd_pic_onchange(struct ui_pic *pic, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        /*if (dir_empty("/dev/storge/sd/")){*/
        /*ui_pic_show_image(pic, 1);*/
        /*}*/
        register_sys_event_handler(SYS_DEVICE_EVENT, 0, dev_event_handler);
        break;
    case ON_CHANGE_RELEASE:
        unregister_sys_event_handler(dev_event_handler);
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_SD_PIC)
.onchange 	= sd_pic_onchange,
};


