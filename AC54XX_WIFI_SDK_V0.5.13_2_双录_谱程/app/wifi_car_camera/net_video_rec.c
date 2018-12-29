
#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/ctp_server.h"
#include "server/video_engine_server.h"
#include "ui_ctr_def.h"
#include "video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"
#include "asm/lcd_config.h"
#include "user_isp_cfg.h"
#include "os/os_compat.h"


#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "power_ctrl.h"
#include "app_database.h"
#include "server/rt_stream_pkg.h"
#include "server/net_server.h"
#include "net_video_rec.h"
#include "video_fps_ctrl.h"
#include "video_bitrate_ctrl.h"
#include "streaming_media_server/fenice_config.h"

struct strm_video_hdl {
    enum VIDEO_REC_STA state;
    enum VIDEO_REC_STA state_ch2;
    enum VIDEO_REC_STA state1_ch2;
    enum VIDEO_DISP_STA disp_state;

    struct server *video_rec0;
    struct server *video_rec1;
    struct server *video_rec2;
    struct server *video_display_0;
    struct server *video_display_1;
    struct server *video_display_2;

    u8 *v0_fbuf;
    u8 *v1_fbuf;
    u8 *v2_fbuf;
    u8 *audio_buf;
    /*u8 avin_dev;*/
    u8 isp_scenes_status;

    int timer_handler;
    fps_ctrl_t fps_ctrl_hdl;
    bitrate_ctrl_t bitrate_ctrl_hdl;
    u32 dy_fr;
    u32 dy_fr_denom;
    u32 dy_bitrate;
    u32 fbuf_fcnt;
    u32 fbuf_ffil;
};


struct strm_video_hdl fv_rec_handler;

#define __this 	(&fv_rec_handler)
#define sizeof_this     (sizeof(struct video_rec_hdl))

#define VREC0_FBUF_SIZE     (6*1024*1024)
#define AUDIO_BUF_SIZE     (300*1024)
#define  STRM_VIDEO_REC_FPS0   20  //不使用0作为默认值，写具体数值

static const u16 rec_pix_w[] = {1920, 1280, 640, 384};
static const u16 rec_pix_h[] = {1088, 720,  480, 240};
static int strm_source_type = STRM_SOURCE_VIDEO0;
static u16 REC_PIX_W = 0;
static u16 REC_PIX_H = 0;
static u8 REC_FPS = 0;
static u8 CHANNEL_NUM = 0;
static u16 SAMPLE_RATE = 0;
char video_net_osd_buf[512] ALIGNE(64);
int video0_open_flag;

/*码率控制，根据具体分辨率设置*/
static int video_rec_get_abr(u32 width)
{
    if (width <= 384) {
        return 2500;
    } else if (width <= 640) {
        return 3000;//2000;
    } else if (width <= 1280) {
        return 3500;
        /* return 10000; */
    } else if (width <= 1920) {
        return 5000;
    } else {
        return 18000;
    }
}

/*
 *场景切换使能函数，如果显示打开就用显示句柄控制，否则再尝试用录像句柄控制
 */
static int video_rec_start_isp_scenes()
{
    if (__this->isp_scenes_status) {
        return 0;
    }

    stop_update_isp_scenes();

    if (__this->video_display_0) {
        __this->isp_scenes_status = 1;
        return start_update_isp_scenes(__this->video_display_0);
//    } else if (__this->video_rec0 && (__this->state == VIDREC_STA_START)) {
    } else if (__this->video_rec0) {
        __this->isp_scenes_status = 2;
        return start_update_isp_scenes(__this->video_rec0);
    }

    __this->isp_scenes_status = 0;

    return 1;
}

static int video_rec_stop_isp_scenes(u8 status, u8 restart)
{

    if (__this->isp_scenes_status == 0) {
        return 0;
    }

    if ((status != __this->isp_scenes_status)) {
        return 0;
    }

    __this->isp_scenes_status = 0;
    stop_update_isp_scenes();

    if (restart) {
        video_rec_start_isp_scenes();
    }

    return 0;
}

static void video_rec0_set_fr(int fr_a, int fr_b)
{
    union video_req req = {0};

    struct drop_fps targe_fps;

    targe_fps.fps_a = fr_a;
    targe_fps.fps_b = fr_b;
    req.rec.targe_fps = &targe_fps;

    /* 通道号，分辨率，封装格式，写卡的路径 */
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        req.rec.channel = 1;  /* video0的sd卡录像为:channel0,所以这里不能在占用channel0 */
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        req.rec.channel = 0;  /* video1的sd卡录像为:channel1,所以这里不能在占用channel1 */
    }

    req.rec.state 	= VIDEO_STATE_SET_DR;

    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    }

    //printf("video_rec0_set_frame_rate = %d(%d/%d) \r\n", fr_a / fr_b, fr_a, fr_b);
}

static void video_rec0_set_bitrate(unsigned int bits_rate)
{
    union video_req req = {0};

    /* 通道号，分辨率，封装格式，写卡的路径 */
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        req.rec.channel = 1;  /* video0的sd卡录像为:channel0,所以这里不能在占用channel0 */
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        req.rec.channel = 0;  /* video1的sd卡录像为:channel1,所以这里不能在占用channel1 */
    }

    req.rec.state = VIDEO_STATE_RESET_BITS_RATE;
    req.rec.abr_kbps = bits_rate;

    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    }
}

static void net_fps_fps_bitrate_ctrl_init(void)
{
    __this->dy_fr = STRM_VIDEO_REC_FPS0 * 256;
    __this->dy_fr_denom = 256;
    __this->fbuf_fcnt = __this->fbuf_ffil = 0;
    fps_ctrl_init(&__this->fps_ctrl_hdl, STRM_VIDEO_REC_FPS0 * 256, 20, 16, 18,
                  STRM_VIDEO_REC_FPS0 * 256);
    video_rec0_set_fr(__this->dy_fr, __this->dy_fr_denom);

    __this->dy_bitrate = video_rec_get_abr(REC_PIX_W);
    bitrate_ctrl_init(&__this->bitrate_ctrl_hdl, __this->dy_bitrate, 12, 3, 1024, __this->dy_bitrate);
}

static void net_video_fps_bitrate_ctrl(void)
{
    int fps, fps_denom, bitrate;
    fps_ctrl_update(&__this->fps_ctrl_hdl, __this->fbuf_fcnt, &fps, &fps_denom);
    bitrate_ctrl_update(&__this->bitrate_ctrl_hdl, __this->fbuf_fcnt, &bitrate);
    if (fps != __this->dy_fr || fps_denom != __this->dy_fr_denom || bitrate != __this->dy_bitrate) {
        __this->dy_fr = fps;
        __this->dy_fr_denom = fps_denom;
        __this->dy_bitrate = bitrate;
        video_rec0_set_fr(fps, fps_denom);
        video_rec0_set_bitrate(bitrate);
        printf("^*^nfcnt= %d, fps= %d[%d/%d], bitrate = %d \r\n", __this->fbuf_fcnt,
               fps / fps_denom, fps, fps_denom, bitrate);
    }
}

static void net_video_timer_hdl(void *parm)
{
    static int timer_cnt;
    ++timer_cnt;

    if ((timer_cnt % 1) == 0) {
        /*net_video_fps_bitrate_ctrl();*/
    }
}

//编码器每编出一帧会放进队列,并且进入此函数统计, 参数fbuf就是帧指针, frame_size 就是帧大小
void strm_264_pkg_get_in_frame(char *fbuf, u32 frame_size)
{
    __this->fbuf_fcnt += 1;
    __this->fbuf_ffil += frame_size;
}

//RTSP 实时流 每发送完成一帧, 会进入此函数进行统计参数 ,fbuf就是帧指针, frame_size 就是帧大小
void strm_264_pkg_get_out_frame(char *fbuf, u32 frame_size)
{
    __this->fbuf_fcnt -= 1;
    __this->fbuf_ffil -= frame_size;

#if 1
    static u32 vdo_frame_cnt;
    ++vdo_frame_cnt;
    static int time_hdl;
    int t_ret = time_lapse(&time_hdl, 1000);
    if (t_ret) {
        printf("rtsp  out fr = %dms, %d\r\n", t_ret, vdo_frame_cnt);
        vdo_frame_cnt = 0;
    }
#endif

}

static int fv_video_rec_open(void)
{
#if 1
    if (!__this->v0_fbuf) {
        __this->v0_fbuf = malloc(VREC0_FBUF_SIZE);
        if (!__this->v0_fbuf) {
            printf("malloc fv_v0_buf err\n");
            return -1;
        }
    }

    if (!__this->audio_buf) {
        __this->audio_buf = malloc(AUDIO_BUF_SIZE);
        if (!__this->audio_buf) {
            printf("malloc fv_audio_buf err\n");
            free(__this->v0_fbuf);
            return -ENOMEM;
        }
    }
#endif

    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        if (!__this->video_rec0) {
            __this->video_rec0 = server_open("video_server", "video0");
            if (!__this->video_rec0) {
                return VREC_ERR_V0_SERVER_OPEN;
            }
        }
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        if (!__this->video_rec1) {
#if defined CONFIG_VIDEO1_ENABLE
            __this->video_rec1 = server_open("video_server", "video1");
#endif
#if defined CONFIG_VIDEO3_ENABLE
            __this->video_rec1 = server_open("video_server", "video3");
#endif
            if (!__this->video_rec1) {
                return VREC_ERR_V1_SERVER_OPEN;
            }
        }
    } else {
        printf("unknown strm source type to open\n");
        return -EINVAL;
    }

    return 0;
}


static int fv_video_rec_start(void)
{
    printf("In fv_rec_start\n");

    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;

    u32 res = db_select("res");

    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    /* 通道号，分辨率，封装格式，写卡的路径 */
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        req.rec.channel = 1;  /* video0的sd卡录像为:channel0,所以这里不能在占用channel0 */
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        req.rec.channel = 0;  /* video1的sd卡录像为:channel1,所以这里不能在占用channel1 */
#if defined CONFIG_VIDEO3_ENABLE
        req.rec.camera_type = VIDEO_CAMERA_UVC;
        req.rec.uvc_id = 0;
        if (!dev_online("uvc")) {
            return -1;
        }
#endif

    }
    req.rec.width 	= REC_PIX_W;
    req.rec.height 	= REC_PIX_H;
    req.rec.format  = STRM_VIDEO_FMT_MOV;
//    req.rec.format  = STRM_VIDEO_FMT_AVI;
//    req.rec.format  = STRM_IMC_DISP_AVI;
    req.rec.state 	= VIDEO_STATE_START;

    req.rec.buf = __this->v0_fbuf;
    req.rec.buf_len = VREC0_FBUF_SIZE;
    /* 帧率为0表示使用摄像头的帧率 */
    req.rec.quality = VIDEO_MID_Q;
    req.rec.fps 	= REC_FPS;

    /* 采样率，通道数，录像音量，音频使用的循环BUF,录不录声音 */
    req.rec.audio.sample_rate = SAMPLE_RATE;  /* 采样率设为0,可关闭录音 */
    req.rec.audio.channel 	= CHANNEL_NUM;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.volume    = 63;
//  req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = db_select("mic");
//    req.rec.pkg_mute.aud_mute = !db_select("mic");

    /* 码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     * roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，
     *			[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数 */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 0;

#if 0
    /* 感兴趣区域为下方 中间 2/6 * 4/6 区域,可以调整感兴趣区域qp为其他区域的70%可以调整 */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 |
                          (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 |
                          (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 |
                          (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 |
                          (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;
#endif

    /* osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下 */
    osd_info.osd_w = 16;
    osd_info.osd_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);//21;

    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height ;
    osd_info.x = (req.rec.width - max_one_line_strnum * osd_info.osd_w) / 64 * 64;
    osd_info.y = (osd_max_heigh - osd_info.osd_h * osd_line_num) / 16 * 16;
    osd_info.osd_yuv = 0xe20095;
    osd_info.osd_str = video_rec_osd_buf;
    osd_info.osd_matrix_str = osd_str_total;
    osd_info.osd_matrix_base = osd_str_matrix;
    osd_info.osd_matrix_len = sizeof(osd_str_matrix);
    req.rec.osd = 0;
    if (db_select("dat")) {
        req.rec.osd = &osd_info;
    }





    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("ERR:fv_video_rec0_start err\n");
            return VREC_ERR_V0_REQ_START;
        }
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("ERR:fv_video_rec1_start err\n");
            return VREC_ERR_V1_REQ_START;
        }
    }

    video_rec_start_isp_scenes();
    net_fps_fps_bitrate_ctrl_init();

    if (__this->timer_handler == 0) {
        __this->timer_handler = sys_timer_add(NULL, net_video_timer_hdl, 200);
    }

    return 0;
}


static int fv_video_rec_stop(void)
{
    union video_req req = {0};
    int err;

    video_rec_stop_isp_scenes(2, 0);

    if (__this->timer_handler) {
        sys_timer_del(__this->timer_handler);
        __this->timer_handler = 0;
    }

    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        req.rec.channel = 1;  /* video0的sd卡录像为:channel0,所以这里不能在占用channel0 */
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        req.rec.channel = 0;  /* video1的sd卡录像为:channel1,所以这里不能在占用channel1 */
    }

    req.rec.state = VIDEO_STATE_STOP;
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("ERR:stop video rec0 err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("ERR:stop video rec1 err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }
    }

    return 0;
}


static int fv_video_rec_close(void)
{
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        if (__this->video_rec0) {
            server_close(__this->video_rec0);
            __this->video_rec0 = NULL;
        }
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        if (__this->video_rec1) {
            server_close(__this->video_rec1);
            __this->video_rec1 = NULL;
        }
    }

#if 1
    if (__this->v0_fbuf) {
        free(__this->v0_fbuf);
        __this->v0_fbuf = NULL;
    }
    if (__this->audio_buf) {
        free(__this->audio_buf);
        __this->audio_buf = NULL;
    }
#endif

    return 0;
}


void fenice_video_type(int type)
{
    strm_source_type = type;
}

void fenice_video_rec_open(void)
{
    int ret;

    ret = fv_video_rec_open();
    if (ret) {
        printf("ERR:fv_video_rec_open fail\n");
    }
}

void fenice_video_rec_start(void)
{
    int ret;

    ret = fv_video_rec_start();
    if (ret) {
        printf("ERR:fv_video_rec_start fail\n");
    }
}

void fenice_video_rec_stop(void)
{
    int ret;

    ret = fv_video_rec_stop();
    if (ret) {
        printf("ERR:fv_video_rec_stop fail\n");
    }
}

void fenice_video_rec_close(void)
{
    int ret;

    ret = fv_video_rec_close();
    if (ret) {
        printf("ERR:fv_video_rec_close fail\n");
    }
}

/* 用于开启实时流时,stream_media_server回调 */
int fenice_video_rec_setup(void)
{
#if 1
    fenice_video_rec_open();
    fenice_video_rec_start();
#else //单路
    u32 mark = 0;
    struct intent it;
    init_intent(&it);
    struct rt_stream_app_info info;

    it.name = "video_rec";
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        it.action = ACTION_VIDEO0_OPEN_RT_STREAM;
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        it.action = ACTION_VIDEO1_OPEN_RT_STREAM;
    }
    info.width = REC_PIX_W;
    info.height = REC_PIX_H;
    info.fps    = REC_FPS;
    info.type = STRM_VIDEO_FMT_MOV;
    info.priv = NULL;

    mark = 2;
    it.data = (const char *)&mark;//打开视频
    it.exdata = (u32) &info; //视频参数

    start_app(&it);

#endif


    return 0;
}

/* 用于实时流异常退出时,stream_media_server回调 */
int fenice_video_rec_exit(void)
{
#if 1
    fenice_video_rec_stop();
    fenice_video_rec_close();
#else

    u32 mark = 2;
    struct intent it;
    init_intent(&it);

    it.name = "video_rec";
    if (strm_source_type == STRM_SOURCE_VIDEO0) {
        it.action = ACTION_VIDEO0_CLOSE_RT_STREAM;
    } else if (strm_source_type == STRM_SOURCE_VIDEO1) {
        it.action = ACTION_VIDEO1_CLOSE_RT_STREAM;
    }
    it.data = (char *)&mark;
    start_app(&it);

#endif


    return 0;
}

int fenice_video_rec_info(struct fenice_source_info *info)
{
    REC_PIX_W = rec_pix_w[2];
    REC_PIX_H = rec_pix_h[2];
    REC_FPS = 0;
    SAMPLE_RATE = 8000;
    CHANNEL_NUM = 1;

    info->width = REC_PIX_W;
    info->height = REC_PIX_H;
    info->fps = REC_FPS;

    info->sample_rate = SAMPLE_RATE;
    info->channel_num = CHANNEL_NUM;

    return 0;
}




