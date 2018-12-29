#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/ctp_server.h"
#include "server/video_engine_server.h"
#include "ui_ctr_def.h"
#include "video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"
#include "user_isp_cfg.h"
#include "os/os_compat.h"


#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "power_ctrl.h"
#include "app_database.h"
#include "server/net2video.h"
#include "server/net_server.h"

#define  QR_DECODE  0
#if QR_DECODE
#include "qr_decode.h"
#endif


#if defined CONFIG_WIFI_ENABLE
static int net_rt_video1_open(struct intent *it);
static int  net_rt_video0_open(struct intent *it);
static int net_video_rec_start(u8 mark);
static int net_video_rec_stop(u8 close);
#endif


extern int video_rec_set_config(struct intent *it);
extern int video_rec_get_config(struct intent *it);
extern const u16 rec_province_gb2312[];
extern const char rec_num_table[];


extern void wifi_on(void); 
extern void wifi_off(void);
extern char *get_wifi_ssid(void);
extern char *get_wifi_pwd(void);


struct video_rec_hdl rec_handler;
struct net_video_hdl net_rec_handler;

#define VIDEO0_REC_FORMAT   VIDEO_FMT_MOV

#if  defined __CPU_AC5401__
#define VIDEO1_REC_FORMAT   VIDEO_FMT_MOV
#define VIDEO2_REC_FORMAT   VIDEO_FMT_MOV
#elif defined __CPU_AC5601__
#define VIDEO1_REC_FORMAT   VIDEO_FMT_AVI
#define VIDEO2_REC_FORMAT   VIDEO_FMT_AVI
#else
#error "undefined CPU"
#endif
#define VIDEO3_REC_FORMAT   VIDEO_FMT_AVI


char video_rec_osd_buf[64] ALIGNE(64);

#define __this 	(&rec_handler)
#define sizeof_this     (sizeof(struct video_rec_hdl))

#define __this_net 	(&net_rec_handler)

#define sizeof_this_net     (sizeof(struct net_video_hdl))


#define VREC0_FBUF_SIZE     (6*1024*1024)
#define VREC1_FBUF_SIZE     (4*1024*1024)
#define VREC2_FBUF_SIZE     (4*1024*1024)
#define VREC3_FBUF_SIZE     (12*1024*1024)
#define AUDIO_BUF_SIZE     (300*1024)

#define NET_VREC0_FBUF_SIZE     (4*1024*1024)
#define NET_VREC1_FBUF_SIZE     (2*1024*1024)

#define  AUDIO_SAM_RATE  8000
#define  VIDEO_REC_FPS0       0  //不使用0作为默认值，写具体数值
#define  VIDEO_REC_FPS1       0  //不使用0作为默认值，写具体数值
#define  NET_VIDEO_REC_FPS0   25  //不使用0作为默认值，写具体数值

#define SCREEN_W        LCD_DEV_WIDTH//1280//480     //16 aline
#define SCREEN_H        LCD_DEV_HIGHT//720//272     //16 aline
#ifdef CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
#define SMALL_SCREEN_W          480 //16 aline
#define SMALL_SCREEN_H          400 //16 aline
#else
#define SMALL_SCREEN_W          320 //16 aline
#define SMALL_SCREEN_H          240 //16 aline
#endif

#define MOTION_STOP_SEC         20
#define MOTION_START_SEC        2

#define DEL_LOCK_FILE_EN		0
#define LOCK_FILE_PERCENT		40    //0~100

#ifdef CONFIG_TOUCH_UI_ENABLE
#define SCREEN_H_OFFSET (720 - SCREEN_H)
#else
#define SCREEN_H_OFFSET (0)
#endif


#define NET_VREC2_FBUF_SIZE     (SCREEN_W*SCREEN_H*3/2)



extern int storage_device_ready();
static int video_rec_control(void *_run_cmd);
static int video_rec_stop(u8 close);
static int video_disp_control();
static int ve_mdet_start();
static int ve_mdet_stop();
static int ve_lane_det_start(u8 fun_sel);
static int ve_lane_det_stop(u8 fun_sel);
static int video0_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
static int video1_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
static int video2_disp_start(u16 width, u16 height, u16 xoff, u16 yoff);
static void video0_disp_stop();
static void video1_disp_stop();
static void video2_disp_stop();

static int  net_rt_video0_stop(struct intent *it);
static int  net_rt_video1_stop(struct intent *it);
static int video_rec_start_isp_scenes();
static int video_rec_stop_isp_scenes(u8 status, u8 restart);
static void video_rec_cycle(void *parm);

static int video_rec_get_abr(u32 width);
static void ctp_cmd_notity(const char *path, u32 status);
static void video_rec_get_app_status(struct intent *it);
static void video_rec_get_path(struct intent *it);
static int video_rec_get_abr(u32 width);
static int video_rec_savefile(int dev_id);
static void video_rec_rename_file(int id, FILE *file, int fsize, int format);
static int video_rec_cmp_fname(void *afile, void *bfile);

//后视专用
static int net_h264_start(u8 flag);
static int net_jpeg_start(u8 flag);
static int net_jpeg_stop(u8 close);
static int net_h264_stop(u8 close, u8 flag);



#if DEL_LOCK_FILE_EN
static int video_rec_del_lock_file();
#endif
u32 get_video_disp_state()
{
    return __this->disp_state;
}

u8 get_wifi_on()
{
	return __this->wifi_on;
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











//NET USE API

static void video_rec_get_app_status(struct intent *it)
{
//   printf("__this->state %d tmp 0x%x\n", __this->state, tmp);

    it->data = (const char *)__this;
    it->exdata = (u32)__this_net;
}

static char file_str[64];
static void video_rec_get_path(struct intent *it)
{

    u8  buf[32];
#ifdef CONFIG_VIDEO0_ENABLE

    if (!strcmp(it->data, "video_rec0") && __this->file[0] != NULL) {
        fget_name((FILE *)__this->file[0], buf, sizeof(buf));
        sprintf(file_str, CONFIG_REC_PATH_1"%s", buf);
        it->data = file_str;
    }

#endif
    /* #ifdef CONFIG_VIDEO1_ENABLE || CONFIG_VIDEO3_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    if (!strcmp(it->data, "video_rec1") && __this->file[1] != NULL) {
        fget_name((FILE *)__this->file[1], buf, sizeof(buf));
        sprintf(file_str, CONFIG_REC_PATH_2"%s", buf);
        it->data = file_str;
    } else if (!strcmp(it->data, "video_rec2") && __this->file[2] != NULL) {
        fget_name((FILE *)__this->file[2], buf, sizeof(buf));
        sprintf(file_str, CONFIG_REC_PATH_2"%s", buf);
        it->data = file_str;
    }


    else
#endif
    {
        puts("get file name fail\n");
        it->data = NULL;
    }

    printf("file %s \n", it->data);
}
static void ctp_cmd_notity(const char *path, u32 status)
{

#if defined CONFIG_ENABLE_VLIST
    FILE_LIST_ADD(status, path);
#endif
}

int get_video_fps0() //前视
{

    return VIDEO_REC_FPS0;
}

int get_video_fps1() //后视
{
    return VIDEO_REC_FPS1;
}

int get_net_video_fps() //前视
{

    return NET_VIDEO_REC_FPS0;
}

int get_audio_rate()
{
    return AUDIO_SAM_RATE;
}


/*************************************************************/



static const u16 rec_pix_w[] = {1920, 1280, 640};
static const u16 rec_pix_h[] = {1088, 720,  480};
static const u16 pic_pix_w[] = {1920, 1280, 640, 3072};
static const u16 pic_pix_h[] = {1080,  720, 480, 2208};

static void rec_dev_server_event_handler(void *priv, int argc, int *argv)
{
    char buf[32];
    /*
     *该回调函数会在录像过程中，写卡出错被当前录像APP调用，例如录像过程中突然拔卡
     */
    switch (argv[0]) {
    case VIDEO_SERVER_PKG_ERR:
        if ((__this->video1_online) || (__this->video2_online)  || (__this->video3_online)) {
            if (__this->state == VIDREC_STA_STOP) {
                if (__this->sd_wait) {
                    wait_completion_del(__this->sd_wait);
                    __this->sd_wait = 0;
                }
                __this->sd_wait = wait_completion(storage_device_ready, video_rec_control, NULL);
                break;
            }
        }
#if defined CONFIG_WIFI_ENABLE
        sprintf(buf, "status:%d", 0);
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CTRL", "NOTIFY", buf);
#endif

        video_rec_post_msg("offREC");
        video_home_post_msg("offREC");
        video_parking_post_msg("offREC");

        if (__this->state == VIDREC_STA_START) {
            video_rec_stop(0);
#if defined CONFIG_WIFI_ENABLE

            struct intent *it = NULL;
            if (__this_net->net_video0_vrt_on) {
                net_rt_video0_open(it);

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
            } else if (__this_net->net_video1_vrt_on) {
                net_rt_video1_open(it);

#endif
            }

#endif
        }
        if ((__this->video1_online == 0) && (__this->video2_online == 0) && (__this->video3_online == 0)) {
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
    case VIDEO_SERVER_PKG_NET_ERR:
        puts("VIDEO_SERVER_PKG_NET_ERR\n\n\n");

#if defined CONFIG_WIFI_ENABLE
        /* net_video_rec_stop(0); */

        int mark = 2;
        struct intent it;
        init_intent(&it);
        it.data = &mark;

        net_rt_video0_stop(&it);

        net_rt_video1_stop(&it);

#ifdef CONFIG_NET_CLIENT
        if (__this_net->net_video0_vrt_on) {
            net_h264_stop(0, 0);
        }
        if (__this_net->net_video1_vrt_on) {
            net_jpeg_stop(0);
        }
#endif
#endif


        break;
    default :
        puts("\n\nwrong rec server cmd !\n\n");
        break;
    }
}
static int video_rec_online_nums()
{
    u8 nums = 1;

    if (__this->video1_online) {
        nums++;
    }
    if (__this->video2_online) {
        nums++;
    }
    if (__this->video3_online) {
        nums++;
    }

    return nums;
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
        if (__this->state == VIDREC_STA_START) {
            video_rec_control(0);
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

        puts("==lane dete waring==\n");
        break;
    case VE_MSG_LANE_DETCET_LEFT:
        puts("==lane dete waring==l\n");
        break;
    case VE_MSG_LANE_DETCET_RIGHT:
        puts("==lane dete waring==r\n");
        break;
    case VE_MSG_VEHICLE_DETECT_WARNING:
        //printf("x = %d,y = %d,w = %d,hid = %d\n",argv[1],argv[2],argv[3],argv[4]);
        //位置
        video_rec_post_msg("carpos:p=%4", ((u32)(argv[1]) | (argv[2] << 16))); //x:x y:y
        //颜色
        if (argv[3] > 45) {
            video_rec_post_msg("carpos:w=%4", ((u32)(argv[3]) | (3 << 16)));    //w:width c:color,0:transparent, 1:green,2:yellow,3:red
        } else {
            video_rec_post_msg("carpos:w=%4", ((u32)(argv[3]) | (1 << 16)));
        }
        //隐藏
        if (argv[4] == 0) {
            video_rec_post_msg("carpos:w=%4", ((u32)(1) | (0 << 16)));
        }
        //刷新
        video_rec_post_msg("carpos:s=%4", 1);
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
        ve_req.hint_info.hint = ((1 << VE_MODULE_MOTION_DETECT) | (1 << VE_MODULE_LANE_DETECT));

        if (fun_sel) {
            ve_req.hint_info.hint = 0;
            ve_req.hint_info.hint = (1 << VE_MODULE_LANE_DETECT);
        }

#ifdef USE_VE_MOTION_DETECT_MODE_ISP
        ve_req.hint_info.mode_hint0 = (VE_MOTION_DETECT_MODE_ISP
                                       << (VE_MODULE_MOTION_DETECT * 4));
#else
        ve_req.hint_info.mode_hint0 = (VE_MOTION_DETECT_MODE_NORMAL
                                       << (VE_MODULE_MOTION_DETECT * 4));
#endif

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
#ifdef USE_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif
    ve_req.cmd = 0;

    server_request(__this->video_engine, VE_REQ_MODULE_OPEN, &ve_req);


    server_request(__this->video_engine, VE_REQ_MODULE_GET_PARAM, &ve_req);


    /*
    *移动侦测的检测启动时间和检测静止的时候
    **/
    ve_req.md_params.level = 2;
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
#ifdef USE_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

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
#if QR_DECODE
    void get_yuv_uninit(void);
    u8 get_qr_init_state(void);
    if (get_qr_init_state()) {
        qr_decode_uninit();
        get_yuv_uninit();
    }
#endif

    ve_req.module = VE_MODULE_LANE_DETECT;
#ifdef USE_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

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
#ifdef USE_VE_MOTION_DETECT_MODE_ISP
    ve_req.md_mode = VE_MOTION_DETECT_MODE_ISP;
#else
    ve_req.md_mode = VE_MOTION_DETECT_MODE_NORMAL;
#endif

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
static u32 video_rec_get_fsize(u8 cycle_time, u16 vid_width, int format)
{
    u32 fsize;
    //
    //8250 = 1000*60/8 *110 /100 ;
    //
    if (cycle_time == 0) {
        cycle_time = 5;
    }
    fsize  = ((video_rec_get_abr(vid_width)) * cycle_time) * 8250;
    if (format == VIDEO_FMT_AVI) {
        fsize *= 2;
    }

    return fsize;
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
static FILE *video_rec_get_first_file(int id, int *arg)
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
#ifdef CONFIG_VIDEO4_ENABLE
    video_rec_fscan_dir(3, CONFIG_REC_PATH_3);
#endif

    for (int i = 0; i < 4; i++) {
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
        /* 查看优先删除的文件夹是否满足删除条件 */
        if (__this->file_number[id] + 3 > __this->file_number[max_index]) {
            max_index = id;
        }
    }

    *arg = max_index; // 用于文件列表删除
    FILE *f = fselect(__this->fscan[max_index], FSEL_FIRST_FILE, 0);
    if (f) {

        if (video_rec_cmp_fname(__this->file[max_index], f)) {
            puts("\n\n\n\ndelout same_file!!!\n\n\n");
            fclose(f);
            return NULL;
        }

        __this->file_number[max_index]--;
        __this->old_file_number[max_index]--;
    }

    return f;
}

static void video_rec_rename_file(int id, FILE *file, int fsize, int format)
{
    char filename[64];
    u8 name[20];

    char file_name[64];

    int err = fcheck(file);
    if (err) {
        puts("\n\nfcheck fail\n\n");
        fget_name(file, name, 20);
        fdelete(file);
#if defined CONFIG_ENABLE_VLIST
        if (id == 0) {
            sprintf(filename, CONFIG_REC_PATH_1"%s", name);
        } else {

            sprintf(filename, CONFIG_REC_PATH_2"%s", name);
        }
        FILE_DELETE(filename);
#endif
        return;
    }

    int present = (flen(file) / 1024) * 100 / (fsize / 1024);
    if (present >= 90 && present <= 110) {
        fget_name(file, name, 20);
        if (format == VIDEO_FMT_MOV) {
            sprintf(file_name, "DCIM/%d/vid_***.mov", (id == 0) ? 1 : id);
        } else if (format == VIDEO_FMT_AVI) {
            sprintf(file_name, "DCIM/%d/vid_***.avi", (id == 0) ? 1 : id);
        }

        int err = fmove(file, file_name, &__this->new_file[id], 1);
        if (err == 0) {

#if defined CONFIG_ENABLE_VLIST
            if (id == 0) {
                sprintf(filename, CONFIG_REC_PATH_1"%s", name);
            } else {

                sprintf(filename, CONFIG_REC_PATH_2"%s", name);
            }
            FILE_DELETE(filename);
#endif

            return;
        }
    }

    puts("\n\ndel file\n\n");
    fget_name(file, name, 20);
    fdelete(file);
#if defined CONFIG_ENABLE_VLIST
    if (id == 0) {
        sprintf(filename, CONFIG_REC_PATH_1"%s", name);
    } else {

        sprintf(filename, CONFIG_REC_PATH_2"%s", name);
    }
    FILE_DELETE(filename);
#endif

}

static int video_rec_del_old_file(int id)
{
    int err;
    FILE *file;
    int arg = 0;
    int fsize[4];
    u32 cur_space;
    u32 need_space = 0;

    char filename[64];
    u8 name[20];
    int cyc_time = db_select("cyc");
    int format[4] = {VIDEO0_REC_FORMAT, VIDEO1_REC_FORMAT, VIDEO2_REC_FORMAT, VIDEO3_REC_FORMAT};

#ifdef CONFIG_VIDEO0_ENABLE
    fsize[0] =  video_rec_get_fsize(cyc_time, rec_pix_w[db_select("res")], VIDEO0_REC_FORMAT);
    need_space += fsize[0];
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    fsize[1] =  video_rec_get_fsize(cyc_time, AVIN_WIDTH, VIDEO1_REC_FORMAT);
    need_space += fsize[1];
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    fsize[2] =  video_rec_get_fsize(cyc_time, UVC_ENC_WIDTH, VIDEO2_REC_FORMAT);
    need_space += fsize[2];
#endif

#ifdef CONFIG_VIDEO4_ENABLE
    fsize[3] =  video_rec_get_fsize(cyc_time, VIR_ENC_WIDTH, VIDEO3_REC_FORMAT);
    need_space += fsize[3];
#endif
    err = fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    if (err) {
        return err;
    }

    printf("space: %x, %x\n", cur_space / 1024, need_space / 1024 / 1024);

    if (cur_space >= 3 * (need_space / 1024)) {
        return 0;
    }

    while (1) {
        if (cur_space >= (need_space / 1024) * 2) {
            break;
        }
        file = video_rec_get_first_file(-1, &arg);
        if (!file) {
            return -ENOMEM;
        }

        fget_name(file, name, 20);
        fdelete(file);
#if defined CONFIG_ENABLE_VLIST
        if (arg == 0) {
            sprintf(filename, CONFIG_REC_PATH_1"%s", name);
        } else {

            sprintf(filename, CONFIG_REC_PATH_2"%s", name);
        }
        FILE_DELETE(filename);
#endif


        fget_free_space(CONFIG_ROOT_PATH, &cur_space);
    }

    file = video_rec_get_first_file(id, &arg);
    if (file) {
        video_rec_rename_file(id, file, fsize[id], format[id]);
    }

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

#ifdef CONFIG_VIDEO4_ENABLE
        sel_mode = FSEL_FIRST_FILE;
        do {
            if (__this->fscan[3] == NULL) {
                __this->fscan[3] = fscan(CONFIG_REC_PATH_3, "-tMOVAVI -sn -ar");
                if (__this->fscan[3] == NULL) {
                    break;
                }
            }
            file = fselect(__this->fscan[3], sel_mode, 0);
            if (file) {
                __this->total_lock_fsize += (flen(file) / 1024);
                sel_mode = FSEL_NEXT_FILE;
                fclose(file);//!!! must close
            } else {
                break;
            }

        } while (1);

        if (__this->fscan[3]) {
            fscan_release(__this->fscan[3]);
            __this->fscan[3] = NULL;
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


/*码率控制，根据具体分辨率设置*/
static int video_rec_get_abr(u32 width)
{
    if (width <= 720) {
        /* return 8000; */
        return 3200;
    } else if (width <= 1280) {
        /* return 3200; */
        return 7000;
        /* return 10000; */
    } else if (width <= 1920) {
        return 10000;
    } else {
        return 18000;
    }
}

int video_rec_get_file_name_and_size(int id,  const char **fname, u32 *fsize, int width, int format)
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

    if (format == VIDEO_FMT_MOV) {
        *fname = "vid_***.mov";
    } else {
        *fname = "vid_***.avi";
    }

    *fsize = video_rec_get_fsize(db_select("cyc"), width, format);

    return 0;
}

/*后拉专用*/
/*
 *  *场景切换使能函数，如果显示打开就用显示句柄控制，否则再尝试用录像句柄控制
 *   */
static int net_video_rec_start_isp_scenes()
{
    if (__this_net->isp_scenes_status) {
        return 0;
    }

    stop_update_isp_scenes();
    if (__this_net->net_video_rec2 && ((__this_net->net_state1 == VIDREC_STA_START) ||
                                       (__this_net->net_state1 == VIDREC_STA_STARTING))) {
        __this_net->isp_scenes_status = 1;
        return start_update_isp_scenes(__this_net->net_video_rec2);
    } else if (__this_net->net_video_rec && ((__this_net->net_state == VIDREC_STA_START) ||
               (__this_net->net_state == VIDREC_STA_STARTING))) {
        __this_net->isp_scenes_status = 2;
        return start_update_isp_scenes(__this_net->net_video_rec);
    }

    __this_net->isp_scenes_status = 0;

    return 1;
}
static int net_video_rec_stop_isp_scenes(u8 status, u8 restart)
{

    if (__this_net->isp_scenes_status == 0) {
        return 0;
    }

    if ((status != __this_net->isp_scenes_status) && (status != 3)) {
        return 0;
    }

    __this_net->isp_scenes_status = 0;
    stop_update_isp_scenes();

    if (restart) {
        video_rec_start_isp_scenes();
    }

    return 0;
}

static int net_h264_start(u8 flag)
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    /* char buf[128]; */
    puts("start_net_video_rec\n");

    __this_net->net_state = VIDREC_STA_STARTING;
    if (!__this_net->net_video_rec) {
        __this_net->net_video_rec = server_open("video_server", "video0");

        if (!__this_net->net_video_rec) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this_net->net_video_rec, (void *)0, rec_dev_server_event_handler);
    }

    /*
     *       *通道号，分辨率，封装格式，写卡的路径
     *            */

//   memcpy((void *)&req.rec, (void *)&__this_net->remote.rec_info, sizeof(struct vs_video_rec));
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.channel = 0;

    req.rec.format  = NET_VIDEO_FMT_MOV;
    req.rec.state   = VIDEO_STATE_START;
    req.rec.fpath   = CONFIG_REC_PATH_1;
    req.rec.width   = 1280; // __this_net->remote.width;
    req.rec.height  = 720;//__this_net->remote.height;

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);

    if (flag) {
        req.rec.net_par.net_vidrt_onoff = 1;

        req.rec.net_par.net_audrt_onoff  = 1;
    } else {
        req.rec.net_par.small_pic_first_start = 1;
    }
    req.rec.camera_config = NULL;

    req.rec.audio.buf = __this_net->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.volume    = 63;
    req.rec.pkg_mute.aud_mute = !db_select("mic");


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








    req.rec.buf = __this_net->net_v0_fbuf;
    req.rec.buf_len = NET_VREC0_FBUF_SIZE;

    req.rec.buf = __this->v0_fbuf;
    req.rec.buf_len = VREC0_FBUF_SIZE;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif

    req.rec.cycle_time = 0;

    strcpy(req.rec.net_par.netpath, "cty://192.168.1.1:10000");

    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }
    __this_net->net_video0_vrt_on = 1;
    __this_net->net_video0_art_on = 1;


    __this_net->net_state = VIDREC_STA_START;

    net_video_rec_start_isp_scenes();
    return 0;
}

static int net_h264_stop(u8 close, u8 flag)
{
    union video_req req = {0};
    int err;
    puts("\nnet video h264 stop\n");
    if (__this_net->net_video_rec) {
        __this_net->net_state = VIDREC_STA_STOPING;
        net_video_rec_stop_isp_scenes(2, 0);
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        if (flag) {
            req.rec.net_par.small_pic_first_start = 1;
        }
        log_d("rec->net_par.small_pic_first_start2222=%d\n", req.rec.net_par.small_pic_first_start);
        err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }

        net_video_rec_start_isp_scenes();
        if (close) {
            if (__this_net->net_video_rec) {
                server_close(__this_net->net_video_rec);
                __this_net->net_video_rec = NULL;
            }
        }

        __this_net->net_state = VIDREC_STA_STOP;
    }

    __this_net->net_video0_vrt_on = 0;
    __this_net->net_video0_art_on = 0;
    return 0;
}

static int net_jpeg_start(u8 flag)
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    puts("start_net_video_rec\n");

    __this_net->net_state1 = VIDREC_STA_STARTING;
    if (!__this_net->net_video_rec2) {
        __this_net->net_video_rec2 = server_open("video_server", "video0");

        if (!__this_net->net_video_rec2) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this_net->net_video_rec2, (void *)0, rec_dev_server_event_handler);
    }

    /*
     *       *通道号，分辨率，封装格式，写卡的路径
     *            */
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.channel = 1;
    req.rec.width   = __this_net->remote.width;
    req.rec.height  = __this_net->remote.height;
    log_d("set display width:%d  height:%d\n\n\n", __this_net->remote.width, __this_net->remote.height);

    req.rec.format  = NET_VIDEO_FMT_AVI;
    req.rec.state   = VIDEO_STATE_START;
    req.rec.fpath   = CONFIG_REC_PATH_1;


    req.rec.quality = VIDEO_LOW_Q;

    req.rec.fps = 0;

    if (flag) {
        req.rec.net_par.net_vidrt_onoff = 1;
    } else {
        req.rec.net_par.small_pic_first_start = 1;
    }
    req.rec.camera_config = NULL;

    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    req.rec.audio.channel   = 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this_net->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = 1;
    /*
     *      *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *           *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     *                * roio_ratio : 区域比例系数
     *                     */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    if (req.rec.width >= 1280) {
        req.rec.abr_kbps = 9000;
    }
    /* req.rec.abr_kbps = 2000;//video_rec_get_abr(req.rec.width); */
    req.rec.IP_interval = 0;
    /*感兴趣区域为下方 中间 2/6 *4/6 区域，可以调整
     *      感兴趣区域qp 为其他区域的 70% ，可以调整
     *          */
#if 0
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

    /*
     *          * osd 相关的参数，注意坐标位置，x要64对齐，y要16对齐,底下例子是根据图像大小偏移到右下
     *                   */
    osd_info.osd_w = 16;
    osd_info.osd_h = 32;
    max_one_line_strnum = strlen(video_rec_osd_buf);//21;

    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }

    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height;
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
#endif
    /*
     *      *慢动作倍数(与延时摄影互斥,无音频); 延时录像的间隔ms(与慢动作互斥,无音频)
     *           */
    /* req.rec.slow_motion = 0; */
    /* req.rec.tlp_time = db_select("gap"); */

    /* printf("\n\ntl_time : %d\n\n", req.rec.tlp_time); */
    /* if (req.rec.slow_motion || req.rec.tlp_time) { */
    /* } */

    req.rec.buf = __this_net->net_v1_fbuf;
    req.rec.buf_len = NET_VREC1_FBUF_SIZE;
    /*
     *      *循环录像时间，文件大小
     *           */
    req.rec.cycle_time = 0;

    strcpy(req.rec.net_par.netpath, "cty://192.168.1.1:10001");

    err = server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }

    __this_net->net_video1_vrt_on = 1;
    __this_net->net_video1_art_on = 1;
    __this_net->net_state1 = VIDREC_STA_START;
    net_video_rec_start_isp_scenes();
    return 0;
}


static int net_jpeg_stop(u8 close)
{
    union video_req req = {0};
    int err;

    puts("\nnet video jpeg stop\n");
    __this_net->net_state1 = VIDREC_STA_STOPING;
    if (__this_net->net_video_rec2) {
        net_video_rec_stop_isp_scenes(2, 0);
        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }

        net_video_rec_start_isp_scenes();
        if (close) {
            if (__this_net->net_video_rec2) {
                server_close(__this_net->net_video_rec2);
                __this_net->net_video_rec2 = NULL;
            }
        }
    }

    __this_net->net_video1_vrt_on = 0;
    __this_net->net_video1_art_on = 0;

    __this_net->net_state1 = VIDREC_STA_STOP;
    return 0;
}


#ifdef CONFIG_VIDEO0_ENABLE
/******* 不要单独调用这些子函数 ********/
static int video0_rec_ch2_start()
{
    int err;
#if defined CONFIG_WIFI_ENABLE



    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;

    puts("start_video_rec ch2 \n");
    if (!__this->audio_buf) {
        __this->audio_buf = malloc(300 * 1024);
        if (!__this->audio_buf) {
            return -ENOMEM;
        }
    }


    u32 res = db_select("res");
    /*
     *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.channel = 1;
    req.rec.width 	= rec_pix_w[res];
    req.rec.height 	= rec_pix_h[res];
    /* req.rec.format 	= VIDEO_FMT_MOV; */
    /* req.rec.state 	= VIDEO_STATE_START; */
    req.rec.fpath 	= CONFIG_REC_PATH_1;

    /*
     *录像名字，加*表示自动增加序号，支持全英文的长文件名；帧率为0表示使用摄像头的帧率
     */
    req.rec.fname    = "vid_****.mov";
    req.rec.quality = VIDEO_MID_Q;
    /* req.rec.fps 	= VIDEO_REC_FPS; */
    req.rec.fps     = get_net_video_fps();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video0_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;
    video_rec_get_file_name_and_size(0, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);
    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    /* req.rec.audio.sample_rate = 8000; */
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;

    if (res == 0) {
        req.rec.pkg_mute.aud_mute = 0 ;
    } else {
        req.rec.pkg_mute.aud_mute = 1 ;
    }

    /* req.rec.pkg_mute.aud_mute = !db_select("mic"); */

    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;


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

    struct drop_fps targe_fps;
    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        targe_fps.fps_a = 1000;
        targe_fps.fps_b = req.rec.tlp_time;
        req.rec.targe_fps = &targe_fps;
    }
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
    puts("\nstart video0 wifi ch1 \n");
    req.rec.format 	= __this_net->net_videoreq[0].rec.format;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video0_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;

    {
        struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
        /*
         *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
          *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
        *
         * */
#if defined CONFIG_NET_UDP_ENABLE
        sprintf(req.rec.net_par.netpath, "udp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _FORWARD_PORT);
#else
        sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _FORWARD_PORT);
#endif

    }




    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }

    __this->state_ch2 = VIDREC_STA_START;

    puts("start_video_rec ch2-\n");
#endif

    return 0;
}
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
    /* req.rec.width 	= rec_pix_w[2]; */
    /* req.rec.height 	= rec_pix_h[2]; */
#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
    req.rec.format 	=  NET_VIDEO_FMT_MOV;
#else
    req.rec.format 	= VIDEO0_REC_FORMAT;
#endif
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_1;

    /*
     *录像名字，加*表示自动增加序号，支持全英文的长文件名；帧率为0表示使用摄像头的帧率
     */
    video_rec_get_file_name_and_size(0, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);

    req.rec.quality = VIDEO_MID_Q;
    /* req.rec.fps 	= VIDEO_REC_FPS; */
#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
    sprintf(req.rec.net_par.netpath, "rf://--");
    req.rec.fps 	= 0;
    req.rec.net_par.net_audrt_onoff = 0;
    req.rec.net_par.net_vidrt_onoff = 1;
#else
    req.rec.fps     = get_video_fps0();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video0_art_on;
    req.rec.net_par.net_vidrt_onoff  = __this_net->net_video0_vrt_on;
#endif
    /*
     *采样率，通道数，录像音量，音频使用的循环BUF,录不录声音
     */
    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    /* req.rec.audio.sample_rate = 8000; */
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
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

    req.rec.net_par.net_type = NET_FMT_RF;
#if  defined __CPU_AC5401__
    req.rec.IP_interval = 0;
#elif defined __CPU_AC5601__
    if (req.rec.height > 720) {
        req.rec.IP_interval = 1;
    } else {
        req.rec.IP_interval = 4;
    }

#endif

#if 0
    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;
#endif


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

    struct drop_fps targe_fps;
    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        targe_fps.fps_a = 1000;
        targe_fps.fps_b = req.rec.tlp_time;
        req.rec.targe_fps = &targe_fps;
    }
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;

        puts("\n\n\n audio 00000000 \n\n\n");

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
    union video_req req = {0};

    if (!__this->video_rec0) {
        return -EINVAL;
    }

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
}

static int video0_rec_set_dr()
{
    union video_req req = {0};
    struct drop_fps targe_fps;

    if (!__this->video_rec0) {
        return -EINVAL;
    }

    targe_fps.fps_a = 20;
    targe_fps.fps_b = 1;
    req.rec.targe_fps = &targe_fps;

    req.rec.channel = 0;
    req.rec.state 	= VIDEO_STATE_SET_DR;

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);

}

static int video0_rec_stop_count_down(u32 time)
{
    union video_req req = {0};

    if ((!__this->video_rec0) || (!__this->file[0])) {
        return -EINVAL;
    }
    req.rec.channel = 0;
    req.rec.format 	= VIDEO0_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
}

static int video0_rec_ch2_stop(u8 close)
{
    union video_req req = {0};
    int err;
#if defined CONFIG_WIFI_ENABLE

    if (__this->state_ch2 == VIDREC_STA_START) {
        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }

        __this->state_ch2 = VIDREC_STA_STOP;
        puts("stop_video_rec ch2-\n");
    }

#endif
    return 0;

}




static int video0_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;
    char path[128] = {0};
    u8 name[21] = {0};
    int ret = 0;
    fget_name(__this->file[0], name, 20);
    ret = sprintf(path, "%s%s", CONFIG_REC_PATH_1, name);
    video_rec_stop_isp_scenes(2, 0);

    if (__this->gsen_lock) {
        video_rec_lock_file(__this->file[0], 1);
    }
#if defined CONFIG_WIFI_ENABLE
    if (__this->state_ch2 == VIDREC_STA_START) {
        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_STOP;
        }

        __this->state_ch2 = VIDREC_STA_STOP;

    }

#endif


    req.rec.channel = 0;
    req.rec.state = VIDEO_STATE_STOP;

    err = server_request(__this->video_rec0, VIDEO_REQ_REC, &req);
    if (err != 0) {
        printf("\nstop rec err 0x%x\n", err);
        return VREC_ERR_V0_REQ_STOP;
    }
    __this->file[0] = NULL;
    ctp_cmd_notity(path, 0);


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
    union video_req req = {0};
    int err;
    char path[64];
    u8 name[20];

    if (__this->gsen_lock) {
        video_rec_lock_file(__this->file[0], 1);
    }

    u32 res = db_select("res");

    req.rec.channel = 0;
    req.rec.width 	= rec_pix_w[res];
    req.rec.height 	= rec_pix_h[res];
    req.rec.format 	= VIDEO0_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_SAVE_FILE;
    req.rec.fpath 	= CONFIG_REC_PATH_1;

    req.rec.fps     = get_video_fps0();
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
    video_rec_get_file_name_and_size(0, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    /* req.rec.audio.sample_rate = 8000; */
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
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

    fget_name(__this->file[0], name, 20);
    sprintf(path, "%s%s", CONFIG_REC_PATH_1, name);

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

    ctp_cmd_notity(path, 1);
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
    union video_req req = {0};
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
    union video_req req = {0};
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


struct imc_presca_ctl *video_disp_get_pctl()
{
#ifdef CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
    static struct imc_presca_ctl imc_presca_t = {0};

    imc_presca_t.presca_en = false;
    imc_presca_t.gs_parma = 0;
    imc_presca_t.gs_parmb = 0;
    imc_presca_t.gs_parmc = 255;
    imc_presca_t.gs_parmd = 0;

    return &imc_presca_t;
#else
    return NULL;
#endif
}

static int video0_disp_start(u16 width, u16 height, u16 xoff, u16 yoff)
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
    req.display.border_bottom = 0;
#endif

    req.display.camera_config = load_default_camera_config;
    req.display.camera_type = VIDEO_CAMERA_NORMAL;

    req.display.state 	= VIDEO_STATE_START;
    req.display.pctl = video_disp_get_pctl();
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
    union video_req req = {0};

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
static int video1_rec_ch2_start()
{
    int err;
#if defined CONFIG_WIFI_ENABLE
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;

    u32 res2 = db_select("res2");
    puts("\nvideo1 rec ch2 start\n");
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.channel = 0;
#if 0
    req.rec.width 	= AVIN_WIDTH;
    req.rec.height 	= AVIN_HEIGH;
#else
    req.rec.width 	= rec_pix_w[res2];
    req.rec.height 	= rec_pix_h[res2] ;
#endif
    /* req.rec.format 	= VIDEO_FMT_MOV; */
    /* req.rec.format 	= NET_VIDEO_FMT_MOV; */
    req.rec.format 	= __this_net->net_videoreq[1].rec.format;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;

    req.rec.fname    = "vid_***.mov";
    req.rec.quality = VIDEO_LOW_Q;
    /* req.rec.fps 	= VIDEO_REC_FPS; */
    req.rec.fps     = get_net_video_fps();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video1_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;
    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    /* req.rec.audio.sample_rate = 8000; */
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = 1;
    /* req.rec.pkg_mute.aud_mute = !db_select("mic"); */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

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
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic 	= 1;
#else
    req.rec.rec_small_pic 	= 0;
#endif

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
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }

    req.rec.cycle_time = req.rec.cycle_time * 60;
    {
        struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
        /*
         *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
          *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
        *
         * */
#if defined CONFIG_NET_UDP_ENABLE
        sprintf(req.rec.net_par.netpath, "udp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _BEHIND_PORT);
#else
        sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _BEHIND_PORT);
#endif

    }




    err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec2 ch2  err\n\n\n");
        return VREC_ERR_V1_REQ_START;
    }

    __this->state1_ch2 = VIDREC_STA_START;
#endif
    return 0;
}
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

    u32 res2 = db_select("res2");
    req.rec.channel = 1;
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.width 	= rec_pix_w[res2];
    req.rec.height 	= rec_pix_h[res2];
    req.rec.format 	= VIDEO1_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;


    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps     = get_video_fps1();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video1_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;
    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
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
    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

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

    struct drop_fps targe_fps;
    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        targe_fps.fps_a = 1000;
        targe_fps.fps_b = req.rec.tlp_time;
        req.rec.targe_fps = &targe_fps;
    }
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
    video_rec_get_file_name_and_size(1, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);

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
    union video_req req = {0};

    if (!__this->video_rec1) {
        return -EINVAL;
    }

    req.rec.channel = 1;
    req.rec.state 	= VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
}

static int video1_rec_set_dr()
{
    union video_req req = {0};
    struct drop_fps targe_fps;

    if (!__this->video_rec1) {
        return -EINVAL;
    }

    targe_fps.fps_a = 7;
    targe_fps.fps_b = 1;
    req.rec.targe_fps = &targe_fps;

    req.rec.channel = 1;
    req.rec.state 	= VIDEO_STATE_SET_DR;

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);

}

static int video1_rec_stop_count_down(u32 time)
{
    union video_req req = {0};

    if ((!__this->video_rec1) || (!__this->file[1])) {
        return -EINVAL;
    }
    req.rec.channel = 1;
    req.rec.format 	= VIDEO1_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
}
static int video1_rec_ch2_stop(u8 close)
{
    union video_req req = {0};
    int err;

    if (__this->state1_ch2 == VIDREC_STA_START) {
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec2 ch2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }

        __this->state1_ch2 = VIDREC_STA_STOP;
    }

    return  0;

}
static int video1_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;
    char path[128];
    u8 name[21];

    if (__this->video_rec1) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[1], 1);
        }

        req.rec.channel = 1;
        req.rec.state = VIDEO_STATE_STOP;
        fget_name(__this->file[1], name, 20);
        sprintf(path, "%s%s", CONFIG_REC_PATH_2, name);

        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }
        if (__this->file[1]) {
            ctp_cmd_notity(path, 0);
        }
    }
    __this->file[1] = NULL;
    if (__this->state1_ch2 == VIDREC_STA_START) {
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec1, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec2 ch2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }

        __this->state1_ch2 = VIDREC_STA_STOP;
    }
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
    union video_req req = {0};
    int err;
    char path[64];
    u8 name[20];

    if (__this->video_rec1) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[1], 1);
        }

        u32 res2 = db_select("res2");
        req.rec.channel = 1;
#if 0
        req.rec.width 	= AVIN_WIDTH;
        req.rec.height 	= AVIN_HEIGH;
#else
        req.rec.width 	= rec_pix_w[res2];
        req.rec.height 	= rec_pix_h[res2];
#endif
        req.rec.format 	= VIDEO1_REC_FORMAT;
        req.rec.state 	= VIDEO_STATE_SAVE_FILE;
        req.rec.fpath 	= CONFIG_REC_PATH_2;

        /* req.rec.fps 	= VIDEO_REC_FPS; */

        req.rec.fps     = get_video_fps1();
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
        video_rec_get_file_name_and_size(1, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);


        /* if (__this->rec_info->voice_on) { */
        req.rec.audio.sample_rate = AUDIO_SAM_RATE;
        /* req.rec.audio.sample_rate = 8000; */
        req.rec.audio.channel 	= 1;
        req.rec.audio.type 	= AUDIO_FMT_PCM;
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
        fget_name(__this->file[1], name, 20);
        sprintf(path, "%s%s", CONFIG_REC_PATH_2, name);



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

    ctp_cmd_notity(path, 1);
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
    union video_req req = {0};
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
    union video_req req = {0};
    struct imc_osd_info osd_info;
    int err;
    u32 res2 = db_select("res2");

    if (__this->video_rec1) {
        req.rec.width 	= rec_pix_w[res2];
        req.rec.height 	= rec_pix_h[res2];

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
    req.display.pctl = video_disp_get_pctl();

    server_request(__this->video_display_1, VIDEO_REQ_DISPLAY, &req);

#endif
    return 0;
}

static void video1_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO1_ENABLE)
    union video_req req = {0};
    if (__this->video_display_1) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(__this->video_display_1, VIDEO_REQ_DISPLAY, &req);

        server_close(__this->video_display_1);
        __this->video_display_1 = NULL;
    }
#endif
}



#ifdef CONFIG_VIDEO3_ENABLE
static int video2_rec_ch2_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;

    puts("start_video_ch2_rec2 \n");
    /* if (!__this->video_rec2) { */
    /*     __this->video_rec2 = server_open("video_server", "video3"); */
    /*     if (!__this->video_rec2) { */
    /*         return -EINVAL; */
    /*     } */
    /*  */
    /* server_register_event_handler(__this->video_rec2, NULL, rec_dev_server_event_handler); */
    /* } */
    /*  */
    u32 res2 = db_select("res2");
    req.rec.channel = 0;
    req.rec.camera_type = VIDEO_CAMERA_UVC;
#if 0
    req.rec.width 	= UVC_ENC_WIDTH;
    req.rec.height 	= UVC_ENC_HEIGH;
#else
    req.rec.width 	= rec_pix_w[res2];
    req.rec.height 	= rec_pix_h[res2];
#endif
    /* req.rec.format 	= VIDEO_FMT_MOV; */
    /* req.rec.format 	= NET_VIDEO_FMT_MOV; */
    req.rec.format 	= __this_net->net_videoreq[1].rec.format;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;
    req.rec.uvc_id = __this->uvc_id;

    req.rec.fname    = "vid_****.mov";
    req.rec.quality = VIDEO_LOW_Q;
    /* req.rec.fps 	= VIDEO_REC_FPS; */
    req.rec.fps     = get_net_video_fps();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video1_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    /* req.rec.audio.sample_rate = 8000; */
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.type  = AUDIO_FMT_PCM;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = !db_select("mic");
    /* req.rec.pkg_mute.aud_mute = 1; */

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

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
    req.rec.rec_small_pic 	= 0;

    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }

    req.rec.cycle_time = req.rec.cycle_time * 60;

    {
        struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
        /*
         *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
          *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
        *
         * */
#if defined CONFIG_NET_UDP_ENABLE
        sprintf(req.rec.net_par.netpath, "udp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _BEHIND_PORT);
#else
        sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _BEHIND_PORT);
#endif

    }

    req.rec.rec_small_pic 	= 0;



    err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return -EINVAL;
    }
    __this->state1_ch2 = VIDREC_STA_START;

    /* req.rec.channel = 2; */
    /* req.rec.state 	= VIDEO_STATE_GET_INFO; */
    /* server_request(__this->video_rec2, VIDEO_REQ_REC, &req); */
    /* __this->file2 = req.rec.get_info.file; */

    return 0;
}

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

    u32 res2 = db_select("res2");
    req.rec.channel = 2;
    req.rec.camera_type = VIDEO_CAMERA_UVC;
    req.rec.width 	= rec_pix_w[res2];
    req.rec.height 	= rec_pix_h[res2];
    req.rec.format 	= VIDEO2_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;
    req.rec.uvc_id = __this->uvc_id;

    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps     = get_video_fps1();

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.type  = AUDIO_FMT_PCM;
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
    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

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

    struct drop_fps targe_fps;
    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        targe_fps.fps_a = 1000;
        targe_fps.fps_b = req.rec.tlp_time;
        req.rec.targe_fps = &targe_fps;
    }
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->v2_fbuf;
    req.rec.buf_len = VREC2_FBUF_SIZE;
    req.rec.rec_small_pic 	= 0;


    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    video_rec_get_file_name_and_size(2, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);

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
    union video_req req = {0};

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
    union video_req req = {0};

    if ((!__this->video_rec2) || (!__this->file[2])) {
        return -EINVAL;
    }
    req.rec.channel = 2;
    req.rec.format 	= VIDEO2_REC_FORMAT;
    req.rec.state 	= VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
}
static int video2_rec_ch2_stop(u8 close)
{
    union video_req req = {0};
    int err;
    char path[64];
    char name[20];

    if (__this->state1_ch2 == VIDREC_STA_START) {

        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;

        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec2 ch2 err 0x%x\n", err);
            return -EINVAL;
        }
        __this->state1_ch2 = VIDREC_STA_STOP;
    }


    return 0;
}
static int video2_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;
    char path[128];
    char name[21];

    if (__this->video_rec2) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[2], 1);
        }

        puts("\n stop video rec2  \n");
        req.rec.channel = 2;
        req.rec.state = VIDEO_STATE_STOP;
        fget_name(__this->file[2], name, 20);
        sprintf(path, "%s%s", CONFIG_REC_PATH_2, name);


        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return -EINVAL;
        }
        if (__this->file[2]) {
            ctp_cmd_notity(path, 0);
        }

    }
    __this->file[2] = NULL;
    if (__this->state1_ch2 == VIDREC_STA_START) {
        puts("\n stop video rec2 ch2 \n");
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec2 ch2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }

        __this->state1_ch2 = VIDREC_STA_STOP;
    }
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
    union video_req req = {0};
    int err;
    char path[64];
    char name[20];

    if (__this->video_rec2) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[2], 1);
        }

        u32 res2 = db_select("res2");
        req.rec.channel = 2;
        req.rec.width 	= rec_pix_w[res2];
        req.rec.height 	= rec_pix_h[res2];
        req.rec.format 	= VIDEO2_REC_FORMAT;
        req.rec.state 	= VIDEO_STATE_SAVE_FILE;
        req.rec.fpath 	= CONFIG_REC_PATH_2;
        req.rec.fps     = get_video_fps1();
        req.rec.uvc_id = __this->uvc_id;
        req.rec.rec_small_pic 	= 0;

        req.rec.cycle_time = db_select("cyc");
        if (req.rec.cycle_time == 0) {
            req.rec.cycle_time = 5;
        }
        req.rec.cycle_time = req.rec.cycle_time * 60;
        video_rec_get_file_name_and_size(2, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);


        /* if (__this->rec_info->voice_on) { */
        req.rec.audio.sample_rate = AUDIO_SAM_RATE;
        req.rec.audio.channel 	= 1;
        req.rec.audio.volume    = 63;
        req.rec.audio.type  = AUDIO_FMT_PCM;
        req.rec.audio.buf = __this->audio_buf;
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;
        /* } else { */
        /* req.rec.audio.sample_rate = 0; */
        /* req.rec.audio.channel 	= 0; */
        /* req.rec.audio.buf = 0; */
        /* req.rec.audio.buf_len = 0; */
        /* } */
        req.rec.pkg_mute.aud_mute = !db_select("mic");
        fget_name(__this->file[2], name, 20);
        sprintf(path, "%s%s", CONFIG_REC_PATH_2, name);


        err = server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec2 err 0x%x\n", err);
            return -EINVAL;
        }

        req.rec.channel = 2;
        req.rec.format 	= VIDEO_FMT_MOV;
        req.rec.state 	= VIDEO_STATE_GET_INFO;
        server_request(__this->video_rec2, VIDEO_REQ_REC, &req);
        __this->file[2] = req.rec.get_info.file;
        //fcheck(__this->file[2]);
    }

    ctp_cmd_notity(path, 1);
    return 0;
}

static void video2_rec_close()
{
    if (__this->video_rec2) {
        server_close(__this->video_rec2);
        __this->video_rec2 = NULL;
    }
}

#endif

/*
 *必须在启动录像之后才可调用该函数，并且确保启动录像时已经打开了osd
 *新设置的osd的整体结构要和启动录像时一样，只是内容改变!!!
 */
static int video2_rec_set_osd_str(char *str)
{
    union video_req req = {0};
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
    union video_req req = {0};
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
    req.display.pctl = video_disp_get_pctl();

    server_request(__this->video_display_2, VIDEO_REQ_DISPLAY, &req);

#endif
    return 0;
}

static void video2_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO3_ENABLE)
    union video_req req = {0};
    if (__this->video_display_2) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(__this->video_display_2, VIDEO_REQ_DISPLAY, &req);

        server_close(__this->video_display_2);
        __this->video_display_2 = NULL;
    }
#endif
}


#ifdef CONFIG_VIDEO4_ENABLE
static int video3_rec_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;

    puts("start_video_rec3 \n");
    if (!__this->video_rec3) {
        __this->video_rec3 = server_open("video_server", "video4");
        if (!__this->video_rec3) {
            return -EINVAL;
        }

        server_register_event_handler(__this->video_rec3, (void *)3, rec_dev_server_event_handler);
    }

    req.rec.channel = 3;
    req.rec.camera_type = VIDEO_CAMERA_VIRTUAL;
    req.rec.width   = VIR_ENC_WIDTH;
    req.rec.height  = VIR_ENC_HEIGH;
    req.rec.format  = VIDEO3_REC_FORMAT;
    req.rec.state   = VIDEO_STATE_START;
    req.rec.fpath   = CONFIG_REC_PATH_3;
    req.rec.uvc_id = 0;
    req.rec.stream_type = 1;

    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps     = 0;


    req.rec.audio.data_source = 1;  //使用网络音频数据源
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    req.rec.audio.channel   = 1;
    req.rec.audio.volume    = 63;
    req.rec.audio.type  = AUDIO_FMT_PCM;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;

    req.rec.pkg_mute.aud_mute = !db_select("mic");

    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval = 0;

    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
     *      感兴趣区域qp 为其他区域的 70% ，可以调整
     *          */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

    osd_info.osd_w = 16;
    osd_info.osd_h = 32;

    max_one_line_strnum = strlen(video_rec_osd_buf);
    osd_line_num = 1;
    if (db_select("num")) {
        osd_line_num = 2;
    }

    req.rec.net_par.osd_line_num = osd_line_num;
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
        memcpy((void *)&req.rec.net_par.video_osd_buf, (void *)video_rec_osd_buf, sizeof(video_rec_osd_buf));
    }
    req.rec.slow_motion = 0;
    req.rec.tlp_time = db_select("gap");

    struct drop_fps targe_fps;
    if (req.rec.tlp_time && (req.rec.camera_type != VIDEO_CAMERA_UVC)) {
        targe_fps.fps_a = 1000;
        targe_fps.fps_b = req.rec.tlp_time;
        req.rec.targe_fps = &targe_fps;
    }
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel   = 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.buf = __this->v3_fbuf;
    req.rec.buf_len = VREC3_FBUF_SIZE;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
    req.rec.rec_small_pic   = 1;
#else
    req.rec.rec_small_pic   = 0;
#endif


    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = req.rec.cycle_time * 60;
    video_rec_get_file_name_and_size(3, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);

    err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
    if (err != 0) {
        puts("\n\n\nstart rec3 err\n\n\n");
        return -EINVAL;
    }

    req.rec.channel = 3;
    req.rec.state   = VIDEO_STATE_GET_INFO;
    server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
    __this->file[3] = req.rec.get_info.file;

    return 0;
}

static int video3_rec_aud_mute()
{
    union video_req req = {0};

    if (!__this->video_rec3) {
        return -EINVAL;
    }

    req.rec.channel = 3;
    req.rec.state   = VIDEO_STATE_PKG_MUTE;
    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
}

static int video3_rec_stop_count_down(u32 time)
{
    union video_req req = {0};

    if ((!__this->video_rec3) || (!__this->file[3])) {
        return -EINVAL;
    }
    req.rec.channel = 3;
    req.rec.format  = VIDEO3_REC_FORMAT;
    req.rec.stream_type = 1;
    req.rec.state   = VIDEO_STATE_STOP_COUNT_DOWN;
    req.rec.count_down = time;

    return server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
}

static int video3_rec_stop(u8 close)
{
    union video_req req = {0};
    int err;

    if (__this->video_rec3) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[3], 1);
        }

        req.rec.channel = 3;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nstop rec3 err 0x%x\n", err);
            return -EINVAL;
        }
    }
    __this->file[3] = NULL;

    if (close) {
        if (__this->video_rec3) {
            server_close(__this->video_rec3);
            __this->video_rec3 = NULL;
        }
    }

    return 0;
}

static int video3_rec_savefile()
{
    union video_req req = {0};
    int err;

    if (__this->video_rec3) {

        if (__this->gsen_lock) {
            video_rec_lock_file(__this->file[3], 1);
        }

        req.rec.channel = 3;
        req.rec.width   = VIR_ENC_WIDTH;
        req.rec.height  = VIR_ENC_HEIGH;
        req.rec.camera_type = VIDEO_CAMERA_VIRTUAL;
        req.rec.format  = VIDEO3_REC_FORMAT;
        req.rec.state   = VIDEO_STATE_SAVE_FILE;
        req.rec.fpath   = CONFIG_REC_PATH_3;
        req.rec.fps     = 0;
        req.rec.uvc_id = 0;
        req.rec.stream_type = 1;
#ifdef CONFIG_FILE_PREVIEW_ENABLE
        req.rec.rec_small_pic   = 1;
#else
        req.rec.rec_small_pic   = 0;
#endif

        req.rec.cycle_time = db_select("cyc");
        if (req.rec.cycle_time == 0) {
            req.rec.cycle_time = 5;
        }
        req.rec.cycle_time = req.rec.cycle_time * 60;
        video_rec_get_file_name_and_size(3, &req.rec.fname, &req.rec.fsize, req.rec.width, req.rec.format);

        req.rec.audio.sample_rate = 8000;
        req.rec.audio.channel   = 1;
        req.rec.audio.volume    = 63;
        req.rec.audio.type  = AUDIO_FMT_PCM;
        req.rec.audio.buf = __this->audio_buf;
        req.rec.audio.buf_len = AUDIO_BUF_SIZE;

        req.rec.pkg_mute.aud_mute = !db_select("mic");

        err = server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec3 err 0x%x\n", err);
            return -EINVAL;
        }

        req.rec.channel = 3;
        req.rec.state   = VIDEO_STATE_GET_INFO;
        server_request(__this->video_rec3, VIDEO_REQ_REC, &req);
        __this->file[3] = req.rec.get_info.file;
    }

    return 0;
}

static void video3_rec_close()
{
    if (__this->video_rec3) {
        server_close(__this->video_rec3);
        __this->video_rec3 = NULL;
    }
}


#endif

#ifndef CONFIG_BOARD_BBM_RECEIVE_BOARD

static int video3_disp_start(u16 width, u16 height, u16 xoff, u16 yoff)
{

#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO4_ENABLE)
    union video_req req = {0};

    if (!__this->video3_online) {
        return 0;//-EFAULT;
    }

    if (!__this->video_display_3) {
        __this->video_display_3 = server_open("video_server", "video4");
        if (!__this->video_display_3) {
            puts("server_open:faild\n");
            return -EFAULT;
        }
    }

    req.display.fb      = "fb2";

    req.display.left    = xoff;
    req.display.top     = yoff;
    req.display.width   = width;
    req.display.height  = height;

    if (req.display.width < 1280) {
        req.display.width   += 32;
        req.display.height  += 32;

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

    req.display.uvc_id = 0;
    req.display.camera_config = NULL;
    req.display.camera_type = VIDEO_CAMERA_VIRTUAL;
    req.display.stream_type = 1;

    req.display.state   = VIDEO_STATE_START;
    req.display.pctl = NULL;

    server_request(__this->video_display_3, VIDEO_REQ_DISPLAY, &req);

#endif
    return 0;
}

static void video3_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO4_ENABLE)
    union video_req req = {0};
    puts("video3_disp_stop in.\n");
    if (__this->video_display_3) {
        req.display.state   = VIDEO_STATE_STOP;
        server_request(__this->video_display_3, VIDEO_REQ_DISPLAY, &req);

        server_close(__this->video_display_3);
        __this->video_display_3 = NULL;
    }
    puts("video3_disp_stop out.\n");
#endif
}
#endif




static int net_h264_malloc()
{
    if (!__this_net->net_v0_fbuf) {
        __this_net->net_v0_fbuf = malloc(NET_VREC0_FBUF_SIZE);
        if (!__this_net->net_v0_fbuf) {
            puts("malloc v0_buf err\n\n");
            return -ENOMEM;
        }
    }
    if (!__this_net->audio_buf) {
        __this_net->audio_buf = malloc(300 * 1024);
        if (!__this_net->audio_buf) {
            free(__this_net->net_v0_fbuf);
            return -ENOMEM;
        }
    }
    return 0;


}
static int net_jpeg_malloc()
{
    if (!__this_net->net_v1_fbuf) {
        __this_net->net_v1_fbuf = malloc(NET_VREC1_FBUF_SIZE);
        if (!__this_net->net_v1_fbuf) {
            puts("malloc v1_buf err\n\n");
            return -ENOMEM;
        }
    }
    if (!__this_net->audio_buf) {
        __this_net->audio_buf = malloc(300 * 1024);
        if (!__this_net->audio_buf) {
            free(__this_net->net_v1_fbuf);
            return -ENOMEM;
        }
    }
    return 0;



}


#ifdef CONFIG_BOARD_BBM_RECEIVE_BOARD
static int video3_disp_start(u16 width, u16 height, u16 xoff, u16 yoff)
{

#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO4_ENABLE)
    union video_req req = {0};

    if (!__this->video3_online) {
        return -EFAULT;
    }

    if (!__this->video_display_2) {
        __this->video_display_2 = server_open("video_server", "video4");
        if (!__this->video_display_2) {
            puts("server_open:faild\n");
            return -EFAULT;
        }
    }

    req.display.fb 		= "fb2";

    req.display.left  	= xoff;
    req.display.top 	= yoff;
    req.display.width 	= width - 256 ;
    req.display.height 	= height;

    if (0) {
        /* if (req.display.width < 1280) { */
        /* if(0){ */
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

    /* req.display.uvc_id = __this->uvc_id; */
    req.display.camera_config = NULL;
    req.display.camera_type = VIDEO_CAMERA_RF;

    req.display.state 	= VIDEO_STATE_START;
    req.display.pctl = NULL;

    puts("\nopen video 4-----------\n");
    server_request(__this->video_display_2, VIDEO_REQ_DISPLAY, &req);

    puts("\nopen video 4-----------222\n");
#endif
    return 0;
}


static void video3_disp_stop()
{
#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO4_ENABLE)
    union video_req req = {0};
    if (__this->video_display_2) {
        req.display.state 	= VIDEO_STATE_STOP;
        server_request(__this->video_display_2, VIDEO_REQ_DISPLAY, &req);

        server_close(__this->video_display_2);
        __this->video_display_2 = NULL;
    }
#endif
}
#endif



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
#ifdef CONFIG_VIDEO4_ENABLE
    if (!__this->v3_fbuf) {
        __this->v3_fbuf = malloc(VREC3_FBUF_SIZE);
        if (!__this->v3_fbuf) {
            puts("malloc v3_buf err\n\n");
            return -1;
        }
    }
#endif

#if DEL_LOCK_FILE_EN
    video_rec_scan_lock_fszie();
#endif


    __this->state = VIDREC_STA_STARTING;

#ifdef CONFIG_VIDEO0_ENABLE
#ifndef CONFIG_BOARD_BBM_SENDER_BOARD
    err = video_rec_del_old_file(0);
    if (err) {
        puts("\nstart free space err\n");
        video_rec_post_msg("fsErr");
        return VREC_ERR_START_FREE_SPACE;
    }
#endif
    err = video0_rec_start();
    if (err) {
        goto __start_err0;
    }
#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
    video0_rec_set_dr();
#endif

#endif

    video_rec_post_msg("onREC");
#ifdef CONFIG_VIDEO1_ENABLE
    if (__this->video1_online && db_select("two")) {
        err = video_rec_del_old_file(1);
        if (err) {
            puts("\nstart free space err\n");
            video_rec_post_msg("fsErr");
            return VREC_ERR_START_FREE_SPACE;
        }
        err = video1_rec_start();
        if (err) {
            free(__this->v1_fbuf);
            __this->v1_fbuf = NULL;
            /*goto __start_err1;*/
        }
    }
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    if (__this->video2_online && db_select("two")) {
        err = video_rec_del_old_file(2);
        if (err) {
            puts("\nstart free space err\n");
            video_rec_post_msg("fsErr");
            return VREC_ERR_START_FREE_SPACE;
        }
        err = video2_rec_start();
        if (err) {
            free(__this->v2_fbuf);
            __this->v2_fbuf = NULL;
        }
    }
#endif
#ifdef CONFIG_VIDEO4_ENABLE
    if (__this->video3_online) {
        err = video3_rec_start();
    }
#endif
    __this->state = VIDREC_STA_START;


    return 0;

#ifdef CONFIG_VIDEO4_ENABLE
__start_err3:
    puts("\nstart3 err\n");
    err = video3_rec_stop(0);
#endif


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


static int video_rec_aud_mute()
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

#ifdef CONFIG_VIDEO4_ENABLE
    video3_rec_aud_mute();
#endif

    return 0;
}

static int video_rec0_netrt_mute()   //
{
    /* static u8 mark =1; */
    puts("\nvideo rec0 netrt mute \n");
    char buf[128];

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    u32 res = db_select("res");
    put_u8hex(__this->state_ch2);

    if (__this->state_ch2 != VIDREC_STA_START) {
        puts("\nch2 start \n");
#if defined CONFIG_VIDEO0_ENABLE
        video0_rec_ch2_start();
#endif
    }

    union video_req req = {0};

    puts("\nvideo rec0 netrt mute2 \n");

    req.rec.channel = 1;

    req.rec.state 	=  VIDEO_STATE_PKG_NETVRT_MUTE;

    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;

    server_request(__this->video_rec0, VIDEO_REQ_REC, &req);

    req.rec.state 	=  VIDEO_STATE_PKG_NETART_MUTE;

    req.rec.net_par.net_audrt_onoff = __this_net->net_video0_art_on ;

    server_request(__this->video_rec0, VIDEO_REQ_REC, &req);


    return 0;
}

#ifdef CONFIG_VIDEO1_ENABLE
static int video_rec1_netrt_mute()   //
{
    char buf[128];

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    if (!__this->video1_online) {
        return 0;
    }

    if (__this->state1_ch2 != VIDREC_STA_START) {
        puts("\nvideo1 ch2 start \n");
        video1_rec_ch2_start();
    }

    union video_req req = {0};

    req.rec.channel = 0;

    req.rec.state 	= VIDEO_STATE_PKG_NETVRT_MUTE;

    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;

    server_request(__this->video_rec1, VIDEO_REQ_REC, &req);

    req.rec.channel = 0;

    req.rec.state 	= VIDEO_STATE_PKG_NETART_MUTE;

    req.rec.net_par.net_audrt_onoff = __this_net->net_video1_art_on ;

    server_request(__this->video_rec1, VIDEO_REQ_REC, &req);


    return 0;

}
#endif

#ifdef CONFIG_VIDEO3_ENABLE
static int video_rec2_netrt_mute()   //for test
{
    char buf[128];

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    if (!__this->video2_online) {
        return 0;
    }

    if (__this->state1_ch2 != VIDREC_STA_START) {
        puts("\nvideo1 ch2 start \n");
        video2_rec_ch2_start();
    }

    union video_req req = {0};

    req.rec.channel = 0;

    req.rec.state 	= VIDEO_STATE_PKG_NETVRT_MUTE;

    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;

    server_request(__this->video_rec2, VIDEO_REQ_REC, &req);

    req.rec.channel = 0;

    req.rec.state 	= VIDEO_STATE_PKG_NETART_MUTE;

    req.rec.net_par.net_audrt_onoff = __this_net->net_video1_art_on ;

    server_request(__this->video_rec2, VIDEO_REQ_REC, &req);

    return 0;

}
#endif


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
#ifdef CONFIG_VIDEO4_ENABLE
    video3_rec_stop_count_down(time);
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

#ifdef CONFIG_VIDEO4_ENABLE
    err = video3_rec_stop(close);
    if (err) {
        puts("\nstop3 err\n");
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
    __this->save_file = 0;

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

#ifdef CONFIG_VIDEO4_ENABLE
    video3_rec_close();
#endif

    return 0;
}





static int video_rec_savefile(int dev_id)
{
    int err;
    int dev_num = 0;
    union video_req req = {0};

    printf("save_file: %d\n", dev_id);

    if (__this->state != VIDREC_STA_START) {
        return 0;
    }

    err = video_rec_del_old_file(dev_id);
    if (err) {
        video_rec_post_msg("fsErr");
        goto __err;
    }
    if (__this->save_file == 0) {
        video_rec_post_msg("saveREC");
        video_home_post_msg("saveREC");//录像切到后台,ui消息由主界面响应
        video_parking_post_msg("saveREC");
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


#ifdef CONFIG_VIDEO4_ENABLE
    if (__this->video3_online) {
        if (dev_id == 3) {
            err = video3_rec_savefile();
            if (err) {
                goto __err;
            }
        }
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
    if (++__this->save_file == video_rec_online_nums()) {
        /* if (++__this->save_file == dev_num) { */
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
#ifdef CONFIG_VIDEO4_ENABLE
    video3_rec_stop(0);
#endif
    __this->save_file = 0;

    video_rec_post_msg("offREC");
    video_home_post_msg("offREC");//录像切到后台,ui消息由主界面响应
    video_parking_post_msg("offREC");

    __this->state = VIDREC_STA_STOP;

    return -EFAULT;

}


/*
 * 录像时拍照的控制函数, 不能单独调用，必须录像时才可以调用，实际的调用地方已有
 * 录像时拍照会需要至少1.5M + 400K的空间，请根据实际情况来使用
 */
static int video_rec_take_photo(u8 sel)
{
    struct server *server;
    union video_req req = {0};
    int err;
    char namebuf[20];
    char buf[128];
    if (sel == 0) {
        server = __this->video_rec0;
        req.icap.width = pic_pix_w[db_select("res")];
        req.icap.height = pic_pix_h[db_select("res")];

    } else if (sel == 1) {
        server = __this->video_rec1;
        req.icap.width = pic_pix_w[db_select("res2")];
        req.icap.height = pic_pix_h[db_select("res2")];

    } else if (sel == 2) {
        server = __this->video_rec2;
        req.icap.width = pic_pix_w[db_select("res2")];
        req.icap.height = pic_pix_h[db_select("res2")];

    } else {
        server = __this->video_rec3;
        req.icap.width = pic_pix_w[db_select("res2")];
        req.icap.height = pic_pix_h[db_select("res2")];

    }
    if ((__this->state != VIDREC_STA_START) || (server == NULL)) {
        return -EINVAL;
    }

    if (__this->cap_buf == NULL) {
        __this->cap_buf = (u8 *)malloc(500 * 1024);
        if (!__this->cap_buf) {
            puts("\ntake photo no mem\n");
            return -ENOMEM;
        }
    }
    req.icap.quality = VIDEO_MID_Q;
    req.icap.label = NULL;
    req.icap.buf = __this->cap_buf;
    req.icap.buf_size = 500 * 1024;
    req.icap.file_name = namebuf;
    req.rec.rec_small_pic 	= 0;
    req.icap.sticker = NULL;
    if (sel == 0) {
        req.icap.path = CAMERA0_CAP_PATH"IMG_***.jpg";
    } else if (sel == 1) {
        req.icap.path = CAMERA1_CAP_PATH"IMG_***.jpg";
    } else {
        req.icap.path = CAMERA3_CAP_PATH"IMG_***.jpg";
    }

    err = server_request(server, VIDEO_REQ_IMAGE_CAPTURE, &req);
    if (err != 0) {
        printf("\n\ntake photo err=%d\n\n\n", err);
        /* puts("\n\n\ntake photo err\n\n\n"); */
        return -EINVAL;
    }

#if defined CONFIG_ENABLE_VLIST
    if (sel == 0) {
        sprintf(buf, CAMERA0_CAP_PATH"%s", req.icap.file_name);
        FILE_LIST_ADD(0, buf);
    } else {
        sprintf(buf, CAMERA1_CAP_PATH"%s", req.icap.file_name);
        FILE_LIST_ADD(0, buf);

    }
#endif
    return 0;
}

static int video_rec_take_photo1(u8 sel)
{
    char buf[128];
    char namebuf[20];
    struct server *server;
    union video_req req = {0};
    int err;

    if (sel == 0) {
        server = __this_net->net_video_rec;
        req.icap.width = __this_net->net_videoreq[0].rec.width;//pic_pix_w[db_select("res")];
        req.icap.height = __this_net->net_videoreq[0].rec.height;//pic_pix_h[db_select("res")];
        /* req.rec.width 	= __this_net->net_videoreq[0].rec.width; */
        /* req.rec.height 	= __this_net->net_videoreq[0].rec.height; */



        if (__this_net->net_state != VIDREC_STA_START) {
            return -EINVAL;
        }
    } else {
        server = __this_net->net_video_rec2;

        req.icap.width = __this_net->net_videoreq[1].rec.width;//pic_pix_w[db_select("res")];
        req.icap.height = __this_net->net_videoreq[1].rec.height;//pic_pix_h[db_select("res")];
        /* req.icap.width = pic_pix_w[db_select("res2")]; */
        /* req.icap.height = pic_pix_h[db_select("res2")]; */


        if (__this_net->net_state1 != VIDREC_STA_START) {
            return -EINVAL;
        }

    }

    if (__this->cap_buf == NULL) {
        __this->cap_buf = (u8 *)malloc(500 * 1024);

        if (!__this->cap_buf) {
            puts("\ntake photo no mem\n");
            return -ENOMEM;
        }
    }

    req.icap.quality = VIDEO_MID_Q;
    req.icap.label = NULL;
    req.icap.buf = __this->cap_buf;
    req.icap.buf_size = 500 * 1024;
    req.icap.file_name = namebuf;
    req.rec.rec_small_pic 	= 0;
    req.icap.sticker = NULL;
    if (sel == 0) {
        req.icap.path = CAMERA0_CAP_PATH"IMG_***.jpg";
    } else {
        req.icap.path = CAMERA1_CAP_PATH"IMG_***.jpg";
    }
    err = server_request(server, VIDEO_REQ_IMAGE_CAPTURE, &req);
    if (err != 0) {
        puts("\n\n\ntake photo err\n\n\n");
        return -EINVAL;
    }

#if defined CONFIG_ENABLE_VLIST
    if (sel == 0) {
        sprintf(buf, CAMERA0_CAP_PATH"%s", req.icap.file_name);
        FILE_LIST_ADD(0, buf);
    } else {
        sprintf(buf, CAMERA1_CAP_PATH"%s", req.icap.file_name);
        FILE_LIST_ADD(0, buf);
    }
#endif

    return 0;
}
static int video_cyc_file(u32 sel)
{
    struct server *server = NULL;
    union video_req req = {0};
    if (sel == 0) {
        server = __this->video_rec0;

        req.rec.channel = 0;
    } else if (sel == 1) {
        server = __this->video_rec1;
        req.rec.channel = 1;
    } else {
        server = __this->video_rec2;
        req.rec.channel = 2;
    }
    if ((__this->state != VIDREC_STA_START) || (server == NULL)) {
        return -EINVAL;
    }

    req.rec.state 	= VIDEO_STATE_CYC_FILE;
    req.rec.cyc_file = 1;
    int err = server_request(server, VIDEO_REQ_REC, &req);
    if (err != 0) {
        return -EINVAL;
    }

    return 0;

}

/*
 * 录像app的录像控制入口, 根据当前状态调用相应的函数
 */
static int video_rec_control(void *_run_cmd)
{
    int err = 0;
    u32 clust;
    int run_cmd = (int)_run_cmd;
    char buf[32];
    struct vfs_partition *part;
#ifndef CONFIG_BOARD_BBM_SENDER_BOARD
    if (storage_device_ready() == 0) {
        if (!dev_online(SDX_DEV)) {
            video_rec_post_msg("noCard");
        } else {
            video_rec_post_msg("fsErr");
        }

        CTP_CMD_COMBINED(NULL, CTP_SDCARD, "VIDEO_CTRL", "NOTIFY", CTP_SDCARD_MSG);
        return 0;
    } else {
        part = fget_partition(CONFIG_ROOT_PATH);

        __this->total_size = part->total_size;

        if (part->clust_size < 32 || (part->fs_attr & F_ATTR_RO)) {
            video_rec_post_msg("fsErr");
            CTP_CMD_COMBINED(NULL, CTP_SDCARD, "VIDEO_CTRL", "NOTIFY", CTP_SDCARD_MSG);
            return 0;
        }
    }
#endif
    switch (__this->state) {
    case VIDREC_STA_IDLE:
    case VIDREC_STA_STOP:
        if (run_cmd) {
            break;
        }
#if (defined CONFIG_WIFI_ENABLE) && ((!defined CONFIG_NET_CLIENT) && (!defined CONFIG_NET_SERVER))
        net_video_rec_stop(0);
#endif
        puts("\n video rec control \n");
        err = video_rec_start();
        if (err == 0) {
            /* video_rec_post_msg("onREC"); */
            if (__this->gsen_lock == 1) {
                video_rec_post_msg("lockREC");
            }
        }

#ifndef CONFIG_UI_ENABLE
        //video_rec_set_white_balance();
        video_rec_set_exposure(db_select("exp"));
#endif // CONFIG_UI_ENABLE


#if (defined CONFIG_WIFI_ENABLE) && ((!defined CONFIG_NET_CLIENT) && (!defined CONFIG_NET_SERVER))
        struct intent *it = NULL;
        net_rt_video0_open(it);

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
        net_rt_video1_open(it);
#endif
        sprintf(buf, "status:%d", 1);
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CTRL", "NOTIFY", buf);
#endif

        break;
    case VIDREC_STA_START:
        if (run_cmd == 0) {
            /*提前UI响应,加快反应速度*/
            video_rec_post_msg("offREC");
            video_parking_post_msg("offREC");
            video_home_post_msg("offREC");//录像切到后台,ui消息由主界面响应
            err = video_rec_stop(0);




#if defined CONFIG_WIFI_ENABLE
            sprintf(buf, "status:%d", 0);
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CTRL", "NOTIFY", buf);
            net_video_rec_start(0);
#endif

        } else if (run_cmd == 1) { //cycle_rec

            sprintf(buf, "status:%d", 1);
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CTRL", "NOTIFY", buf);

        } else {//track_pic

            if (__this->photo_camera_sel == 0) {
                err = video_rec_take_photo(0);
            } else if (__this->photo_camera_sel == 1) {
                err = video_rec_take_photo(1);
            }
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
    union video_req req = {0};

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
    union video_req req = {0};

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





/*
使用限制 !!!
1.延时录像时不能使用
2.两路编码都是摄像头0时不能使用
3.使用智能引擎（除了只使用普通的移动侦测模式 VE_MOTION_DETECT_MODE_ISP 这种情况）时不能使用
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

#if (defined CONFIG_UI_ENABLE && defined CONFIG_VIDEO0_ENABLE && (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO4_ENABLE))

    switch (__this->disp_state) {
    case DISP_BIG_SMALL:

        puts("DISP_BIG_SMALL in.\n");
        video0_disp_stop();
        video1_disp_stop();
        video2_disp_stop();
        video3_disp_stop();

        err = video0_disp_start(SCREEN_W / 2, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err0;
        }


        if (video_rec_online_nums() == 3) {
            if (__this->disp_video_ctrl == 1) {
                err = video1_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
                if (err) {
                    goto __disp_ctl_err1;
                }
            } else if (__this->disp_video_ctrl == 2) {
                err = video2_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
                if (err) {
                    goto __disp_ctl_err2;
                }
            } else {
                err = video3_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
                if (err) {
                    goto __disp_ctl_err3;
                }
            }
        } else {
            err = video1_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
            if (err) {
                goto __disp_ctl_err1;
            }

            err = video2_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
            if (err) {
                goto __disp_ctl_err2;
            }

            err = video3_disp_start(SCREEN_W / 2, SCREEN_H, SCREEN_W / 2, 0);
            if (err) {
                goto __disp_ctl_err3;
            }
        }

        __this->disp_state = DISP_HALF_WIN;
        puts("DISP_BIG_SMALL out.\n");
        break;
    case DISP_HALF_WIN:
        puts("DISP_HALF_WIN in.\n");
        video3_disp_stop();
        video2_disp_stop();
        video1_disp_stop();
        video0_disp_stop();
        err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err0;
        }
        __this->disp_state = DISP_FRONT_WIN;
        puts("DISP_HALF_WIN out.\n");
        break;
    case DISP_FRONT_WIN:

        puts("DISP_FRONT_WIN in.\n");
        video0_disp_stop();
        video1_disp_stop();
        video2_disp_stop();
        video3_disp_stop();
        if (video_rec_online_nums() == 3) {
            if (__this->disp_video_ctrl == 1) {
                err = video1_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
                if (err) {
                    goto __disp_ctl_err1;
                }
            } else if (__this->disp_video_ctrl == 2) {
                err = video2_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
                if (err) {
                    goto __disp_ctl_err2;
                }
            } else {
                err = video3_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
                if (err) {
                    goto __disp_ctl_err3;
                }
            }
        } else {
            err = video1_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
            if (err) {
                goto __disp_ctl_err1;
            }
            err = video2_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
            if (err) {
                goto __disp_ctl_err2;
            }
            err = video3_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
            if (err) {
                goto __disp_ctl_err3;
            }
        }

        __this->disp_state = DISP_BACK_WIN;
        video_rec_post_msg("HlightOff");//进入后视窗口前照灯关闭
        puts("DISP_FRONT_WIN out.\n");
        break;
    case DISP_BACK_WIN:
        puts("DISP_BACK_WIN in.\n");

        video1_disp_stop();
        video2_disp_stop();
        video3_disp_stop();
        err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
        if (err) {
            goto __disp_ctl_err0;
        }

        if (video_rec_online_nums() == 3) {
            if (__this->disp_video_ctrl == 1) {
                err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                if (err) {
                    goto __disp_ctl_err1;
                }
            } else if (__this->disp_video_ctrl == 2) {
                err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                if (err) {
                    goto __disp_ctl_err2;
                }
            } else {
                err = video3_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                if (err) {
                    goto __disp_ctl_err3;
                }
            }

        } else {
            err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            if (err) {
                goto __disp_ctl_err1;
            }
            err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            if (err) {
                goto __disp_ctl_err2;
            }
            err = video3_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            if (err) {
                goto __disp_ctl_err3;
            }
        }

        __this->disp_state = DISP_BIG_SMALL;
        puts("DISP_BACK_WIN out.\n");

        break;

    case DISP_RESET_WIN:
        puts("DISP_BACK_WIN in.\n");

        video3_disp_stop();
        video2_disp_stop();
        video1_disp_stop();
        video0_disp_stop();

        err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);

        if (video_rec_online_nums() == 3) {
            if (__this->disp_video_ctrl == 1) {
                err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                if (err) {
                    break;
                }
            } else if (__this->disp_video_ctrl == 2) {
                err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                if (err) {
                    break;
                }
            } else {
                err = video3_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                if (err) {
                    break;
                }
            }

        } else {
            err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            if (err) {
                break;
            }
            err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            if (err) {
                break;
            }
            err = video3_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            if (err) {
                break;
            }
        }

        __this->disp_state = DISP_BIG_SMALL;
        puts("DISP_BACK_WIN out.\n");

        break;

    case DISP_PARK_WIN:

        puts("DISP_PARK_WIN in.\n");
        video0_disp_stop();
        video1_disp_stop();
        video2_disp_stop();

        video3_disp_stop();
        if (video_rec_online_nums() == 3) {
            if (__this->disp_video_ctrl == 1) {
                err = video1_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            } else if (__this->disp_video_ctrl == 2) {
                err = video2_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            } else {
                err = video3_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            }
        } else {
            err = video1_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            err = video2_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            err = video3_disp_start(SCREEN_W, SCREEN_H, 0, 0);
        }

        __this->disp_state = DISP_BACK_WIN;
        puts("DISP_PARK_WIN out.\n");

        break;

    default:
        puts("\ndisp forbid\n");
        break;
    }
    if (__this->disp_state == DISP_BACK_WIN) {
        //后拉全屏显示（非录像状态），固定屏显场景1
        isp_scr_work_hdl(1);
    }
    puts("disp ctrl ok.\n");
    return 0;

__disp_ctl_err3:
    puts("\n\ndisp ctrl err3\n\n");
    video3_disp_stop();
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


static u8 page_main_flag = 0;
static u8 page_park_flag = 0;
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

static int show_lane_set_ui()
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

    for (int i = 0; i < 4; i++) {
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
    video1_disp_stop();

    u16 dis_w = 640 * SCREEN_H / 352 / 16 * 16;
    dis_w = dis_w > SCREEN_W ? SCREEN_W : dis_w;
    printf("lane dis %d x %d\n", dis_w, SCREEN_H);
    err = video0_disp_start(dis_w, SCREEN_H, ((SCREEN_W - dis_w) / 2 / 16 * 16), 0);
    show_lane_set_ui();
#endif
    return err;
}



static int video_rec_init()
{
    int err = 0;


    ve_server_open(0);

#if (VIDEO_PARK_DECT == 1)
    __this->disp_park_sel = 1;
#elif (VIDEO_PARK_DECT == 3)
    __this->disp_park_sel = 2;
#elif (VIDEO_PARK_DECT == 4)
    __this->disp_park_sel = 3;
#else
    __this->disp_park_sel = 0;
#endif


#ifdef CONFIG_VIDEO0_ENABLE
#ifdef CONFIG_TOUCH_UI_ENABLE
    err = video0_disp_start(SCREEN_W, SCREEN_H + SCREEN_H_OFFSET, 0, 0);
#else
    err = video0_disp_start(SCREEN_W, SCREEN_H, 0, 0);
#endif
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    __this->video1_online = dev_online("video1");
#endif

#ifdef CONFIG_VIDEO3_ENABLE
    __this->video2_online = dev_online("uvc");
#endif

#ifdef CONFIG_VIDEO4_ENABLE


#ifndef CONFIG_BOARD_BBM_RECEIVE_BOARD
    __this->video3_online = dev_online("net2video");
#else
    __this->video3_online = dev_online("rf_phy");
#endif
    /* log_d("video3_online=%d-------\n\n\n\n",__this->video3_online); */
#endif

    if (get_parking_status()) {
        if ((__this->disp_park_sel == 1) && (__this->video1_online)) {
            video0_disp_stop();
            err = video1_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            __this->disp_state = DISP_BACK_WIN;
        }
        if ((__this->disp_park_sel == 2) && (__this->video2_online)) {
            video0_disp_stop();
            err = video2_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            __this->disp_state = DISP_BACK_WIN;
        }
#ifdef CONFIG_VIDEO4_ENABLE
        if ((__this->disp_park_sel == 3) && (__this->video3_online)) {
            video0_disp_stop();
            err = video3_disp_start(SCREEN_W, SCREEN_H, 0, 0);
            __this->disp_state = DISP_BACK_WIN;
        }
#endif

    } else {
        if (__this->video1_online) {
            err = video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            __this->disp_video_ctrl = 1;
        } else if (__this->video2_online) {
            err = video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
            __this->disp_video_ctrl = 2;

#ifdef CONFIG_VIDEO4_ENABLE
        } else if (__this->video3_online) {

#ifndef CONFIG_BOARD_BBM_RECEIVE_BOARD
            err = video3_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
#else
            err = video3_disp_start(SCREEN_W, SCREEN_H, 0, 0);
#endif
            __this->disp_video_ctrl = 3;
#endif
        }
    }

    if (((__this->video1_online == 0) && (__this->video2_online == 0) && (__this->video3_online == 0)) || err) {
        __this->disp_state = DISP_RESET_WIN;
    }

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
    union video_req req = {0};

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


    for (int i = 0; i < 4; i++) {
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

#ifdef CONFIG_VIDEO4_ENABLE
    video3_disp_stop();
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
    if (__this->v3_fbuf) {
        free(__this->v3_fbuf);
        __this->v3_fbuf = NULL;
    }

    if (__this->cap_buf) {
        free(__this->cap_buf);
        __this->cap_buf = NULL;
    }

    if (__this_net->net_v0_fbuf) {
        free(__this_net->net_v0_fbuf);
        __this_net->net_v0_fbuf = NULL;

    }
    if (__this_net->net_v1_fbuf) {
        free(__this_net->net_v1_fbuf);
        __this_net->net_v1_fbuf = NULL;
    }
    if (__this_net->net_v2_fbuf) {
        free(__this_net->net_v2_fbuf);
        __this_net->net_v2_fbuf = NULL;
    }
    if (__this_net->audio_buf) {
        free(__this_net->audio_buf);
        __this_net->audio_buf = NULL;

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
#if defined CONFIG_WIFI_ENABLE
extern int atoi(const char *);
static void rt_stream_cmd_analysis(u8 chl,  u32 add)
{
    char *key;
    char *value;
    struct rt_stream_app_info *info = (struct rt_stream_app_info *)add;

    __this_net->net_videoreq[chl].rec.width = info->width;
    __this_net->net_videoreq[chl].rec.height = info->height;
    __this_net->net_videoreq[chl].rec.format = info->type;
    __this_net->net_videoreq[chl].rec.fps = get_net_video_fps();
    __this_net->priv = info->priv;

}


static void video_rec0_set_fr(int fr_a, int fr_b)
{
    union video_req req = {0};

    struct drop_fps targe_fps;

    targe_fps.fps_a = fr_a;
    targe_fps.fps_b = fr_b;
    req.rec.targe_fps = &targe_fps;

    req.rec.channel = 1;
    req.rec.state 	= VIDEO_STATE_SET_DR;

    server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    /*printf("video_rec0_set_frame_rate = %d \r\n", fr);*/
}

static void video_rec0_set_bitrate(unsigned int bits_rate)
{
    union video_req req = {0};

    req.rec.channel = 1;

    req.rec.state = VIDEO_STATE_RESET_BITS_RATE;
    req.rec.abr_kbps = bits_rate;

    server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
}


static void net_fps_fps_bitrate_ctrl_init(void)
{
    __this_net->dy_fr = NET_VIDEO_REC_FPS0 * 256;
    __this_net->dy_fr_denom = 256;
    __this_net->fbuf_fcnt = __this_net->fbuf_ffil = 0;
    fps_ctrl_init(&__this_net->fps_ctrl_hdl, NET_VIDEO_REC_FPS0 * 256, 12, 3, ((1 * 256) / 256), NET_VIDEO_REC_FPS0 * 256);
    video_rec0_set_fr(__this_net->dy_fr, __this_net->dy_fr_denom);

    __this_net->dy_bitrate = video_rec_get_abr(__this_net->net_videoreq[0].rec.width);
    bitrate_ctrl_init(&__this_net->bitrate_ctrl_hdl, __this_net->dy_bitrate, 12, 3, 1024, __this_net->dy_bitrate);
}

static void net_video_fps_bitrate_ctrl(void)
{
    int fps, fps_denom, bitrate;
    fps_ctrl_update(&__this_net->fps_ctrl_hdl, __this_net->fbuf_fcnt, &fps, &fps_denom);
    bitrate_ctrl_update(&__this_net->bitrate_ctrl_hdl, __this_net->fbuf_fcnt, &bitrate);
    if (fps != __this_net->dy_fr || fps_denom != __this_net->dy_fr_denom || bitrate != __this_net->dy_bitrate) {
        __this_net->dy_fr = fps;
        __this_net->dy_fr_denom = fps_denom;
        __this_net->dy_bitrate = bitrate;
        video_rec0_set_fr(fps, fps_denom);
        video_rec0_set_bitrate(bitrate);
        printf("^*^nfcnt= %d, fps= %d[%d/%d], bitrate = %d \r\n", __this_net->fbuf_fcnt,
               fps / fps_denom, fps, fps_denom, bitrate);
    }
}

static void net_video_timer_hdl(void *parm)
{
    static int timer_cnt;
    ++timer_cnt;

    if ((timer_cnt % 1) == 0) {
//        net_video_fps_bitrate_ctrl();
    }
}

void net_264_pkg_get_in_frame(char *fbuf, u32 frame_size)
{
    __this_net->fbuf_fcnt += 1;
    __this_net->fbuf_ffil += frame_size;
}

void net_264_pkg_get_out_frame(char *fbuf, u32 frame_size)
{
    __this_net->fbuf_fcnt -= 1;
    __this_net->fbuf_ffil -= frame_size;

#if 1
    static u32 vdo_frame_cnt;
    ++vdo_frame_cnt;
    static int time_hdl;
    int t_ret = time_lapse(&time_hdl, 1000);
    if (t_ret) {
        printf("net out fr = %dms, %d\r\n", t_ret, vdo_frame_cnt);
        vdo_frame_cnt = 0;
    }
#endif

}


static int net_video_rec0_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    u16 osd_max_heigh;
    char buf[128];
    puts("start_net_video_rec\n");

    if (!__this_net->net_video_rec) {
        __this_net->net_video_rec = server_open("video_server", "video0");

        if (!__this_net->net_video_rec) {
            return VREC_ERR_V0_SERVER_OPEN;
        }

        server_register_event_handler(__this_net->net_video_rec, (void *)0, rec_dev_server_event_handler);
    }

    /*
      *通道号，分辨率，封装格式，写卡的路径
     */
    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
    req.rec.channel = 1;
    req.rec.width 	= __this_net->net_videoreq[0].rec.width;
    req.rec.height 	= __this_net->net_videoreq[0].rec.height;

    printf(">>>>>>width=%d    height=%d\n\n\n\n", __this_net->net_videoreq[0].rec.width, __this_net->net_videoreq[0].rec.height);
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_1;

    req.rec.format 	= __this_net->net_videoreq[0].rec.format;


    req.rec.quality = VIDEO_LOW_Q;

    req.rec.fps	=  get_net_video_fps();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video0_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;

    /* if (__this->rec_info->voice_on) { */
    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    /* req.rec.audio.sample_rate = 8000; */
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.sample_inv_size 	= 8192;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.rec_small_pic 	= 0;
    /* } else { */
    /* req.rec.audio.sample_rate = 0; */
    /* req.rec.audio.channel 	= 0; */
    /* req.rec.audio.buf = 0; */
    /* req.rec.audio.buf_len = 0; */
    /* } */
    req.rec.pkg_mute.aud_mute = 1;
    /* req.rec.pkg_mute.aud_mute = !__this->rec_info->voice_on; */

    /*
     *码率，I帧和P帧比例，必须是偶数（当录MOV的时候才有效）,
     *roio_xy :值表示宏块坐标， [6:0]左边x坐标 ，[14:8]右边x坐标，[22:16]上边y坐标，[30:24]下边y坐标,写0表示1个宏块有效
     * roio_ratio : 区域比例系数
     */
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    /* req.rec.abr_kbps = 2000;//video_rec_get_abr(req.rec.width); */
    req.rec.IP_interval = 0;
    /*感兴趣区域为下方 中间 2/6 *4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roi1_xy = (req.rec.height * 11 / 12 / 16) << 24 | (req.rec.height * 4 / 12 / 16) << 16 | (req.rec.width * 11 / 12 / 16) << 8 | (req.rec.width) * 1 / 12 / 16;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roio_ratio1 = 256 * 90 / 100;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

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

    osd_max_heigh = (req.rec.height == 1088) ? 1080 : req.rec.height;
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
    /* req.rec.tlp_time = 40; // 0; */

    printf("\n\ntl_time : %d\n\n", req.rec.tlp_time);
    if (req.rec.slow_motion || req.rec.tlp_time) {
        /*         req.rec.audio.sample_rate = 0; */
        /* req.rec.audio.channel 	= 0; */
        /* req.rec.audio.volume    = 0; */
        /* req.rec.audio.buf = 0; */
        /* req.rec.audio.buf_len = 0; */
    }

    req.rec.buf = __this_net->net_v0_fbuf;
    req.rec.buf_len = NET_VREC0_FBUF_SIZE;
    /*
     *循环录像时间，文件大小
     */
    /* req.rec.cycle_time = db_select("cyc"); */
    /* if (req.rec.cycle_time == 0) { */
    /* req.rec.cycle_time = 3 * 60; */
    /* } */

    /* req.rec.cycle_time = req.rec.cycle_time * 60; */
    //net
    {
        struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
        /*
         *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
          *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
        *
         * */
#if defined CONFIG_NET_UDP_ENABLE
        sprintf(req.rec.net_par.netpath, "udp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _FORWARD_PORT);
#else
        sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _FORWARD_PORT);
#endif

    }

    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec err\n\n\n");
        return VREC_ERR_V0_REQ_START;
    }

    net_fps_fps_bitrate_ctrl_init();

    /* if (__this_net->timer_handler == 0) { */
    /* __this_net->timer_handler = sys_timer_add(NULL, net_video_timer_hdl, 200); */
    /* } */

    return 0;
}

static int net_video_rec0_aud_mute()
{
#ifdef CONFIG_VIDEO0_ENABLE

    if (__this_net->net_state != VIDREC_STA_START) {
        return 0;
    }

    union video_req req = {0};

    req.rec.channel = 0;

    req.rec.state = VIDEO_STATE_PKG_MUTE;

    req.rec.pkg_mute.aud_mute = !db_select("mic");
    /* req.rec.pkg_mute.aud_mute = !__this->rec_info->voice_on; */

    printf("net video rec0 aud mute %d", req.rec.pkg_mute.aud_mute);

    return server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

#else
    return 0;

#endif
}


static int net_video_rec0_stop(u8 close)
{
    union video_req req = {0};
    int err;
    puts("\nnet video rec0 stop\n");

    /* if (__this_net->timer_handler) { */
    /* sys_timer_del(__this_net->timer_handler); */
    /* __this_net->timer_handler = 0; */
    /* } */

    net_video_rec_stop_isp_scenes(1, 0);
    req.rec.channel = 1;
    req.rec.state = VIDEO_STATE_STOP;
    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    if (err != 0) {
        printf("\nstop rec err 0x%x\n", err);
        return VREC_ERR_V0_REQ_STOP;
    }

    net_video_rec_start_isp_scenes();
    if (close) {
        if (__this_net->net_video_rec) {
            server_close(__this_net->net_video_rec);
            __this_net->net_video_rec = NULL;
        }
    }

    return 0;
}

/******* 不要单独调用这些子函数 ********/
static int net_video_rec1_start()
{
    int err;
    union video_req req = {0};
    struct imc_osd_info osd_info;
    u16 max_one_line_strnum;
    u16 osd_line_num;
    char buf[128];

    puts("start_net_video_rec2 \n");

    if (!__this_net->net_video_rec2) {
#ifdef CONFIG_VIDEO1_ENABLE
        __this_net->net_video_rec2 = server_open("video_server", "video1");
#endif
#ifdef CONFIG_VIDEO3_ENABLE
        __this_net->net_video_rec2 = server_open("video_server", "video3");
#endif
        if (!__this_net->net_video_rec2) {
            return VREC_ERR_V1_SERVER_OPEN;
        }

        server_register_event_handler(__this_net->net_video_rec2, (void *)1, rec_dev_server_event_handler);
    }
#ifdef CONFIG_VIDEO1_ENABLE
    req.rec.width 	= __this_net->net_videoreq[1].rec.width;
    req.rec.height 	= __this_net->net_videoreq[1].rec.height;

    req.rec.camera_type = VIDEO_CAMERA_NORMAL;
#endif
#ifdef CONFIG_VIDEO3_ENABLE
    req.rec.camera_type = VIDEO_CAMERA_UVC;
    req.rec.width 	= __this_net->net_videoreq[1].rec.width;
    req.rec.height 	= __this_net->net_videoreq[1].rec.height;
    req.rec.uvc_id = __this->uvc_id;
#endif
    req.rec.channel = 0;


    req.rec.format 	= __this_net->net_videoreq[1].rec.format;
    req.rec.state 	= VIDEO_STATE_START;
    req.rec.fpath 	= CONFIG_REC_PATH_2;

    req.rec.quality = VIDEO_LOW_Q;
    req.rec.fps 	= get_net_video_fps();
    req.rec.net_par.net_audrt_onoff  = __this_net->net_video1_art_on;
    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;

    req.rec.audio.sample_rate = AUDIO_SAM_RATE;
    req.rec.audio.channel 	= 1;
    req.rec.audio.type 	= AUDIO_FMT_PCM;
    req.rec.audio.volume    = 63;
    req.rec.audio.buf = __this->audio_buf;
    req.rec.audio.buf_len = AUDIO_BUF_SIZE;
    req.rec.pkg_mute.aud_mute = 1;
    req.rec.abr_kbps = video_rec_get_abr(req.rec.width);
    req.rec.IP_interval =  0;
    /*感兴趣区域为下方 中间 2/6 * 4/6 区域，可以调整
    	感兴趣区域qp 为其他区域的 70% ，可以调整
    */
    req.rec.roi.roio_xy = (req.rec.height * 5 / 6 / 16) << 24 | (req.rec.height * 3 / 6 / 16) << 16 | (req.rec.width * 5 / 6 / 16) << 8 | (req.rec.width) * 1 / 6 / 16;
    req.rec.roi.roio_ratio = 256 * 70 / 100 ;
    req.rec.roi.roi1_xy = 0;
    req.rec.roi.roi2_xy = 0;
    req.rec.roi.roi3_xy = (1 << 24) | (0 << 16) | ((req.rec.width / 16) << 8) | 0;
    req.rec.roi.roio_ratio1 = 0;
    req.rec.roi.roio_ratio2 = 0;
    req.rec.roi.roio_ratio3 = 256 * 80 / 100;

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
    /* req.rec.tlp_time = 0; */

    req.rec.tlp_time = db_select("gap");
    printf("\n\ntl_time : %d\n\n", req.rec.tlp_time);
    if (req.rec.slow_motion || req.rec.tlp_time) {
        req.rec.audio.sample_rate = 0;
        req.rec.audio.channel 	= 0;
        req.rec.audio.volume    = 0;
        req.rec.audio.buf = 0;
        req.rec.audio.buf_len = 0;
    }
    req.rec.rec_small_pic 	= 0;

    req.rec.buf = __this_net->net_v1_fbuf;
    req.rec.buf_len = NET_VREC1_FBUF_SIZE;
    /*
     *循环录像时间，文件大小
     */
    req.rec.cycle_time = db_select("cyc");
    if (req.rec.cycle_time == 0) {
        req.rec.cycle_time = 5;
    }
    req.rec.cycle_time = 0;

    {
        struct sockaddr_in *addr = ctp_srv_get_cli_addr(__this_net->priv);
        /*
         *        sprintf(req.rec.net_par.netpath,"rf://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
          *        sprintf(req.rec.net_par.netpath,"rtp://%s:%d"
         *                        ,inet_addr(addr->sin_addr.s_addr)
         *                                        ,_FORWARD_PORT);
        *
         * */
#if defined CONFIG_NET_UDP_ENABLE
        sprintf(req.rec.net_par.netpath, "udp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _BEHIND_PORT);
#else
        sprintf(req.rec.net_par.netpath, "tcp://%s:%d"
                , inet_ntoa(addr->sin_addr.s_addr)
                , _BEHIND_PORT);
#endif

    }

    err = server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);

    if (err != 0) {
        puts("\n\n\nstart rec2 err\n\n\n");
        return VREC_ERR_V1_REQ_START;
    }

    return 0;
}

static int net_video_rec1_aud_mute()
{
#ifdef CONFIG_VIDEO1_ENABLE

    if (__this_net->net_state1 != VIDREC_STA_START) {
        return	0;
    }

    union video_req req = {0};

    req.rec.channel = 0;

    req.rec.state = VIDEO_STATE_PKG_MUTE;

    req.rec.pkg_mute.aud_mute = !db_select("mic");

    return server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);

#else
    return 0;

#endif
}


static int net_video_rec1_stop(u8 close)
{
    union video_req req = {0};
    int err;

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)

    if (__this_net->net_video_rec2) {
        req.rec.channel = 0;
        req.rec.state = VIDEO_STATE_STOP;
        err = server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);

        if (err != 0) {
            printf("\nstop rec2 err 0x%x\n", err);
            return VREC_ERR_V1_REQ_STOP;
        }
    }

    if (close) {
        if (__this_net->net_video_rec2) {
            server_close(__this_net->net_video_rec2);
            __this_net->net_video_rec2 = NULL;
        }
    }

#endif

    return 0;
}

static int net_video0_rec_start(struct intent *it)
{

    int err = 0;
#ifdef CONFIG_VIDEO0_ENABLE

    if (!__this_net->net_v0_fbuf) {
        __this_net->net_v0_fbuf = malloc(NET_VREC0_FBUF_SIZE);

        if (!__this_net->net_v0_fbuf) {
            puts("malloc v0_buf err\n\n");
            return -1;
        }
    }

    if (!__this->audio_buf) {
        __this->audio_buf = malloc(AUDIO_BUF_SIZE);

        if (!__this->audio_buf) {
            free(__this_net->net_v0_fbuf);
            return -ENOMEM;
        }
    }

    if (__this_net->net_video0_art_on || __this_net->net_video0_vrt_on) {
        err = net_video_rec0_start();

        if (err) {
            goto __start_err0;
        }
        if (__this->state != VIDREC_STA_START) {
            __this_net->videoram_mark = 1;
        } else {
            __this_net->videoram_mark = 0;
        }

        __this_net->net_state = VIDREC_STA_START;
        net_video_rec_start_isp_scenes();
    }

    return 0;

__start_err0:
    puts("\nstart0 err\n");
    err = net_video_rec0_stop(0);

    if (err) {
        printf("\nstart wrong0 %x\n", err);
        //while(1);
    }

#endif
    return err;
}


static int net_video1_rec_start(struct intent *it)
{
    int err = 0;
    /* #ifdef CONFIG_VIDEO1_ENABLE || CONFIG_VIDEO3_ENABLE */

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)


    if (!__this_net->net_v1_fbuf) {
        __this_net->net_v1_fbuf = malloc(NET_VREC1_FBUF_SIZE);

        if (!__this_net->net_v1_fbuf) {
            puts("malloc v1_buf err\n\n");
            return -1;
        }
    }
    if (!__this->audio_buf) {
        __this->audio_buf = malloc(AUDIO_BUF_SIZE);

        if (!__this->audio_buf) {
            free(__this_net->net_v1_fbuf);
            return -ENOMEM;
        }
    }

    if (__this_net->net_video1_art_on || __this_net->net_video1_vrt_on) {
        if (__this->video1_online || __this->video2_online) {
            err = net_video_rec1_start();

            if (err) {
                goto __start_err1;
            }

            __this_net->net_state1 = VIDREC_STA_START;
        }
    }

    return 0;

__start_err1:
    puts("\nstart1 err\n");
    err = net_video_rec1_stop(0);

    if (err) {
        printf("\nstart wrong1 %x\n", err);
        //while(1);
    }

    /* return 1; */
#endif

    return err;
}

/* static int net_video_rec_start(struct intent *it) */
static int net_video_rec_start(u8 mark)
{
    int err;
#if defined CONFIG_WIFI_ENABLE
    puts("start net rec\n");

#ifdef CONFIG_VIDEO0_ENABLE

    if (!__this_net->net_v0_fbuf) {
        __this_net->net_v0_fbuf = malloc(NET_VREC0_FBUF_SIZE);

        if (!__this_net->net_v0_fbuf) {
            puts("malloc v0_buf err\n\n");
            return -1;
        }
    }

    if (!__this->audio_buf) {
        __this->audio_buf = malloc(AUDIO_BUF_SIZE);

        if (!__this->audio_buf) {
            free(__this_net->net_v0_fbuf);
            return -ENOMEM;
        }
    }

#endif

    /* #ifdef CONFIG_VIDEO1_ENABLE || CONFIG_VIDEO3_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)

    if (!__this_net->net_v1_fbuf) {
        __this_net->net_v1_fbuf = malloc(NET_VREC1_FBUF_SIZE);

        if (!__this_net->net_v1_fbuf) {
            puts("malloc v1_buf err\n\n");
            return -1;
        }
    }

    if (!__this->audio_buf) {
        __this->audio_buf = malloc(AUDIO_BUF_SIZE);

        if (!__this->audio_buf) {
            free(__this_net->net_v1_fbuf);
            return -ENOMEM;
        }
    }

#endif

#ifdef CONFIG_VIDEO0_ENABLE

    printf("\n art %d, vrt %d\n", __this_net->net_video0_art_on, __this_net->net_video0_vrt_on);
    /* #ifdef CONFIG_VIDEO1_ENABLE */

    /* if (__this_net->net_video0_art_on || __this_net->net_video0_vrt_on) { */
    /* #else */

    if ((__this_net->net_video0_art_on || __this_net->net_video0_vrt_on)
        && (__this_net->net_state != VIDREC_STA_START)) {
        /* #endif */
        puts("\nnet video rec0 start \n");
        err = net_video_rec0_start();

        if (err) {
            goto __start_err0;
        }

        __this_net->net_state = VIDREC_STA_START;
    }

#endif

    /* #ifdef CONFIG_VIDEO1_ENABLE || CONFIG_VIDEO3_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)

    if ((__this_net->net_video1_art_on || __this_net->net_video1_vrt_on)
        && (__this_net->net_state1 != VIDREC_STA_START))	{
        if (__this->video1_online || __this->video2_online) {
            err = net_video_rec1_start();
            /*if (err) {
            	   goto __start_err1;
             }*/
            __this_net->net_state1 = VIDREC_STA_START;
        }
    }

#endif

    return 0;

    /* #ifdef CONFIG_VIDEO1_ENABLE || CONFIG_VIDEO3_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
__start_err1:
    puts("\nstart1 err\n");
    err = net_video_rec1_stop(0);

    if (err) {
        printf("\nstart wrong1 %x\n", err);
        //while(1);
    }

#endif

#ifdef CONFIG_VIDEO0_ENABLE
__start_err0:
    puts("\nstart0 err\n");
    err = net_video_rec0_stop(0);

    if (err) {
        printf("\nstart wrong0 %x\n", err);
        //while(1);
    }

#endif

#endif
    return -EFAULT;
}

static int net_video0_rec_rt_mute()
{
    int err;
    char buf[128];

    if (__this_net->net_state != VIDREC_STA_START) {
        return 0;
    }

    union video_req req = {0};

    req.rec.channel = 1;

    req.rec.state 	=  VIDEO_STATE_PKG_NETVRT_MUTE;

    req.rec.net_par.net_vidrt_onoff = __this_net->net_video0_vrt_on;

    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);

    req.rec.state 	=  VIDEO_STATE_PKG_NETART_MUTE;

    req.rec.net_par.net_audrt_onoff = __this_net->net_video0_art_on ;

    err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);


    return err;
}


static int net_video1_rec_rt_mute()
{
    int err;
    char buf[128];

    if (__this_net->net_state1 != VIDREC_STA_START) {
        return 0;
    }

    union video_req req = {0};

    req.rec.channel = 0;

    req.rec.state 	=  VIDEO_STATE_PKG_NETVRT_MUTE;

    req.rec.net_par.net_vidrt_onoff = __this_net->net_video1_vrt_on;

    err =	server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);

    req.rec.state 	=  VIDEO_STATE_PKG_NETART_MUTE;

    req.rec.net_par.net_audrt_onoff = __this_net->net_video1_art_on ;

    err = server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);


    return err;
}

static int net_video_rec_aud_mute()
{
#if defined CONFIG_WIFI_ENABLE

    if (__this_net->net_state != VIDREC_STA_START) {
        return 0;
    }

#ifdef CONFIG_VIDEO0_ENABLE
    net_video_rec0_aud_mute();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    net_video_rec1_aud_mute();
#endif
#endif
    return 0;
}


static int net_video0_rec_stop(u8 close)
{
    int err;
#ifdef CONFIG_VIDEO0_ENABLE
    __this_net->net_state = VIDREC_STA_STOPING;
    err = net_video_rec0_stop(close);

    if (err) {
        puts("\n net stop0 err\n");
    }

    __this_net->net_state = VIDREC_STA_STOP;
#endif

    return 0;
}

static int net_video1_rec_stop(u8 close)
{
    int err;
    /* #ifdef CONFIG_VIDEO1_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    puts("\n\n\n\n\n\n net video1 rec stop\n\n\n\n\n\n");
    __this_net->net_state1 = VIDREC_STA_STOPING;
    err = net_video_rec1_stop(close);

    if (err) {
        puts("\n net stop1 err\n");
    }

    __this_net->net_state1 = VIDREC_STA_STOP;
#endif

    return 0;
}


static int net_video_rec_stop(u8 close)
{
    int err;
#if defined CONFIG_WIFI_ENABLE
#ifdef CONFIG_VIDEO0_ENABLE

#if !(defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    if (__this_net->videoram_mark != 1 && close == 0) {
        puts("\n video ram mark\n");
        return 0;
    }
#endif

    if (__this_net->net_state == VIDREC_STA_START) {

        __this_net->net_state = VIDREC_STA_STOPING;
        err = net_video_rec0_stop(close);

        if (err) {
            puts("\n net stop0 err\n");
        }

        __this_net->net_state = VIDREC_STA_STOP;
    }

#endif

    /* #ifdef CONFIG_VIDEO1_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    if (__this_net->net_state1 == VIDREC_STA_START) {
        puts("\n net video rec 1 stop\n");
        __this_net->net_state1 = VIDREC_STA_STOPING;
        err = net_video_rec1_stop(close);

        if (err) {
            puts("\n net stop1 err\n");
        }

        __this_net->net_state1 = VIDREC_STA_STOP;
    }

#endif

#endif
    return 0;
}

static int  net_rt_video0_open(struct intent *it)
{
    puts("\nnet rt video0 iopen \n");
#ifdef CONFIG_VIDEO0_ENABLE
    int ret;

    if (it) {
        u8 mark = *((u8 *)it->data);

        if (mark == 0) {
            __this_net->net_video0_art_on = 1;
            __this_net->net_video0_vrt_on = 0 ;
        } else if (mark == 1) {
            __this_net->net_video0_vrt_on = 1;
            __this_net->net_video0_art_on = 0;
        } else {
            __this_net->net_video0_art_on = 1;
            __this_net->net_video0_vrt_on = 1;
        }

        rt_stream_cmd_analysis(0, it->exdata);
    } else {
        if (__this_net->net_video0_art_on == 0 && __this_net->net_video0_vrt_on == 0) {
            puts("\n rt not open\n");
            return 1;
        }
        if (__this_net->net_state == VIDREC_STA_START) {
            puts("\n net rt is on \n");
            return 1;
        }
    }

    printf("%s  %d\n", __func__, __LINE__);
    u32 res = db_select("res");
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    if (__this->state == VIDREC_STA_START) {
#else
    if ((__this->state == VIDREC_STA_START) && (res == 0) && (__this_net->net_videoreq[0].rec.width > 1280)) {
#endif
        video_rec0_netrt_mute();
    } else {

        if (__this_net->net_state == VIDREC_STA_START) {
            ret = net_video0_rec_rt_mute();
            if (ret < 0) {
                return 0;
            }
        } else {
            ret = net_video0_rec_start(it);
            if (ret < 0) {
                return 0;
            }

        }

    }

    return 1;
#else
    return 0;
#endif

}

static int  net_rt_video0_stop(struct intent *it)
{
    int ret;
    puts("\nnet rt video0 stop \n");
#ifdef CONFIG_VIDEO0_ENABLE
    u8 mark = *((u8 *)it->data);

    if (mark == 0) {
        __this_net->net_video0_art_on = 0;
    } else if (mark == 1) {
        __this_net->net_video0_vrt_on = 0;
    } else {
        __this_net->net_video0_art_on = 0;
        __this_net->net_video0_vrt_on = 0;
    }

    u32 res = db_select("res");
    /* #ifdef CONFIG_VIDEO1_ENABLE */

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    if (__this->state == VIDREC_STA_START) {
#else

    if ((__this->state == VIDREC_STA_START) && (res == 0) && (__this_net->net_videoreq[0].rec.width > 1280)) {
#endif
        video0_rec_ch2_stop(0);
    } else {
        if (__this_net->net_state == VIDREC_STA_START) {
            net_video0_rec_stop(0);
        }
    }

#endif
    return  0;
}

static int net_rt_video1_open(struct intent *it)
{
    puts("\n net rt video1 open\n");
    int ret;
    char buf[128];
    /* #ifdef CONFIG_VIDEO1_ENABLE */

#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    if (it) {
        u8 mark = *((u8 *)it->data);

        if (mark == 0) {
            __this_net->net_video1_art_on = 1;
            __this_net->net_video1_vrt_on = 0;
        } else if (mark == 1) {
            __this_net->net_video1_art_on = 0;
            __this_net->net_video1_vrt_on = 1;
        } else {
            __this_net->net_video1_art_on = 1;
            __this_net->net_video1_vrt_on = 1;
        }

        rt_stream_cmd_analysis(1, it->exdata);
    } else {
        if (__this_net->net_video1_art_on == 0 && __this_net->net_video1_vrt_on == 0) {
            puts("\nvideo1 rt not open \n");
            return 1;
        }
        if (__this_net->net_state1 == VIDREC_STA_START) {
            puts("\nvideo1 rt is open \n");
            return 1;
        }
    }

    if (__this->state == VIDREC_STA_START) {
#ifdef CONFIG_VIDEO1_ENABLE
        video_rec1_netrt_mute();
#endif
#ifdef CONFIG_VIDEO3_ENABLE
        video_rec2_netrt_mute();
#endif
    } else {
        if (__this_net->net_state1 == VIDREC_STA_START) {
            net_video1_rec_rt_mute();
        } else {
            net_video1_rec_start(it);
        }
    }

#endif

    return 1;
}
static int  net_rt_video1_stop(struct intent *it)
{
    int ret;
    /* #ifdef CONFIG_VIDEO1_ENABLE || CONFIG_VIDEO3_ENABLE */
#if (defined CONFIG_VIDEO1_ENABLE || defined CONFIG_VIDEO3_ENABLE)
    u8 mark = *((u8 *)it->data);

    if (mark == 0) {
        __this_net->net_video1_art_on = 0;
    } else if (mark == 1) {
        __this_net->net_video1_vrt_on = 0;
    } else {
        __this_net->net_video1_art_on = 0;
        __this_net->net_video1_vrt_on = 0;
    }

    if (__this->state == VIDREC_STA_START) {
#ifdef CONFIG_VIDEO1_ENABLE
        video1_rec_ch2_stop(0);
#endif
#ifdef CONFIG_VIDEO3_ENABLE
        video2_rec_ch2_stop(0);
#endif
    } else {
        if (__this_net->net_state1 == VIDREC_STA_START) {
            net_video1_rec_stop(0);
        }
    }

#endif
    return  0;
}

void  net_video_rec0_close()
{
    if (__this_net->net_video_rec) {
        server_close(__this_net->net_video_rec);
        __this_net->net_video_rec = NULL;
    }

}
void  net_video_rec1_close()
{
    if (__this_net->net_video_rec2) {
        server_close(__this_net->net_video_rec2);
        __this_net->net_video_rec2 = NULL;
    }
}

static int net_video_rec_close()
{
#ifdef CONFIG_VIDEO0_ENABLE
    net_video_rec0_close();
#endif

#ifdef CONFIG_VIDEO1_ENABLE
    net_video_rec1_close();
#endif

    return 0;
}
#endif


extern u32 bg0808_dvp_mirr(u8 on);


/*
 *录像的状态机,进入录像app后就是跑这里
 */
struct rt_stream_app_info *temp_info;
static int video_rec_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int err = 0;
    int len;
    char buf[128];

    switch (state) {
    case APP_STA_CREATE:
        puts("--------app_rec: APP_STA_CREATE\n");

        memset(__this, 0, sizeof_this);
        memset(__this_net, 0, sizeof_this_net);

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
            if (it->data && !strcmp(it->data, "lan_setting")) {
                __this->lan_det_setting = 1;
                ve_server_open(1);
                lane_det_setting_disp();
            } else {
                video_rec_init();
            }
			if(db_select("rat"))
			{
				printf("==========123456============\n");
				bg0808_dvp_mirr(1);	
			}
			else
			{
				bg0808_dvp_mirr(0);		
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
        case ACTION_VIDEO_OPEN_NET_H264:
            log_i("ACTION_VIDEO_OPEN_NET_H264\n");
            net_h264_malloc();
            net_h264_start(1);

            break;
        case ACTION_VIDEO_STOP_NET_H264:
            log_i("ACTION_VIDEO_STOP_NET_H264\n");
            net_h264_stop(0, 0);
            break;
        case ACTION_VIDEO_OPEN_NET_JPEG:
            log_i("ACTION_VIDEO_OPEN_NET_JPEG\n");
            net_jpeg_malloc();
            net_jpeg_start(1);
            break;

        case ACTION_VIDEO_STOP_NET_JPEG:
            log_i("ACTION_VIDEO_STOP_NET_JPEG\n");
            net_jpeg_stop(0);
            break;


#ifdef CONFIG_TOUCH_UI_ENABLE
        case ACTION_VIDEO_REC_CONTROL:
            video_rec_control(0);
            if (__this->state == VIDREC_STA_STOP) {
                ve_mdet_reset();
                ve_lane_det_reset();
            }
            break;
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
                /* if (video_rec_online_nums() == 3) { */
                if (__this->video1_online) {
                    __this->disp_video_ctrl = 1;
                } else if (__this->video2_online) {
                    __this->disp_video_ctrl = 2;
                } else {
                    __this->disp_video_ctrl = 3;
                }
                /* } */
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
        case ACTION_VIDEO_TAKE_PHOTO:

            puts("===================00000000\n");
            if (__this_net->net_video0_vrt_on && __this->state != VIDREC_STA_START) {
                puts("\n cat pic net ch \n");
                video_rec_take_photo1(0);
            } else if (__this_net->net_video1_vrt_on && __this->state != VIDREC_STA_START) {
                video_rec_take_photo1(1);
            } else if (__this->state == VIDREC_STA_START) {
                if (__this_net->net_video1_vrt_on) {

                    puts("===================1111111111\n");
#ifdef   CONFIG_VIDEO1_ENABLE
                    video_rec_take_photo(1);
#endif
#ifdef   CONFIG_VIDEO3_ENABLE
                    video_rec_take_photo(2);
#endif
                } else {
                    puts("===================222222222\n");
                    video_rec_take_photo(0);
                }
            }

            break;
        case ACTION_VIDEO_REC_CONCTRL:
            err = video_rec_control(0);

            if (__this->state == VIDREC_STA_STOP) {
                ve_mdet_reset();
                ve_lane_det_reset();
            }

            break;

        case ACTION_VIDEO_REC_GET_APP_STATUS:
            video_rec_get_app_status(it);
            break;

        case ACTION_VIDEO_REC_GET_PATH:
            video_rec_get_path(it);
            break;
#if 1
#if defined CONFIG_WIFI_ENABLE
        case ACTION_VIDEO0_OPEN_RT_STREAM:

            err = net_rt_video0_open(it);
            sprintf(buf, "format:%d,w:%d,h:%d,fps:%d,rate:%d"
                    , (__this_net->net_videoreq[0].rec.format == NET_VIDEO_FMT_MOV) ? 1 : 0
                    , __this_net->net_videoreq[0].rec.width
                    , __this_net->net_videoreq[0].rec.height
                    , __this_net->net_videoreq[0].rec.fps
                    , AUDIO_SAM_RATE);
            if (err) {
                printf("<<<<<<<%s\n\n\n\n\n", buf);

                if (__this_net->net_videoreq[0].rec.format < STRM_VIDEO_FMT_AVI) {
                    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "OPEN_RT_STREAM", "NOTIFY", buf);
                }
            } else {

                CTP_CMD_COMBINED(NULL, CTP_RT_OPEN_FAIL, "OPEN_RT_STREAM", "NOTIFY", CTP_RT_OPEN_FAIL_MSG);
            }
            break;

        case ACTION_VIDEO1_OPEN_RT_STREAM:
            puts("\n video1 open rt\n");
            err = net_rt_video1_open(it);
            sprintf(buf, "format:%d,w:%d,h:%d,fps:%d,rate:%d"
                    , (__this_net->net_videoreq[1].rec.format == NET_VIDEO_FMT_MOV) ? 1 : 0
                    , __this_net->net_videoreq[1].rec.width
                    , __this_net->net_videoreq[1].rec.height
                    , __this_net->net_videoreq[1].rec.fps
                    , AUDIO_SAM_RATE);

            if (err) {

                if (__this_net->net_videoreq[1].rec.format < STRM_VIDEO_FMT_AVI) {
                    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "OPEN_PULL_RT_STREAM", "NOTIFY", buf);
                }
            } else {

                CTP_CMD_COMBINED(NULL, CTP_RT_OPEN_FAIL, "OPEN_PULL_RT_STREAM", "NOTIFY", CTP_RT_OPEN_FAIL_MSG);
            }
            break;

        case ACTION_VIDEO0_CLOSE_RT_STREAM:
            err = net_rt_video0_stop(it);
            if (!err) {
                strcpy(buf, "status:1");
            } else {
                strcpy(buf, "status:0");
            }

            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "CLOSE_RT_STREAM", "NOTIFY", buf);

            break;

        case ACTION_VIDEO1_CLOSE_RT_STREAM:
            err =  net_rt_video1_stop(it);
            if (!err) {
                strcpy(buf, "status:1");
            } else {
                strcpy(buf, "status:0");
            }

            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "CLOSE_PULL_RT_STREAM", "NOTIFY", buf);


            break;
#endif

        case ACTION_VIDEO_CYC_SAVEFILE:
            video_cyc_file(0);
#if defined CONFIG_VIDEO1_ENABLE
            video_cyc_file(1);
#endif
#if defined CONFIG_VIDEO3_ENABLE
            video_cyc_file(2);
#endif

            /* video_rec_control(1); */
            strcpy(buf, "status:1");
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "VIDEO_CYC_SAVEFILE", "NOTIFY", buf);
#endif
            break;
        }
        break;
    case APP_STA_PAUSE:
        puts("--------app_rec: APP_STA_PAUSE\n");
        break;
    case APP_STA_RESUME:
        puts("--------app_rec: APP_STA_RESUME\n");
        break;
    case APP_STA_STOP:
        puts("--------app_rec: APP_STA_STOP\n");

        if (__this->state == VIDREC_STA_START) {
            video_rec_control(0);
            if (__this->state == VIDREC_STA_STOP) {
                ve_mdet_stop();
                ve_lane_det_stop(0);
            }
        }
#if defined CONFIG_WIFI_ENABLE

        if (__this_net->net_state == VIDREC_STA_START || __this_net->net_state1 == VIDREC_STA_START) {
            net_video_rec_stop(1);
        }

#endif
        if (video_rec_uninit()) {
            err = 1;
            break;
        }
#if defined CONFIG_NET_CLIENT
        net_h264_stop(1, 0);
        net_jpeg_stop(1);
#endif

#ifdef CONFIG_UI_ENABLE
        puts("--------rec hide ui\n");
        hide_main_ui();
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




/*
 *录像app的按键响应函数
 */
static int video_rec_key_event_handler(struct key_event *key)
{
    int err;

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
            struct cmd_ctl cinfo;
            __this_net->cmd_fd = dev_open("net2video", "cmd_ctl");
            cinfo.cmd = CMD_SWITCH_TO_PHOTO_MODE;
            cinfo.len = 4;
            cinfo.data = NULL;
            dev_ioctl(__this_net->cmd_fd, IPCIOC_SEND_CMD, (u32)&cinfo);
            if (cinfo.cmd != CMD_NO_ERR) {
                log_w("switch photo fail\n");
            }
            dev_close(__this_net->cmd_fd);


            break;
        case KEY_UP:
            puts("KEY_UP in\n");
			sys_key_event_disable();
			if(__this->wifi_on == 0)
			{
				__this->wifi_on = 1;
				wifi_on(); 
				ui_show(LAYER_MSG_WIFI_INFO);
			}
			else
			{
				__this->wifi_on = 0;
				wifi_off(); 
				ui_hide(LAYER_MSG_WIFI_INFO);	
			}
            
			sys_key_event_enable();
			
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
#if QR_DECODE
    case KEY_EVENT_LONG:

        switch (key->value) {
        case KEY_UP:

            puts("\n qr_init \n");

            if (db_select("mot")) {
                ve_mdet_stop();
            }
            if (db_select("lan")) {
                ve_lane_det_stop(0);
            }
            void get_yuv_init(void (*cb)(u8 * data));
            void qr_decode_cb_func(u8 * image_data);

            get_yuv_init(qr_decode_cb_func);
            qr_decode_init();

            break;
        case KEY_DOWN:
            puts("\n qr_uninit \n");
            void get_yuv_uninit(void);
            qr_decode_uninit();
            get_yuv_uninit();
            if (db_select("mot")) {
                ve_mdet_start();
            }
            if (db_select("lan")) {
                ve_lane_det_start(0);
            }
            break;
        default:
            break;
        }
        break;
#endif
    default:
        break;
    }

    return false;
}

/*
 *录像app的设备响应函数
 */
static int video_rec_device_event_handler(struct sys_event *event)
{
    int err;
    struct intent it;
    char buf[12];
    char buffer[32];
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

            hide_main_ui();
#ifdef CONFIG_TOUCH_UI_ENABLE
            hide_home_main_ui();//录像在后台进入倒车隐藏主界面
#endif
            show_park_ui();
            /* if (__this->disp_state == DISP_BACK_WIN) { */
            /* puts("\n back win re open==========\n"); */
            /* return true; */
            /* } */

            puts("\n parking on\n");
            if (video_rec_online_nums() >= 2) {
                __this->disp_video_ctrl = __this->disp_park_sel;
            }

            __this->disp_state = DISP_PARK_WIN;
            video_disp_control();
            return true;

        case DEVICE_EVENT_OUT://parking off
            hide_park_ui();
            show_main_ui();
            puts("\n parking off\n");

#ifdef CONFIG_VIDEO1_ENABLE
            __this->video1_online = dev_online("video1");
#endif

#ifdef CONFIG_VIDEO3_ENABLE
            __this->video2_online = dev_online("uvc");
#endif

#ifdef CONFIG_VIDEO4_ENABLE
            __this->video3_online = dev_online("net2video");
#endif

            if (__this->video1_online || __this->video2_online || __this->video3_online) {
                __this->disp_state = DISP_RESET_WIN;
            } else {
                __this->disp_state = DISP_HALF_WIN;
            }

            if (__this->video1_online) {
                __this->disp_video_ctrl = 1;
            } else if (__this->video2_online) {
                __this->disp_video_ctrl = 2;
            } else if (__this->video3_online) {
                __this->disp_video_ctrl = 3;
            } else {
                __this->disp_video_ctrl = 0;
            }

            __this->disp_sw_flag = 0;
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
                if (__this->disp_video_ctrl == 0) {
                    __this->disp_video_ctrl = 1;

                    video1_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                    __this->disp_state = DISP_BIG_SMALL;
                }
                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }
            strcpy(buffer, "status:1");
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "PULL_VIDEO_STATUS", "NOTIFY", buffer);
            break;
        case DEVICE_EVENT_OUT:
            if (__this->video1_online) {
                __this->video1_online = false;

                __this->disp_sw_flag = 0;
                if (__this->disp_video_ctrl == 1) {

                    if (__this->video2_online) {
                        __this->disp_video_ctrl = 2;
                    } else if (__this->video3_online) {
                        __this->disp_video_ctrl = 3;
                    } else {
                        __this->disp_video_ctrl = 0;
                    }

                    if (__this->disp_state == DISP_BIG_SMALL) {
                        video1_disp_stop();
                        __this->disp_state = DISP_FRONT_WIN;

                        __this->disp_video_ctrl = 0;

                    } else if (__this->disp_state == DISP_HALF_WIN ||
                               __this->disp_state == DISP_BACK_WIN) {
                        __this->disp_state = DISP_RESET_WIN;
                        video_disp_control();
                    }
                }
                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                } else {
#if defined CONFIG_WIFI_ENABLE
                    if (__this_net->net_state1 == VIDREC_STA_START) {
                        net_video1_rec_stop(0);
                    }
#endif
                }
#if defined CONFIG_WIFI_ENABLE
                __this_net->net_video1_art_on = 0;
                __this_net->net_video1_vrt_on = 0;
                strcpy(buffer, "status:0");
                CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "PULL_VIDEO_STATUS", "NOTIFY", buffer);


#endif
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

                if (__this->disp_video_ctrl == 0) {
                    __this->disp_video_ctrl = 2;

                    video2_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                    __this->disp_state = DISP_BIG_SMALL;
                }

                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }

            strcpy(buffer, "status:1");
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "PULL_VIDEO_STATUS", "NOTIFY", buffer);
            break;
        case DEVICE_EVENT_OUT:
            if (__this->video2_online) {
                __this->video2_online = false;

                __this->disp_sw_flag = 0;
                if (__this->disp_video_ctrl == 2) {

                    if (__this->video1_online) {
                        __this->disp_video_ctrl = 1;
                    } else if (__this->video3_online) {
                        __this->disp_video_ctrl = 3;
                    } else {
                        __this->disp_video_ctrl = 0;
                    }

                    if (__this->disp_state == DISP_BIG_SMALL) {
                        video2_disp_stop();
                        __this->disp_state = DISP_FRONT_WIN;
                    } else if (__this->disp_state == DISP_HALF_WIN ||
                               __this->disp_state == DISP_BACK_WIN) {
                        __this->disp_state = DISP_RESET_WIN;
                        video_disp_control();
                    }
                }
                strcpy(buffer, "status:0");
                CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "PULL_VIDEO_STATUS", "NOTIFY", buffer);


                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                } else {


#if defined CONFIG_WIFI_ENABLE
                    if (__this_net->net_state1 == VIDREC_STA_START) {
                        net_video1_rec_stop(0);
                    }
#endif
                }
#if defined CONFIG_WIFI_ENABLE
                __this_net->net_video1_art_on = 0;
                __this_net->net_video1_vrt_on = 0;



#endif
            }
            break;
        }
    }
#endif



#ifdef CONFIG_VIDEO4_ENABLE
    else if (!strncmp((char *)event->arg, "net2video", 9)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
        case DEVICE_EVENT_ONLINE:
            if (!__this->video3_online) {
                __this->video3_online = true;

                if (__this->disp_video_ctrl == 0) {
                    __this->disp_video_ctrl = 3;

                    video3_disp_start(SMALL_SCREEN_W, SMALL_SCREEN_H, 0, 0);
                    __this->disp_state = DISP_BIG_SMALL;
                }

                if (__this->state == VIDREC_STA_START) {
                    video_rec_control(0);
                    video_rec_control(0);
                }
            }
            break;
        case DEVICE_EVENT_OUT:
            if (__this->video3_online) {
                __this->video3_online = false;

                __this->disp_sw_flag = 0;

                if (__this->disp_video_ctrl == 3) {

                    if (__this->video1_online) {
                        __this->disp_video_ctrl = 1;
                    } else if (__this->video2_online) {
                        __this->disp_video_ctrl = 2;
                    } else {
                        __this->disp_video_ctrl = 0;
                    }

                    if (__this->disp_state == DISP_BIG_SMALL) {
                        video3_disp_stop();
                        __this->disp_state = DISP_FRONT_WIN;
                        __this->disp_video_ctrl = 0;
                    } else if (__this->disp_state == DISP_HALF_WIN ||
                               __this->disp_state == DISP_BACK_WIN) {
                        __this->disp_state = DISP_RESET_WIN;
                        video_disp_control();
                    }
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

#if 0
static void video_test_timer(void *p)
{
    struct intent it;

    it.name	= "video_rec";
    it.data = NULL;


    puts("-------------video_test_timer\n");
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

static int video0_net_savefile(u32 cyc_time)
{
    union video_req req = {0};
    int err;

    if (__this_net->net_video_rec) {
        req.rec.channel = 0;
        req.rec.state 	= VIDEO_STATE_NET_CYC;
        req.rec.cycle_time = cyc_time;
        err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_SAVEFILE;
        }
    }


    return 0;
}
#if 1
static int video0_get_image_picture(u8 **buf,  int *len)
{
    union video_req req = {0};
    int err;
    int inited = 0;
    if (__this_net->net_state != VIDREC_STA_START) {
        net_h264_malloc();
        net_h264_start(0);
        inited = 1;
    }

    if (__this_net->net_video_rec) {
        req.rec.channel = 0;
        req.rec.state 	= VIDEO_STATE_SMALL_PIC;
        req.rec.rec_small_pic 	= 1;
        err = server_request(__this_net->net_video_rec, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_SAVEFILE;
        }
    }

    log_i("small_pic_buf addr 0x%x   small_pic_len %d\n", req.rec.net_par.small_pic_buf, req.rec.net_par.small_pic_len);

    *buf = req.rec.net_par.small_pic_buf;
    *len = req.rec.net_par.small_pic_len;
    if (inited && (__this_net->net_state == VIDREC_STA_START)) {
        inited = 0;
        net_h264_stop(1, 1);
    }

    return 0;
}
#else
static int video0_get_image_picture(u8 **buf,  int *len)
{
    union video_req req = {0};
    int err;
    int inited = 0;
    if (__this_net->net_state1 != VIDREC_STA_START) {
        net_jpeg_malloc();
        net_jpeg_start(0);
        inited = 1;
    }

    if (__this_net->net_video_rec2) {
        req.rec.channel = 1;
        req.rec.state 	= VIDEO_STATE_SMALL_PIC;
        req.rec.rec_small_pic 	= 1;
        err = server_request(__this_net->net_video_rec2, VIDEO_REQ_REC, &req);
        if (err != 0) {
            printf("\nsave rec err 0x%x\n", err);
            return VREC_ERR_V0_REQ_SAVEFILE;
        }
    }

    log_i("small_pic_buf addr 0x%x   small_pic_len %d\n", req.rec.net_par.small_pic_buf, req.rec.net_par.small_pic_len);

    *buf = req.rec.net_par.small_pic_buf;
    *len = req.rec.net_par.small_pic_len;
    if (inited && (__this_net->net_state1 == VIDREC_STA_START)) {
        inited = 0;
        net_jpeg_stop(1);
    }

    return 0;
}
#endif

static int capture_image(struct photo_parm *p_parm)
{
    int err = 0;
    union video_req req = {0};
    log_d("width:%d  height:%d  quality:%d\n", p_parm->width, p_parm->height, p_parm->quality) ;
    req.icap.width = p_parm->width;
    req.icap.height = p_parm->height;
    req.icap.quality =  p_parm->quality;
    req.icap.buf = (u8 *)p_parm->databuffer;
    req.icap.buf_size = p_parm->width * p_parm->height;
    req.icap.camera_type = VIDEO_CAMERA_NORMAL;

    if (!__this_net->net_video_rec)  {
        __this_net->net_video_rec = server_open("video_server", "video0");
        if (!__this_net->net_video_rec) {
            log_e("server open fali\n");
            return -EFAULT;
        }
    }
    err = server_request(__this_net->net_video_rec, VIDEO_REQ_IMAGE_CAPTURE, &req);
    if (err) {
        goto __err;
    }

    err = server_request(__this_net->net_video_rec, VIDEO_REQ_GET_IMAGE, &req);
    if (err || !req.image.buf) {
        goto __err;
    }

    p_parm->databuffer = (char *)req.image.buf;
    p_parm->datalen = req.image.size;
    /* log_d("image size =%d\n", p_parm->datalen); */
    /* put_buf(p_parm->databuffer, 32); */

    return 0;
__err:
    return err;
}

static void take_photo(struct photo_parm *p_parm)
{
    p_parm->databuffer = malloc(p_parm->width * p_parm->height);
    if (!p_parm->databuffer) {
        return;
    }

    capture_image(p_parm);
}
static int net_switch_ui(const char *app_name)
{
    struct intent it;
    struct application *app;

    init_intent(&it);
    it.action = ACTION_BACK;
    start_app(&it);

    if (!strcmp(app_name, "video_rec")) {
        it.name = "video_rec";
        it.action = ACTION_VIDEO_REC_MAIN;

    } else if (!strcmp(app_name, "video_photo")) {
        it.name = "video_photo";
        it.action = ACTION_PHOTO_TAKE_MAIN;
    }
    start_app(&it);

    return 0;
}



int user_cmd_cb(int cmd, char *buffer, int len, void *priv)
{
    struct cmd_ctl cinfo;
    struct intent it;
    struct application *app = NULL;
    init_intent(&it);
    int ret = 0;
    switch (cmd) {
    case CMD_OPEN_H264_STREAM:
        log_i("CMD_OPEN_H264_STREAM\n");
        app = get_current_app();
        if (!strcmp(app->name, "video_photo")) {
            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app(&it);
        }

        it.name = "video_rec";
        it.action = ACTION_VIDEO_OPEN_NET_H264;
        ret = start_app(&it);
        if (ret) {
            ret = CMD_TIMEOUT_CMD;
        } else {

            ret = CMD_NO_ERR;
        }
        //it.data  暂不传参
        cinfo.len = 4;
        cinfo.cmd = ret;
        cinfo.data = NULL;
        cmd_send(&cinfo);
        break;

    case CMD_OPEN_JPEG_STREAM:
        log_i("CMD_OPEN_JPEG_STREAM\n");
        app = get_current_app();
        if (!strcmp(app->name, "video_photo")) {
            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app(&it);
        }

        it.name = "video_rec";
        it.action = ACTION_VIDEO_OPEN_NET_JPEG;
        ret = start_app(&it);
        if (ret) {
            ret = CMD_TIMEOUT_CMD;
        } else {

            ret = CMD_NO_ERR;
        }

//       ret = CMD_NO_ERR;
        cinfo.len = 4;
        cinfo.cmd = ret;
        cmd_send(&cinfo);
        break;
    case CMD_STOP_H264_STREAM:
        log_i("CMD_STOP_H264_STREAM\n");
        app = get_current_app();
        if (!strcmp(app->name, "video_photo")) {
            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app(&it);
        }

        it.name = "video_rec";
        it.action = ACTION_VIDEO_STOP_NET_H264;
        ret = start_app(&it);
        if (ret) {
            ret = CMD_TIMEOUT_CMD;
        } else {
            ret = CMD_NO_ERR;
        }
        cinfo.len = 4;
        cinfo.cmd = ret;
        cmd_send(&cinfo);
        break;
    case CMD_STOP_JPEG_STREAM:
        log_i("CMD_STOP_JPEG_STREAM\n");
        app = get_current_app();
        if (!strcmp(app->name, "video_photo")) {
            it.name = "video_photo";
            it.action = ACTION_BACK;
            start_app(&it);
        }

        it.name = "video_rec";
        it.action = ACTION_VIDEO_STOP_NET_JPEG;
        ret = start_app(&it);
        if (ret) {
            ret = CMD_TIMEOUT_CMD;
        } else {

            ret = CMD_NO_ERR;
        }


//       ret = CMD_NO_ERR;
        cinfo.len = 4;
        cinfo.cmd = ret;
        cmd_send(&cinfo);
        break;

    case CMD_GET_FPS:
        log_i("CMD_GET_FPS\n");
        int fps = 30;
        ret = CMD_NO_ERR;
        cinfo.len = 4 + sizeof(fps);
        cinfo.cmd = ret;
        cinfo.data = (char *)&fps;

        cmd_send(&cinfo);

        break;

    case CMD_GET_DISPLAY_RESO:
        log_i("CMD_GET_DISPLAY_RESO\n");
        int reso = (640 << 16) | 480;
        ret = CMD_NO_ERR;
        cinfo.len = 4 + sizeof(reso);
        cinfo.cmd = ret;
        cinfo.data = (char *)&reso;
        cmd_send(&cinfo);
        break;

    case CMD_SET_JPEG_RESO:
        log_i("CMD_SET_JPEG_RESO\n");

        int *tmp = (int *)buffer;

        __this_net->remote.width = (*tmp >> 16) & 0xFFFF;
        __this_net->remote.height = (*tmp) & 0xFFFF;
        ret = CMD_NO_ERR;
        cinfo.len = 4;
        cinfo.cmd = ret;
        cinfo.data = NULL;
        cmd_send(&cinfo);
        break;

    case CMD_SET_VIDEO_REC_PRARM:
        log_i("CMD_SET_VIDEO_REC_PRARM\n");

        memcpy(&__this_net->remote.rec_info, buffer, sizeof(struct vs_video_rec));
        log_i("dataLen=0x%x  len=0x%x\n", sizeof(struct vs_video_rec), len);
        ret = CMD_NO_ERR;
        cinfo.len = 4 ;
        cinfo.cmd = ret;
        cinfo.data = NULL;
        cmd_send(&cinfo);
        break;

    case CMD_SET_VIDEO_SAVE_FILE:
        log_i("CMD_SET_VIDEO_SAVE_FILE\n");
        u32 cyc_time = *((u32 *)buffer);
        log_i("cyc_time=%d\n", cyc_time);
        video0_net_savefile(cyc_time);
        ret = CMD_NO_ERR;
        cinfo.len = 4 ;
        cinfo.cmd = ret;
        cinfo.data = NULL;
        cmd_send(&cinfo);
        break;


    case CMD_GET_IMAGE_PICTURE:
        log_i("CMD_GET_SMALL_PICTURE\n");
        video0_get_image_picture((u8 **)&cinfo.data, &cinfo.len);
        /* put_buf(cinfo.data, 32); */
        /* log_i("cinfo.len = %d", cinfo.len); */
        ret = CMD_NO_ERR;
        cinfo.len += 4 ;
        cinfo.cmd = ret;
        cmd_send(&cinfo);
        break;
    case CMD_SWITCH_TO_PHOTO_MODE:
        log_i("CMD_SWITCH_TO_PHOTO_MODE\n");
        /* net_switch_ui("video_photo"); */
        ret = CMD_NO_ERR;
        cinfo.len = 4 ;
        cinfo.cmd = ret;
        cmd_send(&cinfo);

        break;
    case CMD_SWITCH_TO_VIDEO_MODE:
        log_i("CMD_SWITCH_TO_VIDEO_MODE\n");
        /* net_switch_ui("video_rec"); */
        ret = CMD_NO_ERR;
        cinfo.len = 4 ;
        cinfo.cmd = ret;
        cmd_send(&cinfo);

        break;
    case CMD_TAKE_PHOTO:
        log_i("CMD_TAKE_PHOTO\n");

        take_photo((struct photo_parm *)buffer);

        struct photo_parm *p_parm0 = (struct photo_parm *)buffer;
        cinfo.len = p_parm0->datalen;
        cinfo.data = p_parm0->databuffer;

        ret = CMD_NO_ERR;
        cinfo.len += 4 ;
        cinfo.cmd = ret;
        cmd_send(&cinfo);
        if (p_parm0->databuffer) {
            free(p_parm0->databuffer);
            p_parm0->databuffer = NULL;
        }


        break;

    case CMD_TAKE_PHOTO1:
        log_i("CMD_TAKE_PHOTO1\n");

        it.action = ACTION_PHOTO_TAKE_CONTROL;
        it.data = (char *)buffer;
        it.name = "video_photo";
        ret = start_app(&it);
        if (ret) {
            ret = CMD_TIMEOUT_CMD;
        } else {
            ret = CMD_NO_ERR;
        }

        struct photo_parm *p_parm1 = (struct photo_parm *)buffer;
        cinfo.len = p_parm1->datalen;
        cinfo.data = p_parm1->databuffer;

//       ret = CMD_NO_ERR;
        cinfo.len += 4 ;
        cinfo.cmd = ret;
        cmd_send(&cinfo);

        break;




    default:
        ret = CMD_UNKNOWN_CMD;
        cinfo.len = 4;
        cinfo.cmd = ret;
        cinfo.data = NULL;
        cmd_send(&cinfo);
        break;
    }
    return ret;

}




