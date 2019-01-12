#include "ui/includes.h"
#include "system/includes.h"

#include "main.h"
#include "action.h"


#define STYLE_NAME  JL01

/*REGISTER_UI_STYLE(STYLE_NAME)*/

#define NEWLIST_2 0x5733082


static int main_grid_onkey(struct ui_grid *grid, struct element_key_event *e)
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
    case KEY_DOWN:
        puts("key_down\n");
        break;
    case KEY_UP:
        puts("key_up\n");
        break;
    default:
        break;
    }

    return false;
}


static int main_grid_onchange(struct ui_grid *grid, enum element_change_event e,
                              void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
        grid_on_focus(grid);
        break;
    case ON_CHANGE_RELEASE:
        grid_lose_focus(grid);
        break;
    default:
        break;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(NEWLIST_2)
.onkey 		= main_grid_onkey,
    .onchange 	= main_grid_onchange,
};


