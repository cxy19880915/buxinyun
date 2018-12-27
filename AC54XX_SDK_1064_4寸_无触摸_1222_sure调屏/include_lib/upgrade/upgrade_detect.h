/*************************************************************************
	> File Name: include_lib/upgrade/upgrade_detect.h
	> Author:
	> Mail:
	> Created Time: Tue 23 May 2017 04:59:02 PM HKT
 ************************************************************************/

#ifndef _UPGRADE_DETECT_H
#define _UPGRADE_DETECT_H

#include "fs/fs.h"

#define UPGRADE_FROM_FILE   0x0
#define UPGRADE_FROM_BUFF   0x1

enum upgrade_message {
    UPGRADE_ERR_NONE = 0x0,
    UPGRADE_START,
    UPGRADE_ERR_DEV,
    UPGRADE_ERR_FILE,
    UPGRADE_ERR_ADDR,
    UPGRADE_ERR_KEY,
    UPGRADE_ERR_MEM,
    UPGRADE_ERR_FAILED,
    UPGRADE_CALC_ECC,
    UPGRADE_SUCCESS,
};

struct upgrade_ui {
    int step_msecs;
    int (*show_progress)(int percent);
    int (*show_message)(int msg);
    const char *path;
};

struct upgrade_req {
    u8   type;
    u8   force; //强制升级
    char *path;
    u8   *buf;
    FILE *file;
    int  buf_size;
    int (*complete)(u8 success);
    struct upgrade_ui ui;
    bool (*confirm)(void);
};

extern int upgrade_file_detect(struct upgrade_req *req);

#endif
