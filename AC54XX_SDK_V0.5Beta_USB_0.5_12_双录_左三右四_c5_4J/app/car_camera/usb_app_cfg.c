
#include "system/includes.h"
#include "server/usb_server.h"
#include "app_config.h"
#include "action.h"

extern int set_usb_mass_storage(void);
extern int set_usb_camera(void);
extern int back_to_video_rec(void);


int usb_app_set_config(struct intent *it)
{
    puts("usb_app_set_config.\n");
    printf("it data : %s.\n", it->data);
    if (!strcmp(it->data, "usb:msd")) {
        set_usb_mass_storage();
    } else if (!strcmp(it->data, "usb:uvc")) {
        set_usb_camera();
    } else if (!strcmp(it->data, "usb:rec")) {
        back_to_video_rec();
    }
    return 0;
}

int usb_app_get_config(struct intent *it)
{
    puts("usb_app_get_config.\n");

    return 0;
}
