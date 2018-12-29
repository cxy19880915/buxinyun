#include "system/includes.h"
#include "app_config.h"
#include "server/ui_server.h"
#include "action.h"
#include "style.h"
#include "sys_set.h"


enum {
    SYS_AT_REC_STOP,
    SYS_AT_REC_WAIT_START,
    SYS_AT_REC_START,
};

struct sys_at_list {
    struct list_head entry;
    FILE *file;
    int repeat;
};

struct sys_at_head {
    int cnt;
    int repeat;
    char status[16];
};

struct sys_at_item {
    u8 state;
    u8 repeat;
    u32 jiffies;
    struct sys_event event;
};

//static struct sys_auto_test test_hdl;

//#define __this (&test_hdl)

sys_set_t sys_hdl;

sys_set_t *__this = &sys_hdl;

void video_sys_set_post_msg(const char *msg, ...)
{
    union uireq req;
    va_list argptr;
    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_SYS_SET;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }
    va_end(argptr);

}

static int show_sys_set_ui()
{
    union uireq req;
    if (!__this->ui) {
        return -1;
    }

    puts("show_main_ui\n");
    req.show.id = ID_WINDOW_SYS_SET;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    return 0;
}
static void hide_sys_set_ui()
{
    union uireq req;

    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }
    req.hide.id = ID_WINDOW_SYS_SET;
    server_request(__this->ui, UI_REQ_HIDE, &req);

}


#if 0
static void sys_auto_test_task(void *priv)
{
    int repeat = 0;
    struct sys_at_list *p;
    struct sys_at_item item;
    struct sys_at_head head;

    while (1) {
        os_sem_pend(&__this->sem, 0);

        if (__this->recode_start == SYS_AT_REC_WAIT_START) {

            os_time_dly(300);
            __this->jiffies = jiffies;
            __this->recode_start = SYS_AT_REC_START;
            puts("\n\n(((((((((((( auto test rec start ))))))))))\n\n");

        }

        if (__this->play_start) {

            os_time_dly(300);
            puts("\n\n(((((((((((( auto test replay start ))))))))))\n\n");

            list_for_each_entry(p, &__this->head, entry) {
                if (!__this->play_start) {
                    break;
                }

                fread(p->file, &head, sizeof(head));

                while (p->repeat--) {
                    while (__this->play_start) {
                        int len = fread(p->file, &item, sizeof(item));
                        if (len != sizeof(item)) {
                            break;
                        }
                        if (!repeat || item.repeat) {
                            int delay = (jiffies_to_msecs(item.jiffies) + 9) / 10;
                            if (delay == 0) {
                                delay = 1;
                            }
                            os_time_dly(delay);
                            if (item.event.type != SYS_DEVICE_EVENT) {
                                sys_event_notify(&item.event);
                            }
                        }
                    }
                    repeat = 1;
                    fseek(p->file, 0, SEEK_SET);
                }

                fclose(p->file);
            }

            __this->play_start = 0;
            puts("==========auto_test_replay: end\n");
        }
    }


}


static int sys_auto_test_start()
{
    __this->play_start = 1;
    os_sem_post(&__this->sem);

    return 0;
}

static int sys_auto_test_add(FILE *file, int repeat)
{
    struct sys_at_list *list;

    list = (struct sys_at_list *)malloc(sizeof(*list));
    if (!list) {
        return -ENOMEM;
    }
    list->file = file;
    list->repeat = repeat;

    list_add_tail(&list->entry, &__this->head);


    return 0;
}


int sys_event_recode(struct sys_event *event)
{
    struct sys_at_item item;

    if (event->type == SYS_KEY_EVENT && event->u.key.value == KEY_POWER) {

        if (__this->play_start) {
            __this->play_start = 0;
            return 1;
        }

        if (__this->recode_start != SYS_AT_REC_START) {
            return 0;
        }

        if (event->u.key.event == KEY_EVENT_CLICK) {
            __this->repeat = !__this->repeat;

            if (__this->repeat) {
                puts("\n\n(((((((((((( auto test repeat on))))))))))\n\n");
            } else {
                puts("\n\n(((((((((((( auto test repeat off))))))))))\n\n");
            }

        } else if (event->u.key.event == KEY_EVENT_LONG) {
            __this->recode_start = 0;
            __this->recode_start = SYS_AT_REC_STOP;
            fclose(__this->file);

            puts("\n\n(((((((((((( auto test rec stop ))))))))))\n\n");
        }
        return 1;
    }

    printf("===123 recode_key: %d, %d\n", event->u.key.value, __this->repeat);

    item.state = 0;
    item.repeat = __this->repeat;
    item.jiffies = jiffies - __this->jiffies;
    memcpy(&item.event, event, sizeof(*event));

    fwrite(__this->file, &item, sizeof(item));

    __this->jiffies = jiffies;

    return 0;
}

static int auto_test_recode()
{
    struct sys_at_head head;

    __this->file = fopen(CONFIG_STORAGE_PATH"/C/JL_TEST/TEST_***.bin", "w+");
    if (!__this->file) {
        return -EFAULT;
    }
    __this->repeat = 0;
    __this->jiffies = jiffies;
    __this->recode_start = SYS_AT_REC_WAIT_START;

    head.cnt = 0;
    head.repeat = 0;
    strcpy(head.status, "new");
    fwrite(__this->file, &head, sizeof(head));

    os_sem_post(&__this->sem);

    return 0;
}

static int auto_test_init()
{
    __this->play_start = 0;
    __this->recode_start = SYS_AT_REC_STOP;
    INIT_LIST_HEAD(&__this->head);
    os_sem_create(&__this->sem, 0);
    os_task_create(sys_auto_test_task, NULL, 10, 1024, 0, "sysAutoTest");

    return 0;
}
#endif
extern int video_rec_set_config(struct intent *it);
extern int video_photo_set_config(struct intent *it);
extern int video_sys_set_config(struct intent *it);


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int ret = 0;

    switch (state) {
    case APP_STA_CREATE:
		memset(&sys_hdl, 0, sizeof(sys_hdl));
		sys_hdl.ui = server_open("ui_server", NULL);
		if (!sys_hdl.ui) 
		{
            return -EINVAL;
        }
		sys_hdl.state = 0;
        break;
    case APP_STA_START:
		if (!it) 
		{
            break;
        }
		switch(it->action)
		{
			case ACTION_SYS_SET_MAIN:
				show_sys_set_ui();
				break;
			case ACTION_SYS_SET_CONFIG:
				video_rec_set_config(it);
            	db_flush();
				break;
			case ACTION_SYS_SET_PHOTO_CONFIG:
				video_photo_set_config(it);
            	db_flush();
				break;
			case ACTION_SYS_SET_SYS_CONFIG:	
				video_sys_set_config(it);
				db_flush();
				break;
		}
        break;
    case APP_STA_PAUSE:
        return -EFAULT;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
		sys_hdl.state = 0;
		hide_sys_set_ui();
        break;
    case APP_STA_DESTROY:
		puts("sys_set_destroy\n");
        if (sys_hdl.ui) {
            server_close(sys_hdl.ui);
        }
        break;
    }

    return ret;

}

extern u8 meun_id;
static int app_key_event_handler(struct key_event *key)
{
    struct intent it;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
			if(sys_hdl.state == 0)
			{
				sys_hdl.state = 1;  //进入菜单设置
				video_sys_set_post_msg("inrec");
			}
			if(meun_id == 4)  //退出菜单
			{
				meun_id = 0;
				sys_hdl.state = 0;
				//printf("===123456====\n");
				it.action = ACTION_BACK;
                start_app(&it);

				it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
				start_app(&it);
			}
            break;
        case KEY_UP:
			//puts("==sys_set KEY_UP==\n");
            break;
        case KEY_DOWN:
			//puts("==sys_set KEY_DOWN==\n");
            break;
		case KEY_MODE:
			printf("=====111====\n");
			if(sys_hdl.state == 0)
			{
				return false;
			}
			else
			{
				return true;
			}
            return false;		
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

static int event_handler(struct application app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return app_key_event_handler(&event->u.key);
    default:
        return false;
    }
    return false;
}


static const struct application_operation sys_set_app_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_sys_set) = {
    .name 	= "sys_set",
    .action	= ACTION_SYS_SET_MAIN,
    .ops 	= &sys_set_app_ops,
    .state  = APP_STA_DESTROY,
};


