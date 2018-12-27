#ifndef UI_BATTERY_H
#define UI_BATTERY_H


#include "ui/control.h"
#include "list.h"






struct ui_battery {
    struct element elm;
    int src;
    u8 index;
    struct list_head entry;
    const struct ui_battery_info *info;
    const struct element_event_handler *handler;
};




void ui_battery_level_change(int persent, int incharge);














#endif

