/*************************************************************************
	> File Name: video_engine.h
	> Author:
	> Mail:
	> Created Time: Tue 17 Jan 2017 12:03:57 PM HKT
 ************************************************************************/

#ifndef _VIDEO_ENGINE_H
#define _VIDEO_ENGINE_H
#include "typedef.h"
#include "device/device.h"

#define    JIVE_OP_ATTR_APPEND  (1<<31)
#define    JIVE_OP_ATTR_BATCH   (1<<30)
#define    JIVE_OP_ATTR_TAIL    (1<<29)
#define    JIVE_OP_ATTR_MASK    (0xf<<28)

#define    JIVE_MD_OBJS_MAX       512

typedef enum {
    JIVE_OP_MD,
    JIVE_OP_COPY,
    JIVE_OP_FILTER,
    JIVE_OP_CSC,
    JIVE_OP_CANNY,
    JIVE_OP_ERODE,
    JIVE_OP_DILATE,
    JIVE_OP_BINARY,
    JIVE_OP_AND,
    JIVE_OP_OR,
    JIVE_OP_SUB_ABS,
    JIVE_OP_SUB_SHIFT,
    JIVE_OP_INTEGRAL_2D,
    JIVE_OP_INTEGRAL_2D_SQR,
    JIVE_OP_INTEGRAL_1D,
    JIVE_OP_HISTOGRAM,
    JIVE_OP_GAMMA,
    JIVE_OP_RESIZE,
    JIVE_OP_MOMENT,
    JIVE_OP_JIVE_LOCK,
    JIVE_OP_JIVE_UNLOCK,
    JIVE_OP_JLMD_LOCK,
    JIVE_OP_JLMD_UNLOCK,

    JIVE_OP_MAX,

} jive_operate_t;

typedef enum {
    JIVE_FORMAT_NULL,
    JIVE_FORMAT_GRAY,
    JIVE_FORMAT_RGB,
    JIVE_FORMAT_RGB_P, //planar
    JIVE_FORMAT_YUV420P, //planar
    JIVE_FORMAT_YUV422P, //planar
    JIVE_FORMAT_YUV444P, //planar
    JIVE_FORMAT_HSV,
    JIVE_FORMAT_HSV_P, //planar
    JIVE_FORMAT_DATA_S8,
    JIVE_FORMAT_DATA_U16,
    JIVE_FORMAT_DATA_S16,
    JIVE_FORMAT_DATA_U32,
    JIVE_FORMAT_DATA_S32,
} jive_format_t;

typedef enum {
    JIVE_INTERGRAL_HORIZONTAL,
    JIVE_INTERGRAL_VERTICAL,
} jive_integral_mode_t;

typedef enum {
    JIVE_BINARY_NORMAL,
    JIVE_BINARY_TRUNC_HIGH,
    JIVE_BINARY_TRUNC_LOW,
} jive_binary_mode_t;

typedef enum {
    JIVE_MD_MORPH_KERNEL_RECT,
    JIVE_MD_MORPH_KERNEL_CROSS,
} jive_md_morph_kernel_t;

typedef enum {
    JIVE_ERR_NONE = 0,
    JIVE_ERR_TIMEOUT = -1,
    JIVE_ERR_PARAM = -2,
    JIVE_ERR_MD_FULL = -3,
} jive_errno_t;


typedef struct image {
    u16 w;
    u16 h;
    u16 stride;
    jive_format_t format;

    union {
        u8 *pixels;
        u8 *data;
        s8 *data_s8;
        u16 *data_u16;
        s16 *data_s16;
        u32 *data_u32;
    };

} jive_image_t;

typedef struct  {
    jive_image_t *src;
    jive_image_t *dst;
} jive_param_generic_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *dst;
    s8 coef[9];
    u8 right_shift;
} jive_param_filter_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *mag;
    jive_image_t *angle;
} jive_param_canny_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *dst;
    u8 threshold;
    jive_binary_mode_t binary_mode;
} jive_param_binary_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *dst;
    s8 coef[9];
} jive_param_morph_t;

typedef struct {
    jive_image_t *src;
    u32 *data_u32;
    jive_integral_mode_t intergral_direction;
} jive_param_integral_1d_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *dst;
    u8 min;
    u8 max;
    u8 offset;
    u8 right_shift;
    u32 count;      //output
} jive_param_integral_2d_sqr_t;

typedef struct {
    jive_image_t *src;
    u32 *data_u32;
} jive_param_hist_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *dst;
    u32 *gamma;
} jive_param_gamma_t;

typedef struct {
    jive_image_t *src;
    u8  right_shift;
    u32 *data_u32;
} jive_param_moment_t;


typedef struct {
    u32 count;
    u8 y2;
    u8 y1;
    u8 x2;
    u8 x1;
} jive_md_obj_info_t;

typedef struct {
    jive_image_t *src;
    jive_image_t *bg;
    jive_image_t *ref;
    jive_md_obj_info_t *objs;

    u16 weight;
    u16 th;
    u8  open_times;
    u8  close_times;
    jive_md_morph_kernel_t  dilate_kernel;
    jive_md_morph_kernel_t  erode_kernel;
    u16 obj_num;    //output
    u32 fg_total;   //output
    u32 frame_num;  //output
    s32 errno;      //output

} jive_md_context_t;


struct engine_ops {
    int (*init)(void);
    void *(*open)(void *param);
    int (*ioctl)(void *handle, u32 cmd, void *param);
    int (*close)(void *handle);
};

struct engine_device {
    const char *name;
    const struct engine_ops *ops;
    void *priv;
};

extern const struct device_operations video_engine_ops;



extern struct engine_device video_engine_jive_dev;
extern struct engine_device video_engine_jlmd_dev;
#endif
