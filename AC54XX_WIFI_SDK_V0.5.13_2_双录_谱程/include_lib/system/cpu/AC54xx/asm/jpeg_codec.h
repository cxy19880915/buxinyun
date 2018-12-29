/*****************************************************************
>file name : lib/system/cpu/dv16/video-dev/jpeg/jpeg_codec.h
>author : lichao
>create time : Thu 08 Jun 2017 04:13:07 PM HKT
*****************************************************************/

#ifndef _JPEG_CODEC_H_
#define _JPEG_CODEC_H_
#include "typedef.h"
#include "jpeg_iocmds.h"
#include "device/video.h"



#define JPEG0_SFR_BEGIN  &JPG0_CON0
#define JPEG1_SFR_BEGIN  &JPG1_CON0

/*
#define JPEG_FMT_YUV444    0
#define JPEG_FMT_YUV422    1
#define JPEG_FMT_YUV420    3
*/

enum {
    SOURCE_FROM_IMC,
    SOURCE_FROM_DDR,
};

enum ENC_TYPE {
    JPEG_ENC_TYPE_NONE = 0x0,
    JPEG_ENC_TYPE_STREAM,
    JPEG_ENC_TYPE_IMAGE,
    JPEG_ENC_AUTO_STREAM,
    JPEG_ENC_MANU_STREAM,
    JPEG_ENC_AUTO_IMAGE,
    JPEG_ENC_MANU_IMAGE,
};

#define AUTO_STREAM_CAPTURE     0x0
#define MANU_STREAM_CAPTURE     0x1

struct jpeg_reg {
    volatile u32 CON0            ;
    volatile u32 CON1            ;
    volatile u32 CON2            ;
    volatile u32 YDCVAL          ;        //write only
    volatile u32 UDCVAL          ;        //write only
    volatile u32 VDCVAL          ;        //write only
    volatile u32 YPTR0           ;        //write only
    volatile u32 UPTR0           ;        //write only
    volatile u32 VPTR0           ;        //write only
    volatile u32 YPTR1           ;        //write only
    volatile u32 UPTR1           ;        //write only
    volatile u32 VPTR1           ;        //write only
    volatile u32 BADDR           ;        //write only
    volatile u32 BCNT            ;
    volatile u32 MCUCNT          ;
    volatile u32 PRECNT          ;        //write only
    volatile u32 YUVLINE         ;        //write only
    volatile u32 CFGRAMADDR      ;        //write only
//u32 rev[0x1c00-0x12] ;
    volatile u32 CFGRAMVAL       ;  //0x1c00*4
    volatile u32 PTR_NUM         ;
};

struct mjpg_fb_ops {
    void *(*malloc)(void *fbpipe, u32 size);
    void *(*realloc)(void *fbpipe, void *fb, int newsize);
    void *(*ptr)(void *fb);
    void (*free)(void *fbpipe, void *fb);
    int (*size)(void *fb);
    int (*free_size)(void *fbpipe);
};

struct jpeg_encoder_param {
    u16 width;        //图像宽度
    u16 height;       //图像高度
    u32 kbps; //目标码率
    u8  fps; //帧率
    u8  format; //编码格式
    u8  source;    //数据输入方式，从摄像头或者指定地址
    u8  q;    //编码质量，0-8 9级，越大质量越好，码流越大
    u8  enable_abr;
    u8  enable_dyhuffman;
    u8  prio;
    u8  vbuf_num;//circle video buffer number
    u8  type;
    u32 file_head_len;
    u8  *thumbnails;
    int thumb_len;
    struct jpeg_yuv yuv;
    struct jpg_q_table *qt;
    void *priv;
    void *fbpipe;
    int (*insert_frame)(void *priv, u8 insert_type);
    int (*insert_watermarking)(void *priv);
    void (*reset_source)(void *priv);
    int (*output_frame_buffer)(void *priv, void *ptr);
    struct mjpg_fb_ops *fb_ops;

};

#define JPG_SAMP_FMT_INVALID     0x0 //
#define JPG_SAMP_FMT_YUV444      0x1 //
#define JPG_SAMP_FMT_YUV422      0x2 //
#define JPG_SAMP_FMT_YUV420      0x3 //

#define BITS_MODE_CACHE    0
#define BITS_MODE_UNCACHE  1

#define SINGLE_BUF_MODE    0x0
#define DOUBLE_BUF_MODE    0x1
#define CIRCLE_BUF_MODE    0x2
#define WHOLE_BUF_MODE     0x3

#define INSERT_EMPTY_FRAME      0x1
#define INSERT_SPECAIL_FRAME    0x2
#define INVALID_Q_VAL           0xff

struct jpeg_decoder_param {
    u8  bits_mode;
    u8  out_mode;
    u8  yuv_type;
    void *priv;
    int (*yuv_out_dest)(void *priv, void *arg);
    int (*reset_output)(void *priv);
    u8  *cbuf;
    //int (*jpg_info_wait_data)(void *priv);
};

enum {
    JPEG_INPUT_TYPE_FILE,
    JPEG_INPUT_TYPE_DATA,
};

enum {
    JPEG_DECODE_TYPE_YUV420 = 0,
    JPEG_DECODE_TYPE_DEFAULT,
};

#define DEC_YUV_ALIGN_SIZE(size)  (((size) + 32 - 1) & ~(32 - 1))
struct decoder_yuv_out {
    int line;
    int width;
    int total_line;
    struct jpeg_yuv *yuv;
    u8 frame_begin;
    u8 mode;//
    u8 yuv_type;
};

struct jpeg_file {
    const char *name;
};

struct jpeg_data {
    u8 *buf;
    u32 len;
};


struct jpeg_decode_req {
    u8 input_type;
    u8 output_type; //0 -- yuv420, 1 -- original mode
    union {
        struct jpeg_file file;
        struct jpeg_data data;
    } input;
    u8 *buf_y;
    u8 *buf_u;
    u8 *buf_v;
    u16 buf_width;
    u16 buf_height;
    u16 buf_xoffset;
    u16 buf_yoffset;
    u16 out_width;
    u16 out_height;
    u16 out_xoffset;
    u16 out_yoffset;
    void *priv;
    void (*stream_end)(void *priv);
};

struct jpeg_image_info {
    union {
        struct jpeg_data data;
    } input;
    int sample_fmt;
    u16 width;
    u16 height;
};

struct jpeg_encode_req {
    u8 format;
    u8 q;
    struct jpeg_data data;
    u8 *y;
    u8 *u;
    u8 *v;
    u16 width;
    u16 height;
};

struct jpeg_codec_handle {
    int id;
    volatile int state;
    int timer;
    struct jpeg_reg *reg;
    /*
    struct jpeg_encoder_fd *encoder_fd;
    struct jpeg_encoder_fd *auto_enc_fd;
    struct jpeg_encoder_fd *manual_enc_fd;
    struct jpeg_decoder_fd *decode_fd;
    */
    struct list_head encoder;
    struct list_head decoder;
    int (*frame_end)(void *priv, void *fb);
    OS_SEM sem;
};

#define jpg_be16_to_cpu(x) (((x)[0] << 8) | (x)[1])


int jpeg_codec_init(void);
/*
 * jpeg encode
 * JPEG编码使用函数
 */
int jpeg_encode_one_image(struct jpeg_encode_req *req);
int jpeg_manual_encode_frame(void *fd, struct jpeg_encode_req *req);
int mjpg_stream_cap_resume(void *fd);
int mjpg_stream_cap_pause(void *fd);
int mjpg_stream_cap_start(void *fd);
int mjpg_stream_cap_stop(void *fd);
int mjpg_stream_cap_reset(void *fd);
int mjpg_stream_cap_next_frame(void *fd);
int mjpg_get_stream_fnum(void *fd);
int mjpg_image_cap_start(void *fd, struct video_image_enc *info);
int mjpg_capture_stop(void *fd);
int mjpg_capture_close(void *fd, u8 auto_stream);
int mjpg_stream_set_source(void *fd, void *arg);
int mjpg_stream_reset_bits_rate(void *fd, u32 bits_rate);
int mjpg_stream_reduce_bits_rate(void *fd, int setp);
void *mjpg_image_cap_open(void *arg, int type);
void *mjpg_stream_cap_open(void *arg, int type);
int mjpg_capture_reset_param(void *fd, void *arg);
/*
 * jpeg decode
 * JPEG解码使用函数
 */
//u8 *find_jpg_thumbnails(u8 *buf, int len, int *thm_len);
u8 *find_jpg_frame(u8 *buf, int limit);
void *jpeg_decode_open(void *arg);
int jpeg_decode_reset_param(void *fd, void *arg);
int jpeg_decode_close(void *fd);
int jpeg_decode_reset(void *fd, void *arg);
int jpeg_decode_start(void *fd, u8 *buf, int len);
int jpeg_decode_release(void *fd);
int jpeg_dec_change_omode(void *fd, void *buf, u8 mode);
int jpeg_dec_manual_start(void *fd, u8 *buf, int len);

int jpeg_yuv_to_yuv420(struct jpeg_yuv *src_yuv, struct jpeg_yuv *dst_yuv, u16 stride, u16 image_w, u16 out_w, u8 yuv_type, u16 lines);
int jpeg_decode_image_info(struct jpeg_image_info *info);
int jpeg_decode_one_image(struct jpeg_decode_req *req);
int jpeg_yuv_to_yuv420(struct jpeg_yuv *src_yuv, struct jpeg_yuv *dst_yuv, u16 stride, u16 image_w, u16 out_w, u8 yuv_type, u16 lines);

#endif

