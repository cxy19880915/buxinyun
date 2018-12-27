#ifndef UI_PIC_H
#define UI_PIC_H

#include "ui/ui_core.h"




struct ui_pic {
    struct element elm;
    char index;
    const struct ui_pic_info *info;
    const struct element_event_handler *handler;
};






void *new_ui_pic(const void *_info, struct element *parent);

int ui_pic_show_image_by_id(int id, int index);

int ui_pic_set_image_index(struct ui_pic *pic, int index);










#endif

