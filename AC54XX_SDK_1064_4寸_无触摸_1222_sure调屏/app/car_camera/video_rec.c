#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_engine_server.h"
#include "ui_ctr_def.h"
#include "video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"
#include "asm/lcd_config.h"
#include "user_isp_cfg.h"


#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "power_ctrl.h"
#include "app_database.h"
#include "video_rec.h"

extern int video_rec_set_config(struct intent *it);
extern int video_rec_get_config(struct intent *it);
extern const u16 rec_province_gb2312[];
extern const char rec_num_table[];

struct video_rec_hdl rec_handler;

#ifdef CONFIG_TOUCH_UI_ENABLE
#define SCREEN_H_OFFSET (720 - SCREEN_H)
#else
#define SCREEN_H_OFFSET (0)
#endif



char video_rec_osd_buf[64] ALIGNE(64);

#define __this 	(&rec_handler)
#define sizeof_this     (sizeof(struct video_rec_hdl))


#define VREC0_FBUF_SIZE     (7*1024*1024)
#define VREC1_FBUF_SIZE     (3*1024*1024)
#define VREC2_FBUF_SIZE     (3*1024*1024)
#define AUDIO_BUF_SIZE     (300*1024)


#define SCREEN_W        LCD_DEV_WIDTH//1280//480     //16 aline
#define SCREEN_H        LCD_DEV_HIGHT//720//272     //16 aline
#define SMALL_SCREEN_W          320 //16 aline
#define SMALL_SCREEN_H          240 //16 aline

#define MOTION_STOP_SEC         20
#define MOTION_START_SEC        2

#define DEL_LOCK_FILE_EN		0
#define LOCK_FILE_PERCENT		40    //0~100

#ifdef CONFIG_TOUCH_UI_ENABLE
#define SCREEN_H_OFFSET (720 - SCREEN_H)
#else
#define SCREEN_H_OFFSET (0)
#endif


extern u8 home_rec_touch;

extern int storage_device_ready();
static int video_rec_control(void *_run_cmd);
static int video_rec_stop(u8 close);
static int video_disp_control();
static int ve_mdet_start();
static int ve_mdet_stop();
static int ve_lane_det_start(u8 fun_sel);
static int ve_lane_det_stop(u8 fun_sel);
int video0_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
static int video1_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
static int video2_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
static void video0_disp_stop();
static void video1_disp_stop();
static void video2_disp_stop();
static int video_rec_start_isp_scenes();
static int video_rec_stop_isp_scenes(u8 status, u8 restart);
static void video_rec_cycle(void *parm);

static int video_rec_get_abr(u32 width);
static int video_rec_savefile(int dev_id);
static void video_rec_rename_file(int id, FILE *file, int fsize);
static int video_rec_cmp_fname(void *afile, void *bfile);

static u8 page_main_flag = 0;
static u8 page_park_flag = 0;
static u8 move_start = 0;  //???????????????

#if DEL_LOCK_FILE_EN
static int video_rec_del_lock_file();
#endif
u32 get_video_disp_state()
{
    return __this->disp_state;
}
static void video_home_post_msg(const char *msg, ...)
{
#ifdef CONFIG_TOUCH_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_MAIN_PAGE;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);

#endif
}
static void video_dzg_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_DZG;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);

#endif
}
void video_parking_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_PARKING;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif
}
void video_rec_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_VIDEO_REC;
        req.msg.msg = msg;
        req.msg.exdata = argptr;

        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);

#endif

}
static const u16 rec_pix_w[] = {1920, 1280, 640};
static const u16 rec_pix_h[] = {1296, 720,  480};
static const u16 pic_pix_w[] = {1280, 1920, 2560, 3072};
static const u16 pic_pix_h[] = {720,  1088, 1600, 2208};

static void rec_dev_server_event_handler(void *priv, int argc, int *argv)
{
    /*
     *该回调函数会在录像过程中，写卡出错被当前录像APP调用，例如录像过程中突然拔卡
     */
    switch (argv[0]) {
    case VIDEO_SERVER_PKG_ERR:
        puts("\n\n**************APP_VREC_DEAL_ERR**********\n\n");

        if ((__this->video1_online) || (__this->video2_online)) {
            if (__this->state == VIDREC_STA_STOP) {
                if (__this->sd_wait) {
                    wait_completion_del(__this->sd_wait);
                    __this->sd_wait = 0;
                }
                __this->sd_wait = wait_completion(storage_device_ready, video_rec_control, NULL);
                break;
            }
        }

        video_rec_post_msg("offREC");
	  video_dzg_post_msg("offREC");
        video_home_post_msg("offREC");
        video_parking_post_msg("offREC");

        if (__this->state == VIDREC_STA_START) {
            video_rec_stop(0);
        }

        if ((__this->video1_online == 0) && (__this->video2_online == 0)) {
            if (__this->state == VIDREC_STA_STOP) {
                if (__this->sd_wait) {
                    wait_completion_del(__this->sd_wait);
                    __this->sd_wait = 0;
                }
                __this->sd_wait = wait_completion(storage_device_ready, video_rec_control, NULL);
                break;
            }
        }
        break;
    case VIDEO_SERVER_PKG_END:
        video_rec_savefile((int)priv);
        break;
    default :
        puts("\n\nwrong rec server cmd !\n\n");
        break;
    }
}

extern void play_voice_file(const char *file_name);

static void ve_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case VE_MSG_MOTION_DETECT_STILL:
        /*
         *录像时，移动侦测打开的情况下，画面基本静止20秒，则进入该分支
         */
        printf("\n\n**************VE_MSG_MOTION_DETECT_STILL**********\n\n");
        if (!db_select("mot") || (__this->menu_inout)) {
            return;
        }
        if(move_start)
        {
            if (__this->state == VIDREC_STA_START) {
            video_rec_control(0);
            }
            //pmsg("VE_MSG_MOTION_DETECT_STILL\n");
            move_start = 0;
        }

        break;
    case VE_MSG_MOTION_DETECT_MOVING:
        /*
         *移动侦测打开，当检测到画面活动一段时间，则进入该分支去启动录像
         */
        printf("\n\n**************VE_MSG_MOTION_DETECT_MOVING**********\n\n");
        if (!db_select("mot") || (__this->menu_inout)) {
            return;
        }
        if ((__this->state == VIDREC_STA_STOP) || (__this->state == VIDREC_STA_IDLE)) {
            move_start = 1;
            video_rec_control(0);
        }

        break;
    case VE_MSG_LANE_DETECT_WARNING:

        if (!__this->lan_det_setting) {
            if (!db_select("lan")) {
                return;
            }
        }

        play_voice_file("mnt/spiflash/audlogo/lane.adp");

        puts("\n==8i==========lane dete waring=====\n");
        break;
    case VE_MSG_LANE_DETCET_LEFT:
        puts("==lane dete waring==l\n");
        break;
    case VE_MSG_LANE_DETCET_RIGHT:
        puts("==lane dete waring==r\n");
        break;

    default :
        break;
    }
}

/*
 *智能引擎服务打开，它包括移动侦测等一些功能,在打开这些功能之前，必须要打开这个智能引擎服务
 */
static s32 ve_server_open(u8 fun_sel)
{
    if (!__this->video_engine) {
        __this->video_engine = server_open("video_engine_server", NULL);
        if (!__this->video_engine) {
            puts("video_engine_server:faild\n");
            return -1;
        }

        server_register_event_handler(__this->video_engine, NULL, ve_server_event_handler);

        struct video_engine_req ve_req;
        ve_req.module = 0;
        ve_req.md_mode = 0;
        ve_req.cmd = 0;
        if (db_select("mot") && db_select("lan")) {
            ve_req.hint_info.hint = ((1 << VE_MODULE_MOTION_DETECT) | (1 << VE_MODULE_LANE_DETECT));
        } else if (db_select("mot")) {
            ve_req.hint_info.hint = (1 << VE_MODULE_MOTION_DETECT);
        } else if (db_select("lan")) {
            ve_req.hint_info.hint = (1 << VE_MODULE_LANE_DETECT);
        }

        if (fun_sel) {
            ve_req.hint_info.hint = 0;
            ve_req.hint_info.hint = (1 << VE_MODULE_LANE_DETECT);
        }


        ve_req.hint_info.mode_hint0 = (VE_MOTION_DETECT_MODE_ISP
                                       << (VE_MODULE_MOTION_DETECT * 4));
        ve_req.hint_info.mode_hint1 = 0;
        server_request(__this->video_engine, VE_REQ_SET_HINT, &ve_req);
    }

    if (fun_sel) {
        ve_lane_det_start(1);
    } else {
        __this->car_head_y = db_select("lan") & 0x0000ffff;
        __this->vanish_y   = (db_select("lan") >> 16) & 0x0000ffff;
        ve_mdet_start();
        ve_lane_det_start(0);
    }

    return 0;
}

static s32 ve_server_close()
{
    if (__this->video_engine) {

        if (!__this->lan_det_setting) {
            ve_mdet_stop();
        }
        ve_lane_det_stop(0);

        server_close(__this->video_engine);

        __this->video_engine = NULL;
    }
    return 0;
}

static int ve_mdet_start()
{
    struct video_engine_req ve_req;

    if ((__this->video_engine == NULL) || !db_select("mot")) {
        return -EINVAL;
    }

    ve_req.module = VE_MODULE_MOTION_DETECT;
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
    ve_req.cmd = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_OPEN, &ve_req);


    server_request(__this->video_engine, VE_REQ_MODULE_GET_PARAM, &ve_req);


    /*
    *移动侦测的检测启动时间和检测静止的时候
    **/
    ve_req.md_params.level = 0;
    ve_req.md_params.move_delay_ms = MOTION_START_SEC * 1000;
    ve_req.md_params.still_delay_ms = MOTION_STOP_SEC * 1000;
    server_request(__this->video_engine, VE_REQ_MODULE_SET_PARAM, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_START, &ve_req);

    return 0;
}


static int ve_mdet_stop()
{
    struct video_engine_req ve_req;

    if ((__this->video_engine == NULL) || !db_select("mot")) {
        return -EINVAL;
    }

    ve_req.module = VE_MODULE_MOTION_DETECT;
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
    ve_req.cmd = 0;
    server_request(__this->video_engine, VE_REQ_MODULE_STOP, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_CLOSE, &ve_req);
    return 0;
}

static void ve_mdet_reset()
{
    ve_mdet_stop();
    ve_mdet_start();
}


static int ve_lane_det_start(u8 fun_sel)
{
    struct video_engine_req ve_req;

    if (!fun_sel) {
        if ((__this->video_engine == NULL) || !db_select("lan")) {
            return -EINVAL;
        }
    }

    ve_req.module = VE_MODULE_LANE_DETECT;
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
    ve_req.cmd = 0;
    server_request(__this->video_engine, VE_REQ_MODULE_OPEN, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_GET_PARAM, &ve_req);

    /**
     *轨道偏移 配置车头 位置，视线结束为止，以及车道宽度
     * */
    /* ve_req.lane_detect_params.car_head_y = 230; */
    /* ve_req.lane_detect_params.vanish_y = 170; */
    /* ve_req.lane_detect_params.len_factor = 0; */
    ve_req.lane_detect_params.car_head_y = __this->car_head_y;
    ve_req.lane_detect_params.vanish_y = __this->vanish_y;
    ve_req.lane_detect_params.len_factor = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_SET_PARAM, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_START, &ve_req);

    return 0;
}


static int ve_lane_det_stop(u8 fun_sel)
{
    struct video_engine_req ve_req;

    if (!fun_sel) {
        if ((__this->video_engine == NULL) || !db_select("lan")) {
            return -EINVAL;
        }
    }

    ve_req.module = VE_MODULE_LANE_DETECT;
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
    ve_req.cmd = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_STOP, &ve_req);

    server_request(__this->video_engine, VE_REQ_MODULE_CLOSE, &ve_req);

    return 0;
}

void ve_lane_det_reset()
{
    ve_lane_det_stop(0);
    ve_lane_det_start(0);
}

void ve_server_reopen()
{
    if (!__this->run_rec_start) {
        puts("ve_server_reopen\n");
        return;
    }

    ve_mdet_stop();
    ve_lane_det_stop(0);

    ve_server_close();
    ve_server_open(0);
}

/*
 *根据录像不同的时间和分辨率，设置不同的录像文件大小
 */
static u32 video_rec_get_fsize(u8 cycle_time, u16 vid_width)
{
    u32 fsize;
    //
    //8250 = 1000*60/8 *110 /100 ;
    //
    if (cycle_time == 0) {
        cycle_time = 5;
    }
    fsize  = ((video_rec_get_abr(vid_width)) * cycle_time) * 8250;

    return fsize  ;

    if (cycle_time <= 3) {
        if (vid_width <= 720) {
            fsize = 150 * 1024 * 1024;
        } else if (vid_width <= 1280) {
            fsize = 250 * 1024 * 1024;
        } else if (vid_width <= 1920) {
            fsize = 450 * 1024 * 1024;
        } else {
            fsize = 800 * 1024 * 1024;
        }
    } else if (cycle_time <= 5) {
        if (vid_width <= 720) {
            fsize = 150 * 1024 * 1024;
        } else if (vid_width <= 1280) {
            fsize = 300 * 1024 * 1024;
        } else if (vid_width <= 1920) {
            fsize = 700 * 1024 * 1024;
        } else {
            fsize = 1400 * 1024 * 1024;
        }
    } else if (cycle_time <= 8) {
        if (vid_width <= 720) {
            fsize = 250 * 1024 * 1024;
        } else if (vid_width <= 1280) {
            fsize = 500 * 1024 * 1024;
        } else if (vid_width <= 1920) {
            fsize = 1200 * 1024 * 1024;
        } else {
            fsize = 2300UL * 1024 * 1024;
        }
    } else {
        if (vid_width <= 720) {
            fsize = 300 * 1024 * 1024;
        } else if (vid_width <= 1280) {
            fsize = 700 * 1024 * 1024;
        } else if (vid_width <= 1920) {
            fsize = 1400 * 1024 * 1024;
        } else {
            fsize = 2800UL * 1024 * 1024;
        }
    }

    if (__this->park_wakeup) {
        if (vid_width <= 720) {
            fsize = 50 * 1024 * 1024;
        } else if (vid_width <= 1280) {
            fsize = 90 * 1024 * 1024;
        } else if (vid_width <= 1920) {
            fsize = 160 * 1024 * 1024;
        } else {
            fsize = 200 * 1024 * 1024;
        }

    }

    return fsize;
}

/*
 *这里要注意！！！这个是录像前要提前判断一下是否足够空间录，和录像的大小有关系（video_rec_get_fsize），
 * 返回的空间值必须要比将要录的文件大或者等于,如果是双路，要比双路的两个文件加起来的空间大
 */
static int video_rec_get_need_space()
{
    u32 need_space;
    u8 tsel = db_select("cyc");

    switch (tsel) {
    case 3:
        need_space = 500 * 1024;
        break;
    case 5:
        need_space = 900 * 1024;
        break;
    case 10:
        need_space = 1800 * 1024;
        break;
    default:
        need_space = 900 * 1024;
        break;
    }
    if (__this->park_wakeup) {
        switch (tsel) {
        case 3:
            need_space = 70 * 1024;
            break;
        case 5:
            need_space = 100 * 1024;
            break;
        case 10:
            need_space = 200 * 1024;
            break;
        default:
            need_space = 100 * 1024;
            break;
        }

    }
    return need_space + 300 * 1024;
}


static void video_rec_fscan_dir(int id, const char *path)
{
    char *str;
    int del_lock_file = 0;

#if DEL_LOCK_FILE_EN
    del_lock_file = video_rec_del_lock_file();
#endif

    str = del_lock_file ? "-tMOVAVI -sn" : "-tMOVAVI -sn -a/r";

    if (__this->fscan[id]) {
        if (__this->old_file_number[id] == 0) {
            puts("--------delete_all_scan_file\n");
            fscan_release(__this->fscan[id]);
            __this->fscan[id] = NULL;
        }
    }

    if (!__this->fscan[id]) {
        __this->fscan[id] = fscan(path, str);
        if (!__this->fscan[id]) {
            __this->old_file_number[id] = 0;
        } else {
            __this->old_file_number[id] = __this->fscan[id]->file_number;
        }
        __this->file_number[id] = __this->old_file_number[id];
    }
}

static FILE *video_rec_get_first_file(int id)
{
    int max_index = -1;
    int max_file_number = 0;

#ifdef CONFIG_VIDEO0_ENABLE
    video_rec_fscan_dir(0, CONFIG_REC_PATH_1);
#endif
#ifdef CONFIG_VIDEO1_ENABLE
    video_rec_fscan_dir(1, CONFIG_REC_PATH_2);
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    video_rec_fscan_dir(2, CONFIG_REC_PATH_2);
#endif

    for (int i = 0; i < 3; i++) {
        if (__this->fscan[i]) {
            if (max_file_number < __this->file_number[i]) {
                max_file_number = __this->file_number[i];
                max_index = i;
            }
        }
    }

    if (max_index < 0) {
        return NULL;
    }
    if (max_index != id && id >= 0) {
        /* ?????????????????? */
        if (__this->file_number[id] >= 5 &&
            __this->file_number[id] + 5 > __this->file_number[max_index]) {
            max_index = id;
        }
    }

    FILE *f = fselect(__this->fscan[max_index], FSEL_FIRST_FILE, 0);
    if (f) {

        if (video_rec_cmp_fname(__this->file[max_index], f)) {
            puts("\n\n\n\ndelout same_file!!!\n\n\n");
            fclose(f);
            return NULL;
        }

        __this->old_file_number[max_index]--;
    }

    return f;
}

static void video_rec_rename_file(int id, FILE *file, int fsize)
{
    char file_name[32];

    if (id == 0) {
        strcpy(file_name, "DCIM/1/vid_***.mov");
    } else {
        strcpy(file_name, "DCIM/2/vid_***.mov");
    }

    int err = fcheck(file);
    if (err) {
        puts("\n\nfcheck fail\n\n");
        fdelete(file);
        return;
    }

    int present = (flen(file) / 1024) * 100 / (fsize / 1024);
    if (present >= 90 && present <= 110) {
        int err = fmove(file, file_name, &__this->new_file[id], 1);
        if (err == 0) {
            return;
        }
    }

    puts("\n\ndel file\n\n");
    fdelete(file);
}

static int video_rec_del_old_file()
{
    int err;
    FILE *file;
    int fsize[3];
    u32 cur_space;
    u32 need_space = 0;
    int cyc_time = db_select("cyc");

#ifdef CONFIG_VIDEO0_ENABLE
    fsize[0] =  video_rec_get_fsize(cyc_time, rec_pix_w[db_select("res")]);
    need_space += fsize[0];
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    fsize[1] =  video_rec_get_fsize(cyc_time, AVIN_WIDTH);
    need_space += fsize[1];
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    fsize[2] =  video_rec_get_fsize(cyc_time, UVC_ENC_WIDTH);
    need_space += fsize[2];
#endif


    err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    if (err) {
        return err;
    }

    printf("space: %x, %x\n", cur_space / 1024, need_space / 1024 / 1024);

    if (cur_space >= (need_space / 1024) + 500 * 1024) {
        return 0;
    }

    while (1) {
        if (cur_space >= (need_space / 1024) + 500 * 1024) {
            puts("cur_space > 500M\n");
            break;
        }
        file = video_rec_get_first_file(-1);
        if (!file) {
            return -ENOMEM;
        }

        fdelete(file);
        fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    }

#ifdef CONFIG_VIDEO0_ENABLE
    if (__this->video_rec0) {
        file = video_rec_get_first_file(0);
        if (file) {
            video_rec_rename_file(0, file, fsize[0]);
        }
    }
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    if (__this->video_rec1) {
        file = video_rec_get_first_file(1);
        if (file) {
            video_rec_rename_file(1, file, fsize[1]);
        }
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video_rec2) {
        file = video_rec_get_first_file(2);
        if (file) {
            video_rec_rename_file(2, file, fsize[2]);
        }
    }
#endif

    return 0;
}

static int video_rec_cmp_fname(void *afile, void *bfile)
{
    int alen, blen;
    char *afname = __this->fname[0];
    char *bfname = __this->fname[1];

    if ((afile == NULL) || (bfile == NULL)) {
        return 0;
    }

    alen = fget_name(afile, (u8 *)afname, MAX_FILE_NAME_LEN);
    if (alen) {
        ASCII_ToUpper(afname, alen);
    }

    blen = fget_name(bfile, (u8 *)bfname, MAX_FILE_NAME_LEN);
    if (blen) {
        ASCII_ToUpper(bfname, blen);
    }

    printf("\nafname : %s, bfname : %s\n", afname, bfname);

    if (!strcmp(afname, bfname)) {
        return 1;
    }

    return 0;
}

#if DEL_LOCK_FILE_EN

static int video_rec_scan_lock_fszie()
{
    FILE *file;
    int attr;
    u8 sel_mode;

    if (__this->lock_fsize_count == 0) {

        __this->total_lock_fsize = 0;

#ifdef CONFIG_VIDEO0_ENABLE
        sel_mode = FSEL_FIRST_FILE;
        do {
            if (__this->fscan[0] == NULL) {
                __this->fscan[0] = fscan(CONFIG_REC_PATH_1, "-tMOVAVI -sn -ar");
                if (__this->fscan[0] == NULL) {
                    break;
                }
            }
            file = fselect(__this->fscan[0], sel_mode, 0);
            if (file) {
                __this->total_lock_fsize += (flen(file) / 1024);
                sel_mode = FSEL_NEXT_FILE;
                fclose(file);//!!! must close
            } else {
                break;
            }

        } while (1);

        if (__this->fscan[0]) {
            fscan_release(__this->fscan[0]);
            __this->fscan[0] = NULL;
        }
#endif

#ifdef CONFIG_VIDEO1_ENABLE
        sel_mode = FSEL_FIRST_FILE;
        do {
            if (__this->fscan[1] == NULL) {
                __this->fscan[1] = fscan(CONFIG_REC_PATH_2, "-tMOVAVI -sn -ar");
                if (__this->fscan[1] == NULL) {
                    break;
                }
            }
            file = fselect(__this->fscan[1], sel_mode, 0);
            if (file) {
                __this->total_lock_fsize += (flen(file) / 1024);
                sel_mode = FSEL_NEXT_FILE;
                fclose(file);//!!! must close
            } else {
                break;
            }

        } while (1);

        if (__this->fscan[1]) {
            fscan_release(__this->fscan[1]);
            __this->fscan[1] = NULL;
        }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
        sel_mode = FSEL_FIRST_FILE;
        do {
            if (__this->fscan[2] == NULL) {
                __this->fscan[2] = fscan(CONFIG_REC_PATH_2, "-tMOVAVI -sn -ar");
                if (__this->fscan[2] == NULL) {
                    break;
                }
            }
            file = fselect(__this->fscan[2], sel_mode, 0);
            if (file) {
                __this->total_lock_fsize += (flen(file) / 1024);
                sel_mode = FSEL_NEXT_FILE;
                fclose(file);//!!! must close
            } else {
                break;
            }

        } while (1);

        if (__this->fscan[2]) {
            fscan_release(__this->fscan[2]);
            __this->fscan[2] = NULL;
        }
#endif

        __this->lock_fsize_count = 1;
    }

    return __this->total_lock_fsize;
}

static int video_rec_del_lock_file()
{
    if (__this->lock_fsize_count == 0) {
        return 0;
    }

    /* if ((__this->total_lock_fsize + __this->total_size / 6) > __this->total_size) { */
    if (LOCK_FILE_PERCENT <= 0 || LOCK_FILE_PERCENT >= 100) {
        return 0;
    }
    if (__this->total_lock_fsize > __this->total_size * LOCK_FILE_PERCENT / 100) {
        return 1;
    }
    return 0;
}

static void video_rec_add_lock_fsize(void *file)
{
    if (__this->lock_fsize_count == 0) {
        return ;
    }
    __this->total_lock_fsize += (flen(file) / 1024);
}

#endif

/*
 *设置保护文件，必须要在关闭文件之前调用
 */
static int video_rec_lock_file(void *file, u8 lock)
{
    int attr;

    if (!file) {
        puts("lock file null\n");
        return -1;
    }

    fget_attr(file, &attr);

    if (lock) {
        if (attr & F_ATTR_RO) {
            return 0;
        }
        attr |= F_ATTR_RO;
    } else {
        if (!(attr & F_ATTR_RO)) {
            return 0;
        }
        attr &= ~F_ATTR_RO;
    }
    fset_attr(file, attr);

#if DEL_LOCK_FILE_EN
    video_rec_add_lock_fsize(file);
#endif

    return 0;
}


/*????,?????????*/
static int video_rec_get_abr(u32 width)
{
    if (width <= 720) {
        /* return 8000; */
        return 4000;
    } else if (width <= 1280) {
        return 8000;
        /* return 10000; */
    } else if (width <= 1920) {
        return 18000; // 14000
    } else {
        return 22000;
    }
}

int video_rec_get_file_name_and_size(int id,  const char **fname, u32 *fsize, int width)
{
    __this->file_number[id]++;

    if (__this->new_file[id]) {
        int len = fget_name(__this->new_file[id], (u8 *)__this->fname[id], MAX_FILE_NAME_LEN);
        if (len > 0) {
            *fname = __this->fname[id];
            *fsize = flen(__this->new_file[id]);
            fclose(__this->new_file[id]);
            __this->new_file[id] = NULL;
            return 0;
        }
        fclose(__this->new_file[id]);
        __this->new_file[id] = NULL;
    }

    *fname = "vid_***.mov";
    *fsize = video_rec_get_fsize(db_select("cyc"), width);

    return 0;
}


#ifdef CONFIG_VIDEO0_ENABLE
/******* 不要单独调用这些子函数 ********/
static int video0_rec_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;

    puts("start_video_rec0\n");
    if (!__this->video_rec0) {
        __this->video_rec0 = server_open("video_server", "video0");
        if (!__this->video_rec0) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this->video_rec0, (void *)0, rec_dev_server_event_handler);
    }

    u32 res = db_select("res");
    /*
     *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.channel = 0;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.width 	= rec_pix_w[res];
    req.rec.height 	= rec_pix_h[res];
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_1;

    /*
     *录像名字，加*表示自动增加序号，支持全英文的长文件名；帧率为0表示使用摄像头的帧率
     */
    video_rec_get_file_name_and_size(0, &req.rec.fname, &req.rec.fsize, req.rec.width);

    req.rec.quality = VIDEO_MID_Q;
    req.rec.fps 	= 0;

    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = 8000;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);



    #if(SLELE_5XX == JL5601)

        req.rec.IP_interval = 1;
    #else
        req.rec.IP_interval = 0;
    #endif

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 6 / 6 / 16) << 8 | (req.rec.width) * 0 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 12 / 12 / 16) << 8 | (req.rec.width) * 0 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 100 / 100 ; // 70
    req.rec.roi.roio_ratio1 = 256 * 100 / 100; // 90
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 100 / 100;// 80


    /*
     * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     */
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

    /*
     *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     */
    req.rec.slow_motion = 0;
    req.rec.tlp_time = db_select("gap");
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->v0_fbuf;
    req.rec.buf_len = VREC0_FBUF_SIZE;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif

    /*
     *循环录像时间，文件大小
     */
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }


    req.rec.cycle_time = req.rec.cycle_time * 60;

    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }

    /*
     *获取文件句柄
     */
    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_GET_INFO;
    server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    __this->file[0] = req.rec.get_info.file;
    //fcheck(__this->file[0]);

    video_rec_start_isp_scenes();

    return 0;
}

static int video0_rec_aud_mute()
{
    union video_req req;

    if (!__this->video_rec0) {
        return -EINVAL;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
}

static int video0_rec_stop_count_down(u32 time)
{
    union video_req req;

    if ((!__this->video_rec0) || (!__this->file[0])) {
        return -EINVAL;
    }
    req.rec.channel = 0;
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
}

static int video0_rec_stop(u8 close)
{
    union video_req req;
    int err;

    video_rec_stop_isp_scenes(2, 0);

    if (__this->gsen_lock) {
        video_rec_lock_file(__this->file[0], 1);
    }

    req.rec.channel = 0;
    req.rec.state = VIDEO_STATE_STOP;
    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nstop rec err 0x%x\n", err);
        return VREC_ERR_V0_REQ_STOP;
    }
    __this->file[0] = NULL;

    video_rec_start_isp_scenes();
    if (close) {
        if (__this->video_rec0) {
            server_close(__this->video_rec0);
            __this->video_rec0 = NULL;
        }
    }

    return 0;
}

/*
 *注意：循环录像的时候，虽然要重新传参，但是要和start传的参数保持一致！！！
 */
static int video0_rec_savefile()
{
    union video_req req;
    int err;

    if (__this->gsen_lock) {
        video_rec_lock_file(__this->file[0], 1);
    }

    u32 res = db_select("res");

    req.rec.channel = 0;
    req.rec.width 	= rec_pix_w[res];
    req.rec.height 	= rec_pix_h[res];
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_SAVE_FILE;
    req.rec.fpath 	= CONFIG_REC_PATH_1;
    req.rec.fps 	= 0;

#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif

    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    video_rec_get_file_name_and_size(0, &req.rec.fname, &req.rec.fsize, req.rec.width);

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = 8000;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nsave rec err 0x%x\n", err);
        return VREC_ERR_V0_REQ_SAVEFILE;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_GET_INFO;
    server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    __this->file[0] = req.rec.get_info.file;
    //fcheck(__this->file[0]);

    return 0;
}

static void video0_rec_close()
{
    if (__this->video_rec0) {
        server_close(__this->video_rec0);
        __this->video_rec0 = NULL;
    }
}

/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video0_rec_set_osd_str(char *str)
{
    union video_req req;
    int err;
    if (!__this->video_rec0) {
        return -1;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nset osd rec0 str err 0x%x\n", err);
        return -1;
    }

    return 0;
}

static int video0_rec_osd_ctl(u8 onoff)
{
    union video_req req;
    struct imc_osd_info osd_info;
    int err;

    if (__this->video_rec0) {

        u32 res = db_select("res");
        req.rec.width 	= rec_pix_w[res];
        req.rec.height 	= rec_pix_h[res];

        osd_info.osd_w = 16;
        osd_info.osd_h = 32;
        osd_info.x = (req.rec.width - strlen(video_rec_osd_buf) * osd_info.osd_w) / 64 * 64;
        osd_info.y = (req.rec.height - osd_info.osd_h) / 16 * 16;
        osd_info.osd_yuv = 0xe20095;
        osd_info.osd_str = video_rec_osd_buf;
        osd_info.osd_matrix_str = osd_str_total;
        osd_info.osd_matrix_base = osd_str_matrix;
        osd_info.osd_matrix_len = sizeof(osd_str_matrix);
        req.rec.osd = 0;
        if (onoff) {
            req.rec.osd = &osd_info;
        }
        req.rec.channel = 0;
        req.rec.state 	= VIDEO_STATE_SET_OSD;

        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nset osd rec0 err 0x%x\n", err);
            return -1;
        }
    }

    return 0;
}
#endif



int video0_disp_start(u16 width, u16 height, u16 xoff, u16 yoff)
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO0_ENABLE)
    union video_req req = {0};

    if (!__this->video_display_0) {
        __this->video_display_0 = server_open("video_server", "video0");
        if (!__this->video_display_0) {
            puts("server_open:faild\n");
            return -EFAULT;
        }
    }

    /*
     *起始偏移x，起始偏移y，显示宽和高，显存x起始，显存y起始，显存x结束，显存y结束
     */
    req.display.fb 		= "fb1";
    req.display.left  	= xoff;
    req.display.top 	= yoff;
    req.display.width 	= width;
#ifdef CONFIG_TOUCH_UI_ENABLE
    /*change size*/
    if (height == 720) {
        req.display.height 	= height;
        req.display.border_left  = 0;
        req.display.border_top   = (height - SCREEN_H) / 2 / 16 * 16; // 0;
        req.display.border_right = 0;
        req.display.border_bottom = (height - SCREEN_H) / 2 / 16 * 16; //0;
    } else if (height == 480) {
        req.display.height 	= height;
        req.display.border_left  = 0;
        req.display.border_top   = 0;
        req.display.border_right = 0;
        req.display.border_bottom = 0;
    }
#else
    req.display.height 	= height;
    req.display.border_left  = 0;
    req.display.border_top   = 0;
    req.display.border_right = 0;
    req.display.border_bottom = 1;
#endif

    req.display.camera_config = load_default_camera_config;
    req.display.camera_type = VIDEO_CAMERA_NORMAL;

    req.display.state 	= VIDEO_STATE_START;
    req.display.pctl = NULL;

    server_request(__this->video_display_0, VIDEO_REQ_DISPLAY, &req);
    video_rec_start_isp_scenes();

    /*rec显示重设曝光补偿*/
    __this->exposure_set = 1;
    video_rec_set_exposure(db_select("exp"));

#endif
    return 0;
}

static void video0_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO0_ENABLE)
    union video_req req;

    if (__this->video_display_0) {
        video_rec_stop_isp_scenes(1, 0);
        req.display.state 	= VIDEO_STATE_STOP;

        server_request(__this->video_display_0, VIDEO_REQ_DISPLAY, &req);

        puts("\n\nstop disp out\n\n");

        server_close(__this->video_display_0);
        __this->video_display_0 = NULL;

        video_rec_start_isp_scenes();
    }
#endif
}




/******* 不要单独调用这些子函数 ********/
#ifdef CONFIG_VIDEO1_ENABLE
static int video1_rec_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;

    puts("start_video_rec1 \n");
    if (!__this->video_rec1) {
        __this->video_rec1 = server_open("video_server", "video1");
        if (!__this->video_rec1) {
            return VREC_ERR_V1_SERVER_OPEN;
        }

        server_register_event_handler(__this->video_rec1, (void *)1, rec_dev_server_event_handler);
    }

    req.rec.channel = 1;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.width 	= AVIN_WIDTH;
    req.rec.height 	= AVIN_HEIGH;
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;


    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps 	= 0;

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = 8000;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    #if(SLELE_5XX == JL5601)

        req.rec.IP_interval = 1;

    #else
    req.rec.IP_interval = 0;
    #endif
    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 100 / 100 ; // 70
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 100 / 100; // 80

    osd_info.osd_w = 16;
    osd_info.osd_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);//20;
    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    osd_info.x = (req.rec.width - max_one_line_strnum * osd_info.osd_w) / 64 * 64;
    osd_info.y = (req.rec.height - osd_info.osd_h * osd_line_num) / 16 * 16;

    osd_info.osd_yuv = 0xe20095;
    osd_info.osd_str = video_rec_osd_buf;
    osd_info.osd_matrix_str = osd_str_total;
    osd_info.osd_matrix_base = osd_str_matrix;
    osd_info.osd_matrix_len = sizeof(osd_str_matrix);

    req.rec.osd = 0;
    if (db_select("dat")) {
        req.rec.osd = &osd_info;
    }
    req.rec.slow_motion = 0;
    req.rec.tlp_time = db_select("gap");

    printf("\n\ntl_time : %d\n\n", req.rec.tlp_time);
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->v1_fbuf;
    req.rec.buf_len = VREC1_FBUF_SIZE;

#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif


    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    video_rec_get_file_name_and_size(1, &req.rec.fname, &req.rec.fsize, req.rec.width);

    err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return VREC_ERR_V1_REQ_START;
    }

    req.rec.channel = 1;
    req.rec.state 	= VIDEO_STATE_GET_INFO;
    server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    __this->file[1] = req.rec.get_info.file;

    return 0;
}

static int video1_rec_aud_mute()
{
    union video_req req;

    if (!__this->video_rec1) {
        return -EINVAL;
    }

    req.rec.channel = 1;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
}

static int video1_rec_stop_count_down(u32 time)
{
    union video_req req;

    if ((!__this->video_rec1) || (!__this->file[1])) {
        return -EINVAL;
    }
    req.rec.channel = 1;
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
}

static int video1_rec_stop(u8 close)
{
    union video_req req;
    int err;

    if (__this->video_rec1) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[1], 1);
        }

        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }
    }
    __this->file[1] = NULL;

    if (close) {
        if (__this->video_rec1) {
            server_close(__this->video_rec1);
            __this->video_rec1 = NULL;
        }
    }

    return 0;
}

static int video1_rec_savefile()
{
    union video_req req;
    int err;

    if (__this->video_rec1) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[1], 1);
        }

        req.rec.channel = 1;
        req.rec.width 	= AVIN_WIDTH;
        req.rec.height 	= AVIN_HEIGH;
        req.rec.format 	= VIDEO_FMT_MOV;
        req.rec.state 	= VIDEO_STATE_SAVE_FILE;
        req.rec.fpath 	= CONFIG_REC_PATH_2;
        req.rec.fps 	= 0;

#ifdef CONFIG_FILE_PREVIEW_ENABLE
        req.rec.rec_small_pic 	= 1;
#else
        req.rec.rec_small_pic 	= 0;
#endif

        req.rec.cycle_time = db_select("cyc");
        if (req.rec.cycle_time == 0) {
            req.rec.cycle_time = 5;
        }
        req.rec.cycle_time = req.rec.cycle_time * 60;
        video_rec_get_file_name_and_size(1, &req.rec.fname, &req.rec.fsize, req.rec.width);


        /* if (__this->rec_info->voice_on) { */
        req.rec.audio.sample_rate = 8000;
        req.rec.audio.channel 	= 1;
        req.rec.audio.volume    = 63;
        req.rec.audio.buf = __this->audio_buf;
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;
        /* } else { */
        /* req.rec.audio.sample_rate = 0; */
        /* req.rec.audio.channel 	= 0; */
        /* req.rec.audio.buf = 0; */
        /* req.rec.audio.buf_len = 0; */
        /* } */
        req.rec.pkg_mute.aud_mute = !db_select("mic");

        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_SAVEFILE;
        }

        req.rec.channel = 1;
        req.rec.state 	= VIDEO_STATE_GET_INFO;
        server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        __this->file[1] = req.rec.get_info.file;
    }

    return 0;
}

static void video1_rec_close()
{
    if (__this->video_rec1) {
        server_close(__this->video_rec1);
        __this->video_rec1 = NULL;
    }
}


/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video1_rec_set_osd_str(char *str)
{
    union video_req req;
    int err;
    if (!__this->video_rec1) {
        return -1;
    }

    req.rec.channel = 1;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nset osd rec1 str err 0x%x\n", err);
        return -1;
    }

    return 0;
}

static int video1_rec_osd_ctl(u8 onoff)
{
    union video_req req;
    struct imc_osd_info osd_info;
    int err;

    if (__this->video_rec1) {
        req.rec.width 	= AVIN_WIDTH;
        req.rec.height 	= AVIN_HEIGH;

        osd_info.osd_w = 16;
        osd_info.osd_h = 32;
        osd_info.x = (req.rec.width - strlen(osd_str_buf) * osd_info.osd_w) / 64 * 64;
        osd_info.y = (req.rec.height - osd_info.osd_h) / 16 * 16;
        osd_info.osd_yuv = 0xe20095;
        osd_info.osd_str = osd_str_buf;
        osd_info.osd_matrix_str = osd_str_total;
        osd_info.osd_matrix_base = osd_str_matrix;
        osd_info.osd_matrix_len = sizeof(osd_str_matrix);
        req.rec.osd = 0;
        if (onoff) {
            req.rec.osd = &osd_info;
        }
        req.rec.channel = 1;
        req.rec.state 	= VIDEO_STATE_SET_OSD;

        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nset osd rec1 err 0x%x\n", err);
            return -1;
        }
    }

    return 0;
}
#endif




static int video1_disp_start(u16 width, u16 height, u16 xoff, u16 yoff)
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO1_ENABLE)
    union video_req req = {0};

    if (!__this->video1_online) {
        return -EFAULT;
    }

    if (!__this->video_display_1) {
        __this->video_display_1 = server_open("video_server", "video1");
        if (!__this->video_display_1) {
            puts("server_open:faild\n");
            return -EFAULT;
        }
    }

    req.display.fb 		= "fb1";

    req.display.left  	= xoff;
    req.display.top 	= yoff;
    req.display.width 	= width;
    req.display.height 	= height;

    if (req.display.width < 1280) {
        req.display.width 	+= 32;
        req.display.height 	+= 32;

        req.display.border_left   = 16;
        req.display.border_top    = 16;
        req.display.border_right  = 16;
        req.display.border_bottom = 16;
    } else {
        req.display.border_left   = 0;
        req.display.border_top    = 0;
        req.display.border_right  = 0;
        req.display.border_bottom = 0;
    }

    req.display.camera_config = NULL;
    req.display.camera_type = VIDEO_CAMERA_NORMAL;

    req.display.state 	= VIDEO_STATE_START;
    req.display.pctl = NULL;

    server_request(__this->video_display_1, VIDEO_REQ_DISPLAY, &req);

#endif
    return 0;
}

static void video1_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO1_ENABLE)
    union video_req req;
    if (__this->video_display_1) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(__this->video_display_1, VIDEO_REQ_DISPLAY, &req);

        server_close(__this->video_display_1);
        __this->video_display_1 = NULL;
    }
#endif
}



#ifdef CONFIG_VIDEO3_ENABLE
static int video2_rec_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;

    puts("start_video_rec2 \n");
    if (!__this->video_rec2) {
        __this->video_rec2 = server_open("video_server", "video3");
        if (!__this->video_rec2) {
            return -EINVAL;
        }

        server_register_event_handler(__this->video_rec2, (void *)2, rec_dev_server_event_handler);
    }

    req.rec.channel = 2;
    req.rec.camera_type = VIDEO_CAMERA_UVC;
    req.rec.width 	= UVC_ENC_WIDTH;
    req.rec.height 	= UVC_ENC_HEIGH;
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;
    req.rec.uvc_id = __this->uvc_id;

    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps 	= 0;

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = 8000;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    #if(SLELE_5XX == JL5601)

        req.rec.IP_interval = 1;

    #else
    req.rec.IP_interval = 0;
    #endif

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 100 / 100 ; // 70
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 100 / 100; // 80

    osd_info.osd_w = 16;
    osd_info.osd_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);//20;
    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }
    osd_info.x = (req.rec.width - max_one_line_strnum * osd_info.osd_w) / 64 * 64;
    osd_info.y = (req.rec.height - osd_info.osd_h * osd_line_num) / 16 * 16;

    osd_info.osd_yuv = 0xe20095;
    osd_info.osd_str = video_rec_osd_buf;
    osd_info.osd_matrix_str = osd_str_total;
    osd_info.osd_matrix_base = osd_str_matrix;
    osd_info.osd_matrix_len = sizeof(osd_str_matrix);

    req.rec.osd = 0;
    if (db_select("dat")) {
        req.rec.osd = &osd_info;
    }
    req.rec.slow_motion = 0;
    req.rec.tlp_time = db_select("gap");

    printf("\n\ntl_time : %d\n\n", req.rec.tlp_time);
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->v2_fbuf;
    req.rec.buf_len = VREC2_FBUF_SIZE;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif


    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    video_rec_get_file_name_and_size(2, &req.rec.fname, &req.rec.fsize, req.rec.width);

    err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return -EINVAL;
    }

    req.rec.channel = 2;
    req.rec.state 	= VIDEO_STATE_GET_INFO;
    server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
    __this->file[2] = req.rec.get_info.file;
    //fcheck(__this->file[2]);

    return 0;
}

static int video2_rec_aud_mute()
{
    union video_req req;

    if (!__this->video_rec2) {
        return -EINVAL;
    }

    req.rec.channel = 2;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
}

static int video2_rec_stop_count_down(u32 time)
{
    union video_req req;

    if ((!__this->video_rec2) || (!__this->file[2])) {
        return -EINVAL;
    }
    req.rec.channel = 2;
    req.rec.format 	= VIDEO_FMT_MOV;
    req.rec.state 	= VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
}

static int video2_rec_stop(u8 close)
{
    union video_req req;
    int err;

    if (__this->video_rec2) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[2], 1);
        }

        req.rec.channel = 2;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return -EINVAL;
        }
    }
    __this->file[2] = NULL;

    if (close) {
        if (__this->video_rec2) {
            server_close(__this->video_rec2);
            __this->video_rec2 = NULL;
        }
    }

    return 0;
}

static int video2_rec_savefile()
{
    union video_req req;
    int err;

    if (__this->video_rec2) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[2], 1);
        }

        req.rec.channel = 2;
        req.rec.width 	= UVC_ENC_WIDTH;
        req.rec.height 	= UVC_ENC_HEIGH;
        req.rec.format 	= VIDEO_FMT_MOV;
        req.rec.state 	= VIDEO_STATE_SAVE_FILE;
        req.rec.fpath 	= CONFIG_REC_PATH_2;
        req.rec.fps 	= 0;
        req.rec.uvc_id = __this->uvc_id;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
        req.rec.rec_small_pic 	= 1;
#else
        req.rec.rec_small_pic 	= 0;
#endif

        req.rec.cycle_time = db_select("cyc");
        if (req.rec.cycle_time == 0) {
            req.rec.cycle_time = 5;
        }
        req.rec.cycle_time = req.rec.cycle_time * 60;
        video_rec_get_file_name_and_size(2, &req.rec.fname, &req.rec.fsize, req.rec.width);


        /* if (__this->rec_info->voice_on) { */
        req.rec.audio.sample_rate = 8000;
        req.rec.audio.channel 	= 1;
        req.rec.audio.volume    = 63;
        req.rec.audio.buf = __this->audio_buf;
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;
        /* } else { */
        /* req.rec.audio.sample_rate = 0; */
        /* req.rec.audio.channel 	= 0; */
        /* req.rec.audio.buf = 0; */
        /* req.rec.audio.buf_len = 0; */
        /* } */
        req.rec.pkg_mute.aud_mute = !db_select("mic");

        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec2 err 0x%x\n", err);
            return -EINVAL;
        }

        req.rec.channel = 2;
        req.rec.state 	= VIDEO_STATE_GET_INFO;
        server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        __this->file[2] = req.rec.get_info.file;
        //fcheck(__this->file[2]);
    }

    return 0;
}

static void video2_rec_close()
{
    if (__this->video_rec2) {
        server_close(__this->video_rec2);
        __this->video_rec2 = NULL;
    }
}


/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video2_rec_set_osd_str(char *str)
{
    union video_req req;
    int err;
    if (!__this->video_rec2) {
        return -1;
    }

    req.rec.channel = 2;
    req.rec.state 	= VIDEO_STATE_SET_OSD_STR;
    req.rec.new_osd_str = str;
    err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nset osd rec2 str err 0x%x\n", err);
        return -1;
    }

    return 0;
}

static int video2_rec_osd_ctl(u8 onoff)
{
    union video_req req;
    struct imc_osd_info osd_info;
    int err;

    if (__this->video_rec2) {
        req.rec.width 	= UVC_ENC_WIDTH;
        req.rec.height 	= UVC_ENC_HEIGH;

        osd_info.osd_w = 16;
        osd_info.osd_h = 32;
        osd_info.x = (req.rec.width - strlen(osd_str_buf) * osd_info.osd_w) / 64 * 64;
        osd_info.y = (req.rec.height - osd_info.osd_h) / 16 * 16;
        osd_info.osd_yuv = 0xe20095;
        osd_info.osd_str = osd_str_buf;
        osd_info.osd_matrix_str = osd_str_total;
        osd_info.osd_matrix_base = osd_str_matrix;
        osd_info.osd_matrix_len = sizeof(osd_str_matrix);
        req.rec.osd = 0;
        if (onoff) {
            req.rec.osd = &osd_info;
        }
        req.rec.channel = 2;
        req.rec.state 	= VIDEO_STATE_SET_OSD;

        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nset osd rec2 err 0x%x\n", err);
            return -1;
        }
    }

    return 0;
}

#endif




static int video2_disp_start(u16 width, u16 height, u16 xoff, u16 yoff)
{

#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO3_ENABLE)
    union video_req req = {0};

    if (!__this->video2_online) {
        return -EFAULT;
    }

    if (!__this->video_display_2) {
        __this->video_display_2 = server_open("video_server", "video3");
        if (!__this->video_display_2) {
            puts("server_open:faild\n");
            return -EFAULT;
        }
    }

    req.display.fb 		= "fb1";

    req.display.left  	= xoff;
    req.display.top 	= yoff;
    req.display.width 	= width;
    req.display.height 	= height;

    if (req.display.width < 1280) {
        req.display.width 	+= 32;
        req.display.height 	+= 32;

        req.display.border_left   = 16;
        req.display.border_top    = 16;
        req.display.border_right  = 16;
        req.display.border_bottom = 16;
    } else {
        req.display.border_left   = 0;
        req.display.border_top    = 0;
        req.display.border_right  = 0;
        req.display.border_bottom = 0;
    }

    req.display.uvc_id = __this->uvc_id;
    req.display.camera_config = NULL;
    req.display.camera_type = VIDEO_CAMERA_UVC;

    req.display.state 	= VIDEO_STATE_START;
    req.display.pctl = NULL;

    server_request(__this->video_display_2, VIDEO_REQ_DISPLAY, &req);

#endif
    return 0;
}

static void video2_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO3_ENABLE)
    union video_req req;
    if (__this->video_display_2) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(__this->video_display_2, VIDEO_REQ_DISPLAY, &req);

        server_close(__this->video_display_2);
        __this->video_display_2 = NULL;
    }
#endif
}







/******* 调用接口 ********/
static int video_rec_start()
{
    int err;
    u8 state = __this->state;
    puts("start rec\n");

#ifdef CONFIG_VIDEO0_ENABLE
    if (!__this->v0_fbuf) {
        __this->v0_fbuf = malloc(VREC0_FBUF_SIZE);
        if (!__this->v0_fbuf) {
            puts("malloc v0_buf err\n\n");
            return -1;
        }
    }
    if (!__this->audio_buf) {
        __this->audio_buf = malloc(300 * 1024);
        if (!__this->audio_buf) {
            free(__this->v0_fbuf);
            return -ENOMEM;
        }
    }
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    if (!__this->v1_fbuf) {
        __this->v1_fbuf = malloc(VREC1_FBUF_SIZE);
        if (!__this->v1_fbuf) {
            puts("malloc v1_buf err\n\n");
            return -1;
        }
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    if (!__this->v2_fbuf) {
        __this->v2_fbuf = malloc(VREC2_FBUF_SIZE);
        if (!__this->v2_fbuf) {
            puts("malloc v2_buf err\n\n");
            return -1;
        }
    }
#endif

#if DEL_LOCK_FILE_EN
    video_rec_scan_lock_fszie();
#endif

    err = video_rec_del_old_file();
    if (err) {
        puts("\nstart free space err\n");
        video_rec_post_msg("fsErr");
        return VREC_ERR_START_FREE_SPACE;
    }

    __this->state = VIDREC_STA_STARTING;

#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_start();
    if (err) {
        goto __start_err0;
    }
#endif

    video_rec_post_msg("onREC");
      video_dzg_post_msg("onREC");
#ifdef CONFIG_VIDEO1_ENABLE
    if (__this->video1_online && db_select("two")) {
        err = video1_rec_start();
        /*if (err) {
            goto __start_err1;
        }*/
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video2_online && db_select("two")) {
        err = video2_rec_start();
    }
#endif
    __this->state = VIDREC_STA_START;


    return 0;

#ifdef CONFIG_VIDEO3_ENABLE
__start_err2:
    puts("\nstart2 err\n");
    err = video2_rec_stop(0);
    if (err) {
        printf("\nstart wrong2 %x\n", err);
        //while(1);
    }
#endif

#ifdef CONFIG_VIDEO1_ENABLE
__start_err1:
    puts("\nstart1 err\n");
    err = video1_rec_stop(0);
    if (err) {
        printf("\nstart wrong1 %x\n", err);
        //while(1);
    }
#endif

#ifdef CONFIG_VIDEO0_ENABLE
__start_err0:
    puts("\nstart0 err\n");
    err = video0_rec_stop(0);
    if (err) {
        printf("\nstart wrong0 %x\n", err);
        //while(1);
    }
#endif

    __this->state = state;
    return -EFAULT;
}
void flith()
{
   
}
 int video_rec_aud_mute()
{

    if (db_select("mic")) {
        puts("mic on\n");
        video_rec_post_msg("onMIC");
    } else {
        puts("mic off\n");
        video_rec_post_msg("offMIC");
    }

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_aud_mute();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    video1_rec_aud_mute();
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    video2_rec_aud_mute();
#endif
    return 0;
}

static int video_rec_stop_count_down(u32 time)
{
    if (__this->state != VIDREC_STA_START) {
        return 0;
    }
    if (!db_select("cyc")) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_stop_count_down(time);
#endif
#ifdef CONFIG_VIDEO1_ENABLE
    video1_rec_stop_count_down(time);
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    video2_rec_stop_count_down(time);
#endif
    return 0;
}

static int video_rec_stop(u8 close)
{
    int err;

    __this->state = VIDREC_STA_STOPING;

#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_stop(close);
    if (err) {
        puts("\nstop0 err\n");
    }
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    err = video1_rec_stop(close);
    if (err) {
        puts("\nstop1 err\n");
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    err = video2_rec_stop(close);
    if (err) {
        puts("\nstop2 err\n");
    }
#endif

#ifdef CONFIG_GSENSOR_ENABLE
    if (get_gsen_active_flag()) {
        clear_gsen_active_flag();
    }
#endif

    if (__this->disp_state == DISP_BACK_WIN) {
        video_rec_post_msg("HlightOff"); //后视停录像关闭前照灯
    }

    __this->state = VIDREC_STA_STOP;
    __this->park_wakeup = 0;
    __this->gsen_lock = 0;

    return 0;
}




static int video_rec_close()
{
#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_close();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    video1_rec_close();
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    video2_rec_close();
#endif

    return 0;
}





static int video_rec_savefile(int dev_id)
{
    int err;
    int dev_num = 0;
    union video_req req;

    printf("save_file: %d\n", dev_id);

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    if (__this->save_file == 0) {
        err = video_rec_del_old_file();
        if (err) {
            video_rec_post_msg("fsErr");
            goto __err;
        }
        video_rec_post_msg("saveREC");
        video_home_post_msg("saveREC");//??????,ui????????
        video_parking_post_msg("saveREC");
	video_dzg_post_msg("saveREC");
    }

#ifdef CONFIG_VIDEO0_ENABLE
    if (dev_id == 0) {
        err = video0_rec_savefile();
        if (err) {
            goto __err;
        }
    }
    dev_num++;
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    if (__this->video1_online) {
        if (dev_id == 1) {
            err = video1_rec_savefile();
            if (err) {
                goto __err;
            }
        }
        dev_num++;
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video2_online) {
        if (dev_id == 2) {
            err = video2_rec_savefile();
            if (err) {
                goto __err;
            }
        }
        dev_num++;
    }
#endif

    if (__this->save_file == 0) {
#ifdef CONFIG_GSENSOR_ENABLE
        if (get_gsen_active_flag()) {
            clear_gsen_active_flag();
        }
#endif
        __this->state = VIDREC_STA_START;
        __this->park_wakeup = 0;
        __this->gsen_lock = 0;
    }
    if (++__this->save_file == dev_num) {
        __this->save_file = 0;
    }

    return 0;

__err:
#ifdef CONFIG_VIDEO0_ENABLE
    video0_rec_stop(0);
#endif
#ifdef CONFIG_VIDEO1_ENABLE
    video1_rec_stop(0);
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    video2_rec_stop(0);
#endif

    video_rec_post_msg("offREC");
    video_home_post_msg("offREC");//??????,ui????????
    video_parking_post_msg("offREC");
    video_dzg_post_msg("offREC");

    __this->state = VIDREC_STA_STOP;

    return -EFAULT;

}

/*
 * ??????????, ??????,??????????,?????????
 * ??????????1.5M + 400K???,??????????
 */
int video_rec_take_photo(void)
{
    struct server *server;
    union video_req req = {0};
    int err;

    if (__this->photo_camera_sel == 0) {
        server = __this->video_rec0;
    } else if (__this->photo_camera_sel == 1) {
        server = __this->video_rec1;
    } else {
        server = __this->video_rec2;
    }
    if ((__this->state != VIDREC_STA_START) || (server == NULL)) {
        return -EINVAL;
    }

    if (__this->cap_buf == NULL) {
        __this->cap_buf = (u8 *)malloc(400 * 1024);
        if (!__this->cap_buf) {
            puts("\ntake photo no mem\n");
            return -ENOMEM;
        }
    }
    req.icap.width = pic_pix_w[VIDEO_RES_720P];
    req.icap.height = pic_pix_h[VIDEO_RES_720P];
    req.icap.quality = VIDEO_MID_Q;
    req.icap.label = NULL;
    req.icap.buf = __this->cap_buf;
    req.icap.buf_size = 400 * 1024;
    if (__this->photo_camera_sel == 0) {
        req.icap.path = CAMERA0_CAP_PATH"jpeg*.jpg";
    } else if (__this->photo_camera_sel == 1) {
        req.icap.path = CAMERA1_CAP_PATH"jpeg*.jpg";
    } else {
        req.icap.path = CAMERA1_CAP_PATH"jpeg*.jpg";
    }

    err = server_request(server, VIDEO_REQ_IMAGE_CAPTURE, &req);
    if (err != 0) {
        puts("\n\n\ntake photo err\n\n\n");
        return -EINVAL;
    }

    return 0;
}

/*
 * 录像app的录像控制入口, 根据当前状态调用相应的函数 //2???
 */
 int video_rec_control(void *_run_cmd)
{
    int err = 0;
    u32 clust;
    int run_cmd = (int)_run_cmd;
    struct vfs_partition *part;
printf("kkkkllllllkkkk  run_cmd %d\n",run_cmd);
    if (storage_device_ready() == 0) {
        if (!dev_online(SDX_DEV)) {
            video_rec_post_msg("noCard");
        } else {
            video_rec_post_msg("fsErr");
        }
        return 0;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);

        __this->total_size = part->total_size;

        if (part->clust_size < 32 || (part->fs_attr & F_ATTR_RO)) {
            video_rec_post_msg("fsErr");
            return 0;
        }
    }

    switch (__this->state) {
    case VIDREC_STA_IDLE:
    case VIDREC_STA_STOP:
        if (run_cmd) {
            break;
        }
        err = video_rec_start();
	printf("err   %d\n",err);
        if (err == 0) {
	     printf("000000000\n");
            video_rec_post_msg("onREC");
			/*************************************************/
	    video_dzg_post_msg("onREC");
            video_home_post_msg("onREC");
			/************************************************/
            if (__this->gsen_lock == 1) {
                video_rec_post_msg("lockREC");
            }
        }
#ifndef CONFIG_UI_ENABLE
        //video_rec_set_white_balance();
        video_rec_set_exposure(db_select("exp"));
#endif // CONFIG_UI_ENABLE

        break;

    case VIDREC_STA_START:
        if (run_cmd == 0) {
            /*提前UI响应,加快反应速度*/
		 printf("1111111111\n");
            video_rec_post_msg("offREC");
            video_parking_post_msg("offREC");
	     video_dzg_post_msg("offREC");
            video_home_post_msg("offREC");//录像切到后台,ui消息由主界面响应
            err = video_rec_stop(0);
        } else if (run_cmd == 1) { //cycle_rec
            err = 0;
        } else {//track_pic
            err = video_rec_take_photo();
        }
        break;
    default:
        puts("\nvrec forbid\n");
        err = 1;
        break;
    }

    return err;
}

#if 0
int video_rec_osd_ctl(u8 onoff)
{
    int err;

    if (__this->state == VIDREC_STA_START) {
        return -EFAULT;
    }

    __this->rec_info->osd_on = onoff;
#ifdef CONFIG_VIDEO0_ENABLE
    err = video0_rec_osd_ctl(__this->rec_info->osd_on);
#endif // VREC0_EN

#ifdef CONFIG_VIDEO1_ENABLE
    err = video1_rec_osd_ctl(__this->rec_info->osd_on);
#endif

    return err;
}

int video_rec_set_white_balance()
{
    union video_req req;

    if (!__this->white_balance_set) {
        return 0;
    }

    req.camera.mode = ISP_MODE_IMAGE_CAPTURE;
    req.camera.white_blance = __this->rec_info->wb_val;
    req.camera.cmd = SET_CAMERA_WB ;

    if (__this->video_display_0) {
        server_request(__this->video_display_0, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else if (__this->video_rec0 && (__this->state == VIDREC_STA_START)) {
        server_request(__this->video_rec0, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else {
        puts("\nvrec set wb fail\n");
        return 1;
    }

    __this->white_balance_set = 0;

    return 0;
}
#endif


int video_rec_set_exposure(u32 exp)
{
    union video_req req;

    if (!__this->exposure_set) {
        return 0;
    }

    req.camera.mode = ISP_MODE_IMAGE_CAPTURE;
    req.camera.ev = exp;
    req.camera.cmd = SET_CAMERA_EV;

    if (__this->video_display_0) {
        server_request(__this->video_display_0, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else if (__this->video_rec0 && (__this->state == VIDREC_STA_START)) {
        server_request(__this->video_rec0, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else {
        return 1;
    }

    __this->exposure_set = 0;

    return 0;
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
    } else if (__this->video_rec0 && ((__this->state == VIDREC_STA_START) ||
                                      (__this->state == VIDREC_STA_STARTING))) {
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

    if ((status != __this->isp_scenes_status) && (status != 3)) {
        return 0;
    }

    __this->isp_scenes_status = 0;
    stop_update_isp_scenes();

    if (restart) {
        video_rec_start_isp_scenes();
    }

    return 0;
}

static int show_main1_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    printf("rrrrrrrrrrrrrrrr\n");
    if (page_main_flag) {
        return 0;
    }

    if (!__this->ui) {
        return -EINVAL;
    }
    req.show.id = ID_WINDOW_DZG;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    page_main_flag = 1;
#endif
    return 0;
}
static void hide_main1_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
       printf("222222222222\n");
    if (page_main_flag == 0) {
        return ;
    }

    if (!__this->ui) {
        return;
    }


    req.hide.id = ID_WINDOW_DZG;
    server_request(__this->ui, UI_REQ_HIDE, &req);
    page_main_flag = 0;
#endif
}

#if 1
static int video_layout1_up_onchange(void *ctr, enum element_change_event e, void *arg)
{
    int index;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * ????????????????????????
         */
      

        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(BASEFORM_86)
.onchange = video_layout1_up_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif



/*
???? !!!
1.?????????
2.?????????0?????
3.??????(?????????????? VE_MOTION_DETECT_MODE_ISP ????)?????
*/
#if 1
/* int video_rec_digital_zoom(u8 big_small) */
/* { */
/* #ifdef CONFIG_UI_ENABLE */
/* union video_req req; */
/* struct video_crop_sca *crop_result; */

/* if ((!__this->video_display_0) || db_select("gap")) { */
/* return 1; */
/* } */

/* req.sca.sca_modify = big_small; */
/* req.sca.step = 16;//4 align */
/* req.sca.max_sca = 5; */
/* crop_result = (struct video_crop_sca *)server_request(__this->video_display_0, VIDEO_REQ_CAMERA_SCA, &req); */
/* if (crop_result) { */
/* printf("src_w %d, src_h %d, crop_w %d, crop_h %d\n", */
/* crop_result->src_w, crop_result->src_h, crop_result->crop_w, crop_result->crop_h); */
/* } */
/* #endif */
/* return 0; */
/* } */
#endif



static int video_disp_control()
{
    int err = 0;

#if ((defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO1_ENABLE) || (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO3_ENABLE))

    switch (__this->disp_state) {
    case DISP_BIG_SMALL:
        video0_disp_stop();
        video1_disp_stop();
        video2_disp_stop();

        err = video0_disp_start(SCREEN_W / 2, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err0;
        }

        err = video1_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
        if (err) {
            goto __disp_ctl_err1;
        }

        err = video2_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
        if (err) {
            goto __disp_ctl_err2;
        }

        __this->disp_state = DISP_HALF_WIN;
        break;
    case DISP_HALF_WIN:
        video2_disp_stop();
        video1_disp_stop();
        video0_disp_stop();
        err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err0;
        }
        __this->disp_state = DISP_FRONT_WIN;
        break;
    case DISP_FRONT_WIN:
        video0_disp_stop();
        err = video1_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err1;
        }
        err = video2_disp_start(SCREEN_W, SCREEN_H, 0, 0);
        if (err) {
            goto __disp_ctl_err2;
        }
        __this->disp_state = DISP_BACK_WIN;
        video_rec_post_msg("HlightOff");//进入后视窗口前照灯关闭
        break;
    case DISP_BACK_WIN:
        video1_disp_stop();
        video2_disp_stop();
        err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err0;
        }
        err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
        if (err) {
            goto __disp_ctl_err1;
        }
        err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
        if (err) {
            goto __disp_ctl_err2;
        }
        __this->disp_state = DISP_BIG_SMALL;

        break;

    case DISP_RESET_WIN:
        video2_disp_stop();
        video1_disp_stop();
        video0_disp_stop();
        err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
//            goto __disp_ctl_err0;
        }
        err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
        if (err) {
            break;
        }
        err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
        if (err) {
            break;
        }
        __this->disp_state = DISP_BIG_SMALL;

        break;

    case DISP_PARK_WIN:
        video0_disp_stop();
        video1_disp_stop();
        video2_disp_stop();
        err = video1_disp_start(SCREEN_W, SCREEN_H, 0, 0);
        err = video2_disp_start(SCREEN_W, SCREEN_H, 0, 0);
        __this->disp_state = DISP_BACK_WIN;

        break;

    default:
        puts("\ndisp forbid\n");
        break;
    }
    if (__this->disp_state == DISP_BACK_WIN) {
        //后拉全屏显示（非录像状态），固定屏显场景1
        isp_scr_work_hdl(1);
    }
    return 0;

__disp_ctl_err2:
    puts("\n\ndisp ctrl err2\n\n");
    video2_disp_stop();
__disp_ctl_err1:
    puts("\n\ndisp ctrl err1\n\n");
    video1_disp_stop();
__disp_ctl_err0:
    puts("\n\ndisp ctrl err0\n\n");
    video0_disp_stop();

#endif
    return err;
}



static int show_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_main_flag) {
        return 0;
    }
    if (!__this->ui) {
        return -1;
    }
    	     u32 bu1 = db_select("bu");
        if( bu1 == 0){
                  gpio_direction_output(IO_PORTG_00, 0);
    	}else if(bu1 == 1) {
                  gpio_direction_output(IO_PORTG_00, 1);
    		}
    puts("show_main_ui\n");
    req.show.id = ID_WINDOW_VIDEO_REC;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    page_main_flag = 1;
#endif

    return 0;
}


static int show_park_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_park_flag) {
        return 0;
    }
    if (!__this->ui) {
        return -1;
    }

    req.show.id = ID_WINDOW_PARKING;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
    page_park_flag = 1;
#endif

    return 0;
}

int show_lane_set_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return -1;
    }

    req.show.id = ID_WINDOW_LANE;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif

    return 0;
}




static void hide_main_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_main_flag == 0) {
        return;
    }
    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_main_ui\n");

    req.hide.id = ID_WINDOW_VIDEO_REC;
    server_request(__this->ui, UI_REQ_HIDE, &req);
    page_main_flag = 0;
#endif
}

static void hide_home_main_ui()
{
#ifdef CONFIG_TOUCH_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_home_main_ui\n");

    req.hide.id = ID_WINDOW_MAIN_PAGE;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}
static void hide_park_ui()
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (page_park_flag == 0) {
        return;
    }
    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_park_ui\n");

    req.hide.id = ID_WINDOW_PARKING;
    server_request(__this->ui, UI_REQ_HIDE, &req);
    page_park_flag = 0;
#endif
}


static int video_rec_sd_in()
{
    puts("\n----rec app sd in----\n");

    if (__this->state == VIDREC_STA_IDLE) {
        ve_mdet_stop();
        ve_lane_det_stop(0);
    }

    if (__this->menu_inout == 0) {
        ve_mdet_start();
        ve_lane_det_start(0);
    }

    __this->lock_fsize_count = 0;

    return 0;
}

static int video_rec_sd_out()
{

    ve_mdet_stop();
    ve_lane_det_stop(0);

    for (int i = 0; i < 3; i++) {
        if (__this->fscan[i]) {
            fscan_release(__this->fscan[i]);
            __this->fscan[i] = NULL;
        }
    }

    return 0;
}

static int rec_app_ready()
{
    return __this->run_rec_start;
}

/* static void video1_control(void *p) */
/* { */
/* video1_disp_start(SCREEN_W, SCREEN_H, 0, 0); */
/* } */

static void video_rec_park_call_back(void *priv)
{
    if (__this->state == VIDREC_STA_START) {
        video_rec_control(0);
        if (usb_is_charging() && (__this->state == VIDREC_STA_STOP)) {
            video_rec_control(0);
        } else {
            puts("\n park rec off power close\n");
            send_key_long_msg();
        }
    }
}
static int video_rec_park_wait(void *priv)
{
    if (__this->state != VIDREC_STA_START) {
        video_rec_control(0);
    }
    if (__this->state == VIDREC_STA_START) {
        sys_timeout_add(NULL, video_rec_park_call_back, 30 * 1000);
    } else {
        __this->gsen_lock = 0;
        __this->park_wakeup = 0;
        clear_gsen_active_flag();
    }

    return 0;
}

/*
 * 启动录像app之后，如果有些功能要提前跑，则在下面增加
 */
static void video_rec_begin_run()
{
#ifdef CONFIG_GSENSOR_ENABLE
    if (get_park_guard_status(db_select("par"))) {
        __this->gsen_lock = 1;
        __this->park_wakeup = 1;
        if (__this->state != VIDREC_STA_START) {
            __this->park_wait = wait_completion(storage_device_ready, video_rec_park_wait, NULL);
        }
    }
#endif

    //todo other run init...
}

int lane_det_setting_disp()
{
    u32 err = 0;
#ifdef CONFIG_VIDEO0_ENABLE
    u16 dis_w = 640 * SCREEN_H / 352 / 16 * 16;
    dis_w = dis_w > SCREEN_W ? SCREEN_W : dis_w;
    printf("lane dis %d x %d\n", dis_w, SCREEN_H);
    err = video0_disp_start(dis_w, SCREEN_H, ((SCREEN_W - dis_w) / 2 / 16 * 16), 0);
    show_lane_set_ui();
#endif
    return err;
}
static int video_rec_init1()
{
    int err = 0;


    //ve_server_open(0);



    err = video0_disp_start(SCREEN_W, SCREEN_H, 0, 0);





    if (((__this->video1_online == 0) && (__this->video2_online == 0)) || err) {
        __this->disp_state = DISP_RESET_WIN;
    }

    /**********************************************/

/**********************************************/

    if (get_parking_status()) {
        show_park_ui();
    } else {
        show_main1_ui();
    }

  //  video_rec_begin_run();
    return err;
}



static int video_rec_init()
{
    int err = 0;


    ve_server_open(0);


#ifdef CONFIG_VIDEO0_ENABLE
#ifdef CONFIG_TOUCH_UI_ENABLE
    err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
#else
    err = video0_disp_start(SCREEN_W, SCREEN_H, 0, 0);
#endif
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    __this->video1_online = dev_online("video1");
    if (__this->video1_online) {
        if (get_parking_status()) {
            video0_disp_stop();
            err = video1_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            __this->disp_state = DISP_BACK_WIN;
        } else {
            err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
        }
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    __this->video2_online = dev_online("uvc");
    if (__this->video2_online) {
        if (get_parking_status()) {
            video0_disp_stop();
            err = video2_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            __this->disp_state = DISP_BACK_WIN;
        } else {
            err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
        }
    }
#endif

    if (((__this->video1_online == 0) && (__this->video2_online == 0)) || err) {
        __this->disp_state = DISP_RESET_WIN;
    }

    /**********************************************/
	if(home_rec_touch == 1){
		video_rec_begin_run();
		page_main_flag = 1;
		home_rec_touch = 0;
		return err;
	}
/**********************************************/

    if (get_parking_status()) {
        show_park_ui();
    } else {
        show_main_ui();
    }

    video_rec_begin_run();
    return err;
}




static int video_rec_uninit()
{
    int err;
    union video_req req;

    if (__this->state == VIDREC_STA_START) {
        return -EFAULT;
    }
    if (__this->wait) {
        wait_completion_del(__this->wait);
        __this->wait = 0;
    }
    if (__this->park_wait) {
        wait_completion_del(__this->park_wait);
        __this->park_wait = 0;
    }
    if (__this->avin_wait) {
        wait_completion_del(__this->avin_wait);
        __this->avin_wait = 0;
    }
    if (__this->sd_wait) {
        wait_completion_del(__this->sd_wait);
        __this->sd_wait = 0;
    }

    video_rec_stop_isp_scenes(3, 0);

    ve_server_close();

    if (__this->state == VIDREC_STA_START) {
        err = video_rec_stop(1);
    }
    video_rec_close();


    for (int i = 0; i < 3; i++) {
        if (__this->fscan[i]) {
            fscan_release(__this->fscan[i]);
            __this->fscan[i] = NULL;
        }
    }

#ifdef CONFIG_UI_ENABLE
#ifdef CONFIG_VIDEO0_ENABLE
    video0_disp_stop();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    video1_disp_stop();
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    video2_disp_stop();
#endif
#endif

    __this->disp_state = DISP_FORBIDDEN;
    __this->state = VIDREC_STA_FORBIDDEN;
    __this->lan_det_setting = 0;

    return 0;

}


static int video_rec_mode_sw()
{
    if (__this->state != VIDREC_STA_FORBIDDEN) {
        return -EFAULT;
    }

    if (__this->v0_fbuf) {
        free(__this->v0_fbuf);
        __this->v0_fbuf = NULL;
    }
    if (__this->audio_buf) {
        free(__this->audio_buf);
        __this->audio_buf = NULL;
    }
    if (__this->v1_fbuf) {
        free(__this->v1_fbuf);
        __this->v1_fbuf = NULL;
    }
    if (__this->v2_fbuf) {
        free(__this->v2_fbuf);
        __this->v2_fbuf = NULL;
    }

    if (__this->cap_buf) {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }

    return 0;
}


/*
 *菜单相关的函数
 */
static int video_rec_change_status(struct intent *it)
{
    static char retime_buf[30];
    int err;
    u32 cur_space;
    u32 one_pic_size;
    int hour, min, sec;

    if (!strcmp(it->data, "opMENU:")) { /* ui要求打开rec菜单 */
        puts("ui ask me to opMENU:.\n");

        if ((__this->state != VIDREC_STA_START) && (__this->state != VIDREC_STA_FORBIDDEN)) { /* 允许ui打开菜单 */
            __this->menu_inout = 1;
            if (db_select("mot")) {
                ve_mdet_stop();
            }
            if (db_select("lan")) {
                ve_lane_det_stop(0);
            }

            it->data = "opMENU:en";
        } else { /* 禁止ui打开菜单 */
            it->data = "opMENU:dis";
        }
    } else if (!strcmp(it->data, "exitMENU")) { /* ui已经关闭rec菜单 */
        puts("ui tell me exitMENU.\n");
        __this->menu_inout = 0;
		    u32 bu = db_select("bu");
            if( bu == 0){
                  gpio_direction_output(IO_PORTG_00, 0);
    	}else if(bu == 1) {
                  gpio_direction_output(IO_PORTG_00, 1);
    		}
        video_rec_fun_restore();
        if (db_select("mot")) {
            ve_mdet_start();
        }
        if (db_select("lan")) {
            ve_lane_det_start(0);
        }
    } else if (!strcmp(it->data, "reTIME:")) {
        /*
         * 这里填入SD卡剩余录像时间
         */
        err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
        if (err) {
            hour = 0;
            min = 0;
            sec = 0;
        } else {
            u32 res = db_select("res");
            if (res == VIDEO_RES_1080P) {
                one_pic_size = (0x21000 + 0xa000) / 1024;
            } else if (res == VIDEO_RES_720P) {
                one_pic_size = (0x13000 + 0xa000) / 1024;
            } else {
                one_pic_size = (0xa000 + 0xa000) / 1024;
            }
            hour = (cur_space / one_pic_size) / 30 / 60 / 60;
            min = (cur_space / one_pic_size) / 30 / 60 % 60;
            sec = (cur_space / one_pic_size) / 30 % 60;
        }
        sprintf(retime_buf, "%2d.%2d.%2d\\0", hour, min, sec);
        printf("retime_buf: %s\n", retime_buf);
        it->data = retime_buf;

    } else if (!strcmp(it->data, "sdCard:")) {
        if (storage_device_ready() == 0) {
            it->data = "offline";
        } else {
            it->data = "online";
        }
    } else {
        puts("unknow status ask by ui.\n");
    }

    return 0;
}
static int video_gps_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int len;
    struct intent mit;


    switch (state) {
    case APP_STA_CREATE:

        server_load(video_server);
	  //   memset(__this, 0, sizeof_this);


        __this->ui = server_open("ui_server", NULL);
       if (!__this->ui) {
            return -EINVAL;
       }


        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_VIDEO_DZG_MAIN:
   //       video_photo_start();
            show_main1_ui();

              // video_rec_init1();

            break;
        case ACTION_VIDEO_DZG_SET_CONFIG:
           
            break;
        case ACTION_VIDEO_DZG_GET_CONFIG:

            break;
          case ACTION_VIDEO_DZG_CHANGE_STATUS:
		  	
                   break;

        }
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
			
        break;
    case APP_STA_STOP:
     hide_main1_ui();
	   //  video_rec_control(0);
   // ui_hide(BASEFORM_85);
 //  ui_hide(BASEFORM_86);
        break;
    case APP_STA_DESTROY:
		
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }



        break;
    }

    return 0;

}
/*
 *录像的状态机,进入录像app后就是跑这里
 */
static int video_rec_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int err = 0;
    int len;

    switch (state) {
    case APP_STA_CREATE:
        puts("--------app_rec: APP_STA_CREATE\n");

        memset(__this, 0, sizeof_this);
        hide_main1_ui();

            //    it.action= ID_WINDOW_DZG;
            //    server_request(ui, UI_REQ_HIDE, &req); /* 隐藏主界面ui */
		//ui_hide(BASEFORM_85);
        server_load(video_server);
#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EINVAL;
        }
	
#endif
        video_rec_config_init();
        __this->state = VIDREC_STA_IDLE;

        break;
    case APP_STA_START:
        puts("--------app_rec: APP_STA_START\n");
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_VIDEO_REC_MAIN:
            puts("ACTION_VIDEO_REC_MAIN\n");
		 ui_hide(ID_WINDOW_DZG);	     
	    //  hide_main1_ui();
            if (it->data && !strcmp(it->data, "lan_setting")) {
                __this->lan_det_setting = 1;
                ve_server_open(1);
                lane_det_setting_disp();
            } else {
                video_rec_init();
            }
            __this->run_rec_start = 1;
            break;
        case ACTION_VIDEO_REC_SET_CONFIG:
            if (!__this->ui) {
                return -EINVAL;
            }
            video_rec_set_config(it);
            db_flush();
            break;
        case ACTION_VIDEO_REC_CHANGE_STATUS:
            video_rec_change_status(it);
            break;
        case ACTION_VIDEO_DZG_REC:
                  video_rec_control(0);
            break;   
	        case ACTION_VIDEO_REC_CONTROL:
            video_rec_control(0);
            if (__this->state == VIDREC_STA_STOP) {
                ve_mdet_reset();
                ve_lane_det_reset();
            }
            break;
#ifdef CONFIG_TOUCH_UI_ENABLE

        case ACTION_VIDEO_REC_LOCK_FILE:
            if (it->data && !strcmp(it->data, "get_lock_statu")) {
                it->exdata = __this->gsen_lock;
                break;
            }

            if (__this->state == VIDREC_STA_START) {
                __this->gsen_lock = it->exdata;
            }
            break;
        case ACTION_VIDEO_REC_SWITCH_WIN:
#if THREE_WAY_ENABLE
            if ((video_rec_online_nums() < 2) || (get_parking_status() == 1)) {
                break;
            }

            printf("\n\nbefore disp_state %x, ctrl %x, sw_flag %x\n", __this->disp_state, __this->disp_video_ctrl, __this->disp_sw_flag);
#ifdef CONFIG_VIDEO1_ENABLE
            if ((video_rec_online_nums() == 3) && (__this->disp_state != DISP_FRONT_WIN) && (__this->disp_sw_flag == 0) && (__this->disp_video_ctrl == 1)) {
#else
            if ((video_rec_online_nums() == 3) && (__this->disp_state != DISP_FRONT_WIN) && (__this->disp_sw_flag == 0) && (__this->disp_video_ctrl == 2)) {
#endif
                if (__this->video2_online) {
                    __this->disp_video_ctrl = 2;
                } else {
                    __this->disp_video_ctrl = 3;
                }

                __this->disp_sw_flag = 1;
                if (__this->disp_state == DISP_BIG_SMALL) {
                    __this->disp_state = DISP_RESET_WIN;
                } else if (__this->disp_state == DISP_HALF_WIN) {
                    __this->disp_state = DISP_BIG_SMALL;
                } else if (__this->disp_state == DISP_BACK_WIN) {
                    __this->disp_state = DISP_FRONT_WIN;
                } else {
                    __this->disp_state = DISP_RESET_WIN;
                }

                printf("\n\na1 disp_state %x, ctrl %x\n", __this->disp_state, __this->disp_video_ctrl);

            } else {
                __this->disp_sw_flag = 0;
                if (video_rec_online_nums() == 3) {
                    if (__this->video1_online) {
                        __this->disp_video_ctrl = 1;
                    } else if (__this->video2_online) {
                        __this->disp_video_ctrl = 2;
                    } else {
                        __this->disp_video_ctrl = 3;
                    }
                }
                if (__this->disp_state == DISP_HALF_WIN) {
                    __this->disp_video_ctrl = 0;
                }
                printf("\n\na2 disp_state %x, ctrl %x\n", __this->disp_state, __this->disp_video_ctrl);
            }

            video_disp_control();
#else

#if (defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO1_ENABLE)
            if ((__this->video1_online == 0) || (get_parking_status() == 1)) {
                break;
            }
            video_disp_control();
#endif

#if (defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO3_ENABLE)
            if ((__this->video2_online == 0) || (get_parking_status() == 1)) {
                break;
            }
            video_disp_control();
#endif

#endif
            break;
#endif
        }
        break;
    case APP_STA_PAUSE:
        puts("--------app_rec: APP_STA_PAUSE\n");
        break;
    case APP_STA_RESUME:
        puts("--------app_rec: APP_STA_RESUME\n");
        break;
    case APP_STA_STOP:
#if 1
        puts("--------app_rec: APP_STA_STOP\n");
        if (__this->state == VIDREC_STA_START) {
            video_rec_control(0);
            if (__this->state == VIDREC_STA_STOP) {
                ve_mdet_stop();
                ve_lane_det_stop(0);
            }
        }

        if (video_rec_uninit()) {
            err = 1;
            break;
        }
#endif
#ifdef CONFIG_UI_ENABLE
        puts("--------rec hide ui\n");
        hide_main_ui();
	//  hide_main1_ui();
#endif

        break;
    case APP_STA_DESTROY:
        puts("--------app_rec: APP_STA_DESTROY\n");
        if (video_rec_mode_sw()) {
            err = 2;
            break;
        }
#ifdef CONFIG_UI_ENABLE
        puts("--------rec close ui\n");
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
#endif
        __this->run_rec_start = 0;
        break;
    }

    return err;
}

//static u8 i = 0;
/*
 *录像app的按键响应函数
 */
static int video_rec_key_event_handler(struct key_event *key)
{
    int err;
            u8 i = 0;
    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            err = video_rec_control(0);
            if (__this->state == VIDREC_STA_STOP) {
                ve_mdet_reset();
                ve_lane_det_reset();
            }
            break;
        case KEY_MENU:
            break;
        case KEY_MODE:
            puts("rec key mode\n");
            if ((__this->state != VIDREC_STA_STOP) && (__this->state != VIDREC_STA_IDLE)) {
                if (__this->state == VIDREC_STA_START) {
                    if (!__this->gsen_lock) {
                        __this->gsen_lock = 1;
                        video_rec_post_msg("lockREC");
                    } else {
                        __this->gsen_lock = 0;
                        video_rec_post_msg("unlockREC");
                    }
                }

                return true;
            }
            break;
        case KEY_UP:

            /* video_rec_digital_zoom(1); */
            /* break; */
	  flith();
#ifdef CONFIG_UI_ENABLE
#if (defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO1_ENABLE)
            if ((__this->video1_online == 0) || (get_parking_status() == 1)) {
                break;
            }
            video_disp_control();
#endif

#if (defined CONFIG_VIDEO0_ENABLE && defined CONFIG_VIDEO3_ENABLE)
            if ((__this->video2_online == 0) || (get_parking_status() == 1)) {
                break;
            }
            video_disp_control();
#endif
#endif // CONFIG_UI_ENABLE
            break;
        case KEY_DOWN:


            /* video_rec_take_photo(); */
            /* video_rec_digital_zoom(0); */
            /* break; */

            if (__this->run_rec_start) {
                mic_set_toggle();
                video_rec_aud_mute();
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

/*
 *录像app的设备响应函数
 */
 static void rec_control_ok(void *p, int err)
{
    if (err == 0) {
        puts("---rec control ok\n");
    } else {
        printf("---rec control faild: %d\n", err);
    }
    sys_touch_event_enable();
    //__this->key_disable = 0;
}
#if 1
 static int video_dzg_key_event_handler(struct key_event *key)
{
	 struct intent it;
    struct application *app;
    int err ;
#if 1
            u8 i = 0;
    switch (key->event) {
    case KEY_EVENT_CLICK:
		//hide_main1_ui();
		printf("jjjjj\n");
		
        switch (key->value) {
        case KEY_OK:
#if 1
            //__this->key_disable = 1;
            sys_touch_event_disable();
            it.name = "video_rec";
            it.action = ACTION_VIDEO_DZG_REC;
            start_app_async(&it, rec_control_ok, NULL);
#endif

            break;	
        case KEY_MENU:
            break;
        case KEY_MODE:
            break;
        case KEY_UP:
            break;
        case KEY_DOWN:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
#endif
    return false;
}
#endif
static int video_rec_device_event_handler(struct sys_event *event)
{
    int err;
    struct intent it;

    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            video_rec_sd_in();
            break;
        case DEVICE_EVENT_OUT:
            puts("\n\n\n\ndev out\n\n\n\n");
            if (!fdir_exist(CONFIG_STORAGE_PATH)) {
                video_rec_sd_out();
            }
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "charger", 7)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("\n\ncharger in\n\n");
            if (__this->run_rec_start) {
                /* __this->run_rec_start = 0; */
                if ((__this->state == VIDREC_STA_IDLE) || (__this->state == VIDREC_STA_STOP)) {
                    video_rec_control(0);
                }
            } else {

                if (__this->wait) {
                    wait_completion_del(__this->wait);
                    __this->wait = 0;
                }
                __this->wait = wait_completion(rec_app_ready, (int (*)(void *))video_rec_control, NULL);
            }
            break;
        case DEVICE_EVENT_OUT:
            /*puts("charger out\n");
            if (__this->state == VIDREC_STA_START) {
                video_rec_stop(0);
                video_rec_post_msg("offREC");
            }*/
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "parking", 7)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("parking on\n");	//parking on
            pmsg("__this->lan_det_setting = %d\n", __this->lan_det_setting);
            #if 1
            if(__this->lan_det_setting == 1)  //by peng
            {
                ui_hide(ui_get_current_window_id());
                init_intent(&it);
                it.name = "video_rec";
                it.action = ACTION_BACK;
                start_app_async(&it, NULL, NULL);
                __this->lan_det_setting = 0;

                it.name = "video_rec";
                it.action = ACTION_VIDEO_REC_MAIN;
                start_app_async(&it, NULL, NULL);
            }
            #endif

            hide_main_ui();
#ifdef CONFIG_TOUCH_UI_ENABLE
            hide_home_main_ui();//录像在后台进入倒车隐藏主界面
#endif



            show_park_ui();
            if (__this->disp_state == DISP_BACK_WIN) {
                puts("\n back win re open==========\n");
                return true;
            }


            puts("\n parking on\n");
            __this->disp_state = DISP_PARK_WIN;
            video_disp_control();
            return true;

        case DEVICE_EVENT_OUT://parking off
            hide_park_ui();
            show_main_ui();
            puts("\n parking off\n");

#ifdef CONFIG_VIDEO1_ENABLE
            __this->video1_online = dev_online("video1");
            if (__this->video1_online) {
                __this->disp_state = DISP_RESET_WIN;
            } else {
                __this->disp_state = DISP_HALF_WIN;
            }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
            __this->video2_online = dev_online("uvc");
            if (__this->video2_online) {
                __this->disp_state = DISP_RESET_WIN;
            } else {
                __this->disp_state = DISP_HALF_WIN;
            }
#endif
            video_disp_control();
            return true;
        }
    }
#ifdef CONFIG_VIDEO1_ENABLE
    else if (!strcmp(event->arg, "video1")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            if (!__this->video1_online) {
                __this->video1_online = true;
                video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                __this->disp_state = DISP_BIG_SMALL;
                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }
            break;
        case DEVICE_EVENT_OUT:
            if (__this->video1_online) {
                __this->video1_online = false;
                if (__this->disp_state == DISP_BIG_SMALL) {
                    video1_disp_stop();
                    __this->disp_state = DISP_FRONT_WIN;
                } else if (__this->disp_state == DISP_HALF_WIN ||
                           __this->disp_state == DISP_BACK_WIN) {
                    __this->disp_state = DISP_RESET_WIN;
                    video_disp_control();
                }

                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }
            break;
        }
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    else if (!strncmp((char *)event->arg, "uvc", 3)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            if (!__this->video2_online) {
                __this->video2_online = true;
                __this->uvc_id = ((char *)event->arg)[3] - '0';
                video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                __this->disp_state = DISP_BIG_SMALL;
                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }
            break;
        case DEVICE_EVENT_OUT:
            if (__this->video2_online) {
                __this->video2_online = false;
                if (__this->disp_state == DISP_BIG_SMALL) {
                    video2_disp_stop();
                    __this->disp_state = DISP_FRONT_WIN;
                } else if (__this->disp_state == DISP_HALF_WIN ||
                           __this->disp_state == DISP_BACK_WIN) {
                    __this->disp_state = DISP_RESET_WIN;
                    video_disp_control();
                }

                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }
            break;
        }
    }
#endif

    else if (!strcmp(event->arg, "gsen_lock")) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE:
#ifdef CONFIG_GSENSOR_ENABLE
            if (get_gsen_active_flag()) {
                if (__this->state == VIDREC_STA_START) {
                    if (db_select("gra")) {
                        if (__this->gsen_lock == 0) {
                            /* video_rec_stop_count_down(10); */
                        }
                        __this->gsen_lock = 1;
                        video_rec_post_msg("lockREC");
                    }
                }
                if (((__this->state != VIDREC_STA_START) && (!__this->park_wakeup))
                    || (db_select("gra") == 0)) {
                    clear_gsen_active_flag();
                }
            }
#endif
            break;
        }
    } else if (!strncmp(event->arg, "lane_set_open", strlen("lane_set_open"))) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE: {
            u32 aToint;
            ASCII_StrToInt(event->arg + strlen("lane_set_open"), &aToint, strlen(event->arg) - strlen("lane_set_open"));
            __this->car_head_y = aToint & 0x0000ffff;
            __this->vanish_y   = (aToint >> 16) & 0x0000ffff;
            ve_lane_det_start(1);
        }
        break;
        }
    } else if (!strncmp(event->arg, "lane_set_close", strlen("lane_set_close"))) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_CHANGE:
            ve_lane_det_stop(1);
            break;
        }
    } else if (!strcmp(event->arg, "camera0_err")) {
        __this->disp_state = DISP_RESET_WIN;
        video_disp_control();

        if (__this->state == VIDREC_STA_START) {
            video_rec_control(0);
            ve_mdet_reset();
            ve_lane_det_reset();

            video_rec_control(0);
        }
    }

    return false;
}

/*录像app的事件总入口*/
static int video_rec_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return video_rec_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return video_rec_device_event_handler(event);
    default:
        return false;
    }
}
static int video_dzg_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return video_dzg_key_event_handler(&event->u.key);
    default:
        return false;
    }
}
static const struct application_operation video_rec_ops = {
    .state_machine  = video_rec_state_machine,
    .event_handler 	= video_rec_event_handler,
};

REGISTER_APPLICATION(app_video_rec) = {
    .name 	= "video_rec",
    .action	= ACTION_VIDEO_REC_MAIN,
    .ops 	= &video_rec_ops,
    .state  = APP_STA_DESTROY,
};
static const struct application_operation video1_rec_ops = {
    .state_machine  = video_gps_state_machine,
    .event_handler 	= video_dzg_event_handler,
};

REGISTER_APPLICATION(app_video_gps) = {
    .name 	= "video_gps",
    .action	= ACTION_VIDEO_DZG_MAIN,
    .ops 	= &video1_rec_ops,
    .state  = APP_STA_DESTROY,
};
#if 0
static void video_test_timer(void *p)
{
    struct intent it;

    it.name	= "video_rec";
    it.data = NULL;


    puts("-------------video_test_timer\n");
    it.action = ACTION_BACK;
    start_app(&it);

    puts("-------------video_test_timer\n");
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}


void video_rec_test()
{
//    sys_timeout_add(0, video_test_timer, 5000);
}
#endif





