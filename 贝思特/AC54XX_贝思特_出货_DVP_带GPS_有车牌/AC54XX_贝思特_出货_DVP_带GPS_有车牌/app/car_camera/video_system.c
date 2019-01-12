#include "system/includes.h"
#include "server/ui_server.h"
#include "video_system.h"

#include "action.h"
#include "style.h"
#include "app_config.h"


extern int video_sys_set_config(struct intent *it);

struct video_system_hdl sys_handler;

#define __this 	(&sys_handler)
#define sizeof_this     (sizeof(struct video_system_hdl))


static int video_sys_init(void)
{
    static u8 init = 0;
    if (init == 0) {
        init = 1;
        memset(__this, 0, sizeof_this);
    }
    return 0;
}
static int show_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -1;
    }

    req.show.id = ID_WINDOW_VIDEO_SYS;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif

    return 0;
}
static void hide_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_sys_main_ui\n");

    req.hide.id = ID_WINDOW_VIDEO_SYS;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}

//extern int lane_entry_flag;

static int state_machine(struct application *app, enum app_state state, struct intent *it)
{

#ifdef CONFIG_TOUCH_UI_ENABLE
	static u8 first_flag = 0;
#endif

    switch (state) {
    case APP_STA_CREATE:
        video_sys_init();
        sys_fun_restore();

#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EFAULT;
        }
#endif

        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_SYSTEM_MAIN:
#ifdef CONFIG_TOUCH_UI_ENABLE
            if (first_flag) {
                //if(lane_entry_flag == 0)
                dec_main_flag = 0;
                show_main_ui();
            } else { //防止开机获取系统参数时刷出system的ui
                first_flag = 1;
            }
#endif
            break;
        case ACTION_SYSTEM_SET_CONFIG:
            video_sys_set_config(it);
            db_flush();
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
#ifdef CONFIG_TOUCH_UI_ENABLE
#ifdef CONFIG_UI_ENABLE
        hide_main_ui();
#endif
#endif
        break;
    case APP_STA_DESTROY:
#ifdef CONFIG_UI_ENABLE
        /*puts("--------sys close ui\n");*/
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
#endif

        break;
    }


    return 0;
}






static const struct application_operation video_system_ops = {
    .state_machine  = state_machine,
    .event_handler 	= NULL,
};

REGISTER_APPLICATION(app_video_system) = {
    .name 	= "video_system",
    .action	= ACTION_SYSTEM_MAIN,
    .ops 	= &video_system_ops,
    .state  = APP_STA_DESTROY,
};
