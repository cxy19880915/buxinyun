#include "system/includes.h"
#include "server/video_server.h"
#include "user_isp_cfg.h"
#include "user_scr_tool.h"

struct imc_presca_ctl *video_disp_get_pctl();


/*
 * 调屏效时的前后视启动停止接口
 * */

static int screen_tool_disp_start(struct screen_tool_cfg *p, u16 width, u16 height, u16 xoff, u16 yoff)
{
    static union video_req req = {0};
    static u8 scene = 0;
    int err;

    if (!p->display) {
        p->display = server_open("video_server", "video0");
        req.display.camera_type 	 = VIDEO_CAMERA_NORMAL;
        if (!p->display) {
            if (dev_online("video1")) {
                p->display = server_open("video_server", "video1");
                req.display.camera_type 	 = VIDEO_CAMERA_NORMAL;
            } else if (dev_online("uvc")) {
                p->display = server_open("video_server", "video3");
                req.display.camera_type 	 = VIDEO_CAMERA_UVC;
            } else {
                return -EFAULT;
            }
            if (!p->display) {
                return -EFAULT;
            }
        }
    }

    req.display.fb 		        = "fb1";
    req.display.left  	        = xoff;
    req.display.top 	        = yoff;
    req.display.width 	        = width;
    req.display.height 	        = height;
    req.display.border_left     = 0;
    req.display.border_top      = 0;
    req.display.border_right    = 0;
    req.display.border_bottom   = 0;

    req.display.uvc_id 			= 0;
    req.display.camera_config   = NULL;

    req.display.state 	        = VIDEO_STATE_START;
    req.display.pctl            = video_disp_get_pctl();;

    if (p->isp_scene == (s8) - 1) {
        if (!p->isp_update) {
            if (p->info.update_isp_scenes_begin) {
                p->info.update_isp_scenes_begin(p->display);
            }
            p->isp_update = true;
        }
    }

    return server_request(p->display, VIDEO_REQ_DISPLAY, &req);
}

static void screen_tool_disp_stop(struct screen_tool_cfg *p)
{
    union video_req req;

    if (p->display) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(p->display, VIDEO_REQ_DISPLAY, &req);

        if (p->isp_scene == (s8) - 1) {
            if (p->isp_update) {
                if (p->info.update_isp_scenes_end) {
                    p->info.update_isp_scenes_end();
                }
                p->isp_update = false;
            }
        }
        server_close(p->display);
        p->display = NULL;
    }
}

static int screen_tool_disp_start1(struct screen_tool_cfg *p, u16 width, u16 height, u16 xoff, u16 yoff)
{
    static union video_req req = {0};
    static u8 scene = 0;
    int err;

    if (!p->display1) {
        if (dev_online("uvc")) {
            p->display1 = server_open("video_server", "video3");
        } else if (dev_online("video1")) {
            p->display1 = server_open("video_server", "video1");
        } else {
            return -EFAULT;
        }
        if (!p->display1) {
            puts("open video_server failxxx\n");
            return -EFAULT;
        }
    }
    printf("width =%d\n", width);
    printf("height =%d\n", height);

    req.display.fb 		        = "fb1";
    req.display.left  	        = xoff;
    req.display.top 	        = yoff;
    req.display.width 	        = width;
    req.display.height 	        = height;
    req.display.border_left     = 0;
    req.display.border_top      = 0;
    req.display.border_right    = 0;
    req.display.border_bottom   = 0;

    req.display.uvc_id 		    = 0;
    req.display.camera_config   = NULL;

    if (dev_online("uvc")) {
        req.display.camera_type 	 = VIDEO_CAMERA_UVC;
    } else if (dev_online("video1")) {
        req.display.camera_type 	 = VIDEO_CAMERA_NORMAL;
    } else {
        req.display.camera_type 	 = VIDEO_CAMERA_NORMAL;
    }

    req.display.state 	      = VIDEO_STATE_START;
    req.display.pctl            = video_disp_get_pctl();

    return server_request(p->display1, VIDEO_REQ_DISPLAY, &req);
}

static void screen_tool_disp_stop1(struct screen_tool_cfg *p)
{
    union video_req req;

    if (p->display1) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(p->display1, VIDEO_REQ_DISPLAY, &req);
        server_close(p->display1);
        p->display1 = NULL;
    }
}

REGISTER_SCREEN_TOOL(screen) = {
    .screen_tool_video0_start = screen_tool_disp_start,
    .screen_tool_video0_stop  = screen_tool_disp_stop,
    .screen_tool_video1_start = screen_tool_disp_start1,
    .screen_tool_video1_stop  = screen_tool_disp_stop1,

    .update_isp_scenes_begin  = start_update_isp_scenes,
    .update_isp_scenes_end    = stop_update_isp_scenes,
    .set_isp_scr_cfg          = set_isp_scr_cfg,
};
