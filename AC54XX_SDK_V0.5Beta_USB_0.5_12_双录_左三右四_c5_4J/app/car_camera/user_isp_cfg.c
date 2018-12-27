/*************************************************************************
	> File Name: user_isp_cfg.c
	> Author:
	> Mail:
	> Created Time: Thu 27 Apr 2017 09:12:38 AM HKT
 ************************************************************************/


#include "server/video_server.h"
#include "user_isp_cfg.h"
#include "video_rec.h"



#define DEBUG_ISP 0

/**********************************isp效果配置修改****************************************/


//isp场景效果总段数修改
#define MAX_ISP_SCENES 4

//添加场景名字从0开始往后递增0,1,2,3,4,5.........
#define SCENE_NIGHT             0
#define SCENE_INDOOR            1
#define SCENE_OUTCLUDY          2
#define SCENE_OUTDOOR           3
//将场景添加到scane_type数组中
unsigned char scane_type[] = {SCENE_NIGHT, SCENE_INDOOR, SCENE_OUTCLUDY, SCENE_OUTDOOR};

//添加isp效果对应段的阈值范围
#define USER_ISP_CFG_NIGHT_L    ((0 << 8))
#define USER_ISP_CFG_NIGHT_H    ((0 << 8) 	+ 450)

#define USER_ISP_CFG_INDOOR_L   ((0 << 8) 	+ 500)
#define USER_ISP_CFG_INDOOR_H   ((7 << 8) 	- 128)

#define USER_ISP_CFG_OUTCLUDY_L ((7 << 8) 	+ 128)
#define USER_ISP_CFG_OUTCLUDY_H ((13 << 8) 	- 128)

#define USER_ISP_CFG_OUTDOOR_L  ((12 << 8) 	+ 128)
#define USER_ISP_CFG_OUTDOOR_H  ((20 << 8)  + 128)

//将isp效果阈值范围添加到isp_par数组,按照场景的顺序添加
struct usr_isp_cfg_par isp_par[] = {
    {USER_ISP_CFG_NIGHT_L, 		USER_ISP_CFG_NIGHT_H},
    {USER_ISP_CFG_INDOOR_L, 	USER_ISP_CFG_INDOOR_H},
    {USER_ISP_CFG_OUTCLUDY_L, 	USER_ISP_CFG_OUTCLUDY_H},
    {USER_ISP_CFG_OUTDOOR_L,	USER_ISP_CFG_OUTDOOR_H},
};

unsigned char wdr_en[] = {1, 1, 1, 0};/* 0:该场景不开wdr(无论菜单是否打开wdr使能，该场景都不开wdr)
										 1:该场景开wdr(菜单在开启wdr使能时，该场景的wdr才开)*/
const char *isp_cfg_file[] = {
    "mnt/spiflash/res/isp_cfg_0.bin",
    "mnt/spiflash/res/isp_cfg_1.bin",
    "mnt/spiflash/res/isp_cfg_2.bin",
    "mnt/spiflash/res/isp_cfg_3.bin",
};


/**************************************************************************/


#if (DEBUG_ISP == 1)
#define isp_puts  puts
#define user_isp_cfg_printf  printf
#else
#define isp_puts(...)
#define user_isp_cfg_printf(...)
#endif

#define USER_ISP_MODE_NORMAL    0x0
#define USER_ISP_MODE_SPECIAL   0x1

#define USER_CFG_STATE_UNINIT    0x0
#define USER_CFG_STATE_INIT     0x1
#define USER_CFG_STATE_RUN      0x2
#define USER_CFG_STATE_SWITCH   0x3
#define USER_CFG_STATE_STOP     0x4



struct isp_scenes {
    u8 *buf;
    u32 size;
};

struct screen_scenes {
    u8 valid;
    u8 *buf;
    u32 size;
};

struct user_isp_cfg {
    u8 state;
    u8 mode;
    s8 current_scene;
    int cur_lv;
    int prev_lv;
    struct isp_scenes scenes[MAX_ISP_SCENES];
    int timer;
    struct server *server;
    OS_SEM sem;
};

static struct user_isp_cfg  local_cfg = {
    .state = USER_CFG_STATE_UNINIT,
    .mode = USER_ISP_MODE_NORMAL,
    .current_scene = 1,
    .timer = 0,
};

#define     __this   (&local_cfg)

extern void video_rec_post_msg(const char *msg, ...);
extern void video_photo_post_msg(const char *msg, ...);
void video_headlight_post_to_ui(u8 scene)
{
    static u8 last_scene;
	static u8 cnt = 0;
    struct application *app;
    app = get_current_app();

    if (scene == SCENE_NIGHT) {
		//cnt++;
		//if(cnt > 5)
		{
			//cnt = 0;
			if (db_select("hlw")) {
	            if (app) {
	                if (!strcmp(app->name, "video_rec")) {
	                    video_rec_post_msg("HlightOn");//前照灯显示
	                } else if (!strcmp(app->name, "video_photo")) {
	                    video_photo_post_msg("HlightOn");//前照灯显示
	                }
	            }
	            last_scene = scene;
	        }
		}   
    } else {
    	cnt = 0;
        if (last_scene == SCENE_NIGHT) {
            if (app) {
                if (!strcmp(app->name, "video_rec")) {
                    video_rec_post_msg("HlightOff");//前照灯关闭
                } else if (!strcmp(app->name, "video_photo")) {
                    video_photo_post_msg("HlightOff");//前照灯关闭
                }
            }
        }
        last_scene = scene;
    }

}
int isp_scr_work_hdl(u8 scene)
{
    if (get_usb_in_status()) {
        return 0;
    }

    if ((get_video_disp_state() == DISP_BACK_WIN)
        || (get_parking_status() != 0)) {
        //后拉全屏显示（录像时），屏显效果固定为场景1
        scene = 1;
        __this->current_scene = scene;
    }

    struct isp_scr_work *work = NULL;
    extern struct isp_scr_work isp_scr_begin[];
    extern struct isp_scr_work isp_scr_end[];

    for (work = isp_scr_begin; work < isp_scr_end; work++) {
        if (scene == work->parm) {
            work->handler(work->parm);
            goto __END;
        }
    }

    if (work == isp_scr_end && work > isp_scr_begin) {
        //处理屏幕效果场景个数，少于isp效果场景数
        work = (isp_scr_end - 1);
        if (work->parm < scene) {
            work->handler(work->parm);
        }
    }

__END:

    return 0;

}

static int user_isp_cfg_init(void)
{
    FILE *fp;
    int size;
    int i;
    if (__this->state == USER_CFG_STATE_UNINIT) {
        memset(__this->scenes, 0x0, sizeof(__this->scenes));
        for (i = 0; i < MAX_ISP_SCENES; i++) {
            fp = fopen(isp_cfg_file[i], "r");
            if (!fp) {
                isp_puts("cfg file err.\n");
                return -ENFILE;
            }

            size = flen(fp);

            __this->scenes[i].buf = malloc(size);
            if (!__this->scenes[i].buf) {
                isp_puts("no mem.\n");
                return -ENOMEM;
            }
            __this->scenes[i].size = size;
            fread(fp, __this->scenes[i].buf, size);
            fclose(fp);
        }
    }

    return 0;
}

int flush_isp_cfg(struct server *server, u8 scene_type)
{
    struct ispt_customize_cfg cfg;
    union video_req req;

    if (!server || (scene_type > MAX_ISP_SCENES)) {
        return -EINVAL;
    }

    if (__this->scenes[scene_type].buf) {
        cfg.mode = ISP_CUSTOMIZE_MODE_FILE;
        cfg.data = __this->scenes[scene_type].buf;
        cfg.len = __this->scenes[scene_type].size;

        req.camera.cmd = SET_CUSTOMIZE_CFG;
        req.camera.customize_cfg = &cfg;
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
    }

    return 0;
}
int isp_cfg_judge_first(u32 lev)
{
    u32 i;
    u32 i_num = sizeof(isp_par) / sizeof(isp_par[0]);
    if (i_num >= 2) {
        for (i = 0; i < (i_num - 2); i++) {
            if (lev >= (isp_par[i].levh + isp_par[i + 1].levl) / 2  && lev <= (isp_par[i + 1].levh + isp_par[i + 2].levl) / 2) {
                //中间
                user_isp_cfg_printf("\n isp_cfg_judge_first mid %d\n", i + 1);
                return (i + 1);
            }
        }

        if (lev >= (isp_par[i_num - 2].levh + isp_par[i_num - 1].levl) / 2) {
            //最高
            user_isp_cfg_printf("\n isp_cfg_judge_first high %d\n", i_num - 1);
            return (i_num - 1);
        }
        if (lev <= (isp_par[0].levh + isp_par[1].levl) / 2) {
            //最低
            user_isp_cfg_printf("\n isp_cfg_judge_first low %d\n", 0);
            return 0;
        }
    } else {
        user_isp_cfg_printf("\n isp_cfg_judge_first err\n");
    }
    return 0;
}
int isp_cfg_gap(u32 lev, u32 i_pre)
{
    u32 i_gap;
    u32 i_num;
    i_num = sizeof(isp_par) / sizeof(isp_par[0]);
    for (i_gap = 0;  i_gap < (i_num - 1); i_gap++) {
        if (lev >= isp_par[i_gap].levh && lev <= isp_par[i_gap + 1].levl) {
            if (i_gap > i_pre || (i_gap + 1) < i_pre) {
                //非相邻段,重新配置
                user_isp_cfg_printf("\n isp_cfg_gap no adjacent %d \n", i_gap);
                return i_gap;
            }
        }
    }
    user_isp_cfg_printf("\n isp_cfg_gap adjacent %d \n", i_pre);
    return i_pre;//相邻段
}
int isp_cfg_judge(u32 lev)
{
    static u32 i_pre;
    u32 i_num = sizeof(isp_par) / sizeof(isp_par[0]);
    u32 i;
    for (i = 0; i < i_num; i++) {
        if (lev > isp_par[i].levl && lev < isp_par[i].levh) {
            i_pre = i;
            user_isp_cfg_printf("\n isp_cfg_judge mid %d\n", i);
            return i;
        }
    }
    if (lev >= isp_par[i_num - 1].levh) {
        i_pre = i_num - 1;
        user_isp_cfg_printf("\n isp_cfg_judge high %d\n", i - 1);
        return (i_num - 1);
    }
    if (lev <= isp_par[0].levl) {
        i_pre = 0;
        user_isp_cfg_printf("\n isp_cfg_judge low %d\n", 0);
        return 0;
    }
    i_pre = isp_cfg_gap(lev, i_pre);
    //printf("\ni_pre %d\n", i_pre);
    return i_pre;
}
extern int video_rec_set_wdr(struct server *server, u8 scene);

static u32 _isp_scene_set;
void set_isp_scr_cfg(u32 parm)
{
    _isp_scene_set = parm;
}

u32 get_isp_scr_cfg()
{
    return _isp_scene_set;
}
static void isp_scene_switch(void *arg, u8 force)
{

    int err;
    struct server *server = (struct server *)arg;
    union video_req req;
    int lv;
    u8 scene;
    u8 i;


    if (__this->state == USER_CFG_STATE_UNINIT) {
        req.camera.cmd = GET_CAMERA_LV;
        err = server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
        if (!err) {
            lv = req.camera.lv;
            __this->prev_lv = lv;
        }
    } else {
        __this->prev_lv = __this->cur_lv;
        req.camera.cmd = GET_CAMERA_LV;
        err = server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
        if (!err) {
            __this->cur_lv = req.camera.lv;
        }
    }

    scene = __this->current_scene;
    if (force) {
        i = isp_cfg_judge_first(__this->cur_lv);
    } else {
        i = isp_cfg_judge(__this->cur_lv);
        user_isp_cfg_printf("\n cur scene %d\n", scane_type[i]);
    }
    scene = scane_type[i];

    video_headlight_post_to_ui(scene);

    if (get_isp_scr_cfg()&BIT(31)) {
        scene = (get_isp_scr_cfg() & ~BIT(31));
        set_isp_scr_cfg(0);
    } else {
        user_isp_cfg_printf("%s() %d, scene : %d,current_scene : %d.\n", __FUNCTION__, __LINE__, scene, __this->current_scene);
        if ((!force)  && (scene == __this->current_scene)) {
            __this->state = USER_CFG_STATE_RUN;
            return;
        }
    }

    video_rec_set_wdr(server, i);
    user_isp_cfg_printf("\nscene %d\n", scene);
    __this->current_scene = scene;

    flush_isp_cfg(server, scene);

    isp_scr_work_hdl(scene);

    __this->state = USER_CFG_STATE_RUN;
}

static void isp_scene_switch_timer(void *arg)
{
    int err;

    err = os_sem_pend(&__this->sem, 10);
    if (err) {
        puts("isp scene switch timeout.\n");
        return;
    }

    isp_scene_switch(arg, 0);

    os_sem_post(&__this->sem);
}

int stop_update_isp_scenes(void)
{
    if (__this->timer) {
        os_sem_pend(&__this->sem, 0);
        sys_timer_del(__this->timer);
        __this->timer = 0;
    }

    if ((__this->state == USER_CFG_STATE_UNINIT) ||
        (__this->state == USER_CFG_STATE_INIT) ||
        (__this->state == USER_CFG_STATE_STOP)) {
        return 0;
    }

    //os_sem_del(&__this->sem, 0);

    __this->state = USER_CFG_STATE_STOP;

    return 0;
}

/*
 *开始使能更新isp效果配置
 *
 */
int start_update_isp_scenes(struct server *server)
{
    if (!server) {
        return -EINVAL;
    }

    if ((__this->state != USER_CFG_STATE_STOP) &&
        (__this->state != USER_CFG_STATE_INIT) &&
        (__this->state != USER_CFG_STATE_UNINIT)) {
        if (__this->server == server) {
            return 0;
        }
        stop_update_isp_scenes();
    }

    __this->server = server;

    os_sem_create(&__this->sem, 1);

    if (__this->state == USER_CFG_STATE_UNINIT) {
        user_isp_cfg_init();
        isp_scene_switch(server, 1);
    } else if (__this->state == USER_CFG_STATE_INIT) {
        isp_scene_switch(server, 1);
        isp_scene_switch(server, 0);
    } else {
        if (__this->mode == USER_ISP_MODE_SPECIAL) {
            isp_scene_switch(server, 1);
            __this->mode = USER_ISP_MODE_NORMAL;
        } else {
            isp_scene_switch(server, 0);
        }
    }

    __this->timer = sys_timer_add(server, isp_scene_switch_timer, 1000);
    return 0;
}

int video_rec_set_wdr(struct server *server, u8 scene)
{
    union video_req req;

    req.camera.mode = ISP_MODE_IMAGE_CAPTURE;
    if (wdr_en[scene]) { //不同场景，根据菜单wdr状态开启关闭wdr
        req.camera.drc = 0;//db_select("wdr");
        /* printf("===================== %d\n", scene); */
    } else {
        req.camera.drc = 0;
    }

    req.camera.cmd = SET_CAMERA_DRC_ENABLE ;

    if (server) {
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);
    } else {
        isp_puts("\nvrec set wdr fail\n");
        return -1;
    }

    return 0;
}

/*
 * 获取指定的配置文件
 */
int load_spec_camera_config(u32 index, void *arg)
{
    struct ispt_customize_cfg *cfg = (struct ispt_customize_cfg *)arg;
    u8 scene;

    /*
     *如果已经有效果，则不需要配置
     */
    /* if (__this->state != USER_CFG_STATE_UNINIT) { */
    /* return -EINVAL; */
    /* } */
    printf("load_spec_camera_config %d.\n", index);
    /*
     *效果配置初始化
     */
    user_isp_cfg_init();
    scene = scane_type[index];
    /*
     *获取当前效果文件
     */
    cfg->mode = ISP_CUSTOMIZE_MODE_FILE;
    cfg->data = __this->scenes[scene].buf;
    cfg->len = __this->scenes[scene].size;
    isp_scr_work_hdl(scene);

    __this->current_scene = scene;
    __this->state = USER_CFG_STATE_INIT;
    return 0;
}

/*
 *获取一个当前lv对应的配置文件(仅在第一次video服务使用)
 *
 */
int load_default_camera_config(u32 lv, void *arg)
{
    u8 i;
    u8 scene;

    struct ispt_customize_cfg *cfg = (struct ispt_customize_cfg *)arg;

    /*
     *如果已经有效果，则不需要配置
     */
    if (__this->state != USER_CFG_STATE_UNINIT) {
        return -EINVAL;
    }

    /*
     *效果配置初始化
     */
    user_isp_cfg_init();
    __this->cur_lv = lv;
    __this->prev_lv = lv;

    i = isp_cfg_judge_first(__this->cur_lv);

    scene = scane_type[i];

    /*
     *获取当前效果文件
     */
    cfg->mode = ISP_CUSTOMIZE_MODE_FILE;
    cfg->data = __this->scenes[scene].buf;
    cfg->len = __this->scenes[scene].size;
    isp_scr_work_hdl(scene);

    __this->current_scene = scene;
    __this->state = USER_CFG_STATE_INIT;
    return 0;
}

/*
 *发黄颜色矩阵
 */
static s16 ccm3x4_sepia[12] = {0.393 * 128, 0.769 * 128, 0.189 * 128, 0,
                               0.349 * 128, 0.686 * 128, 0.168 * 128, 0,
                               0.272 * 128, 0.534 * 128, 0.131 * 128, 0,
                              };

/*
 *特殊效果或特殊色彩设置
 */
int set_isp_special_effect(struct server *server, u8 type)
{
    struct ispt_customize_cfg cfg;
    union video_req req;

    if (!server) {
        return -EINVAL;
    }

    switch (type) {
    case SE_NORMAL:
        if (__this->mode == USER_ISP_MODE_NORMAL) {
            break;
        }
        /*
         *正常效果从配置文件刷新
         */
        flush_isp_cfg(server, __this->current_scene);
        start_update_isp_scenes(server);

        /*
         *设置为正常模式
         */
        __this->mode = USER_ISP_MODE_NORMAL;
        break;
    case SE_BNW: {
        u32 saturation = 0;

        /*
         *停止isp效果更新
         */
        stop_update_isp_scenes();

        if (__this->mode == USER_ISP_MODE_SPECIAL) {
            /*
             *特殊效果模式下重新恢复正常再配置特殊效果
             */
            flush_isp_cfg(server, __this->current_scene);
        }

        cfg.mode = ISP_CUSTOMIZE_MODE_SPECIAL;
        cfg.cmd =  ISP_SET_SATURATION;
        cfg.data = (u8 *)&saturation;
        cfg.len = sizeof(saturation);

        req.camera.cmd = SET_CUSTOMIZE_CFG;
        req.camera.customize_cfg = &cfg;
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);

        /*
         *设置为特殊模式
         */
        __this->mode = USER_ISP_MODE_SPECIAL;
    }
    break;
    case SE_RETRO: {
        stop_update_isp_scenes();

        if (__this->mode == USER_ISP_MODE_SPECIAL) {
            flush_isp_cfg(server, __this->current_scene);
        }

        cfg.mode = ISP_CUSTOMIZE_MODE_SPECIAL;
        cfg.cmd =  ISP_SET_CCM;
        cfg.data = (u8 *)ccm3x4_sepia;
        cfg.len = sizeof(ccm3x4_sepia);

        req.camera.cmd = SET_CUSTOMIZE_CFG;
        req.camera.customize_cfg = &cfg;
        server_request(server, VIDEO_REQ_CAMERA_EFFECT, &req);

        __this->mode = USER_ISP_MODE_SPECIAL;

    }
    break;
    default:
        break;
    }

    return 0;
}
