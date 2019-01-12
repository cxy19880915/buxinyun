/*****************************************************************
>file name : ui_action_upd.c
>author : lichao
>create time : Wed 11 Oct 2017 05:13:07 PM HKT
*****************************************************************/
#include "ui/includes.h"
#include "ui/ui_battery.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "upgrade_style.h"
#include "action.h"
#include "app_config.h"
#include "app_database.h"

#ifdef CONFIG_UI_STYLE_LY_ENABLE

#define STYLE_NAME UPGRADE

static int menu_upgrade_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int choice = 0;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
        choice = sel_item ? 0 : 1;

        init_intent(&it);
        it.name	= "sdcard_upgrade";
        it.action = ACTION_UPGRADE_SET_CONFIRM;
        it.data = (void *)choice;
        start_app_async(&it, NULL, NULL);
        break;
    case KEY_DOWN:
        e->value = KEY_RIGHT;
        break;
    case KEY_UP:
        e->value = KEY_LEFT;
        break;
    case KEY_MENU:

        break;
    case KEY_MODE:
        break;
    default:
        break;
    }

    return false;
}


REGISTER_UI_EVENT_HANDLER(UPD_ID_AFX_MSGBOX)
.onkey = menu_upgrade_onkey,
};

#endif
