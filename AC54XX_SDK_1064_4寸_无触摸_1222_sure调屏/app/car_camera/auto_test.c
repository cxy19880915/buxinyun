
#include "system/includes.h"
#include "app_config.h"
#include "list.h"

#include "storage_device.h"


#define SCRIPT_FILE_NAME    CONFIG_ROOT_PATH"auto.ecp"

static u32 get_num_from_str(char *str);

/**
** Ö¸Áî½âÎöºóµÄÁ´±í±íÍ·
*/
struct auto_test_st {

    struct list_head root;

};

/**
** Ö¸ÁîÀàÐÍ
*/
enum cmd_type {

    TYPE_SINGLE,
    TYPE_LOOP,
};

/**
** Ö¸Áî½Úµã
*/
struct at_func_node {

    enum cmd_type type;
    char *name;
    u16 loop;
    struct list_head entry;
    struct list_head child;
    u8(*func)(void *parm);
    void *parm;

};

/**
** Ö¸Áî¼¯
*/
struct at_func_array {
    char *name;
    u8(*func)(void *parm);
    void *(*get_parm_func)(char *str);  // ´Ó×Ö·û´®»ñÈ¡²ÎÊý
};

struct auto_test_st at_hdl;

#define __this (&at_hdl)


/*****************************************************************************
 Description: up¼üº¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_key_up(void *parm)
{

    struct sys_event event;
    puts("at_func_key_up.\n");

    event.type = SYS_KEY_EVENT;
    event.u.key.event = KEY_EVENT_CLICK;
    event.u.key.value = KEY_UP;
    sys_event_notify(&event);

    return 0;
}

/*****************************************************************************
 Description: down¼üº¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_key_down(void *parm)
{

    struct sys_event event;
    puts("at_func_key_down.\n");

    event.type = SYS_KEY_EVENT;
    event.u.key.event = KEY_EVENT_CLICK;
    event.u.key.value = KEY_DOWN;
    sys_event_notify(&event);

    return 0;
}

/*****************************************************************************
 Description: power¼üº¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_key_power(void *parm)
{

    struct sys_event event;
    puts("at_func_key_power.\n");

    event.type = SYS_KEY_EVENT;
    event.u.key.event = KEY_EVENT_CLICK;
    event.u.key.value = KEY_POWER;
    sys_event_notify(&event);

    return 0;
}

/*****************************************************************************
 Description: menu¼üº¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_key_menu(void *parm)
{

    struct sys_event event;
    puts("at_func_key_menu.\n");

    event.type = SYS_KEY_EVENT;
    event.u.key.event = KEY_EVENT_CLICK;
    event.u.key.value = KEY_MENU;
    sys_event_notify(&event);

    return 0;
}

/*****************************************************************************
 Description: ÑÓÊ±º¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_sleep(void *parm)
{

    u32 ms = (u32)parm;
    puts("at_func_sleep.\n");
    os_time_dly(ms / 10);

    return 0;
}

static void *at_sleep_get_parm(char *str)
{

    u32 ms;

    ms = get_num_from_str(str);
    return (void *)ms;
}

/*****************************************************************************
 Description: ´òÓ¡º¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_echo(void *parm)
{
    char *str = (char *)parm;
    puts("at_func_echo.\n");
    puts(str);

    return 0;
}

static void *at_echo_get_parm(char *str)
{

    char *end, *buf;
    u16 len;

    while (*str == ' ') {
        str++;
    }
    end = strchr(str, ';');
    len = end - str;

    buf = (char *)malloc(len + 1);
    if (!buf) {
        return NULL;
    }
    memset(buf, 0, len + 1);
    memcpy(buf, str, len);

    printf("echo get str:%s\n", buf);
    return (void *)buf;
}

/*****************************************************************************
 Description: ´¥Ãþ°´¼üÌ§Æð

 Note:

 Author:
******************************************************************************/
static u8 at_func_touch_up(void *parm)
{

    struct position *pos;
    struct sys_event event;
    puts("at_func_touch_up.\n");

    pos = (struct position *)parm;

    event.type = SYS_TOUCH_EVENT;
    event.u.touch.event = TOUCH_EVENT_UP;
    event.u.touch.pos.x =  pos->x;
    event.u.touch.pos.y =  pos->y;
    sys_event_notify(&event);

    return 0;
}


/*****************************************************************************
 Description: ´¥Ãþ°´¼ü°´ÏÂ

 Note:

 Author:
******************************************************************************/
static u8 at_func_touch_down(void *parm)
{

    struct position *pos;
    struct sys_event event;
    puts("at_func_touch_down.\n");

    pos = (struct position *)parm;

    event.type = SYS_TOUCH_EVENT;
    event.u.touch.event = TOUCH_EVENT_DOWN;
    event.u.touch.pos.x =  pos->x;
    event.u.touch.pos.y =  pos->y;
    sys_event_notify(&event);

    return 0;
}


/*****************************************************************************
 Description: ´¥Ãþ°´¼üÒÆ¶¯

 Note:

 Author:
******************************************************************************/
static u8 at_func_touch_move(void *parm)
{

    struct position *pos;
    struct sys_event event;
    puts("at_func_touch_move.\n");

    pos = (struct position *)parm;

    event.type = SYS_TOUCH_EVENT;
    event.u.touch.event = TOUCH_EVENT_MOVE;
    event.u.touch.pos.x =  pos->x;
    event.u.touch.pos.y =  pos->y;
    sys_event_notify(&event);

    return 0;
}


/*****************************************************************************
 Description: ´¥Ãþ°´¼ü°´×¡

 Note:

 Author:
******************************************************************************/
static u8 at_func_touch_hold(void *parm)
{

    struct position *pos;
    struct sys_event event;
    puts("at_func_touch_hold.\n");

    pos = (struct position *)parm;

    event.type = SYS_TOUCH_EVENT;
    event.u.touch.event = TOUCH_EVENT_HOLD;
    event.u.touch.pos.x =  pos->x;
    event.u.touch.pos.y =  pos->y;
    sys_event_notify(&event);

    return 0;
}


/*****************************************************************************
 Description: ´¥Ãþ°´¼ü×ø±ê²ÎÊý»ñÈ¡

 Note:

 Author:
******************************************************************************/
static void *at_touch_get_parm(char *str)
{

    struct position *pos;
    char *end;

    while (*str == ' ') {
        str++;
    }
    end = strchr(str, ';');

    pos = (struct position *)malloc(sizeof(struct position));
    if (!pos) {
        puts("pos malloc fail.\n");
        return NULL;
    }

    pos->x = get_num_from_str(str);
    str = strchr(str, ' ');
    if (str >= end) {
        puts("pos get err.\n");
        free(pos);
        return NULL;
    }
    while (*str == ' ') {
        str++;
    }

    pos->y = get_num_from_str(str);

    printf("at_touch_get_parm, x=%d, y=%d\n", pos->x, pos->y);
    return (void *)pos;
}



/*****************************************************************************
 Description: Èí¸´Î»º¯Êý

 Note:

 Author:
******************************************************************************/
static u8 at_func_reboot(void *parm)
{

    puts("at_func_reboot.\n");
    cpu_reset();
    while (1);
    return 0;
}
static u8 at_func_upgrade(void *parm)
{
    puts("at_func_upgrade.\n");

    FILE *fp_a = NULL;
    FILE *fp_b = NULL;
    fp_a = fopen(CONFIG_ROOT_PATH"upgrade/a.bfu", "r");

    if (fp_a) {
        puts("bfu copy");
        fp_b = fopen(CONFIG_ROOT_PATH"b.bfu", "r");
        if (fp_b) {
            fmove(fp_b, "upgrade/", NULL, 0);
            fmove(fp_a, "/", NULL, 0);
        }
        return 0;
    }

    fp_b = fopen(CONFIG_ROOT_PATH"upgrade/b.bfu", "r");
    if (fp_b) {
        puts("bfu copy111");
        fp_a = fopen(CONFIG_ROOT_PATH"a.bfu", "r");
        if (fp_a) {
            fmove(fp_a, "upgrade/", NULL, 0);
            fmove(fp_b, "/", NULL, 0);
        }
        return 0;
    }

    //reboot
    //cpu_reset();

    return 0;
}


/*****************************************************************************
 Description: ×Ô¶¯²âÊÔ½ø³Ì

 Note:

 Author:
******************************************************************************/
static void at_task(void *p)
{

    struct at_func_node *pos;
    u16 loop;

    puts("enter at_task...\n");


    list_for_each_entry(pos, &__this->root, entry) {

        if (pos->type == TYPE_LOOP) {
            struct at_func_node *p;
            printf("---loop: %d\n", pos->loop);
            loop = pos->loop;

            // loopÑ­»·µ÷ÓÃ
            while (loop--) {

                list_for_each_entry(p, &pos->child, entry) {
                    printf("---%s: %d\n", p->name, (int)p->parm);
                    if (p->func) {
                        p->func(p->parm);
                        os_time_dly(1);
                    }

                }
            }

            puts("---loop done.\n");
        } else {
            printf("---%s: %d\n", pos->name, (int)pos->parm);
            if (pos->func) {
                pos->func(pos->parm);
                os_time_dly(1);
            }
        }

    }

    while (1) {
        os_time_dly(500);
    }


}


/**
** º¯ÊýÓëÖ¸Áî×Ö·ûtable
*/
struct at_func_array func_table[] = {

    {"key up", at_func_key_up, NULL},
    {"key down", at_func_key_down, NULL},
    {"key power", at_func_key_power, NULL},
    {"key menu", at_func_key_menu, NULL},
    {"sleep", at_func_sleep, at_sleep_get_parm},
    {"reboot", at_func_reboot, NULL},
    {"echo", at_func_echo, at_echo_get_parm},
    {"touch up", at_func_touch_up, at_touch_get_parm},
    {"touch down", at_func_touch_down, at_touch_get_parm},
    {"touch move", at_func_touch_move, at_touch_get_parm},
    {"touch hold", at_func_touch_hold, at_touch_get_parm},
    {"upgrade", at_func_upgrade, NULL},

    {NULL, NULL},           // ½áÎ²·û
};

/*****************************************************************************
 Description: ¼ÓÔØÒ»¸öº¯Êýµ½Á´±íÎ²²¿

 Note:

 Author:
******************************************************************************/
#if 0
static u8 reg_at_func_tail(void *func, void *parm)
{

    u8(*p)(void *) = (u8(*)(void *))func;
    struct at_func_node *node_p;

    if (!p) {

        return -1;
    }
    node_p = malloc(sizeof(struct at_func_node));
    if (!node_p) {
        return -1;
    }
    memset(node_p, 0, sizeof(struct at_func_node));
    node_p->func = p;
    node_p->parm = parm;
    list_add_tail(&node_p->entry, &__this->root);

    return 0;
}
#endif


/*****************************************************************************
 Description: »ñÈ¡×Ö·ûÄÚµÄÊý×Ö

 Note:

 Author:
******************************************************************************/
static u32 get_num_from_str(char *str)
{

    char *p = str;
    u32 sum = 0;
    u8 s_flag = 0;

    while (*p) {
        if (s_flag == 0 && *p == ' ') { // Ìø¹ý¿Õ¸ñ
            p++;
            continue;
        }
        if (*p < '0' || *p > '9') {
            break;
        }
        s_flag = 1;
        sum = sum * 10;
        sum = sum + (*p - '0');
        p++;
    }
    printf("get str num: %d\n", sum);
    return sum;
}

/*****************************************************************************
 Description: Éú³ÉÒ»¸ö½Úµã

 Note:

 Author:
******************************************************************************/
struct at_func_node *make_a_node(char *name, enum cmd_type type,
                                 u16 loop, u8(*func)(void *parm), void *parm)
{

    struct at_func_node *node;

    node = malloc(sizeof(struct at_func_node));
    if (!node) {
        printf("make a node fail!!!\n");
        return NULL;
    }
    memset(node, 0, sizeof(struct at_func_node));
    node->type = type;


    if (type == TYPE_LOOP) {
        node->loop = loop;
        node->name = "loop";
        INIT_LIST_HEAD(&node->child);
        printf("make a loop_node \"%s\" sucess.\n", node->name);
    } else if (type == TYPE_SINGLE) {
        node->func = func;
        node->parm = parm;
        node->name = name;
        printf("make a single_node \"%s\" sucess.\n", node->name);
    }
    return node;
}

/*****************************************************************************
 Description: ½âÎöÒ»¸ö¶ÎÖÐµÄµ¥Ò»Ö¸Áî

 Note:

 Author:
******************************************************************************/
struct at_func_node *analysis_a_cmd(const char *str)
{

    struct at_func_node *node = NULL;
    const char *start;
    char *end, *p;
    u16 i;

    start = str;
    end = strchr(str, ';');

    // ½ØÈ¡Ò»¸öÖ¸Áî
    for (i = 0;; i++) {
        if (func_table[i].name == NULL) {
            break;
        }

        // Ñ°ÕÒÖ¸Áî²¢´´½¨
        if (strstr(start, func_table[i].name)
            && strstr(start, func_table[i].name) < end) {
            void *parm = NULL;

            p = strstr(start, func_table[i].name) + strlen(func_table[i].name);
            if (func_table[i].get_parm_func) {
                parm = func_table[i].get_parm_func(p);
            }
            node = make_a_node(func_table[i].name, TYPE_SINGLE, 0, func_table[i].func, (void *)parm);

            break;
        }
    }

    return node;
}

/*****************************************************************************
 Description: ½âÎöÑ­»·Ö¸ÁîÄÚ²¿ËùÓÐÖ¸Áî

 Note:

 Author:
******************************************************************************/
struct at_func_node *analysis_a_loop(const char *str)
{

    struct at_func_node *node_loop;
    char *start, *end, *point, *next, *p;

    u16 i, loop;

    start = strstr(str, "loop");
    end = strstr(start, "done");
    point = start + strlen("loop");
    loop = get_num_from_str(point);     // »ñÈ¡Ñ­»·´ÎÊý
    printf("get loop %d.\n", loop);

    node_loop = make_a_node(NULL, TYPE_LOOP, loop, NULL, NULL);

    point = strchr(start, ';') + 1;
    next = strchr(point, ';');

    // ¿ªÊ¼½âÎö loop ÄÚ²¿µÄÖ¸Áî£¬Ìí¼Óµ½child
    while (point < end) {

        // ½ØÈ¡Ò»¸öÖ¸Áî
        for (i = 0;; i++) {
            if (func_table[i].name == NULL) {
                break;
            }

            // Ñ°ÕÒÖ¸Áî²¢´´½¨
            if (strstr(point, func_table[i].name)
                && strstr(point, func_table[i].name) < next) {
                struct at_func_node *node;
                void *parm = NULL;

                p = strstr(point, func_table[i].name) + strlen(func_table[i].name);
                if (func_table[i].get_parm_func) {
                    parm = func_table[i].get_parm_func(p);
                }
                node = make_a_node(func_table[i].name, TYPE_SINGLE, 0, func_table[i].func, parm);

                list_add_tail(&node->entry, &node_loop->child); // ¼ÓÈëµ½loopÁ´±í
                break;
            }
        }

        // Ñ°ÕÒÏÂÒ»¸ö¶Î
        point = strchr(point, ';') + 1;
        next = strchr(point, ';');
    }

    return node_loop;
}


/*****************************************************************************
 Description: ¹Ø±ÕÉý¼¶°æ±¾¿ØÖÆ

 Note:

 Author:
******************************************************************************/
static u8 update_version_control_str(char *str)
{

    /// ¹Ø±ÕÉý¼¶°æ±¾¿ØÖÆ
    if (strstr(str, "update version control disable")) {

        puts("update version control disable!\n");
    }

    return 0;
}



/*****************************************************************************
 Description: ¿ªÊ¼½âÎöÈ«²¿Ö¸Áî

 Note:

 Author:
******************************************************************************/
static u8 analysis_start(char *str, u32 len)
{

    char *start, *end, *point, *next;

    puts("======= analysis_start =======\n");

    update_version_control_str(str);

    INIT_LIST_HEAD(&__this->root);

    start = str;
    end = start + len;

    point = start;
    next = strchr(point, ';');

    printf("str:%s, len:%d\n", str, len);

    //·Ö¶Î½âÎöÈ«²¿Ö¸Áî
    while (point < end) {

        /*printf("point= 0x%x, end= 0x%x\n", point, end);*/

        // ÕÒµ½Ò»¸öloopÑ­»·Ö¸Áî
        if (strstr(point, "loop") && strstr(point, "loop") < next) {
            struct at_func_node *node_loop;

            node_loop = analysis_a_loop(strstr(point, "loop"));
            if (node_loop) {
                list_add_tail(&node_loop->entry, &__this->root);
            }
            point = strstr(point, "done"); // Ö¸Ïòdone£¬Ìø¹ýÕû¸öloop
        }
        // µ¥Ò»Ö¸Áî
        else {
            struct at_func_node *node;

            node = analysis_a_cmd(point);
            if (node) {
                list_add_tail(&node->entry, &__this->root);
            }
        }

        // Ñ°ÕÒÏÂÒ»¶Î
        point = strchr(point, ';');
        if (point == NULL) {
            break;  // ÕÒ²»µ½ÏÂÒ»¸ö ; ºÅ£¬½áÊø
        }
        point += 1; // Ö¸Ïò·ÖºÅºóÃæ
        next = strchr(point, ';');
        if (next == NULL) {
            break;  // ÕÒ²»µ½ÏÂÏÂÒ»¸ö ; ºÅ£¬½áÊø
        }
    }



    puts("======= analysis_end =======\n");
    return 0;
}


/*****************************************************************************
 Description: ´òÓ¡½âÎöºóµÄ½á¹¹

 Note:

 Author:
******************************************************************************/
static void analysis_list_puts(void)
{
#if 1
    struct at_func_node *pos;
    puts("====== auto_list debug ======\n");

    list_for_each_entry(pos, &__this->root, entry) {

        if (pos->type == TYPE_LOOP) {
            struct at_func_node *p;
            printf("---loop: %d\n", pos->loop);

            list_for_each_entry(p, &pos->child, entry) {
                printf("---%s: %d\n", p->name, (int)p->parm);

            }

            puts("---loop done.\n");
        } else {
            printf("---%s: %d\n", pos->name, (int)pos->parm);
        }

    }

    puts("====== auto_list debug end~! ======\n");
#endif
}



/*****************************************************************************
 Description: ½âÎö½Å±¾Æô¶¯

 Note:

 Author:
******************************************************************************/
static u8 analysis_script(void)
{

    void *file;
    char buf[1024] = {0};
    u32 rlen;

    file = fopen(SCRIPT_FILE_NAME, "r");
    if (!file) {
        return -1;
    }
    rlen = fread(file, buf, 1024);

    analysis_start(buf, rlen);  // ¿ªÊ¼½âÎöÈ«²¿Ö¸Áî

    analysis_list_puts();

    fclose(file);

    return 0;
}

/*****************************************************************************
 Description: ¿ªÊ¼²âÊÔ

 Note:

 Author:
******************************************************************************/
u8 auto_test_open(void)
{

    analysis_script();

    task_create(at_task, NULL, "auto_test");
    return 0;
}

/*****************************************************************************
 Description: ¼ì²âÊÇ·ñÓÐ²âÊÔ½Å±¾

 Note:

 Author:
******************************************************************************/
int auto_test_script_check(void *p)
{

    void *file;

    //puts("\n\n------------auto_test_script_check -----------\n\n");

    //printf("open file: %s\n", SCRIPT_FILE_NAME);
    file = fopen(SCRIPT_FILE_NAME, "r");
    if (!file) {
        //puts("auto_test_script_check fail!!!\n");
        return 0;
    }
    fclose(file);
    //puts("auto_test_script_check OK.\n");

    auto_test_open();

    return 1;   // ·µ»Ø1ÔòÕÒµ½²âÊÔ½Å±¾ÎÄ¼þ
}

/*****************************************************************************
 Description: ³õÊ¼»¯

 Note:

 Author:
******************************************************************************/
int auto_test_init(void)
{
    wait_completion(storage_device_ready, auto_test_script_check, NULL);
    return 0;
}
late_initcall(auto_test_init);


