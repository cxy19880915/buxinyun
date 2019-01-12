/*****************************************************************
>file name : usb_cam_dev.h
>author : lichao
>create time : Mon 28 Aug 2017 10:49:01 AM HKT
*****************************************************************/

#ifndef _USB_CAM_DEV_H
#define _USB_CAM_DEV_H
#include "typedef.h"
#include "ioctl.h"

#define USB_CAM_TYPE_JPEG		0x01
#define USB_CAM_TYPE_H264		0x02

struct usb_cam_capability {
    u8 type;
    void *data;
    u32 len;
    int timeout;
};

#define UCM_STATE_FRAME_START       0x0
#define UCM_STATE_FRAME_TRANS       0x80
#define UCM_STATE_FRAME_END         0xff

#define UCMIOC_GET_FRAME_STATE      _IOR('U', 0, sizeof(int))

#endif
