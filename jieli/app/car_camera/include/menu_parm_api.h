#ifndef _MENU_PARM_API__
#define _MENU_PARM_API__

#if 0
#include "system/includes.h"

struct menu_sta_ui_table_type {
    const char *mark;
    const char **table;
    char *sta_set;
};



struct app_cfg_table_type {
    const char *mark; 	/* 配置字符串 */
    const char **table; /* 配置选项表 */
    char *parm_set; 	/* 配置对应的保存变量 */
    int (*setfunc)(const char *item_str, void *parm); /* 配置设置的APP回调函数 */
    void *setparm;
    int (*getfunc)(void *getparm); /* 配置设置的APP回调函数 */
    void *getparm;
};


const char *menu_str_item_checkout(int id, const char *data);
int menu_str_generate_at_tail(const char *str, char *buf);
int menu_table_search_by_str(const char *str, const char **table);
const char *menu_str_search_by_index(int index, const char **table);



int menu_sta_ui_get_deal(const char *str, struct menu_sta_ui_table_type *menu_table, int table_size);

int app_set_config_deal(const char *str, struct app_cfg_table_type *cfg_table, int table_size);
int app_get_config_deal(const char *str, struct app_cfg_table_type *cfg_table, int table_size, u8 *buf, int buf_size);


extern void sys_exit_menu_post(void);

extern void sys_fun_restore();
extern void sys_fun_defualt_set();
#endif

#endif

