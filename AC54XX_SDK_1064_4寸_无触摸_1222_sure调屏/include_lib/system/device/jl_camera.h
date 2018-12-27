/*****************************************************************
>file name : jl_camera.h
>author : lichao
>create time : Mon 31 Jul 2017 09:19:12 AM HKT
*****************************************************************/
#ifndef _JL_CAMERA_H
#define _JL_CAMERA_H

#include "device/usb.h"
#include "video/video_ioctl.h"
#include "server/jl_camera_protocol.h"

#define JLCIOC_WAIT_RESPONSE        _IOR('J', 0, sizeof(struct jlc_control_packet))
#define JLCIOC_RESET                _IOW('J', 1, sizeof(int))
#define JLCIOC_SET_REC_INFO         _IOW('J', 2, sizeof(unsigned int))


extern const struct device_operations jlc_dev_ops;
#endif
