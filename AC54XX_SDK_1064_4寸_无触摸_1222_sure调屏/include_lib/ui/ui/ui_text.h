#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "ui/ui_core.h"
#include "ui/control.h"


struct ui_text {
    struct element elm;
    const char *str;
    const char *format;
    int color;
    const struct ui_text_info *info;
    const struct element_event_handler *handler;
};



void *new_ui_text(const void *_info, struct element *parent);

int ui_text_show_index_by_id(int id, int index);


int ui_text_set_index(struct ui_text *text, int index);
int ui_text_set_str(struct ui_text *text, const char *format, const char *str);
int ui_text_set_str_by_id(int id, const char *format, const char *str);


void text_release(struct ui_text *text);









#endif
