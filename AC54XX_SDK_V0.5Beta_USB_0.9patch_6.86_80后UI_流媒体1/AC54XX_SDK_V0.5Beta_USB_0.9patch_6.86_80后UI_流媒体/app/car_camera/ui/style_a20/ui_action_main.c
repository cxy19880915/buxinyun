#include "ui/ui.h"
#include "sys/event.h"
#include "sys/app_core.h"

#include "app/main.h"
#include "app/action.h"


#define STYLE_NAME  JL01

/*REGISTER_UI_STYLE(STYLE_NAME)*/


static int main_list_onkey(struct ui_list *list, struct element_key_event *e)
{
    struct intent it;

    switch (e->value) {
    case KEY_OK:
        init_intent(&it);
        it.action = ACTION_BACK;
        start_app(&it);
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_MAIN;
        start_app(&it);
        return true;
    default:
        break;
    }

    return false;
}


static int main_list_onchange(struct ui_list *list, enum element_change_event e,
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
REGISTER_UI_EVENT_HANDLER(ID_LIST_MAIN)
.onkey 		= main_list_onkey,
    .onchange 	= main_list_onchange,
};


