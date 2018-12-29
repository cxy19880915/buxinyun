

#include "sc3033_mipi_ae.h"
#include "sc3033_mipi.h"

#include "asm/isp_alg.h"


extern u32 sc3033_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain);
extern u32 sc3033_mipi_set_shutter(isp_ae_shutter_t *shutter);
extern void  *sc3033_mipi_get_ae_curve(u32 type, u32 fps, int *min_ev, int *max_ev);


//Anti-flicker:50Hz
const static u32 sc3033_mipi_ae_50hz_25fps[][AE_CURVE_INFO_MAX] = {

    // EV       ExpoTime    ISO     INTERPOLATE
    {9102000,     30,       100,    AE_INTERP_EXP},  // lv = 20
    {27306,    10000,       100,    AE_INTERP_GAIN}, // lv = 10.74
    {13653,    20000,       100,    AE_INTERP_GAIN},
    {6826,     40000,       100,    AE_INTERP_GAIN},
    {3413,     40000,       200,    AE_INTERP_GAIN},
    {1707,     40000,       400,    AE_INTERP_GAIN},
    {853,      40000,       800,    AE_INTERP_GAIN},
    {427,      40000,      1600,    AE_INTERP_GAIN},
    {213,      40000,      3200,    AE_INTERP_GAIN}, // lv = 3.73
    {0,        40000,      6400,    AE_INTERP_GAIN},
    {AE_CURVE_END, 0,         0,              0}
};

const static u32 sc3033_mipi_ae_50hz_30fps[][AE_CURVE_INFO_MAX] = {
    // EV       ExpoTime    ISO     INTERPOLATE
    {9102000,     30,       100,    AE_INTERP_EXP},  // lv = 20
    {27306,    10000,       100,    AE_INTERP_GAIN}, // lv = 10.74
    {13653,    20000,       100,    AE_INTERP_GAIN},
    {6826,     30000,       133,    AE_INTERP_GAIN},
    {3413,     30000,       266,    AE_INTERP_GAIN},
    {1707,     30000,       533,    AE_INTERP_GAIN},
    {853,      30000,      1066,    AE_INTERP_GAIN},
    {427,      30000,      2133,    AE_INTERP_GAIN},
    {213,      30000,      4266,    AE_INTERP_GAIN}, // lv = 3.73
    {0,        30000,      8533,    AE_INTERP_GAIN},
    {AE_CURVE_END, 0,         0,              0}
};

const static u32 sc3033_mipi_ae_60hz_30fps[][AE_CURVE_INFO_MAX] = {

    // EV       ExpoTime    ISO     INTERPOLATE
    {9102000,     30,       100,    AE_INTERP_EXP},  //lv = 20
    {32768,     8333,       100,    AE_INTERP_GAIN}, //lv=11
    {16348,    16666,       100,    AE_INTERP_GAIN},
    {8192,     33333,       100,    AE_INTERP_GAIN},
    {4096,     33333,       200,    AE_INTERP_GAIN},
    {2048,     33333,       400,    AE_INTERP_GAIN},
    {1024,     33333,       800,    AE_INTERP_GAIN},
    {512,      33333,      1600,    AE_INTERP_GAIN},
    {256,      33333,      3200,    AE_INTERP_GAIN}, //lv=4
    {0,        33333,      6400,    AE_INTERP_GAIN},
    {AE_CURVE_END, 0,         0,              0}
};

static u32 user_ev_value_table[] = {64, 81, 102, 128, 161, 203, 256}; //q7

static u8 ae_weight_cust[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};
static isp_ae_params_t  sc3033_mipi_ae_params = {

    .ae_max_ev = AE_MAX_EV,
    .ae_min_ev = AE_MIN_EV,
    .ae_init_ev = AE_INIT_EV,

    .ae_curve_type = AE_CURVE_TYPE,

    .ae_target = AE_TARGET_LUMA,

    .ae_phase_comp = AE_PHASE_COMP,

    .ae_conver_h = AE_CONVER_H,
    .ae_conver_l = AE_CONVER_L,
    .ae_diver_h = AE_DIVER_H,
    .ae_diver_l = AE_DIVER_L,

    .ae_ratio_max_h = AE_RATIO_MAX_H,
    .ae_ratio_max_l = AE_RATIO_MAX_L,

    .ae_ratio_max2_h = AE_RATIO_MAX2_H,
    .ae_ratio_max2_l = AE_RATIO_MAX2_L,

    .ae_ratio_slope = AE_RATIO_SLOPE,

    .ae_expline_th = AE_EXPLINE_TH,

    .ae_luma_smooth_num = AE_LUMA_SMOOTH_NUM,

    .ae_user_ev_value_table = user_ev_value_table,
    .ae_user_ev_value_table_size = sizeof(user_ev_value_table) / sizeof(u32),
    .ae_user_ev_value = 0, // centered to zero

    .ae_init_weight_type = AE_INIT_WEIGHT_TYPE,
    .ae_win_cust_weights = ae_weight_cust,

    .ae_ops = {
        .calc_shutter = sc3033_mipi_calc_shutter,
        .set_shutter = sc3033_mipi_set_shutter,
        .get_ae_curve = sc3033_mipi_get_ae_curve,
    },
};


void sc3033_mipi_ae_ev_init(u32 fps)
{
    u32 time, gain;
    int minev, maxev;
    static isp_ae_shutter_t ae_shutter;
    u32(*ae_curve)[AE_CURVE_INFO_MAX];

    ae_curve = sc3033_mipi_get_ae_curve(AE_CURVE_TYPE, fps, &minev, &maxev);
    isp_ae_curve_interp(ae_curve, AE_INIT_EV, &time, &gain);

    sc3033_mipi_calc_shutter(&ae_shutter, time, gain);

    sc3033_mipi_set_shutter(&ae_shutter);
}

void   *sc3033_mipi_get_ae_curve(u32 type, u32 fps, int *min_ev, int *max_ev)
{
    const void *curve;

    *min_ev = AE_MIN_EV;
    *max_ev = AE_MAX_EV;
    curve = sc3033_mipi_ae_50hz_25fps;

    if (type == AE_CURVE_50HZ) {
        *min_ev = AE_MIN_EV;
        *max_ev = AE_MAX_EV;

        if (fps == 25) {
            curve = sc3033_mipi_ae_50hz_25fps;
        } else if (fps == 30) {
            curve = sc3033_mipi_ae_50hz_30fps;
        }
    } else {
        *min_ev = AE_MIN_EV;
        *max_ev = AE_MAX_EV;

        curve = sc3033_mipi_ae_60hz_30fps;

    }

    return (void *)curve;
}


void *sc3033_mipi_get_ae_params()
{
    return (void *)&sc3033_mipi_ae_params;
}

