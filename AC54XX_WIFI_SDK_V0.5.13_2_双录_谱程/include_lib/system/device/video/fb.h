#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "typedef.h"
#include "ioctl.h"
#include "device/device.h"
#include "system/spinlock.h"

#define 	SCALE_NONE 		0x1111

#define 	FBIOGET_VSCREENINFO 		_IOR('F', 0, sizeof(struct fb_var_screeninfo))
#define 	FBIOSET_VSCREENINFO 		_IOW('F', 0, sizeof(struct fb_var_screeninfo))
#define 	FBIOGETMAP 					_IOR('F', 1, sizeof(struct fb_map_user))
#define 	FBIOPUTMAP 					_IOW('F', 1, sizeof(struct fb_map_user))
#define 	FBIOBLANK 					_IOW('F', 2, sizeof(int))
#define 	FBIOSET_TRANSP 				_IOW('F', 3, sizeof(int))
#define 	FBIOSET_PALLET 				_IOW('F', 4, sizeof(int))
#define 	FBIOSET_FBUFFER_NUM			_IOW('F', 5, sizeof(int))
#define     FBIOGET_FBUFFER_NUM         _IOR('F', 5, sizeof(int))
#define 	FBIOGET_FBUFFER 			_IOW('F', 6, sizeof(int))
#define 	FBIOPUT_FBUFFER 			_IOW('F', 7, sizeof(int))
#define 	FBIOSET_ENABLE 				_IOW('F', 8, sizeof(int))
#define     FBIOSET_BACKCOLOR           _IOW('F', 9, sizeof(int))
#define     FBIOGET_FBUFFER_INUSED      _IOR('F', 10, sizeof(int))
#define     FBIOPUT_FBUFFER_INUSED      _IOW('F', 10, sizeof(int))
#define     FBIOGET_DEVSCREENINFO       _IOR('F', 11, sizeof(struct lcd_screen_info))
#define     FBIOSET_BLOCK               _IOW('F', 12, sizeof(int))
#define     FBIOGET_MERGEDMODE          _IOR('F', 12, sizeof(int))

enum {
    FB_BUFFER_IDLE,
    FB_BUFFER_LOCK,
    FB_BUFFER_PENDING,
    FB_BUFFER_INUSED,
};

enum {
    MODE_SHARED,
};

//注：以下顺序不可更改
enum {
    FB_COLOR_FORMAT_SOLID_COLOR,
    FB_COLOR_FORMAT_YUV420,
    FB_COLOR_FORMAT_YUV422,
    FB_COLOR_FORMAT_RGB888,
    FB_COLOR_FORMAT_RGB565,
    FB_COLOR_FORMAT_16K,
    FB_COLOR_FORMAT_256,
    FB_COLOR_FORMAT_2BIT,           //水平32对齐
    FB_COLOR_FORMAT_1BIT,            //水平64对齐
};

struct fb_buffer {
    struct list_head entry;
    u8 status;
    u8 *baddr;
    u8 *layer_baddr;
};

struct fb_map_user {
    u16 xoffset;
    u16 yoffset;
    u16 width;
    u16 height;
    u8  *baddr;
    u8  *yaddr;
    u8  *uaddr;
    u8  *vaddr;
    u8 transp;
    u8 format;
};

struct fb_var_screeninfo {
    u16 xres;           //图层宽度
    u16 yres;           //图层高度
    u16 buf_xres;       //图层buffer宽度
    u16 buf_yres;       //图层buffer高度
    u16 xres_virtual;   //图层buffer x坐标
    u16 yres_virtual;   //图层buffer y坐标
    u16 xoffset;        //图层x坐标
    u16 yoffset;        //图层y坐标
    u16 scale;
};

struct fb_info;

struct fb_ops {
    int (*fb_open)(struct fb_info *fb, void *arg);
    int (*fb_ioctl)(struct fb_info *fb, int cmd,  unsigned long arg);
    int (*fb_close)(struct fb_info *fb);
};

struct fb_info {
    const char *name;
    u8 format;
    u8 block;
    struct fb_var_screeninfo var;
    const struct fb_ops *fbops;
    struct device device;
    void *private_data;
    void *pd;
    spinlock_t lock;
    struct fb_buffer *pending;
    struct fb_buffer *inused;
    struct fb_buffer fb_buf[2];
    struct list_head head;
};



extern const struct device_operations fb_dev_ops;

void *fb_get_platform_data(const char *fb_name);

int register_framebuffer(struct fb_info *fb);



void __fb_put_map_complete(struct fb_info *fb, u32 baddr);














#endif

