
#include "include/menu_parm_api.h"


/*
 * 寻找字符串集合类似"res:720p,mic:on,par:off"中的以逗号分隔
 * 开的单个项目类似"res:720p",id从0开始，返回的是该字符串的
 * 起始地址。
 */
const char *menu_str_item_checkout(int id, const char *data)
{
    int cnt = 0;

    ASSERT(data != NULL);
    if (*data == ',') {
        data++;
    }
    if (id == 0) {
        /*
         * 第一个项目
         */
        return data;
    }

    for (cnt = 0; cnt < id; cnt++) {
        data = strchr(data, ',');
        if (data) {
            while (*data == ',') {
                data++;
                /*
                 * 指向逗号后面
                 */
            }
        } else {
            /*
             * 找不到逗号了
             */
            return NULL;
        }
    }
    if (*data == '\0') {
        return NULL;
    } else {
        return data;
    }
}

/*
 * 在buf字符串结尾添加逗号和str，和'\0'。
 * 例如“res:720p,mic:on”添加"par:off"，生成
 * "res:720p,mic:on,par:off\0"
 */
int menu_str_generate_at_tail(const char *str, char *buf)
{
    int i = 0;
    char *ptr;
    ASSERT(str != NULL);
    ASSERT(buf != NULL);

    ptr = buf;
    while (*ptr != '\0') {
        /*
         * 找到字符串尾部\0
         */
        ptr++;
    }
    if (ptr != buf) {
        /*
         * 如果开头就为\0则不添加逗号
         */
        while (*--ptr == ',');
        /*
         * 去掉结尾多余逗号
         */
        ptr++;
        *ptr = ',';
        /*
         * 将\0改为逗号
         */
        ptr++;
    }
    i = sprintf(ptr, "%s", str);
    ptr += i;
    *ptr = '\0';

    return i;
    /*
     * 返回写入了的长度
     */
}



/*
 * 通过字符串匹配表格，返回匹配项下标号。
 * 表格样式例子：
 * static const char* setting[] = {
 * 	"set:hi",
 * 	"set:med",
 * 	"set:low",
 * 	"\0"   //必须以此'\0'作为结尾成员
 * 	};
 */
int menu_table_search_by_str(const char *str, const char **table)
{
    int ret = -1;
    int i;

    ASSERT(str != NULL);
    ASSERT(table != NULL);

    for (i = 0; (*table[i]) != '\0'; i++) {
        if (strstr(str, table[i]) == str) {
            /*
             * 检测子字符串
             */
            ret = i;
            return ret;
        }
    }
    /* printf("table_search_by_str fail!!! str is \"%s\"\r\n", str); */
    return -1;
}


/*
 * 通过索引号返回匹配项的字符串。
 */
const char *menu_str_search_by_index(int index, const char **table)
{
    int i = 0;
    ASSERT(table != NULL);
    while (*table[i] != '\0') {
        i++;
    }
    if (index >= i) {
        printf("\n\n\n--- %s ---index err!,index = %d ---\n\n\n", table[0], index);
        index = 0;
    }
    /* ASSERT(index < i); */
    /* printf("--- %s ---\r\n", table[index]); */
    return table[index];
}




int menu_sta_ui_get_deal(const char *str, struct menu_sta_ui_table_type *menu_table, int table_size)
{

    int i, j;
    const char *str_a;
    int ret;

    for (i = 0; ; i++) {
        /*
         * 总字符串逐个项目轮询
         */
        str_a = menu_str_item_checkout(i, str);
        if (str_a == NULL) {
            /*puts("==== menu get str done.\n");*/
            break;
        }
        for (j = 0; j < table_size; j++) {
            /*
             * 查找所有配置表项
             */
            if (!memcmp(str_a, menu_table[j].mark, 4)) {
                /*
                 * 匹配表项
                 */
                ret = menu_table_search_by_str(str_a, menu_table[j].table);
                if (ret != (int) - 1) {
                    *(menu_table[j].sta_set) = ret;
                    /*
                     * 更新变量状态
                     */
                } else {
                    /*
                     * 找不到对应配置
                     */
                    ASSERT(0, "==== menu get str error! \"%s\", subject:%s\n", str_a, menu_table[j].mark);
                }
                break;
            }
        }

        ASSERT(j < table_size, "menu_sta_get_deal,the sta can't get:%s", str_a);
        /*
         * 如果找不到表，提示错误
         */
    }
    return 0;
}




int app_set_config_deal(const char *str, struct app_cfg_table_type *cfg_table, int table_size)
{
    u8 i;
    int ret;
    int item = 0;
    const char *str_a;
    while (1) {
        str_a = menu_str_item_checkout(item, str);
        /*
         * 从字符串检出每一项
         */
        if (str_a != NULL) {
            item++;
            /*
             * 下一项
             */
        } else {
            /*
             * 没有下一项了，退出
             */
            break;
        }
        for (i = 0; i < table_size ; i++) {
            /*
             * 寻表，找出对应项
             */
            if (!memcmp(str_a, cfg_table[i].mark, 4)) {
                if (cfg_table[i].table) {
                    ret = menu_table_search_by_str(str_a, cfg_table[i].table);
                    if (ret != (int) - 1) {
                        if (cfg_table[i].parm_set) {
                            *(cfg_table[i].parm_set) = ret;
                        }
                        if (cfg_table[i].setfunc) {
                            cfg_table[i].setfunc(cfg_table[i].table[ret], cfg_table[i].setparm);
                        }
                        /*printf("set cfg sucess: \"%s\"\n", cfg_table[i].table[ret]);*/
                        break;
                        /*
                         * 此项配置完成，退出寻表
                         */
                    } else {
                        /*
                         * 找不到配置项
                         */
                        ASSERT(0, "app_set_config_deal err1: %s", str_a);
                    }
                } else {
                    if (cfg_table[i].setfunc) {
                        cfg_table[i].setfunc(&(str_a[4]), cfg_table[i].setparm);
                    }
                    break;
                    /*
                     * 此项配置完成，退出寻表
                     */
                }
            }
        }
        ASSERT(i < table_size, "app_set_config_deal err2: %s", str_a);
        /*
         * 找不到对应表
         */
    }

    return 0;
}

int app_get_config_deal(const char *str, struct app_cfg_table_type *cfg_table, int table_size, u8 *buf, int buf_size)
{
    u8 i;
    int item = 0;
    const char *str_a;
    const char *str_temp;
    while (1) {
        str_a = menu_str_item_checkout(item, str);
        if (str_a != NULL) {
            item++;
        } else {
            break;
        }
        ASSERT(strlen(buf) + 10 <= buf_size, "The data_buf is not enough!!!");
        for (i = 0; i < table_size ; i++) {
            /*
             * 寻表
             */
            if (!memcmp(str_a, cfg_table[i].mark, 4)) {
                if (cfg_table[i].getfunc) {
                    *(cfg_table[i].parm_set) = cfg_table[i].getfunc(cfg_table[i].getparm);
                }
                str_temp = menu_str_search_by_index(*(cfg_table[i].parm_set), cfg_table[i].table);
                menu_str_generate_at_tail(str_temp, buf);
                /* printf("get cfg sucess: \"%s\"\n", buf); */

                break;
                /*
                 * 配置好当前项，退出查表
                 */
            }
        }
        ASSERT(i < table_size, "rec_get_config_deal err: %s", str_a);
        /*
         * 找不到对应表
         */
    }

    return 0;
}
