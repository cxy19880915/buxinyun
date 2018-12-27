/*************************************************************************
	> File Name: user_isp_cfg.h
	> Author:
	> Mail:
	> Created Time: Thu 27 Apr 2017 11:49:02 AM HKT
 ************************************************************************/

#ifndef _USER_ISP_CFG_H
#define _USER_ISP_CFG_H

#include "server/server_core.h"

enum specail_type {
    SE_NORMAL = 0,
    SE_BNW,
    SE_RETRO,
    SE_BINARY,
};

struct isp_scr_work {
    char parm;
    int (*handler)(char parm);
};

#define REGISTER_ISP_SCR_WORK(hdl,p)  \
static struct isp_scr_work isp_scr_##hdl \
sec(.isp_scr_work) = {\
	.parm = (unsigned char)p,\
	.handler = hdl,\
}

struct usr_isp_cfg_par {
    u32 levl;
    u32 levh;
};

extern int start_update_isp_scenes(struct server *server);
extern int stop_update_isp_scenes(void);
extern int set_isp_special_effect(struct server *server, u8 type);
extern int load_default_camera_config(u32 lv, void *arg);
extern int load_spec_camera_config(u32 index, void *arg);
extern void set_isp_scr_cfg(u32 parm);
extern u8 get_usb_in_status();
extern int isp_scr_work_hdl(u8 scene);

#endif
