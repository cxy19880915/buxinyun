
#include "asm/isp_alg.h"
#include "imx179_mipi_awb.h"


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

static u8 awb_zone_green_desc[] = {
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

    .rg_min = 153, //0.6
    .bg_min = 205, //0.8

    .rg_max = 205, //0.8
    .bg_max = 256, //1

    //16x16 array
    .desc = awb_zone_hct_desc,
};

static isp_awb_zone_t mct_zone = {
    .type = ISP_AWB_ZONE_MCT,

    .rg_min = 205, //0.8
    .bg_min = 166, //0.65

    .rg_max = 243, //0.95
    .bg_max = 205, //0.8

    //16x16 array
    .desc = awb_zone_mct_desc,
};

static isp_awb_zone_t lct_zone = {
    .type = ISP_AWB_ZONE_LCT,

    .rg_min = 253, //0.95
    .bg_min = 128, //0.5

    .rg_max = 333, //1.3
    .bg_max = 166, //0.65

    //16x16 array
    .desc = awb_zone_lct_desc,
};

static u8 awb_wp_weights[ISP_AWB_CT1_WP_WEIGHT_SIZE] = {16, 16, 16, 12, 11, 10, 9, 8};

static isp_awb_ct1_params_t awb_ct1_params = {
    .zones = {&hct_zone, &mct_zone, &lct_zone},
    .wp_weights = awb_wp_weights,

    .y_min = 30,
    .y_max = 235,

    .hct_ev_th = (1 << 14) * 16,
    .lct_ev_th = (1 << 10) * 16,
    .prev_w = 3,
    .new_w = 1,

    .ev_th = 1024, // ev = 6
};

static isp_awb_params_t imx179_mipi_awb_params = {
    .awb_alg_type = AWB_ALG_TYPE,
    .awb_scene_type = AWB_SCENE_TYPE,
    .awb_win_type = AWB_WIN_TYPE,

    .awb_init_gain =
    {.r_gain = ISP_AWB_ONE, .g_gain = ISP_AWB_ONE, .b_gain = ISP_AWB_ONE},

    .awb_scene_gains =
    {
        { .r_gain = 1850, .g_gain = ISP_AWB_ONE, .b_gain = 1850 },
        { .r_gain = 2100, .g_gain = ISP_AWB_ONE, .b_gain = 1650 },
        { .r_gain = 1250, .g_gain = ISP_AWB_ONE, .b_gain = 3200 },
        { .r_gain = 2100, .g_gain = ISP_AWB_ONE, .b_gain = 1650 },
        { .r_gain = 1600, .g_gain = ISP_AWB_ONE, .b_gain = 2500 },

    },

    .awb_win_cust_weights = awb_weight_cust,

    .awb_ct_weights = NULL,

    .awb_gw1_params = &awb_gw1_params,
    .awb_gw2_params = &awb_gw2_params,
    .awb_ct1_params = &awb_ct1_params,
    .awb_ct2_params = NULL,

};


void *imx179_mipi_get_awb_params()
{
    return (void *)&imx179_mipi_awb_params;
}
