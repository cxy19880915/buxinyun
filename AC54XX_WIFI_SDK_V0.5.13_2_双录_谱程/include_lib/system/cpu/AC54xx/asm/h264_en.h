#ifndef __H264_H__
#define __H264_H__

#include "asm/cpu.h"
#include "typedef.h"
#include "os/os_api.h"
#include "system/includes.h"

#define SLICE_TYPE_I    2
#define SLICE_TYPE_P    0
#define AVC_NUM 2
/*#define MAX_WIDTH 352
#define MAX_HEIGHT 288*/

#define AVC_BS_LEN  100*1024
#define ROUND_BUFFER
#define Print_stream 0
#define Print_encdata 0
#define Cover_addr 0

#define X264_MIN(a,b) ((a) > (b) ? (b) : (a))
#define CACHE2UNCACHE(addr) {u32 t=addr; t = t & (~0x4000000);   t = t | 0x8000000; addr=t;}
#define UNCACHE2CACHE(addr) {u32 t=addr; t = t & (~0x8000000);   t = t | 0x4000000; addr=t;}

#define AVC_FTYPE_NORMAL		1
#define AVC_FTYPE_PSKIP			2

#define AVC_STA_STOPING     1
#define AVC_STA_STOP     2
#define AVC_STA_PAUSE    3

#define IFRAME_CHECK_EN					1
#define H264_ENC_STATE_FAIL_TH           2
#define H264_ENC_STATE_HISTORY_MAX      64

struct h264_user_ops {
    void *(*malloc)(void *priv, u32 size);
    void *(*realloc)(void *priv, void *buf, u32 size);
    void (*free)(void *priv, void *buf);
    int (*size)(void *priv) ;
    void (*output_frame_end)(void *priv, void *buf);
};

typedef struct avc_sfr {
    volatile u32 BUF_NUM        ;
    volatile u32 CON1           ;
    volatile u32 DB_OFFSET      ;
    volatile u32 MV_RANGE       ;
    volatile u32 CON2           ;
    volatile u32 YENC_BASE_ADR0 ;
    volatile u32 UENC_BASE_ADR0 ;
    volatile u32 VENC_BASE_ADR0 ;
    volatile u32 YENC_BASE_ADR1 ;
    volatile u32 UENC_BASE_ADR1 ;
    volatile u32 VENC_BASE_ADR1 ;
    volatile u32 LDSZA_BASE_ADR;
    volatile u32 LDSZB_BASE_ADR;
    volatile u32 LDCPA_BASE_ADR;
    volatile u32 LDCPB_BASE_ADR;
    volatile u32 WBSZA_BASE_ADR;
    volatile u32 WBSZB_BASE_ADR;
    volatile u32 WBCPA_BASE_ADR;
    volatile u32 WBCPB_BASE_ADR;

    volatile u32  CPA_START_ADR ;
    volatile u32  CPA_END_ADR   ;
    volatile u32  CPB_START_ADR ;
    volatile u32  CPB_END_ADR   ;

    volatile u32 BS_ADDR        ;
    volatile u32 QP_CON0        ;
    volatile u32 QP_CON1        ;
    volatile u32 ROI0_XY        ;
    volatile u32 ROI1_XY        ;
    volatile u32 ROI2_XY        ;
    volatile u32 ROI3_XY        ;
    volatile u32 ROIX_QP        ;
    volatile u32 VAR_MUL16      ;
    volatile u32 DECIMATE       ;
    volatile u32 BCNT           ;
    volatile u32 HEADER_CON     ;
    volatile u32 TOTAL_MB_COST  ;
    volatile u32 INTER_WEIGHT   ;
    volatile u32 DEADZONE       ;
    volatile u32  HAS_BUF       ;
} avc_sfr_t ;

struct cap_tail {
    u8 *data;
    u32 len;
};

typedef struct {
    int frame_num;
    int frame_tag;
    int key_frame;
    int idr_pic_id;
    int ref_flag;

    int i_deblocking_filter_alphac0;
    int i_deblocking_filter_beta;
    int i_disable_deblocking_filter_idc;
    int i_alpha_c0_offset;
    int i_beta_offset;
    int b_deblocking_filter_control;
    int b_deblocking_filter;

    int buf_max_size;
    int qp_min;
    int qp_max;
    int i_pic_init_qp;
    int i_pic_init_qs;
    int i_global_qp;
    int qpadj_en;
    int qpadj_mb_en;
    int rc_aq_autovariance;
    int rc_aq_strength;
    int i_chroma_qp_index_offset;

    int yuv_pic_w;
    int yuv_pic_h;
    int i_mb_width;
    int i_mb_height;
    int i_log2_max_frame_num;

    int i_fps_num;
    int i_fps_den;

    int bs_buf_len;
    int sps_addr, pps_addr, sheader_addr;
    int sps_len, pps_len ;

    int enc_data_addr;
    int enc_data_addr_hw;
    int stream_addr;
    int stream_addr_hw;
    int ref_ini_addr;
    int ref_ini_addr_hw;

    u32 ratio;
    u32 ratio1;
    u32 ratio2;
    u32 ratio3;
    u32 write_cnt;
    u32 header_cnt;
    u8  reset_abr;
    u8  frame_type;
    u8  f_aq_strength;
    u8  b_dct_decimate;
    u8  i_cabac_init_idc;
    u8  i_level_idc;
    u8  enc_buf_num;
    u8 ch;
    u8 owe ;
    u8 need_updata_ref_frame;
    volatile u8 status;
    bool auto_idr ;
    void *priv;
    const struct h264_user_ops *ops;
    avc_sfr_t *sfr;
    u32 max_bs_len ;
    OS_SEM sem_stop ;
    void *abr ;
    u32 i_bitrate;
    void *abr_handle ;
    u32 i_last_idr ;
    u32 i_keyint_min;
    u32 i_keyint_max ;
    u32 asc_mb_cost_threshold ;
    s64 total_bytes;
    s64 frame_bytes;
    u8 *ref_cp_a;
    u8 *ref_cp_b;
    u8 *ref_sz_a;
    u8 *ref_sz_b;
    u8 *ref_addr ;

    int drop_frame;

    u8 b_crop;
    int crop_left;
    int crop_right;
    int crop_top;
    int crop_bottom;

    int skip_stream_addr;
    int skip_stream_len;
    u8 manual_skip;
    u8 special_addr;

    u8 bsta;

    u8 fill_forbidden;
    u32 fill_msecs;
    u32 fill_one_sec_msecs;
    int frm_fill;
    int frm_one_sec;
    u32 fill_interval;
    u32 fill_continue;
    u32 fill_fbase;
    u32 fill_one_sec_fbase;
    u32 fill_fnum;
    u32 fill_cnt_fnum;
    u32 fill_secs;
    volatile u8 fill_enable;
    volatile u8 fill_cri;

    u8 state_history_win[H264_ENC_STATE_HISTORY_MAX];
    s32 state_fail_cnt;
    u32 state_history_win_idx;

    u32 cycle_time;
    u32 cfnum;
    u32 bk_cycle_time;
    u32 cyc_ctime;
    u8 cyc_flag;

    u8 last_frame_type;
    u8 std_header;
    u8 cap_iframe;
    u8 change_bitrate;
    u8 time_stamp;
    u8 real_fps;
    struct cap_tail itail;
} H264_EN_VAR;

typedef struct {
    int level_idc;
    int mbps;        /* max macroblock processing rate (macroblocks/sec) */
    int frame_size;  /* max frame size (macroblocks) */
    int dpb;         /* max decoded picture buffer (bytes) */
    int bitrate;     /* max bitrate (kbit/sec) */
    int cpb;         /* max vbv buffer (kbit) */
    int mv_range;    /* max vertical mv component range (pixels) */
    int mvs_per_2mb; /* max mvs per 2 consecutive mbs. */
    int slice_rate;  /* ?? */
    int bipred8x8;   /* limit bipred to >=8x8 */
    int direct8x8;   /* limit b_direct to >=8x8 */
    int frame_only;  /* forbid interlacing */
} x264_level_t;

static const x264_level_t x264_levels[] = {
    { 10,   1485,    99,   152064,     64,    175,  64, 64,  0, 0, 0, 1 },
//  {"1b",  1485,    99,   152064,    128,    350,  64, 64,  0, 0, 0, 1 },
    { 11,   3000,   396,   345600,    192,    500, 128, 64,  0, 0, 0, 1 },
    { 12,   6000,   396,   912384,    384,   1000, 128, 64,  0, 0, 0, 1 },
    { 13,  11880,   396,   912384,    768,   2000, 128, 64,  0, 0, 0, 1 },
    { 20,  11880,   396,   912384,   2000,   2000, 128, 64,  0, 0, 0, 1 },
    { 21,  19800,   792,  1824768,   4000,   4000, 256, 64,  0, 0, 0, 0 },
    { 22,  20250,  1620,  3110400,   4000,   4000, 256, 64,  0, 0, 0, 0 },
    { 30,  40500,  1620,  3110400,  10000,  10000, 256, 32, 22, 0, 1, 0 },
    { 31, 108000,  3600,  6912000,  14000,  14000, 512, 16, 60, 1, 1, 0 },
    { 32, 216000,  5120,  7864320,  20000,  20000, 512, 16, 60, 1, 1, 0 },
    { 40, 245760,  8192, 12582912,  20000,  25000, 512, 16, 60, 1, 1, 0 },
    { 41, 245760,  8192, 12582912,  50000,  62500, 512, 16, 24, 1, 1, 0 },
    { 42, 522240,  8704, 13369344,  50000,  62500, 512, 16, 24, 1, 1, 1 },
    { 50, 589824, 22080, 42393600, 135000, 135000, 512, 16, 24, 1, 1, 1 },
    { 51, 983040, 36864, 70778880, 240000, 240000, 512, 16, 24, 1, 1, 1 },
    { 0 }
};

static const u8 x264_ue_size_tab[256] = {
    1, 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
};

static u8 nal_tag[4] = {0, 0, 0, 1};

extern H264_EN_VAR x264_en_st0;
extern void x264_sps_write(H264_EN_VAR *x264);
extern void x264_pps_write(H264_EN_VAR *x264);
extern void slice_header_write(H264_EN_VAR *x264);
void x264_sps_write_line0(H264_EN_VAR *x264);
void x264_pps_write_line0(H264_EN_VAR *x264);
void slice_header_write_line0(H264_EN_VAR *x264, u8 pskip);
void x264_sps_write_line1(H264_EN_VAR *x264);
void x264_pps_write_line1(H264_EN_VAR *x264);
void slice_header_write_line1(H264_EN_VAR *x264);

struct roi_cfg {
    u32 roio_xy;
    u32 roi1_xy;
    u32 roi2_xy;
    u32 roi3_xy;
    u32 roio_ratio;
    u32 roio_ratio1;
    u32 roio_ratio2;
    u32 roio_ratio3;
    u32 roio_config;
};

struct h264_enc_info {
    u16 width;
    u16 height;
    struct roi_cfg roi;
    u8  fps;
    u8 real_fps;
    u8 fill_forbidden;
    u8  key_frame_num;
    u8  yuv_buf_num;
    u32 yuv_base;
    u32 i_bitrate ;
    u32 cycle_time;
    u32 std_head;
    u8 quality;
    u8 time_stamp_en;
    void *priv;
    const struct h264_user_ops *ops;
};


void *avc_enc_open(struct h264_enc_info *info);
void avc_enc_cap_iframe(void *fd);
void avc_enc_set_cap_tail(void *fd, struct cap_tail *tail);

int avc_enc_start(void *fd);

int avc_enc_restart(void *fd);
int avc_enc_rekstart(void *fd);
void avc_enc_wait_end(void *fd);

int avc_enc_change_bitrate(void *fd, u32 bitrate);
int avc_enc_reset(void *fd);
int avc_enc_reset_abr(void *fd);
int avc_enc_set_count_down(void *fd, u32 time);
int avc_enc_set_cycle_time(void *fd, int time) ;

int avc_enc_stop(void *fd);
int avc_enc_pause(void *fd);
int avc_enc_continue(void *fd, u8 yuv_buf_num, u32 yuv_base);

int avc_enc_close(void *fd);
void avc_encode_frame_I(H264_EN_VAR *x264, u8 resetbuf) ;
void avc_encode_frame_P(H264_EN_VAR *x264, u8 resetbuf) ;
void build_skip_frame(H264_EN_VAR *x264) ;
void avc_slice_coding(H264_EN_VAR *x264) ;
int avc_enc_get_fnum(H264_EN_VAR *x264);
int manual_skip_frame(H264_EN_VAR *x264);

void avc_imc_reset_down(struct video_endpoint *ep);
void avc_imc_reset_up(struct video_endpoint *ep);
void avc_imc_restart(struct video_endpoint *ep);
void avc_reset_uvc_source(struct video_endpoint *ep, int reset);
void avc_enc_set_targe_fps(void *fd, int fps);
#endif

