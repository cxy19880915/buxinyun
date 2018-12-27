#ifndef DEVICE_CAMERA_H
#define DEVICE_CAMERA_H




#include "device/device.h"
#include "asm/isp_dev.h"
#include "device/video.h"


#define VIDEO_TAG_CAMERA        VIDEO_TAG('c', 'a', 'm', 'e')
#define VIDEO_TAG_UVC           VIDEO_TAG('u', 'v', 'c', ' ')


#define CAMERA_DEVICE_NUM  2

#define CSI2_X0_LANE    0
#define CSI2_X1_LANE    1
#define CSI2_X2_LANE    2
#define CSI2_X3_LANE    3
#define CSI2_X4_LANE    4

struct camera_platform_data {
    u8 xclk_gpio;
    u8 reset_gpio;
    u8 pwdn_gpio;
    u8 power_value;
    u32 interface;
    bool (*online_detect)();
    union {
        struct {
            u32 pclk_gpio;
            u32 hsync_gpio;
            u32 vsync_gpio;
            u32 io_function_sel;
            u32 data_gpio[10];
        } dvp;
        struct {
            u8 data_lane_num;
            u8 clk_inv;
            u8 d0_rmap;
            u8 d0_inv;
            u8 d1_rmap;
            u8 d1_inv;
            u8 d2_rmap;
            u8 d2_inv;
            u8 d3_rmap;
            u8 d3_inv;
            u8 tval_hstt;
            u8 tval_stto;
        } csi2;
    };
};



#define CAMERA_PLATFORM_DATA_BEGIN(data) \
	static const struct camera_platform_data data = { \


#define CAMERA_PLATFORM_DATA_END() \
	};

#define CAMERA_CMD_BASE		0x00400000
#define CAMERA_GET_ISP_SRC_SIZE		(CAMERA_CMD_BASE + 1)
#define CAMERA_GET_ISP_SIZE		(CAMERA_CMD_BASE + 2)
#define CAMERA_SET_CROP_SIZE		(CAMERA_CMD_BASE + 3)
#define CAMERA_CROP_TRIG		(CAMERA_CMD_BASE + 4)
#define CAMERA_NEED_REMOUNT		(CAMERA_CMD_BASE + 5)

extern const struct device_operations camera_dev_ops;

//dvp sensor io sel
#define DVP_SENSOR0(sel)   (((sel?1:0) << 24) | (22))//sel 0 PC3~PC13  1 PA6~PA15
#define DVP_SENSOR1(sel)   (((sel?1:0) << 24) | (23))//sel 0 PH1~PH11  1 PD0~PD10




#endif

