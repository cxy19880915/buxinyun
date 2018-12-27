/*******************************************************************************************
 File Name: jpeg_encode.h
 Version: 1.00
 Discription:
 Author:yulin deng
 Email :flowingfeeze@163.com
 Date: 11 2013
 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/
#ifndef  __JPEG_ENCODE_H__
#define  __JPEG_ENCODE_H__

#include "typedef.h"
#include "jpeg_common.h"
#include "jpeg_enc_if.h"
#include "jpeg_abr.h"
#include "os/os_api.h"
#include "device/video.h"


/*
#define JPEG0_SFR_BEGIN  &JPG0_CON0
#define JPEG1_SFR_BEGIN  &JPG1_CON0


typedef struct JPEG_SFR
{
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
}JPEG_SFR ;
*/


#define JPEG_ADD_LEN        208
extern unsigned char const jpeg_file_header[624 + JPEG_ADD_LEN];




#define FRAME_INT 1
#define JPEG_MODLE_NUM  1   // jpeg 编码模块个数

#define JPEG_ENC_INT_EN     1//是否允许JPEG编码中断

#define JPEG_ENCODE_BITCNT  0x1000//0x800 //位流buffer大小

#define JPEG_INT_EN()   sfr->CON0 |= BIT(2)|BIT(3)
#define JPEG_INT_DIS()  sfr->CON0 &=~(BIT(2)|BIT(3))


#define JPEG_ENC_MODE_FROM_VRAM   0
#define JPEG_ENC_MODE_FROM_SRAM   1

#define BUILD_DYNAMIC_HUFFMAN   0
#define ENABLE_JPEG_ABR			1
#define ENABLE_ZOOM_ENC			0

#define JPEGENC_KSTART      0x80
#define JPEGENC_RESET       0x40
#define JPEGENC_BITS_FULL   0x20
#define JPEGENC_STATUS      0xf

//#define GET_JPEGENC_STATUS(hd) ((hd)->jpeg_status & 0xf)
//#define SET_JPEGENC_STATUS(hd, x) (hd)->jpeg_status = (((hd)->jpeg_status & 0xf0) | ((x) & 0xf))
#define SMP_ENABLE     1

enum {
    JPEG_SEEK_SET = 0x01,
    JPEG_SEEK_CUR = 0x02,
};


enum {
    JPEGENC_UNINIT = 0x0,
    JPEGENC_INIT,
    JPEGENC_IDLE,
    JPEGENC_READY,
    JPEGENC_STOP,
    JPEGENC_RUNNING,
    JPEGENC_STOPING,
    JPEGENC_FRAME_END,
};

#if BUILD_DYNAMIC_HUFFMAN
enum {
    HUFFMAN_NO_INIT = 0x0,
    HUFFMAN_OPEN,
    HUFFMAN_CLOSE,
};
#endif

typedef struct FILEHEAD {
    u16  YQT_DCT[0x40] ;
    u16  UVQT_DCT[0x40];
    u8   filedata[138] ;
} qtfilehead_t ;

struct fill_frame {
    u8 forbidden;
    volatile u8 enable;
    volatile u8 cri;
    int timer;
    u32 msecs;
    u32 interval;
    u32 cont;
    u32 fbase;
    u32 fnum;
    u32 cnt_fnum;
};

typedef struct _JPEG_FD {
//    struct video_ram *input_ram;
    jpg_yuv_addr yuv_addr[2];

    u8 jpeg_enc_mode;
    u8 hori_sample;
    u8 vert_sample;
    u8 sample_rate;
    u8 is_curframe_drop;//判断当前帧是否是丢帧
    volatile u8 jpeg_status;
    volatile u8 line_ctrl;
    u8 index;
    u8 q ;
    u8 type ;
    u8 mode ;// 编码模式 0 -- 连续编码模式   1 -- 编一张。
    JPEG_SFR *sfr ;
    volatile u32 jpeg_mcu_cnt;
    volatile u32 jpeg_one_line_mcu_cnt;
    volatile u32 jpeg_encode_mcu;

    u8 *data ;
    void *fbpipe ;
    void *fb ;
    volatile u32 jpeg_bit_cnt;
    volatile u32 jpeg_file_size;
    int (*output_frame_end)(void *priv, void *ptr);
    void *priv;

    void *(*fb_malloc)(void *fbpipe, u32 size);
    void *(*fb_realloc)(void *fbpipe, void *fb, u32 newsize);
    void (*fb_free)(void *fbpipe, void *fb);
    void *(*fb_ptr)(void *fb);
    u32(*fb_size)(void *fb);

    void (*reset_cbuf)(void *priv);
    JPEG_OPERATE info ;
    OS_SEM sem_stop ;
#if SMP_ENABLE
    spinlock_t lock;
#endif
    u32 frame_cnt;
    struct fill_frame fill;

#if ENABLE_JPEG_ABR
    mabr_t *jpeg_abr;//自适应q值运算数据
#endif
} jpeg_handle_t;


static u8 inline GET_JPEGENC_STATUS(jpeg_handle_t *hd)
{
    u8 status;
#if SMP_ENABLE
    spin_lock(&hd->lock);
#endif
    status = hd->jpeg_status & 0xf;
#if SMP_ENABLE
    spin_unlock(&hd->lock);
#endif
    return status;
}

static void SET_JPEGENC_STATUS(jpeg_handle_t *hd, u8 status)
{
#if SMP_ENABLE
    spin_lock(&hd->lock);
#endif
    hd->jpeg_status = (hd->jpeg_status & 0xf0) | (status & 0xf);
#if SMP_ENABLE
    spin_unlock(&hd->lock);
#endif
}

void *jpeg_encode_init(JPEG_OPERATE *jpeginfo, u32 index, jpeg_handle_t *hd);
void jpeg_encode_kstart(jpeg_handle_t *hd, u16 enc_h, u8 *y, u8 *u, u8 *v);
u32 jpeg_encode_bitbuf_init(jpeg_handle_t *hdl);
void *jpeg_free_fb(void *hdl);
void jpg_enc_isr(void);
void jpeg_encode_isr(jpeg_handle_t *hd);

int jpeg_encode_start(jpeg_handle_t *hdl);
int jpeg_encode_direct_start(jpeg_handle_t *hd);
void jpeg_encode_stop(jpeg_handle_t *hdl);
int jpeg_encode_is_stop(void *hdl);
int jpeg_encode_manual_kstart(jpeg_handle_t *hd, struct video_image_enc *mhdl);

void jpeg_frame_fill_add(u8 add_cnt);
void jpeg_frame_fill(jpeg_handle_t *hdl);


#if BUILD_DYNAMIC_HUFFMAN
void huffman_init(jpeg_handle_t *hd);
void huffman_uninit(jpeg_handle_t *hd);
void updata_huffman_info(jpeg_handle_t *hd, u8 *buf);
void update_huff_freq_data(JPEG_SFR *sfr);
#endif

#endif
