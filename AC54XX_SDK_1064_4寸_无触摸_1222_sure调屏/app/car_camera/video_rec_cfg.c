#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_engine_server.h"
#include "ui_ctr_def.h"
#include "video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"


#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "app_database.h"




extern struct video_rec_hdl rec_handler;

#define __this 	(&rec_handler)


u16 AVIN_WIDTH = 720;
u16 AVIN_HEIGH = 480;

u16 UVC_ENC_WIDTH = 640;
u16 UVC_ENC_HEIGH = 480;

static int res_set_function(u32 parm)
{
    return 0;
}
static int bu_set_function(u32 parm)
{
    return 0;
}
static int mic_set_function(u32 parm)
{
    return 0;
}

void mic_set_toggle()
{
    int mic_en = db_select("mic");

    db_update("mic", !mic_en);
}


static int gra_set_function(u32 parm)
{
#ifdef CONFIG_GSENSOR_ENABLE
    switch (parm) {
    case GRA_SEN_OFF:
        parm = G_SENSOR_CLOSE;
        break;
    case GRA_SEN_LO:
        parm = G_SENSOR_LOW;
        break;
    case GRA_SEN_MD:
        parm = G_SENSOR_MEDIUM;
        break;
    case GRA_SEN_HI:
        parm = G_SENSOR_HIGH;
        break;
    }
    set_gse_sensity(parm);
#endif

    return 0;
}

static int mot_set_function(u32 parm)
{
    return 0;
}

static int par_set_function(u32 parm)
{
    return 0;
}

static int wdr_set_function(u32 parm)
{
    return 0;
}

static int cyc_set_function(u32 parm)
{
    return 0;
}

static int set_car_num_part_a(u32 parm)
{
    memset(video_rec_osd_buf, 0, sizeof(video_rec_osd_buf));

    video_rec_osd_buf[0] = (u8)(parm >> 16);
    video_rec_osd_buf[1] = (u8)(parm >> 24);
    video_rec_osd_buf[2] = (u8)(parm >> 8);
    video_rec_osd_buf[3] = (u8)(parm >> 0);

    return 0;
}

static int set_car_num_part_b(u32 parm)
{
    video_rec_osd_buf[4] = (u8)(parm >> 24);
    video_rec_osd_buf[5] = (u8)(parm >> 16);
    video_rec_osd_buf[6] = (u8)(parm >> 8);
    video_rec_osd_buf[7] = (u8)(parm >> 0);

    if (db_select("dat")) {
        video_rec_osd_buf[8] = '\\';
        memcpy(video_rec_osd_buf + 9, osd_str_buf, strlen(osd_str_buf));
    }

    printf("set_car_num: %s\n", video_rec_osd_buf);

    return 0;
}

static int num_set_function(u32 parm)
{
    if (!parm) {
        memset(video_rec_osd_buf, 0, sizeof(video_rec_osd_buf));
        if (db_select("dat")) {
            memcpy(video_rec_osd_buf, osd_str_buf, strlen(osd_str_buf));
        }
    } else {
        set_car_num_part_a(db_select("cna"));
        set_car_num_part_b(db_select("cnb"));
    }
    printf("num_set_function: %d\n", parm);
    return 0;
}

static int exp_set_function(u32 parm)
{
  //  __this->exposure_set = 1;
   // video_rec_set_exposure(parm);

    return 0;
}

static int lab_set_function(u32 parm)
{
    int offset;

    if (!db_select("num")) {
        offset = 0;
    } else {
        offset = 8;
    }

    if (!parm) {
        video_rec_osd_buf[offset] = '\0';
    } else {
        if (offset) {
            video_rec_osd_buf[offset++] = '\\';
        }
        memcpy(video_rec_osd_buf + offset, osd_str_buf, strlen(osd_str_buf));
    }
    return 0;
}

static int two_set_function(u32 parm)
{
    return 0;
}

static int gap_set_function(u32 parm)
{
    return 0;
}


static int hlight_set_function(u32 parm)
{
    return 0;
}

static int lane_det_set_function(u32 parm)
{
    printf("__debug %s:%d\n", __FILE__, __LINE__);
    return 0;
}

void video_rec_fun_restore(u32 parm)
{
    if (get_default_setting_st()) {
        clear_default_setting_st();

        gra_set_function(db_select("gra"));
        ve_server_reopen();
    }
}

/*
 * 在此处添加所需配置即可
 */
static struct app_cfg cfg_table[] = {
    {"res", res_set_function },
    {"mic", mic_set_function },
    {"gra", gra_set_function },
    {"mot", mot_set_function },
    {"par", par_set_function },
    {"wdr", wdr_set_function },
    {"cyc", cyc_set_function },
    {"num", num_set_function },
    {"cna", set_car_num_part_a },
    {"cnb", set_car_num_part_b },
    {"exp", exp_set_function },
    {"dat", lab_set_function },
    {"two", two_set_function },
    {"gap", gap_set_function },
    {"hlw", hlight_set_function},
    {"lan", lane_det_set_function },
    {"bu", bu_set_function },
};




/*
 * 被请求设置参数
 */
int video_rec_set_config(struct intent *it)
{
    ASSERT(it != NULL);
    ASSERT(it->data != NULL);

    app_set_config(it, cfg_table, ARRAY_SIZE(cfg_table));

    return 0;
}

void video_rec_config_init()
{
    memset(video_rec_osd_buf, 0, sizeof(video_rec_osd_buf));

    if (db_select("num")) {
        set_car_num_part_a(db_select("cna"));
        set_car_num_part_b(db_select("cnb"));
    } else if (db_select("dat")) {
        memcpy(video_rec_osd_buf, osd_str_buf, strlen(osd_str_buf));
    }

    gra_set_function(db_select("gra"));
}








