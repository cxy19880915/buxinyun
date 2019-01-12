/*******************************************************************************************
 File Name: jpeg_encode.h
 Version: 1.00
 Discription:
 Author:yulin deng
 Email :flowingfeeze@163.com
 Date: 11 2013
 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/
#ifndef  __JPEG_ENC_IF_H__
#define  __JPEG_ENC_IF_H__

#include "typedef.h"
#include "jpeg_common.h"
#include "system/includes.h"
/*
typedef struct __jpg_yuv_addr
{
	u8 *y_addr;
	u8 *u_addr;
	u8 *v_addr;
}jpg_yuv_addr;
*/

typedef struct _JPEG_OPERATE {
    u16 width;        //图像宽度
    u16 height;       //图像高度
    u32 abr_kbps; //目标码率
    u8 encode_format; //编码格式
    u8 mode;    //数据输入方式，从摄像头或者指定地址
    u8 q ;    //编码质量，0-8 9级，越大质量越好，码流越大
    u8 abr_en;

    u8 fps; //帧率
    u8 dynamic_huffman_en;
    u8 dynamic_huffman_prio;
    u8 prio;
    u8 c_vbuf_num;//circle video buffer number
    u8 type;
    struct jpg_yuv_buffer yuv_addr[2];

    int (*output_frame_end)(void *priv, void *ptr);
    void *priv;

    void *fbpipe;
    void (*reset_cbuf)(void *priv);
    void *(*fb_malloc)(void *fbpipe, u32 size);
    void *(*fb_realloc)(void *fbpipe, void *fb, u32 newsize);
    void (*fb_free)(void *fbpipe, void *fb);
    void *(*fb_ptr)(void *fb);
    u32(*fb_size)(void *fb);

} JPEG_OPERATE;


typedef struct _JPEG_ENC_OPS_T {
    char *name ;               // 编码码器名称
    int (*open)(void *info, u8 num);
    void (*close)(void *hd);
    s32(*ioctrl)(void *priv, void *parm, u32 cmd);
} JPEG_ENC_OPS_T ;


typedef struct _jpg_manual_info {
    void *priv ;
    u8 *y_addr;
    u8 *u_addr;
    u8 *v_addr;
} jpg_manual_info;

typedef struct _jpeg_enc_blcok_info {
    u16 block_w;
    u16 block_h;
    u16 frame_w;
    u16 frame_h;
    u8 *y_addr;//*ptr;
    u8 *u_addr;
    u8 *v_addr;
} JPEG_ENC_BLOCK_INFO;

enum {
    READ_FROM_BUF,
    READ_FROM_SENSOR,
};

enum ENC_TYPE {
    JPEG_ENC_TYPE_NONE = 0x0,
    JPEG_ENC_TYPE_STREAM,
    JPEG_ENC_TYPE_IMAGE,
};

//只在 READ_FROM_BUF模式下有效!!!
enum {
    JPEG_ENC_MCU_IDLE,
    JPEG_ENC_MCU_START,
    JPEG_ENC_MCU_END,
    JPEG_ENC_FRAME_END,
};


enum {
    JPEG_CMD_ENC_START,
    JPEG_CMD_ENC_STOP,
    JPEG_CMD_ENC_MKSTART,
    JPEG_CMD_ENC_GET_STATUS,
    JPEG_CMD_ENC_RESET,
    JPEG_CMD_ENC_IS_STOP,
    JPEG_CMD_ENC_GET_VRAM,
    JPEG_CMD_ENC_IMC_FRAME_END,
    JPEG_CMD_ENC_IMC_PARM,
    JPEG_CMD_ENC_RESTART,
    JPEG_CMD_ENC_FREE_FB,
    JPEG_CMD_ENC_GET_FRAME_CNT,
};


#define JPEG_FMT_YUV444    1
#define JPEG_FMT_YUV422    2
#define JPEG_FMT_YUV420    3


void *jpeg_enc_open(JPEG_OPERATE *jpeginfo, u8 jpg_num);
void jpeg_enc_close(void *hd);
int jpeg_enc_start(void  *hd);
int jpeg_enc_stop(void *priv);
int jpeg_enc_cap_stop(void *hd);
int jpeg_enc_is_stop(void *priv);
u32 jpeg_stop_abort(void *priv);
int jpeg_enc_restart(void *hd);
void jpeg_enc_reset(void *priv);
int jpeg_enc_direct_kstart(void *hdl);

int jpeg_enc_manual_kstart(void *hd, void *mhdl);
int jpeg_enc_manual_status(void *hd);

int jpeg_get_fnum(void *hdl);


JPEG_ENC_OPS_T *get_jpeg0_enc_ops();
JPEG_ENC_OPS_T *get_jpeg1_enc_ops();

#endif
