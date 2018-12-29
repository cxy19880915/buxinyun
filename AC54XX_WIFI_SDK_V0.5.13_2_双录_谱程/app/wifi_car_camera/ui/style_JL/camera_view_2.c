#include "sys/event.h"
#include "ui/ui_server.h"

#define STYLE_NAME 	LY

/* REGISTER_UI_STYLE(STYLE_NAME); */

#if 0
static int menu_2_onkey(struct ui_menu *menu, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_OK:
        layout_hide(ID_LAYOUT_3_2);
        ui_text_show_by_index(ID_TEXT_1, menu->hi_index - 1);
        return true;
defalut:
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_MENU_2)
.onkey = menu_2_onkey,
};


static int menu_1_onkey(struct ui_menu *menu, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_OK:
        switch (menu->hi_index) {
        case 0:
            layout_show(ID_LAYOUT_3_2);
            break;
        default:
            break;
        }
        break;
    default:
        return false;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(ID_MENU_1)
.onkey = menu_1_onkey,
};
#endif


static int layer_3_onkey(struct layer *layer, struct element_key_event *e)
{
    int id;
    struct element *elm;

    switch (e->value) {
    case KEY_MENU:
        elm = ui_get_highlight_child_by_id(ID_LAYOUT_3_0);
        ASSERT(elm != NULL);
        printf("elm: %x\n", elm->id);
        if (elm->id == ID_VIDEO_SETTING) {
            ui_no_highlight_element(elm);
            /*ui_highlight_sibling(elm, UI_DIR_RIGHT);*/
        } else {
            layer_hide(ID_LAYER_3);
        }
        break;
    default:
        return false;
    }

    return true;
}

static int layer_menu_onchange(struct layer *layer, enum element_change_event e)
{

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_LAYER_3)
.onkey = layer_3_onkey,
 .onchange = layer_menu_onchange,
};


static int window_on_key(struct window *win, struct element_key_event *e)
{
    switch (e->event) {
    case ELM_EVENT_KEY_CLICK:
        switch (e->value) {
        case KEY_MENU:
            layer_toggle(ID_LAYER_3);
            break;
        default:
            return false;
        }
        break;
    default:
        return false;
    }

    return true;
}

REGISTER_WINDOW_EVENT_HANDLER(ID_WINDOW_1)
.onkey = window_on_key,
};










