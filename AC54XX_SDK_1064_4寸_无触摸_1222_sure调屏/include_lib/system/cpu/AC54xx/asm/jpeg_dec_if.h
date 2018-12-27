/*************************************************************************
	> File Name: lib/include/cpu/dv16/asm/jpeg_dec_if.h
	> Author:
	> Mail:
	> Created Time: Mon 31 Oct 2016 09:04:58 AM HKT
 ************************************************************************/

#ifndef _JPEG_DEC_IF_H_
#define _JPEG_DEC_IF_H_

#include "jpeg_common.h"

#define JPG_DEC_FMT_YUV444 2
#define JPG_DEC_FMT_YUV422 1
#define JPG_DEC_FMT_YUV420 0

struct jpeg_dec_info {
    u32 jpg_width;
    u32 jpg_height;
    u32 jpg_org_w;
    u32 jpg_org_h;
    u8 yuv_type;
    u8 dec_frame;

    void *priv;
    int (*jpg_info_yuv_out)(void *priv, void *arg);
    //int (*jpg_info_wait_data)(void *priv);

};

enum {
    JPEG_INPUT_TYPE_FILE,
    JPEG_INPUT_TYPE_DATA,
};

struct jpg_dec_yuv_out {
    u32 line;
    u32 width;
    jpg_yuv_addr *yuv;
    u32 total_line;
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

void *jpeg_decode_open(struct jpeg_dec_info *dec_info, int index);
int jpeg_parse_header(void *_fd, struct jpeg_dec_info *info, u8 *buf);
int jpeg_dec_start(void *_fd, u8 *jframe, u32 len, u8 manual_en);
int jpeg_dec_blk_start(void *_fd);
int jpeg_decode_close(void *hd);

int jpeg_yuv_to_yuv420(struct jpg_yuv_buffer *src_yuv, struct jpg_yuv_buffer *dst_yuv, u16 stride, u16 image_w, u16 out_w, u8 yuv_type, u16 lines);

int jpeg_decode_one_image(struct jpeg_decode_req *req);
#endif
