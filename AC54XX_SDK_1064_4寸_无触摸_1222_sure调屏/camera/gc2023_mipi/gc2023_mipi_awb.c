
#include "asm/isp_alg.h"
#include "gc2023_mipi_awb.h"


static u8 awb_weight_cust[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};


static isp_awb_gw1_params_t awb_gw1_params = {
    .wp_th = 50,
    .r_th = 235,
    .g_th = 235,
    .b_th = 235,

    .wp_ratio_th_numerator = 1,
    .wp_ratio_th_denominator = 32,

    .rgain_min = 256,
    .rgain_max = 3072,
    .bgain_min = 256,
    .bgain_max = 3072,

    .ev_th = 2048,
};

static isp_awb_gw2_params_t awb_gw2_params = {
    .prev_w = 3,
    .new_w = 1,

    .rgain_min = 256,
    .rgain_max = 3072,
    .bgain_min = 256,
    .bgain_max = 3072,
};
static u8 awb_zone_uhct_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static u8 awb_zone_hct_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static u8 awb_zone_mct_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static u8 awb_zone_lct_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static u8 awb_zone_ulct_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static u8 awb_zone_green_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static u8 awb_zone_skin_desc[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

static isp_awb_zone_t uhct_zone = {
    .type = ISP_AWB_ZONE_UHCT,

    .rg_min = 153, //0.6
    .bg_min = 256, //1.00

    .rg_max = 205, //0.8
    .bg_max = 358, //1.4

    .rg_center = 174, //0.68
    .bg_center = 282, //1.1

    .desc = &awb_zone_uhct_desc,
};


static isp_awb_zone_t hct_zone = {
    .type = ISP_AWB_ZONE_HCT,

    .rg_min = 153, //0.60
    .bg_min = 179, //0.70

    .rg_max = 238, //0.93
    .bg_max = 256, //1.00

    .rg_center = 205, //0.8
    .bg_center = 220, //0.86

    .desc = &awb_zone_hct_desc,
};

static isp_awb_zone_t mct_zone = {
    .type = ISP_AWB_ZONE_MCT,

    .rg_min = 205, //0.8
    .bg_min = 166, //0.65

    .rg_max = 282, //1.1
    .bg_max = 179, //0.7

    .rg_center = 251, //0.98
    .bg_center = 161, //0.63

    .desc = &awb_zone_mct_desc,
};

static isp_awb_zone_t lct_zone = {
    .type = ISP_AWB_ZONE_LCT,

    .rg_min = 282, //1.1
    .bg_min = 102, //0.4

    .rg_max = 353, //1.38
    .bg_max = 166, //0.65

    .rg_center = 332, //1.295
    .bg_center = 128, //0.5

    .desc = &awb_zone_lct_desc,
};

static isp_awb_zone_t ulct_zone = {
    .type = ISP_AWB_ZONE_ULCT,

    .rg_min = 353, //1.38
    .bg_min = 102, //0.4

    .rg_max = 410, //1.6
    .bg_max = 166, //0.65

    .rg_center = 369, //1.44
    .bg_center = 125, //0.49

    .desc = &awb_zone_ulct_desc,
};

static isp_awb_zone_t green_zone = {
    .type = ISP_AWB_ZONE_GREEN,

    .rg_min = 128, //0.5
    .bg_min = 102, //0.4

    .rg_max = 218, //0.85
    .bg_max = 179, //0.7

    .desc = &awb_zone_green_desc,
};

static isp_awb_zone_t skin_zone = {
    .type = ISP_AWB_ZONE_SKIN,

    .rg_min = 256, //1.0
    .bg_min = 154, //0.6

    .rg_max = 307, //1.2
    .bg_max = 256, //1.0

    .desc = &awb_zone_skin_desc,
};

static isp_awb_ct2_params_t awb_ct2_params = {
    .zones = {&uhct_zone, &hct_zone, &mct_zone, &lct_zone, &ulct_zone, &green_zone, &skin_zone},

    .y_min = 30,
    .y_max = 235,

    .prev_w = 3,
    .new_w = 1,

    .refine_radius = 20,

    //base 1000
    .subwin_weight_ratio_th = 20,
    .zone_weight_ratio_th = 10,
    .green_weight_ratio_th = 50,
    .gw_weight_ratio_th = 50,

    .daylight_ev_th = (1 << 14) * 16,
    .outdoor_ev_th = (1 << 12) * 16,
    .lowlight_ev_th = (1 << 6) * 16,

    .fallback_gw = 1,
    .inference = 1,
    .inference_d = 1,
    .inference_g = 1,
    .refine = 1,
};

static isp_awb_params_t gc2023_mipi_awb_params = {
    .awb_alg_type = AWB_ALG_TYPE,
    .awb_scene_type = AWB_SCENE_TYPE,
    .awb_win_type = AWB_WIN_TYPE,

    .awb_init_gain =
    {.r_gain = ISP_AWB_ONE, .g_gain = ISP_AWB_ONE, .b_gain = ISP_AWB_ONE},

    .awb_scene_gains =
    {
        { .r_gain = 1138, .g_gain = ISP_AWB_ONE, .b_gain = 1264 },
        { .r_gain = 1327, .g_gain = ISP_AWB_ONE, .b_gain = 1100 },
        { .r_gain =  768, .g_gain = ISP_AWB_ONE, .b_gain = 2104 },
        { .r_gain = 1327, .g_gain = ISP_AWB_ONE, .b_gain = 1100 },
        { .r_gain = 1012, .g_gain = ISP_AWB_ONE, .b_gain = 1682 }
    },

    .awb_win_cust_weights = &awb_weight_cust,

    .awb_ct_weights = NULL,

    .awb_gw1_params = &awb_gw1_params,
    .awb_gw2_params = &awb_gw2_params,
    .awb_ct1_params = NULL,
    .awb_ct2_params = &awb_ct2_params,

};


void *gc2023_mipi_get_awb_params()
{
    return (void *)&gc2023_mipi_awb_params;
}

