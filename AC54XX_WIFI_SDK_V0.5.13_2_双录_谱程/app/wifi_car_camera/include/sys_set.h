#ifndef __SYS_SET_H_
#define __SYS_SET_H_

#include "system/includes.h"
#include "server/video_server.h"

enum emeun_id
{
    MEUN_REC = 0,
    MEUN_CYC,
    MEUN_MOT,
    MEUN_MIC,
    MEUN_DATA,
    MENU_NULL = 0xff,
};

typedef struct _sys_set
{
    u8 recode_start;
    u8 play_start;
    u32 repeat;
    u32 jiffies;
    OS_SEM sem;
    FILE *file;
	u8 state;
	u8 meun_id;
    struct list_head head;
    struct server *ui;
}sys_set_t;


#endif

