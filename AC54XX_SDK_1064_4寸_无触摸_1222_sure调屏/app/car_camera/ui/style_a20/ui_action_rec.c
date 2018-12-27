#include "ui/ui.h"
#include "sys/event.h"
#include "sys/app_core.h"


#include "app/action.h"


#define STYLE_NAME  JL01

//REGISTER_UI_STYLE(STYLE_NAME)

/*static int list_3_onkey(struct ui_list *list, struct element_key_event *e)*/
/*{*/
/*struct intent it;*/

/*switch (e->value)*/
/*{*/
/*case KEY_OK:*/
/*init_intent(&it);*/

/*it.action = ACTION_VIDEO_REC_CONFIG;*/
/*it.data = DATA_IMAGE_CAP_SIZE;*/

/*switch (list->hi_index)*/
/*{*/
/*case 0:*/
/*it.exdata = "1080p";*/
/*break;*/
/*case 1:*/
/*it.exdata = "720p";*/
/*break;*/
/*case 2:*/
/*it.exdata = "VGA";*/
/*break;*/
/*default:*/
/*break;*/
/*}*/
/*start_activity(&it);*/

/*return true;*/
/*defalut:*/
/*break;*/
/*}*/

/*return false;*/
/*}*/

static const char *video_resolution[] = {
    "res:1080p",
    "res:720p",
    "res:VGA",
};


static int list_2_onkey(struct ui_list *list, struct element_key_event *e)
{
    struct intent it;

    switch (e->value) {
    case KEY_OK:
        layout_hide(ID_LAYOUT_3_2);
        ui_text_show_index_by_id(ID_TEXT_RESOLUTION, list->hi_index);

        init_intent(&it);
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = video_resolution[list->hi_index];
        start_app(&it);
        return true;
defalut:
        break;
    }

    return false;
}
static int list_2_onchange(struct ui_list *list, enum element_change_event e,
                           void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        list_on_focus(list);
        break;
    case ON_CHANGE_RELEASE:
        list_lose_focus(list);
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_LIST_2)
.onkey 		= list_2_onkey,
    .onchange 	= list_2_onchange,
};




static int resolution_onchange(struct ui_text *text,
                               enum element_change_event e, void *arg)
{
    int i;
    struct intent it;

    switch (e) {
    case ON_CHANGE_INIT:
        init_intent(&it);
        it.action = ACTION_VIDEO_REC_GET_CONFIG;
        it.data = "res:";
        start_app(&it);
        for (i = 0; i < ARRAY_SIZE(video_resolution); i++) {
            if (!strcmp(it.data, video_resolution[i])) {
                printf("---------res: %s\n", it.data);
                ui_text_set_index(text, i);
                break;
            }
        }
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_TEXT_RESOLUTION)
.onchange 	= resolution_onchange,
};

static int list_1_onkey(struct ui_list *list, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_OK:
        layout_show(ID_LAYOUT_3_2);
        return true;
    default:
        break;
    }

    return false;
}

static int list_1_onchange(struct ui_list *list, enum element_change_event e,
                           void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        list_on_focus(list);
        break;
    case ON_CHANGE_RELEASE:
        list_lose_focus(list);
        break;
    }

    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_LIST_1)
.onkey 		= list_1_onkey,
    .onchange 	= list_1_onchange,
};


static int ui_time_onchange(struct ui_time *time, enum element_change_event e,
                            void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        time->year = 2016;
        time->month = 6;
        time->day = 20;
        time->hour = 15;
        time->min = 20;
        time->sec = 0;
        break;
    default:
        return false;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(ID_TIME_0)
.onchange = ui_time_onchange,
};



static int window_on_key(struct window *win, struct element_key_event *e)
{
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            puts("key_menu\n");
            layer_toggle(ID_VIDEO_REC_MENU);
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
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
.onkey = window_on_key,
};





