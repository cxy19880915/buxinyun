

#include "asm/isp_alg.h"
#include "f22_mipi_awb.h"


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

/*
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

*/

static isp_awb_zone_t uhct_zone = {
    .type = ISP_AWB_ZONE_UHCT,

    .rg_min = (int)(0.52 * 256), //0.52
    .bg_min = (int)(0.83 * 256), //0.83

    .rg_max = (int)(0.72 * 256), //0.72
    .bg_max = (int)(1.00 * 256), //1.00

    .rg_center = (int)(0.66 * 256), //0.66
    .bg_center = (int)(0.82 * 256), //0.82

    .desc = awb_zone_uhct_desc,
};


static isp_awb_zone_t hct_zone = {
    .type = ISP_AWB_ZONE_HCT,

    .rg_min = (int)(0.58 * 256), //0.58
    .bg_min = (int)(0.50 * 256), //0.50

    .rg_max = (int)(0.80 * 256), //0.80
    .bg_max = (int)(0.82 * 256), //0.82

    .rg_center = (int)(0.71 * 256), //0.71
    .bg_center = (int)(0.76 * 256), //0.76

    .desc = awb_zone_hct_desc,
};

static isp_awb_zone_t mct_zone = {
    .type = ISP_AWB_ZONE_MCT,

    .rg_min = (int)(0.85 * 256), //0.85
    .bg_min = (int)(0.50 * 256), //0.50

    .rg_max = (int)(0.95 * 256), //0.95
    .bg_max = (int)(0.74 * 256), //0.74

    .rg_center = (int)(0.90 * 256), //0.90
    .bg_center = (int)(0.59 * 256), //0.59

    .desc = awb_zone_mct_desc,
};

static isp_awb_zone_t lct_zone = {
    .type = ISP_AWB_ZONE_LCT,

    .rg_min = (int)(1.07 * 256), //1.07
    .bg_min = (int)(0.47 * 256), //0.47

    .rg_max = (int)(1.25 * 256), //1.25
    .bg_max = (int)(0.60 * 256), //0.60

    .rg_center = (int)(1.17 * 256), //1.17
    .bg_center = (int)(0.54 * 256), //0.54

    .desc = awb_zone_lct_desc,
};




/*
static isp_awb_zone_t ulct_zone = {
    .type = ISP_AWB_ZONE_ULCT,

    .rg_min = (int)(1.15*256), //1.15
    .bg_min = (int)(0.49*256), //0.49

    .rg_max = (int)(1.25*256), //1.25
    .bg_max = (int)(0.59*256), //0.59

    .rg_center = (int)(1.17*256), //1.17
    .bg_center = (int)(0.54*256), //0.54

    .desc = &awb_zone_ulct_desc,
};

static isp_awb_zone_t green_zone = {
    .type = ISP_AWB_ZONE_GREEN,

    .rg_min = (int)(0.67*256), //0.5
    .bg_min = (int)(0.67*256), //0.4

    .rg_max = (int)(0.67*256), //0.85
    .bg_max = (int)(0.67*256), //0.7

    .desc = &awb_zone_green_desc,
};

static isp_awb_zone_t skin_zone = {
    .type = ISP_AWB_ZONE_SKIN,

    .rg_min = (int)(0.67*256), //1.0
    .bg_min = (int)(0.67*256), //0.6

    .rg_max = (int)(0.67*256), //1.2
    .bg_max = (int)(0.67*256), //1.0

    .desc = &awb_zone_skin_desc,
};
*/
static void F22_awb_post_process(u32 prob, u32 ev, u32 zone_type, u16 *r_gain, u16 *b_gain, void *data)
{
    //printf("%d,%d,%d,%d,%d\n", prob, ev, zone_type, *r_gain, *b_gain);
    *r_gain = ((u32) * r_gain) * 100 / 100;
    *b_gain = ((u32) * b_gain) * 100 / 100;

    if (prob <= 0 && (int)zone_type < 0 && ev > (1 << 16) * 16) {
        *r_gain = 256 * 1024 / hct_zone.rg_center;
        *b_gain = 256 * 1024 / hct_zone.bg_center;
    }
#if 0
    if ((zone_type == 0 || zone_type == 1)) {
        *r_gain = ((u32) * r_gain) * 105 / 100;
        *b_gain = ((u32) * b_gain) * 105 / 100;
    } else if ((zone_type == 3 || zone_type == 4)) {
        *r_gain = ((u32) * r_gain) * 105 / 100;
        *b_gain = ((u32) * b_gain) * 105 / 100;
    } else if (zone_type == 2) {
        *r_gain = ((u32) * r_gain) * 105 / 100;
        *b_gain = ((u32) * b_gain) * 105 / 100;
    }
#endif
}

static isp_awb_ct2_params_t awb_ct2_params = {
    .awb_post_process = F22_awb_post_process,
    .zones = {&uhct_zone, &hct_zone, &mct_zone, &lct_zone, NULL, NULL, NULL},

    .y_min = 10,
    .y_max = 200,

    .prev_w = 5,
    .new_w = 1,

    .refine_radius = 10,

    //base 1000
    .subwin_weight_ratio_th = 300,
    .zone_weight_ratio_th = 50,
    .green_weight_ratio_th = 50,
    .gw_weight_ratio_th = 50,

    .daylight_ev_th = (1 << 15) * 16,
    .outdoor_ev_th = (1 << 13) * 16,
    .lowlight_ev_th = (1 << 6) * 16,

    .fallback_gw = 1,
    .inference = 1,
    .inference_d = 1,
    .inference_g = 0,
    .refine = 1,

    .skylight_high_ev_th = (1 << 16) * 16,
    .skylight_low_ev_th = (1 << 15) * 16,
};



static isp_awb_params_t F22_mipi_awb_params = {
    .awb_alg_type = AWB_ALG_TYPE,
    .awb_scene_type = AWB_SCENE_TYPE,
    .awb_win_type = AWB_WIN_TYPE,

    .awb_init_gain =
    {.r_gain = ISP_AWB_ONE, .g_gain = ISP_AWB_ONE, .b_gain = ISP_AWB_ONE},

    .awb_scene_gains =
    {
        { .r_gain = 1596, .g_gain = ISP_AWB_ONE, .b_gain = 1529 },
        { .r_gain = 1596, .g_gain = ISP_AWB_ONE, .b_gain = 1341 },
        { .r_gain =  971, .g_gain = ISP_AWB_ONE, .b_gain = 2265 },
        { .r_gain = 1596, .g_gain = ISP_AWB_ONE, .b_gain = 1341 },
        { .r_gain = 1240, .g_gain = ISP_AWB_ONE, .b_gain = 1838 }
    },

    .awb_win_cust_weights = awb_weight_cust,

    .awb_ct_weights = NULL,

    .awb_gw1_params = &awb_gw1_params,
    .awb_gw2_params = &awb_gw2_params,
    .awb_ct1_params = NULL,
    .awb_ct2_params = &awb_ct2_params,

};



void *F22_mipi_get_awb_params()
{
    return (void *)&F22_mipi_awb_params;
}

