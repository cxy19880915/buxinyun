#include "ui/includes.h"
#include "system/includes.h"


#include "action.h"
#include "style.h"


#define STYLE_NAME  JL01

//REGISTER_UI_STYLE(STYLE_NAME)

/*static int grid_3_onkey(struct ui_grid *grid, struct element_key_event *e)*/
/*{*/
/*struct intent it;*/

/*switch (e->value)*/
/*{*/
/*case KEY_OK:*/
/*init_intent(&it);*/

/*it.action = ACTION_VIDEO_REC_CONFIG;*/
/*it.data = DATA_IMAGE_CAP_SIZE;*/

/*switch (grid->hi_index)*/
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


static int grid_2_onkey(struct ui_grid *grid, struct element_key_event *e)
{
    struct intent it;

    switch (e->value) {
    case KEY_OK:
        /*layout_hide(ID_LAYOUT_3_2);
        ui_text_show_index_by_id(ID_TEXT_RESOLUTION, grid->hi_index);*/

        init_intent(&it);
        it.action = ACTION_VIDEO_REC_SET_CONFIG;
        it.data = video_resolution[grid->hi_index];
        start_app(&it);
        return true;
    default:
        break;
    }

    return false;
}
static int grid_2_onchange(struct ui_grid *grid, enum element_change_event e, void *arg)
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
/*REGISTER_UI_EVENT_HANDLER(ID_LIST_2)
	.onkey 		= grid_2_onkey,
	.onchange 	= grid_2_onchange,
};*/




static int resolution_onchange(struct ui_text *text, enum element_change_event e, void *arg)
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
    default:
        break;
    }

    return true;
}
/*REGISTER_UI_EVENT_HANDLER(ID_TEXT_RESOLUTION)
	.onchange 	= resolution_onchange,
};*/

static int grid_1_onkey(struct ui_grid *grid, struct element_key_event *e)
{
    switch (e->value) {
    case KEY_OK:
        /*layout_show(ID_LAYOUT_3_2);*/
        return true;
    default:
        break;
    }

    return false;
}
static int grid_1_onchange(struct ui_grid *grid, enum element_change_event e,
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
/*REGISTER_UI_EVENT_HANDLER(UIID_VIDEO_REC_MENU_LIST_MAIN)
	.onkey 		= grid_1_onkey,
	.onchange 	= grid_1_onchange,
};*/


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
        break;
    }

    return true;
}
/*REGISTER_UI_EVENT_HANDLER(ID_TIME_0)
	.onchange = ui_time_onchange,
};*/



static int window_on_key(struct window *win, struct element_key_event *e)
{
    struct intent it;

    switch (e->event) {
    case KEY_EVENT_CLICK:
        switch (e->value) {
        case KEY_MENU:
            puts("key_menu\n");
            /* ui_toggle(IID_VIDEO_REC_MENU); */
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
/*REGISTER_UI_EVENT_HANDLER(ID_WINDOW_VIDEO_REC)
	.onkey = window_on_key,
};*/





