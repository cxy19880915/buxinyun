#ifndef VIDEO_IOCTL_H
#define VIDEO_IOCTL_H



#include "typedef.h"
#include "list.h"
#include "ioctl.h"
#include "lbuf.h"
#include "system/task.h"


struct video_capability {
    char name[16];
    u32 capabilities;
};

/* Values for 'capabilities' field */
#define VIDEO_CAP_VIDEO_CAPTURE		0x00000001  /* Is a video capture device */
#define VIDEO_CAP_VIDEO_OUTPUT		0x00000002  /* Is a video output device */
#define VIDEO_CAP_VIDEO_OVERLAY		0x00000004  /* Can do video overlay */

#define VIDEO_CAP_CAMERA_FPS        "camera_fps"


#define VIDEO_CAP_TUNER			0x00010000  /* has a tuner */
#define VIDEO_CAP_AUDIO			0x00020000  /* has audio support */
#define VIDEO_CAP_RADIO			0x00040000  /* is a radio device */
#define VIDEO_CAP_MODULATOR		0x00080000  /* has a modulator */

#define VIDEO_CAP_READWRITE              0x01000000  /* read/write systemcalls */
#define VIDEO_CAP_ASYNCIO                0x02000000  /* async I/O */
#define VIDEO_CAP_STREAMING              0x04000000  /* streaming I/O ioctls */


#define VIDEO_FPS(a) 	((a)*100)



#define VIDEO_REC_SET_FPS	4
#define VIDEO_REC_SET_FILL_FRAME	5
#define VIDEO_REC_SET_CAMERA_FPS    6


#define VIDEO_PIX_FMT_RAW  			0x00000001
#define VIDEO_PIX_FMT_JPEG 			0x00000002
#define VIDEO_PIX_FMT_H264 			0x00000004

#define VIDEO_PIX_FMT_YUV420 		0x00000100
#define VIDEO_PIX_FMT_YUV 			0x0000ff00



#define VIDIOC_QUERYCAP 		_IOR('V', 0, sizeof(struct video_capability))
#define VIDIOC_GET_FMT 			_IOR('V', 1, sizeof(struct video_pix_format))
#define VIDIOC_SET_FMT 			_IOW('V', 1, sizeof(struct video_pix_format))
#define VIDIOC_OVERLAY 			_IOW('V', 2, sizeof(int))
#define VIDIOC_STREAM_ON 		_IOW('V', 3, sizeof(enum video_buf_type))
#define VIDIOC_STREAM_OFF 		_IOW('V', 4, sizeof(enum video_buf_type))
#define VIDIOC_REQBUFS 			_IOW('V', 5, sizeof(struct video_reqbufs))
#define VIDIOC_QUERYBUF 		_IOR('V', 6, sizeof(struct videobuf_buffer))
#define VIDIOC_QBUF 			_IOR('V', 7, sizeof(struct videobuf_buffer))
#define VIDIOC_DQBUF 			_IOW('V', 8, sizeof(struct videobuf_buffer))
#define VIDIOC_GET_AUDIO 		_IOR('V', 9, sizeof(struct video_audio))
#define VIDIOC_SET_AUDIO 	 	_IOW('V', 9, sizeof(struct video_audio))
#define VIDIOC_IMAGE_CAPTURE 	_IOR('V', 10, sizeof(struct video_image_capture))
#define VIDIOC_DEL_IMAGE 		_IOW('V', 11, sizeof(struct video_image_capture))
#define VIDIOC_PLAY   			_IOW('V', 12, sizeof(int))

enum video_buf_type {
    VIDEO_BUF_TYPE_VIDEO_CAPTURE,
    VIDEO_BUF_TYPE_IMAGE_CAPTURE,
    VIDEO_BUF_TYPE_VIDEO_OVERLAY,
    VIDEO_BUF_TYPE_VIDEO_PLAY,
};

struct video_pix_format {
    u32 fps;
    u32 width;
    u32 height;
    u32 pixelformat;
};

struct video_window {
    u32 left;
    u32 top;
    u32 width;
    u32 height;
    u16 border_left;
    u16 border_top;
    u16 border_right;
    u16 border_bottom;
    u16 ratio;
};

struct video_play {
    u32 width;
    u32 height;
    u32 pixelformat;
    struct video_window win;
};

struct video_format {
    enum video_buf_type type;
    u8 camera_type;
    u8 uvc_id;
    union {
        struct video_pix_format pix;
        struct video_window win;
        struct video_play play;
    } fmt;
    void *private_data;
};

struct jpg_thumbnail {
    u8 enable;
    u8 quality;
    u16 width;
    u16 height;
    u8 *buf;
    int len;
};

struct video_image_capture {
    u16 width;
    u16 height;
    u8  *baddr;
    u32 size;
    struct jpg_thumbnail *thumbnail;
};

struct video_reqbufs {
    void *buf;
    int size;
};

struct videobuf_buffer {
    u32 len;
    u32 magic;
    u8 data[0];
};

struct videobuf_queue {
    u8 ref;
    u8 align;
    u8 streaming;
    OS_SEM sem;
    void *buf;
    struct lbuff_head *lbuf;
    void *private_data;
};

struct video_buffer {
    u8  index;
    u8 	noblock;
    u16 timeout;
    u32 len;
    u32 baddr;
    void *priv;
};

//enum videobuf_state {
//VIDEOBUF_NEEDS_INIT = 0,
//VIDEOBUF_PREPARED   = 1,
//VIDEOBUF_QUEUED     = 2,
//VIDEOBUF_ACTIVE     = 3,
//VIDEOBUF_DONE       = 4,
//VIDEOBUF_ERROR      = 5,
//VIDEOBUF_IDLE       = 6,
//};

//struct videobuf_queue;



//#define VIDEO_MAX_FRAME   16

//struct videobuf_buffer {
//struct list_head stream;
//OS_SEM done;
//u32 baddr;
//u8 state;
//u8 index;
//void *priv_data;
//};

//struct videobuf_queue {
//struct list_head stream;
//struct videobuf_buffer *bufs[VIDEO_MAX_FRAME];
//struct videobuf_buffer *curr;
//struct videobuf_queue_ops *ops;
//OS_SEM sem;
//int msize;
//void *pirv_data;
//u8 streaming;
//};


struct video_audio {
    u32 capabilities;
};

//struct videobuf_queue_ops {
//int (*buf_setup)(struct videobuf_queue *q, int *count);
//int (*buf_prepare)(struct videobuf_queue *q, struct videobuf_buffer *);
//};




struct video_ioctl_ops {
    int (*vidioc_querycap)(void *fh, struct video_capability *cap);

    int (*vidioc_get_fmt_vid_cap)(void *fh, struct video_format *f);
    int (*vidioc_get_fmt_vid_out)(void *fh, struct video_format *f);
    int (*vidioc_get_fmt_vid_overlay)(void *fh, struct video_format *f);


    int (*vidioc_set_fmt_vid_cap)(void *fh, struct video_format *f);
    int (*vidioc_set_fmt_vid_out)(void *fh, struct video_format *f);
    int (*vidioc_set_fmt_vid_overlay)(void *fh, struct video_format *f);

    int (*vidioc_try_fmt_vid_cap)(void *fh, struct video_format *f);
    int (*vidioc_try_fmt_vid_out)(void *fh, struct video_format *f);
    int (*vidioc_try_fmt_vid_overlay)(void *fh, struct video_format *f);

    int (*vidioc_reqbufs)(void *fh, struct video_reqbufs *b);
    int (*vidioc_querybuf)(void *fh, struct video_buffer *b);
    int (*vidioc_qbuf)(void *fh, struct video_buffer *b);
    int (*vidioc_dqbuf)(void *fh, struct video_buffer *b);

    int (*vidioc_overlay)(void *fh, unsigned int i);

    int (*vidioc_streamon)(void *fh, int i);
    int (*vidioc_streamoff)(void *fh, int i);


    int (*vidioc_get_audio)(void *fh, struct video_audio *a);
    int (*vidioc_set_audio)(void *fh, struct video_audio *a);
};




struct video_device {
    const char *name;
    const struct video_ioctl_ops *ioctl;
    const struct video_file_operations *fops;
    void *priv;
};




struct video_file_operations {
    int (*open)(struct video_device *device);
    int (*write)(struct video_device *device, void *buf, u32 len);
    int (*close)(struct video_device *device);
};



struct video_dec_format {
    int pixelformat;
};













































#endif

