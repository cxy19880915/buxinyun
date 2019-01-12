#ifndef UI_TIME_H
#define UI_TIME_H


#include "ui/control.h"
#include "ui/ui_core.h"
#include "ui/p.h"

struct utime {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

struct ui_time {
    struct element_text text;
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
    u16 buf[32];
    void *timer;
    const struct ui_time_info *info;
    const struct element_event_handler *handler;
};

void *new_ui_time(const void *_info, struct element *parent);

int ui_time_update_by_id(int id, struct utime *time);












#endif

