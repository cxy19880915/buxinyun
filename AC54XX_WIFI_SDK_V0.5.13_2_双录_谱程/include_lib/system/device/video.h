#ifndef DEVICE_VIDEO_H
#define DEVICE_VIDEO_H

#include "video/video_ioctl.h"


#define VIDEO_TAG(a,b,c,d)  (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))


enum {
    VIDREQ_GET_OVERLAY_BUFFER,
    VIDREQ_PUT_OVERLAY_BUFFER,
    VIDREQ_FREE_OVERLAY_BUFFER,
    VIDREQ_GET_CAPTURE_BUFFER,
    VIDREQ_PUT_CAPTURE_BUFFER,
    VIDREQ_FREE_CAPTURE_BUFFER,
    VIDREQ_GET_IMAGE_CAPTURE_BUFFER,
    VIDREQ_PUT_IMAGE_CAPTURE_BUFFER,
    VIDREQ_IMAGE_CAPTURE_COMPLETED,
    VIDREQ_IMAGE_ZOOM,
    VIDREQ_IMAGE_DISP_ZOOM,
    VIDREQ_IMAGE_ENC_START,
    VIDREQ_GET_DISP_CAPABILITY,
    VIDREQ_GET_ENCODER_CHANNEL,
    VIDREQ_PUT_IMAGE_TO_BUFFER,
    VIDREQ_RESET_DISP_IMAGE,
    VIDREQ_RESET_CBUFFER,
    VIDREQ_PUT_DEC_DATA,
    VIDREQ_GET_DEC_DATA,
    VIDREQ_RESET_DOWN_CBUFFER,
    VIDREQ_RESET_UP_CBUFFER,
    VIDREQ_RESET_ENCODER,
    VIDREQ_REKSTART_ENCODER,
    VIDREQ_WAIT_ENC_END_STOP_IMC,
    VIDREQ_GET_FNUM,
    VIDREQ_PAUSE_ENC,
    VIDREQ_CONTINUE_ENC,
    VIDREQ_PAUSE_ENC_CHANNEL,
    VIDREQ_CONTINUE_ENC_CHANNEL,
    VIDREQ_GET_TIME_LABEL,
    VIDREQ_SET_SOFT_IMAGE_LABEL,
    VIDREQ_CAMERA_OUT,
    VIDREQ_RESET_UVC_ENC,
    VIDREQ_ENCODE_ONE_IMAGE,
    VIDREQ_SET_JPG_THUMBNAILS,
    VIDREQ_ASYNC_PUT_DEC_DATA,
    VIDREQ_BUFF_MOVE,
    VIDREQ_MANUAL_ENC_STREAM,
    VIDREQ_KICK_UP_ONE_FRMAE,
    VIDREQ_GET_CORRECT_SIZE,
    VIDREQ_GET_NEXT_INPUT_BUF,
    VIDREQ_DEC_FRAME_FINISH,

};

enum video_rec_quality {
    VIDEO_LOW_Q,
    VIDEO_MID_Q,
    VIDEO_HIG_Q,
};

enum video_camera_type {
    VIDEO_CAMERA_NORMAL = 0,
    VIDEO_CAMERA_UVC,
    VIDEO_CAMERA_VIRTUAL,
    VIDEO_CAMERA_RF
};

struct video_frame_yuv {
    u16 width;
    u16 height;
    u8 *y;
    u8 *u;
    u8 *v;
};

struct video_yuv_buffer {
    u8 *y;
    u8 *u;
    u8 *v;
};

struct image_zoom_data {
    u8  zstate;
    u32 src_reg_data;
    u16 in_width;
    u16 in_height;
    u16 out_width;
    u16 out_height;
    u8 *ibuf;
    u8 *obuf;
};

struct video_image_enc {
    u8  mkstart;
    u16 img_width;
    u16 img_height;
    u16 blk_width;
    u16 blk_height;
    struct video_yuv_buffer *blk_buf;
};

struct video_image_dec {
    u8  fkstart;
    u16 width;
    u16 height;
    u8  *buf;
    u32 size;
    struct video_yuv_buffer *blk_buf;
};

struct video_decode_frame {
    u8 type;
    unsigned int reset : 1;
    unsigned int mode : 7; //0 -- display, 1 -- encode
    u16 width;
    int line;
    u8 *buf;
    int cnt;
};

struct video_encode_frame {
    u8 type;
    u8 reset;
    u8 *buf;
    int cnt;
    int lines;
};

struct video_cap_buffer {
    u8 num;
    u32 size;
    u8 *buf;
    u8 ch_num;
};


struct video_subdevice;
struct video_var_param_info;

struct video_subdevice_data {
    int tag;
    void *data;
};

struct video_platform_data {
    int num;
    const struct video_subdevice_data *data;
};



#define VIDEO_REC_NUM       2       //double rec or single rec

#define VIDEO_PLATFORM_DATA_BEGIN(vdata) \
	static const struct video_platform_data vdata = { \


#define VIDEO_PLATFORM_DATA_END() \
	};



struct image_capability {
    u8  zoom;
    u16 width;
    u16 height;
    struct video_image_capture *icap;
};

struct video_subdevice_ops {
    int (*init)(const char *name, struct video_platform_data *);

    bool (*online)(const char *name);

    int (*get_fmt)(struct video_format *f);

    int (*set_fmt)(struct video_format *f);

    struct video_endpoint *(*open)(struct video_var_param_info *);

    int (*overlay)(struct video_endpoint *, int i);

    int (*streamon)(struct video_endpoint *);

    int (*streamoff)(struct video_endpoint *);

    int (*get_image_capability)(struct video_endpoint *, struct image_capability *);
    int (*image_capture)(struct video_endpoint *, struct image_capability *);

    int (*response)(struct video_endpoint *, int cmd, void *);

    int (*write)(struct video_endpoint *, void *buf, u32 len);

    int (*close)(struct video_endpoint *);

    int (*querycap)(struct video_endpoint *, struct video_capability *cap);

    int (*ioctrl)(struct video_endpoint *, u32 cmd, void *arg);
};


#define MANUAL_CHANNEL  3
struct video_var_param_info {
    u32 fps;
    u16 width;
    u16 height;

    int source;
    u8 channel;
    struct video_format *f;
};

struct video_subdevice {
    u8 subid;
    const char *name;
//    u32 input_pixelformat;
    u32 output_pixelformat;
    const struct video_subdevice_ops *ops;
    int (*request)(struct video_endpoint *, int cmd, void *);
};


struct video_endpoint {
    struct list_head entry;
    struct video_subdevice *dev;
    int inused;
    void *parent;
    void *private_data;
    void *imc_data;
};

struct video_crop_ctrl {
    u16 crop_sx;
    u16 crop_ex;
    u16 crop_sy;
    u16 crop_ey;
};

struct video_crop_sca {
    u16 src_w;
    u16 src_h;
    u16 crop_w;
    u16 crop_h;
};

struct video_crop_tri {
    int (*do_crop)(void *priv, void *parm);
    void *priv;
    void *parm;
};


extern struct video_subdevice video_subdev_begin[];
extern struct video_subdevice video_subdev_end[];

extern const struct device_operations video_dev_ops;

#define REGISTER_VIDEO_SUBDEVICE(dev, id) \
	static struct video_subdevice dev sec(.video_subdev.##id) = { \
		.subid = id, \
        .name = #dev, \



int video_subdevice_register(struct video_subdevice *dev);


u32 video_buf_free_space(struct video_endpoint *ep);

void *video_buf_malloc(struct video_endpoint *, u32 size);

void *video_buf_realloc(struct video_endpoint *, void *buf, int size);

void video_buf_free(struct video_endpoint *, void *buf);

void *video_buf_ptr(void *buf);

u32 video_buf_size(void *buf);

void video_buf_stream_finish(struct video_endpoint *, void *buf);

int video_buf_query(struct video_endpoint *ep);

int video_subdev_request(struct video_endpoint *ep, int req, void *arg);










#endif

