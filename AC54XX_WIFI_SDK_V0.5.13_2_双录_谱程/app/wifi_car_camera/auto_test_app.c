#include "system/includes.h"
#include "app_config.h"
#include "server/ui_server.h"
#include "action.h"
#include "style.h"


enum {
    SYS_AT_REC_STOP,
    SYS_AT_REC_WAIT_START,
    SYS_AT_REC_START,
};

struct sys_auto_test {
    u8 recode_start;
    u8 play_start;
    u32 repeat;
    u32 jiffies;
    OS_SEM sem;
    FILE *file;
    struct list_head head;
    struct server *ui;
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

static struct sys_auto_test test_hdl;

#define __this (&test_hdl)





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

    printf("===recode_key: %d, %d\n", event->u.key.value, __this->repeat);

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


static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int ret = 0;

    switch (state) {
    case APP_STA_CREATE:
        struct ui_style style;
        style.file = "mnt/spiflash/audlogo/at.sty";
        ui = server_open("ui_server", &style);
        if (!ui) {
            return -EFAULT;
        }
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_AUTO_TEST_MAIN:
            ret = auto_test_init();
            break;
        case ACTION_AUTO_TEST_RECODE:
            ret = auto_test_recode();
            break;
        case ACTION_AUTO_TEST_ADD_FILE:
            ret = sys_auto_test_add((FILE *)it->data, (int)it->exdata);
            break;
        case ACTION_AUTO_TEST_START:
            ret = sys_auto_test_start();
            break;
        }
        break;
    case APP_STA_PAUSE:
        return -EFAULT;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        if (ui) {
            server_close(ui);
        }
        break;
    }

    return ret;

}
static int app_key_event_handler(struct key_event *key)
{
    struct intent it;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            break;
        case KEY_UP:
            break;
        case KEY_DOWN:
            break;
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


static const struct application_operation at_app_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_at) = {
    .name 	= "auto_test",
    .action	= ACTION_AUTO_TEST_MAIN,
    .ops 	= &at_app_ops,
    .state  = APP_STA_DESTROY,
};


