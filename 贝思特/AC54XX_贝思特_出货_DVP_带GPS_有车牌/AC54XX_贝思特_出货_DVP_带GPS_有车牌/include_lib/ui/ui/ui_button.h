#ifndef UI_BUTTON_H
#define UI_BUTTON_H


#include "ui/control.h"
#include "ui/ui_core.h"

struct button {
    struct element elm;
    u8 image_index;
    const struct ui_button_info *info;
    const struct element_event_handler *handler;
    //struct element_text *text;
    //struct element_image img[0];
};

//#define REGISTER_BUTTON_EVENT_HANDLER(id) \
//REGISTER_CONTROL_EVENT_HANDLER(button, id)






#endif

