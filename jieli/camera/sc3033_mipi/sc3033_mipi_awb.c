
#include "asm/isp_alg.h"
#include "sc3033_mipi_awb.h"


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
    .prev_w = 8,
    .new_w = 1,

    .rgain_min = 256,
    .rgain_max = 3072,
    .bgain_min = 256,
    .bgain_max = 3072,
};

static u8 awb_zone_hct_desc[] = {
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
};

static u8 awb_zone_mct_desc[] = {
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
};

static u8 awb_zone_lct_desc[] = {
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,

};


static isp_awb_zone_t hct_zone = {
    .type = ISP_AWB_ZONE_HCT,

    .rg_min = 141, //0.55
    .bg_min = 172, //0.67

    .rg_max = 192, //0.75
    .bg_max = 230, //0.90

    //16x16 array
    .desc = &awb_zone_hct_desc,
};

static isp_awb_zone_t mct_zone = {
    .type = ISP_AWB_ZONE_MCT,

    .rg_min = 192, //0.75
    .bg_min = 141, //0.55

    .rg_max = 243, //0.95
    .bg_max = 172, //0.67

    //16x16 array
    .desc = &awb_zone_mct_desc,
};

static isp_awb_zone_t lct_zone = {
    .type = ISP_AWB_ZONE_LCT,

    .rg_min = 256, //1.0
    .bg_min = 128, //0.5

    .rg_max = 282, //1.1
    .bg_max = 141, //0.55

    //16x16 array
    .desc = &awb_zone_lct_desc,
};

static u8 awb_wp_weights[ISP_AWB_CT1_WP_WEIGHT_SIZE] = {16, 16, 16, 12, 11, 10, 9, 8};

static isp_awb_ct1_params_t awb_ct1_params = {
    .zones = {&hct_zone, &mct_zone, &lct_zone},
    .wp_weights = &awb_wp_weights,

    .y_min = 15,
    .y_max = 235,

    .hct_ev_th = (1 << 14) * 16,
    .lct_ev_th = (1 << 10) * 16,
    .prev_w = 8,
    .new_w = 1,

    .ev_th = 2048, // ev = 7
};

static isp_awb_params_t sc3033_mipi_awb_params = {
    .awb_alg_type = AWB_ALG_TYPE,
    .awb_scene_type = AWB_SCENE_TYPE,
    .awb_win_type = AWB_WIN_TYPE,

    .awb_init_gain =
    {.r_gain = ISP_AWB_ONE, .g_gain = ISP_AWB_ONE, .b_gain = ISP_AWB_ONE},

    .awb_scene_gains =
    {
        { .r_gain = 1445, .g_gain = ISP_AWB_ONE, .b_gain = 1355 },
        { .r_gain = 1549, .g_gain = ISP_AWB_ONE, .b_gain = 1225 },
        { .r_gain = 980, .g_gain = ISP_AWB_ONE, .b_gain = 1975 },
        { .r_gain = 1445, .g_gain = ISP_AWB_ONE, .b_gain = 1355 },
        { .r_gain = 1242, .g_gain = ISP_AWB_ONE, .b_gain = 1680 }
    },

    .awb_win_cust_weights = &awb_weight_cust,

    .awb_ct_weights = NULL,

    .awb_gw1_params = &awb_gw1_params,
    .awb_gw2_params = &awb_gw2_params,
    .awb_ct1_params = &awb_ct1_params,
    .awb_ct2_params = NULL,

};


void *sc3033_mipi_get_awb_params()
{
    return (void *)&sc3033_mipi_awb_params;
}
