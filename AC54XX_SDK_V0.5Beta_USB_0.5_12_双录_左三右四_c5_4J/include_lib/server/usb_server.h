/*************************************************************************
	> File Name: usb_server.h
	> Author:
	> Mail:
	> Created Time: Wed 15 Feb 2017 11:17:33 AM HKT
 ************************************************************************/

#ifndef _USB_SERVER_H
#define _USB_SERVER_H

#include "server/server_core.h"
#include "system/includes.h"


#define     USB_REQ_SLAVE_MODE      0x01
#define     USB_REQ_HOST_MODE       0x02


#define     USB_MASS_STORAGE        0x1
#define     USB_CAMERA              0x2
#define     USB_ISD_TOOL            0x4
#define     USB_ISP_TOOL            0x8
#define     USB_SCREEN_TOOL         0x10
#define     USB_MANUAL_CAMERA       0x20

#define     USB_TOOLS               0x1C

#define     USB_VIDEO_FMT_MJPG      0x1
#define     USB_VIDEO_FMT_H264      0x2
enum {
    USB_STATE_SLAVE_CONNECT,
    USB_STATE_SLAVE_DISCONNECT,
    USB_STATE_SLAVE_RESET,
    UVC_STATE_GET_FRMAE_BUF,
    UVC_STATE_PUT_FRMAE_BUF,
    UVC_STATE_GET_FREE_SIZE,
    /*
    USB_STATE_UVC_VIDEO_START,
    USB_STATE_UVC_VIDEO_READ,
    USB_STATE_UVC_VIDEO_STOP,
    */
};

struct uvc_reso {
    u16 width;
    u16 height;
};

struct uvc_reso_info {
    int num;
    struct uvc_reso reso[5];
};

struct mass_storage_fmt {
    int   dev_num;
    const char **dev;
    const char *name;
};

struct msg_connect {
    struct mass_storage_fmt *fmt;
    void *tool;
    int (*response)(void *tool, void *arg, u8 type);
};

struct uvc_private_req {
    u16 cmd;
    u16 data;
};

struct usb_camera {
    const char *name;
    u8  enable;
    u8  format;
    u8  quality;
    u8  fmt_num;
    u32 bits_rate;
    u16 width;
    u16 height;
    u8 *buf;
    u32 buf_size;
    int (*video_open)(int width, int height, int fmt, int fps);
    int (*video_close)(void);
    int (*processing_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*private_cmd_response)(struct uvc_private_req *req);
    struct uvc_reso_info *info;
};


struct uvc_frame {
    u8 *buf;
    int size;
};

/*
struct usb_uvc_video {
    u8 format;
    u16 width;
    u16 height;
    u8 *buf;
    u32 buf_size;
};

struct usb_uvc_frame {
    u8 format;
    u8 *buf;
    u32 len;
};
*/
struct usb_req {
    u8 type;
    u8 state;
    union {
        struct mass_storage_fmt storage;
        struct usb_camera camera[2];
        struct uvc_frame frame;
    };
};

struct usb_event {
    u32 type;
    int (*callback)(void *arg);
    void *priv;
};

#endif
