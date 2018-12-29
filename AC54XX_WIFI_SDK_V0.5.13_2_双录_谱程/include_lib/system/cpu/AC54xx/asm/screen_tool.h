#ifndef __SCREEN_TOOL_H__
#define __SCREEN_TOOL_H__

#include "system/includes.h"
#include "server/server_core.h"

struct screen_tool_cfg;

struct screen_tool_info {
    int (*screen_tool_video0_start)(struct screen_tool_cfg *p, u16 width, u16 height, u16 xoff, u16 yoff);
    int (*screen_tool_video1_start)(struct screen_tool_cfg *p, u16 width, u16 height, u16 xoff, u16 yoff);
    void (*screen_tool_video0_stop)(struct screen_tool_cfg *p);
    void (*screen_tool_video1_stop)(struct screen_tool_cfg *p);

    int (*update_isp_scenes_begin)(struct server *server);
    int (*update_isp_scenes_end)(void);
    void (*set_isp_scr_cfg)(u32 arg);
};

struct screen_tool_cfg {
    struct server *display;
    struct server *display1;
    s8 isp_scene;
    u8 isp_update;
    struct screen_tool_info info;
};

#endif
