/*******************************************************************************************
 File Name: jpeg_encode.h
 Version: 1.00
 Discription:
 Author:yulin deng
 Email :flowingfeeze@163.com
 Date: 11 2013
 Copyright:(c)JIELI  2011  @ , All Rights Reserved.
*******************************************************************************************/
#ifndef  __JPEG_ENCODER_H__
#define  __JPEG_ENCODER_H__

#include "typedef.h"
//#include "jpeg_codec.h"
#include "asm/jpeg_abr.h"
#include "device/video.h"
#include "jpeg_iocmds.h"


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


#define JPEG_DEFAULT_EXIF_LEN        0xA8//208
//extern unsigned char const jpeg_file_header[624 + JPEG_ADD_LEN];




#define FRAME_INT 1
#define JPEG_MODLE_NUM  1   // jpeg 编码模块个数

#define JPEG_ENC_INT_EN     1//是否允许JPEG编码中断

#define JPEG_ENCODE_BITCNT  0x1000//0x800 //位流buffer大小

#define JPEG_INT_EN()   reg->CON0 |= BIT(2)|BIT(3)
#define JPEG_INT_DIS()  reg->CON0 &=~(BIT(2)|BIT(3))

#define JPEG_FTYPE_NORMAL		1
#define JPEG_FTYPE_SKIP			2

#define BUILD_DYNAMIC_HUFFMAN   0
#define ENABLE_JPEG_ABR			1
#define ENABLE_ZOOM_ENC			0

#define JPEGENC_KSTART      0x80
#define JPEGENC_RESET       0x40
#define JPEGENC_BITS_FULL   0x20
#define JPEGENC_STATUS      0xf

//#define GET_JPEGENC_STATUS(hd) ((hd)->state & 0xf)
//#define SET_JPEGENC_STATUS(hd, x) (hd)->state = (((hd)->state & 0xf0) | ((x) & 0xf))
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

/*
typedef struct FILEHEAD {
    u16  YQT_DCT[0x40] ;
    u16  UVQT_DCT[0x40];
    u8   filedata[138] ;
} qtfilehead_t ;
*/
typedef struct jpg_q_table qtfilehead_t;

struct fill_frame {
    u8 forbidden;
    volatile u8 enable;
    volatile u8 cri;
    int timer;
    int one_sec_timer;
    u32 msecs;
    u32 one_sec_msecs;
    u32 secs;
    u32 interval;
    u32 cont;
    u32 fbase;
    u32 one_sec_fbase;
    u32 fnum;
    u32 cnt_fnum;
    u32 fps;
};

struct jpeg_fb_ops {
    void *(*malloc)(void *pipe, u32 size);
    void *(*realloc)(void *pipe, void *fb, int size);
    void *(*ptr)(void *fb);
    void (*free)(void *pipe, void *fb);
    int (*size)(void *fb);
    int (*free_size)(void *pipe);
};

struct jpeg_encode_info {
    u16 width;
    u16 height;
    u32 kbps;
    u8  fps;
    u8  source; //imc / buffer
    u8  fmt; // yuv420 yuv422 yuv444
    u8  type;// stream / image
    u8  mode;// 编码模式 0 -- 连续编码模式   1 -- 编一张。
    u8  q_val;
    u8  vbuf_num;
    struct jpg_q_table *qt;
};

typedef struct jpeg_encoder_fd {
    struct jpeg_yuv yuv;
    void *parent;
    struct jpeg_encode_info info;

    volatile u8 active;
    u8 enable_abr;
    u8 enable_dyhuffman;
    u8 hori_sample;
    u8 vert_sample;
    u8 sample_rate;
    u8 frame_drop;//判断当前帧是否是丢帧
    u8 last_frame_type;
    volatile u8 state;
    volatile u8 mcu_pnd;
    volatile u32 mcu_cnt;
    volatile u32 mcu_line_cnt;
    volatile u32 encode_mcu;
    volatile u32 bits_cnt;
    volatile u32 file_size;
    u32 frame_cnt;
    u32 buf_len;

    u8 *data;
    u8 *exif;
    int exif_size;
    u8 *thumbnails;
    int thumb_len;
    void *priv;
    int  head_len;
    void *fb;
    //void *fbpipe;
    struct mjpg_fb_ops *fb_ops;
    struct fill_frame fill;
    void (*reset_source)(void *priv);
    int (*output_frame_end)(void *priv, void *ptr);
    int (*insert_frame)(void *priv, u8 insert_type);
    int (*insert_watermarking)(void *priv);
    OS_SEM sem_stop ;
    OS_SEM sem_dhuffman;
#if SMP_ENABLE
    spinlock_t lock;
#endif
#if ENABLE_JPEG_ABR
    struct jpeg_abr_fd *abr_fd;//自适应q值运算数据
#endif
    struct list_head entry;
} jpeg_handle_t;


static inline u8 GET_ENCODER_STATE(struct jpeg_encoder_fd *fd)
{
    u8 state;
#if SMP_ENABLE
    spin_lock(&fd->lock);
#endif
    state = fd->state & 0xf;
#if SMP_ENABLE
    spin_unlock(&fd->lock);
#endif
    return state;
}

static inline void SET_ENCODER_STATE(struct jpeg_encoder_fd *fd, u8 state)
{
#if SMP_ENABLE
    spin_lock(&fd->lock);
#endif
    fd->state = (fd->state & 0xf0) | (state & 0xf);
    __asm_csync();
#if SMP_ENABLE
    spin_unlock(&fd->lock);
#endif
}

extern int jpeg_manual_encoder_init(struct jpeg_encoder_fd *fd, void *arg);
extern int jpeg_encoder_init(struct jpeg_encoder_fd *fd, void *arg);
int jpeg_encoder_start(void *_fd);
int jpeg_encoder_restart(void *_fd);
int jpeg_encoder_manual_start(void *_fd, struct video_image_enc *info);
int jpeg_encoder_set_source_buffer(void *_fd, void *arg);
int jpeg_encoder_reset_bits_rate(void *_fd, u32 bits_rate);
int jpeg_encoder_reset(void *_fd, u8 init_hw);
int jpeg_encoder_stop(void *_fd);
int jpeg_encoder_pause(void *_fd);
int jpeg_encoder_close(void *_fd);
int jpeg_encoder_frame_num(void *_fd);
int jpeg_encoder_image_start(void *_fd, void *arg, u8 for_stream);

int encoder_dhuffman_irq_handler(struct jpeg_encoder_fd *fd);
int encoder_speed_irq_handler(struct jpeg_encoder_fd *fd);
int encoder_bits_irq_handler(struct jpeg_encoder_fd *fd);
int encoder_mcu_irq_handler(struct jpeg_encoder_fd *fd);

int jpeg_frame_fill_start(struct jpeg_encoder_fd *fd);
void jpeg_frame_fill(struct jpeg_encoder_fd *fd);
void jpeg_stop_frame_fill(struct jpeg_encoder_fd *fd);


#if BUILD_DYNAMIC_HUFFMAN
extern void huffman_init(struct jpeg_encoder_fd *fd);
extern void huffman_uninit(struct jpeg_encoder_fd *fd);
extern void updata_huffman_info(struct jpeg_encoder_fd *fd, u8 *buf);
extern void update_huff_freq_data(struct jpeg_reg *sfr);
#endif

#endif



