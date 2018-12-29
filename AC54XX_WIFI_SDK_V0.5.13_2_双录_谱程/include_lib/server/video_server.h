#ifndef VIDEO_SERVER_H
#define VIDEO_SERVER_H

#include "server/server_core.h"
#include "server/vpkg_server.h"
#include "system/includes.h"


#define VIDEO_SERVER_PKG_ERR       0x01
#define VIDEO_SERVER_PKG_END       0x02
#define VIDEO_SERVER_PKG_NET_ERR   0x03


enum video_state {
    VIDEO_STATE_START,
    VIDEO_STATE_PAUSE,
    VIDEO_STATE_RESUME,
    VIDEO_STATE_STOP,
    VIDEO_STATE_SAVE_FILE,
    VIDEO_STATE_SET_OSD,
    VIDEO_STATE_SET_VOICE,
    VIDEO_STATE_GET_INFO,
    VIDEO_STATE_PKG_MUTE,
    VIDEO_STATE_SET_OSD_STR,
    VIDEO_STATE_CFG,
    VIDEO_STATE_CFG_ROI,
    VIDEO_STATE_STOP_COUNT_DOWN,
    VIDEO_STATE_SET_DR,
    VIDEO_STATE_CAP_IFRAME,
    VIDEO_STATE_RESET_BITS_RATE,
    VIDEO_STATE_PKG_NETVRT_MUTE,
    VIDEO_STATE_PKG_NETART_MUTE,
    VIDEO_STATE_CYC_FILE,
    VIDEO_STATE_TOTAL_FRAME,
    VIDEO_STATE_NET_CYC,
    VIDEO_STATE_SMALL_PIC,
};

enum video_rec_format {
    VIDEO_FMT_AVI,
    VIDEO_FMT_MOV,
    VIDEO_FMT_YUYV,
    VIDEO_FMT_NV12,
    NET_VIDEO_FMT_AVI,
    NET_VIDEO_FMT_MOV,
    STRM_VIDEO_FMT_AVI,
    STRM_VIDEO_FMT_MOV,
};

enum net_format {
    NET_FMT_TCP,
    NET_FMT_UDP,
    NET_FMT_RAW,
    NET_FMT_RF,
};

struct vs_audio {
    u8 data_source;
    u8 channel;
    u8 volume;
    u8 kbps;
    u16 sample_rate;
    u8 *buf;
    int buf_len;
    const char *fname;
    int type;
    u16 sample_inv_size;
};
struct net_rec_par {

    u8 net_type;
    u32 total_frame;
    u32 net_vidrt_onoff;
    u32 net_audrt_onoff;
    u8 *small_pic_buf;
    int  small_pic_len;
    u32 small_pic_first_start;
    u8 video_osd_buf[64];
    u32 osd_line_num;
    char netpath[64];
};

struct vs_video_rec {
    u8  state;
    u8  format;
    u8  channel;
    u8 fps;                         //需要录的帧率
    u8 slow_motion;                 //慢动作倍数(与延时摄影互斥,无音频)
    u8 camera_type;
    u8 stream_type;
    u8 uvc_id;
    u8 out_obj;
    u8 rec_small_pic;
    u16 width;
    u16 height;
    u8 *buf;
    char *new_osd_str;
    u32 cycle_time;
    u32 cyc_file;
    u32 count_down;
    u32 buf_len;
    u32 tlp_time;                   //延时录像的间隔ms(与慢动作互斥,无音频)
    u32 abr_kbps;
    u32 IP_interval;                //max IP interval
    struct drop_fps *targe_fps;
    u32 delay_limit;
    struct roi_cfg roi;
    u32 fsize;
    const char *fpath;
    const char *fname;
    const char *usb_cam_name;
    enum video_rec_quality quality;//(图片质量(高中低))
    struct vs_audio audio;
    struct imc_osd_info *osd;
    struct vpkg_get_info get_info;
    struct vpkg_aud_mute pkg_mute;
    int (*camera_config)(u32 lv, void *arg);
    struct net_rec_par net_par;
};


struct vs_video_display {
    u16 left;
    u16 top;
    u16 width;
    u16 height;
    u16 border_left;
    u16 border_top;
    u16 border_right;
    u16 border_bottom;
    u8  camera_type;
    u8  uvc_id;
    u8 stream_type;
    const char *fb;
    struct imc_presca_ctl *pctl;
    enum video_state state;
    int (*camera_config)(u32 lv, void *arg);
};

struct vs_image_capture {
    u16 width;
    u16 height;
    u8 camera_type;
    u8 uvc_id;
    u8 out_obj;
    u32 quality;
    u8  *buf;
    u32 buf_size;
    struct icap_auxiliary_mem *aux_mem;
    struct jpg_thumbnail *thumbnails;
    struct imc_osd_info *label;
    struct image_sticker *sticker;
    const char *path;
    struct jpg_q_table *qt;
    char *file_name;
};

#define  SET_CAMERA_MODE        BIT(0)
#define  SET_CAMERA_EV          BIT(1)
#define  SET_CAMERA_WB          BIT(2)
#define  SET_CAMERA_SHP_LEVEL   BIT(3)
#define  SET_CAMERA_DRC_ENABLE  BIT(4)
#define  GET_CAMERA_LV          BIT(5)
#define  GET_CAMERA_INFO        BIT(6)
#define  SET_CUSTOMIZE_CFG      BIT(7)
#define  GET_CUSTOMIZE_CFG      BIT(8)

struct vs_camera_effect {
    u8 mode;
    s8 ev;
    u8 white_blance;
    u8 shpn_level;
    u8 drc;
    u32 cmd;
    int lv;
    void *customize_cfg;
};

struct vs_camera_sca {
    u8 step;
    u8 max_sca;// max sca ratio
    u8 sca_modify;//1 big, 0 small
};

struct vs_image_info {
    u8 *buf;
    u32 size;
};

#define VIDEO_TO_USB    0x10
/*
 * video_server支持的请求命令列表，每个请求命令对应union video_req中的一个结构体
 */
enum {
    VIDEO_REQ_REC,
    VIDEO_REQ_DISPLAY,
    VIDEO_REQ_IMAGE_CAPTURE,
    VIDEO_REQ_CAMERA_EFFECT,
    VIDEO_REQ_CAMERA_SCA,
    VIDEO_REQ_GET_IMAGE,
};

union video_req {
    struct vs_video_rec rec;
    struct vs_image_capture icap;
    struct vs_video_display display;
    struct vs_camera_effect camera;
    struct vs_camera_sca sca;
    struct vs_image_info image;
};































#endif

