#ifndef __VIDEO_REC_H_
#define __VIDEO_REC_H_

#include "system/includes.h"
#include "server/video_server.h"
#include "app_config.h"

enum VIDEO_REC_STA {
    VIDREC_STA_IDLE,
    VIDREC_STA_START,
    VIDREC_STA_STARTING,
    VIDREC_STA_STOP,
    VIDREC_STA_STOPING,
    VIDREC_STA_FORBIDDEN,
};

enum vrec_err_code {
    VREC_ERR_NONE,
    VREC_ERR_V0_SERVER_OPEN,
    VREC_ERR_V1_SERVER_OPEN,
    VREC_ERR_START_FREE_SPACE,
    VREC_ERR_SAVE_FREE_SPACE,

    VREC_ERR_V0_REQ_START,
    VREC_ERR_V1_REQ_START,
    VREC_ERR_V0_REQ_STOP,
    VREC_ERR_V1_REQ_STOP,
    VREC_ERR_V0_REQ_SAVEFILE,
    VREC_ERR_V1_REQ_SAVEFILE,

    VREC_ERR_PKG,
    VREC_ERR_MKDIR,

};

enum VIDEO_DISP_STA {
    DISP_BIG_SMALL,     //大小窗
    DISP_HALF_WIN,      //各半屏
    DISP_FRONT_WIN,     //只前窗
    DISP_BACK_WIN,     //只后窗
    DISP_PARK_WIN,     //parking win
    DISP_RESET_WIN,
    DISP_FORBIDDEN,     //forbidden
};


struct video_rec_sys_info {

    u32 ctime;
    u32 tlp_time;
    u8 pixel_sel;
    u8 photo_pixel_sel;
    u8 osd_on;
    u8 voice_on;
    u8 ctime_sel;
    u8 mdet_on;
    u8 tlp_time_sel;
    u8 park_guad;
    s8 exposure_val;
    s8 wb_val;
    u8 double_rec;

};


struct video_menu_sta {

    char video_resolution;
    char video_double_route;
    char video_mic;
    char video_gravity;
    char video_motdet;
    char video_park_guard;
    char video_wdr;
    char video_cycle_rec;
    char video_net;
    char video_car_num;
    char video_dat_label;
    char video_white_balance;
    char video_exposure;
    char video_gap;
    char car_num_str[20];
};

struct video_rec_hdl {
    enum VIDEO_REC_STA state;
    enum VIDEO_DISP_STA disp_state;
    struct server *ui;
    struct server *video_rec0;
    struct server *video_rec1;
    struct server *video_rec2;
    struct server *video_display_0;
    struct server *video_display_1;
    struct server *video_display_2;
    struct server *video_engine;


    struct vfscan *fscan[3];
    u16 file_number[3];
    u16 old_file_number[3];

    u32 total_size;
    u32 total_lock_fsize;

    u8 *v0_fbuf;
    u8 *v1_fbuf;
    u8 *v2_fbuf;
    u8 *cap_buf;
    u8 *audio_buf;

    u32 start_jiffies;
    void *file[3];
    FILE *new_file[3];
    char fname[3][MAX_FILE_NAME_LEN];

    u16 wait;
    u16 park_wait;
    u16 avin_wait;
    u16 sd_wait;

    u8 photo_camera_sel;
    u8 exposure_set;
    u8 white_balance_set;
    u8 run_rec_start;

    u8 lock_fsize_count;
    /*u8 avin_dev;*/
    u8 video1_online;
    u8 video2_online;
    int uvc_id;
    u8 avin_cnt;
    u8 menu_inout;
    u8 isp_scenes_status;
    u8 gsen_lock;
    u8 park_wakeup;
    u8 ui_get_config;

    u16 car_head_y;//lane det parm
    u16 vanish_y;
    u16 len_factor;
    u8  lan_det_setting;
    u8 save_file;
};








extern u16 AVIN_WIDTH;
extern u16 AVIN_HEIGH;
extern u16 UVC_ENC_WIDTH;
extern u16 UVC_ENC_HEIGH;


extern void video_rec_config_init();
extern int video_rec_set_white_balance();
extern int video_rec_set_exposure(u32 exp);
extern void mic_set_toggle();

extern char video_rec_osd_buf[64] ;

extern int get_parking_status();

extern int video0_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
extern int show_lane_set_ui();
extern u32 get_video_disp_state();


extern void ve_server_reopen();
extern void video_rec_fun_restore();
#endif

