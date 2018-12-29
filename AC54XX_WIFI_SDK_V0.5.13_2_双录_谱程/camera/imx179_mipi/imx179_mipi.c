#ifndef IMX179_MIPI_C
#define IMX179_MIPI_C


#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "imx179_mipi.h"
#include "asm/isp_alg.h"


static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};
static u8 cur_sensor_type = 0xff;

extern void *imx179_mipi_get_ae_params();
extern void *imx179_mipi_get_awb_params();
extern void *imx179_mipi_get_iq_params();
extern void imx179_mipi_ae_ev_init(u32 fps);

typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;

#define imx179_720p  0
#define imx179_1080p 1
#define imx179_1296p 2
#define imx179_xxxp  imx179_1296p
static Sensor_reg_ini IMX179_MIPI_INI_REG[] = {

#if (imx179_xxxp == imx179_720p)
    0x0100, 0x00,//mode_select
    0x0101, 0x00,//image_orientation
    0x0202, 0x09,//coarse_integratrion_time
    0x0203, 0xAD,
//pll
//extclk/prepllck * pll_mpy /vtpxck_div/vtpxck_div --> calc pixel clk
    0x0301, 0x05,//vtpxck_div[3:0]
    0x0303, 0x01,//visyck_div[1:0]
    0x0305, 0x06,//prepllck_div[3:0]
    0x0309, 0x0a,//oppxck_div[3:0]
    0x030B, 0x01,//opsyck_div[1:0]
    0x030C, 0x00,//pll_mpy[10:8]
    0x030D, 0xa0,//pll_mpy[7:0]

//height = 2481
    //width = 3440
    0x0340, (2481 >> 8 & 0xff), //height
    0x0341, (2481 & 0xff),
    0x0342, (3440 >> 8 & 0xff), //width
    0x0343, (3440 & 0xff),

    0x0344, 0x00, 	//xstart[11:8]
    0x0345, 0x00, 	//xstart[7:0]
    0x0346, (310 >> 8 & 0x0f), 	//ystart[11:8]
    0x0347, (310 & 0xff),	//ystart[7:0]

    0x0348, (3279 >> 8 & 0xff), 	//xend[11:8]
    0x0349, (3279 & 0xff), 		//xend[7:0]
    0x034A, (2110 >> 8 & 0xff),	//yend[11:8]
    0x034B, (2110 & 0xff),		//yend[7:0]

    0x034C, (1280 >> 8 & 0xff), 	//xout put size
    0x034D, (1280 & 0xff),
    0x034E, (720 >> 8 & 0xff), //yout put size
    0x034F, (720 & 0xff),

    0x0383, 0x01,//x_odd_inc
    0x0387, 0x01,//y_odd_inc
    0x0390, 0x00,//binning mode
    0x0401, 0x02,//scale_mode
    /* 0x0405, 0x1B,//scale_m 16~256 (3279.0/1940)*16= 27 = 0x1b */
    0x0405, 0x28,//scale_m 16~256 (3279.0/1280)*16= 40 = 0x28
    0x3020, 0x10,
    0x3041, 0x15,
    0x3042, 0x87,
    0x3089, 0x4F,
    0x3309, 0x9A,
    0x3344, 0x6f,
    0x3345, 0x1F,
    0x3362, 0x0A,
    0x3363, 0x0A,
    0x3364, 0x02,//lane 2
    0x3368, 0x18,//inck_freq[15:8]
    0x3369, 0x00,//inck_freq[7:0]
    /* 0x3370, 0x77, */
    /* 0x3371, 0x2F, */
    /* 0x3372, 0x4F, */
    /* 0x3373, 0x2F, */
    /* 0x3374, 0x2F, */
    /* 0x3375, 0x37, */
    /* 0x3376, 0x9F, */
    /* 0x3377, 0x37, */

    0x3370, 0x7F,//tclk_post
    0x3371, 0x37,//ths_prepare
    0x3372, 0x67,//ths_zero_min
    0x3373, 0x3F,//ths_trail
    0x3374, 0x3F,//tclk_trail_min
    0x3375, 0x47,//tclk_prepare
    0x3376, 0xCF,//tclk_zero
    0x3377, 0x47,//tlpx

    0x33C8, 0x00,//binning_cal 0average  1sum
    0x33D4, 0x0C,
    0x33D5, 0xD0,
    0x33D6, 0x09,
    0x33D7, 0xA0,
    0x4100, 0x0E,
    0x4108, 0x01,
    0x4109, 0x7C,

    0x0100, 0x01,//STREAM ON

#elif (imx179_xxxp == imx179_1080p)
    0x0100, 0x00,//mode_select
    0x0101, 0x00,//image_orientation
    0x0202, 0x09,//coarse_integratrion_time
    0x0203, 0xAD,
//pll
//extclk/prepllck * pll_mpy /vtpxck_div/vtpxck_div --> calc pixel clk
    0x0301, 0x05,//vtpxck_div[3:0]
    0x0303, 0x01,//visyck_div[1:0]
    0x0305, 0x06,//prepllck_div[3:0]
    0x0309, 0x0a,//oppxck_div[3:0]
    0x030B, 0x01,//opsyck_div[1:0]
    0x030C, 0x00,//pll_mpy[10:8]
    0x030D, 0xa0,//pll_mpy[7:0]


//height = 2481
//width = 3440
    0x0340, ((1660) >> 8 & 0xff), //height
    0x0341, ((1660) & 0xff),
    0x0342, ((3440 + 1800) >> 8 & 0xff), //width
    0x0343, ((3440 + 1800) & 0xff),

    0x0344, (204 >> 8 & 0x0f), 	//xstart[11:8]
    0x0345, (204 & 0xff), 	//xstart[7:0]
    0x0346, (446 >> 8 & 0x0f), 	//ystart[11:8]
    0x0347, (446 & 0xff),	//ystart[7:0]

    0x0348, ((2879 + 204) >> 8 & 0x0f), 	//xend[11:8]
    0x0349, ((2879 + 204) & 0xff), 		//xend[7:0]
    0x034A, ((1619 + 446) >> 8 & 0x0f),	//yend[11:8]
    0x034B, ((1619 + 446) & 0xff),		//yend[7:0]

    0x034C, (1920 >> 8 & 0xff), 	//xout put size
    0x034D, (1920 & 0xff),
    0x034E, (1080 >> 8 & 0xff), //yout put size
    0x034F, (1080 & 0xff),

    0x0383, 0x01,//x_odd_inc
    0x0387, 0x01,//y_odd_inc
    0x0390, 0x00,//binning mode
    0x0401, 0x02,//scale_mode
    0x0405, 0x17,//scale_m 16~256 (2880.0/1920)*16= 24 = 0x
    0x3020, 0x10,
    0x3041, 0x15,
    0x3042, 0x87,
    0x3089, 0x4F,
    0x3309, 0x9A,
    0x3344, 0x6f,
    0x3345, 0x1F,
    0x3362, 0x0A,
    0x3363, 0x0A,
    0x3364, 0x02,//lane 2
    0x3368, 0x18,//inck_freq[15:8]
    0x3369, 0x00,//inck_freq[7:0]

    0x3370, 0x7F,//tclk_post
    0x3371, 0x37,//ths_prepare
    0x3372, 0x67,//ths_zero_min
    0x3373, 0x3F,//ths_trail
    0x3374, 0x3F,//tclk_trail_min
    0x3375, 0x47,//tclk_prepare
    0x3376, 0xCF,//tclk_zero
    0x3377, 0x47,//tlpx

    0x33C8, 0x00,//binning_cal 0average  1sum
    0x33D4, 0x0C,
    0x33D5, 0xD0,
    0x33D6, 0x09,
    0x33D7, 0xA0,
    0x4100, 0x0E,
    0x4108, 0x01,
    0x4109, 0x7C,

    0x0100, 0x01,//STREAM ON
#else
    0x0100, 0x00,//mode_select
    0x0101, 0x00,//image_orientation
    0x0202, 0x09,//coarse_integratrion_time
    0x0203, 0xAD,
//pll
//extclk/prepllck * pll_mpy /vtpxck_div/vtpxck_div --> calc pixel clk
#if 1
    0x0301, 0x05,//vtpxck_div[3:0]
    0x0303, 0x01,//visyck_div[1:0]
    0x0305, 0x06,//prepllck_div[3:0]
    0x0309, 0x0a,//oppxck_div[3:0]
    0x030B, 0x01,//opsyck_div[1:0]
    0x030C, 0x00,//pll_mpy[10:8]
    0x030D, 0xdd,//pll_mpy[7:0]
#else
    0x0301, 0x05,//vtpxck_div[3:0]
    0x0303, 0x01,//visyck_div[1:0]
    0x0305, 0x06,//prepllck_div[3:0]
    0x0309, 0x0a,//oppxck_div[3:0]
    0x030B, 0x01,//opsyck_div[1:0]
    0x030C, 0x00,//pll_mpy[10:8]
    0x030D, 0xa0,//pll_mpy[7:0]
#endif

//height = 2481
//width = 3440
#if 1
    0x0340, ((1881) >> 8 & 0xff), //height
    0x0341, ((1881) & 0xff),
    0x0342, ((3440 + 2900) >> 8 & 0xff), //width
    0x0343, ((3440 + 2900) & 0xff),
#else
    0x0340, ((1660) >> 8 & 0xff), //height
    0x0341, ((1660) & 0xff),
    0x0342, ((3440 + 1800) >> 8 & 0xff), //width
    0x0343, ((3440 + 1800) & 0xff),
#endif
    0x0344, (((0) >> 8) & 0x0f), 	//xstart[11:8]
    0x0345, ((0) & 0xff), 	//xstart[7:0]
    0x0346, ((500) >> 8 & 0x0f), 	//ystart[11:8]
    0x0347, ((500) & 0xff),	//ystart[7:0]

    0x0348, ((3219) >> 8 & 0x0f), 	//xend[11:8]
    0x0349, ((3219) & 0xff), 		//xend[7:0]
    0x034A, ((1810 + 500) >> 8 & 0x0f),	//yend[11:8]
    0x034B, ((1810 + 500) & 0xff),		//yend[7:0]

    0x034C, (2304 >> 8 & 0xff), 	//xout put size
    0x034D, (2304 & 0xff),
    0x034E, (1296 >> 8 & 0xff), //yout put size
    0x034F, (1296 & 0xff),

    0x0383, 0x01,//x_odd_inc
    0x0387, 0x01,//y_odd_inc
    0x0390, 0x00,//binning mode
    0x0401, 0x02,//scale_mode
    /* 0x0405, 0x17,//scale_m 16~256 (3440.0/2304)*16= 24 = 0x17 */
    0x0405, 0x16,//scale_m 16~256 (3220.0/2304)*16= 22 = 0x16
    0x3020, 0x10,
    0x3041, 0x15,
    0x3042, 0x87,
    0x3089, 0x4F,
    0x3309, 0x9A,
    0x3344, 0x6f,
    0x3345, 0x1F,
    0x3362, 0x0A,
    0x3363, 0x0A,
    0x3364, 0x02,//lane 2
    0x3368, 0x18,//inck_freq[15:8]
    0x3369, 0x00,//inck_freq[7:0]

    0x3370, 0x7F,//tclk_post
    0x3371, 0x37,//ths_prepare
    0x3372, 0x67,//ths_zero_min
    0x3373, 0x3F,//ths_trail
    0x3374, 0x3F,//tclk_trail_min
    0x3375, 0x47,//tclk_prepare
    0x3376, 0xCF,//tclk_zero
    0x3377, 0x47,//tlpx

    0x33C8, 0x00,//binning_cal 0average  1sum
    0x33D4, 0x0C,
    0x33D5, 0xD0,
    0x33D6, 0x09,
    0x33D7, 0xA0,
    0x4100, 0x0E,
    0x4108, 0x01,
    0x4109, 0x7C,

    0x0100, 0x01,//STREAM ON

#endif

};


static void *iic = NULL;
/* #define WRCMD  0x20 */
/* #define RDCMD  0x21 */
unsigned char wrIMX179_MIPIReg(unsigned short regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT,	0x20)) {
        ret = 0;
        puts("wr err 0\n");
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        ret = 0;
        puts("wr err 1\n");
        goto __wend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, low)) {
        ret = 0;
        puts("wr err 2\n");
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        puts("wr err 3\n");
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

unsigned char rdIMX179_MIPIReg(unsigned short regID, unsigned char *regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x20)) {
        ret = 0;
        puts("rd err 0\n");
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        puts("rd err 1\n");
        ret = 0;
        goto __rend;
    }

    delay(10);

    /* if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, low)) { */
    if (dev_ioctl(iic, IIC_IOCTL_TX, low)) {
        ret = 0;
        puts("rd err 2\n");
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x21)) {
        ret = 0;
        puts("rd err 3\n");
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat)) {
        ret = 0;
        puts("rd err 4\n");
        goto __rend;
    }
__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}


/*************************************************************************************************
 sensor api
 *************************************************************************************************/
void IMX179_MIPI_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u16 dsc_addr;
    u8 pid;
    u8 value;

    for (i = 0; i < sizeof(IMX179_MIPI_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrIMX179_MIPIReg(IMX179_MIPI_INI_REG[i].addr, IMX179_MIPI_INI_REG[i].value);
    }
    *format = SEN_IN_FORMAT_RGGB;

    imx179_mipi_ae_ev_init(*frame_freq);
    return;
}

s32 IMX179_MIPI_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

s32 IMX179_MIPI_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void IMX179_MIPI_xclk_set(u8 isp_dev)
{

}

s32 IMX179_MIPI_ID_check(void)
{

    u8 id_hi;
    u8 id_lo;
    u32 i;
    for (i = 0; i < 5; i++) {

        rdIMX179_MIPIReg(0x0002, &id_hi);
        rdIMX179_MIPIReg(0x0003, &id_lo);

        if ((id_hi & 0x0f) == 0x01 && id_lo == 0x79) {
            printf("imx179 id_hi=0x%x\n", id_hi & 0x0f);
            printf("imx179 id_lo=0x%x\n", id_lo);
            puts("\n----hello IMX179_MIPI-----\n");
            return 0;
        }
    }
    puts("\n----not IMX179_MIPI-----\n");
    return -1;
}

void IMX179_MIPI_reset(u8 isp_dev)
{
    puts("C2392_MIPI reset \n");

    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }

    printf("gpio=%d\n", reset_gpio);
    gpio_direction_output(pwdn_gpio, 1);
    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    delay(40000);
}

s32 IMX179_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    printf("\n\n IMX179_MIPI_check reset pin :%d\n\n", reset_gpio);
    if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic1", 0);
        }
        if (!iic) {
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }
    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpios[isp_dev] = reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

    IMX179_MIPI_reset(isp_dev);

    if (0 != IMX179_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;
    return 0;
}


s32 IMX179_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n IMX179_MIPI_init \n\n");
    IMX179_MIPI_config_SENSOR(width, height, format, frame_freq);
    return 0;
}



static void set_again(u32 again)
{
    /* printf("gain = %x\n", again); */
    wrIMX179_MIPIReg(0x0104, 0x01);
    wrIMX179_MIPIReg(0x0204, again >> 8);
    wrIMX179_MIPIReg(0x0205, again & 0xff);
    wrIMX179_MIPIReg(0x0104, 0x00);
    return;
}

static void cal_texp_mck(u32 exp_time, u32 *texp, u32 *gain)
{
    u32 tmp2;
//line_time_sec  =  1/(30fps * frame_len)//frame_len  = 1660
//line_time_us  =  1/(30fps * frame_len)/(1000*1000)
//texp = exp_time/(line_time_us)
//texp = exp_time*30*1660/1000*1000
#if (imx179_xxxp == imx179_720p)
    *texp = exp_time * 100 / 2687; //*(30*1296)/(1000*1000*1);//
    if (*texp < 1) {
        *texp = 1;
    }
    tmp2 = (*texp) * 2687 / 100;
#elif (imx179_xxxp == imx179_1080p)
    *texp = exp_time * 100 / 2008;
    if (*texp < 1) {
        *texp = 1;
    }

    tmp2 = (*texp) * 2008 / 100;
#elif (imx179_xxxp == imx179_1296p)
    *texp = exp_time * 100 / 1772;
    if (*texp < 1) {
        *texp = 1;
    }

    tmp2 = (*texp) * 1772 / 100;
#endif

    *gain = exp_time * (1 << 10) / tmp2;
    if (*gain < 1024) {
        *gain = 1024;
    }
    return;
}


// Gain Index
#define MaxGainIndex (97)
static u16  sensorGainMapping[MaxGainIndex][2] = {
    { 64, 0  },
    { 68, 12 },
    { 71, 23 },
    { 74, 33 },
    { 77, 42 },
    { 81, 52 },
    { 84, 59 },
    { 87, 66 },
    { 90, 73 },
    { 93, 79 },
    { 96, 85 },
    { 100, 91 },
    { 103, 96 },
    { 106, 101},
    { 109, 105},
    { 113, 110},
    { 116, 114},
    { 120, 118},
    { 122, 121},
    { 125, 125},
    { 128, 128},
    { 132, 131},
    { 135, 134},
    { 138, 137},
    { 141, 139},
    { 144, 142},
    { 148, 145},
    { 151, 147},
    { 153, 149},
    { 157, 151},
    { 160, 153},
    { 164, 156},
    { 168, 158},
    { 169, 159},
    { 173, 161},
    { 176, 163},
    { 180, 165},
    { 182, 166},
    { 187, 168},
    { 189, 169},
    { 193, 171},
    { 196, 172},
    { 200, 174},
    { 203, 175},
    { 205, 176},
    { 208, 177},
    { 213, 179},
    { 216, 180},
    { 219, 181},
    { 222, 182},
    { 225, 183},
    { 228, 184},
    { 232, 185},
    { 235, 186},
    { 238, 187},
    { 241, 188},
    { 245, 189},
    { 249, 190},
    { 253, 191},
    { 256, 192},
    { 260, 193},
    { 265, 194},
    { 269, 195},
    { 274, 196},
    { 278, 197},
    { 283, 198},
    { 288, 199},
    { 293, 200},
    { 298, 201},
    { 304, 202},
    { 310, 203},
    { 315, 204},
    { 322, 205},
    { 328, 206},
    { 335, 207},
    { 342, 208},
    { 349, 209},
    { 357, 210},
    { 365, 211},
    { 373, 212},
    { 381, 213},
    { 400, 215},
    { 420, 217},
    { 432, 218},
    { 443, 219},
    { 468, 221},
    { 482, 222},
    { 497, 223},
    { 512, 224},
    { 529, 225},
    { 546, 226},
    { 566, 227},
    { 585, 228},
    { 607, 229},
    { 631, 230},
    { 656, 231},
    { 683, 232}
};
//q10
static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{



    u8 i;
    if (gain > 8 * 1024) {
        /* gain = 8 * 1024; */
    } else if (gain < 1024) {
        gain = 1024;
    }
#if 0
    gain = (gain * 64 >> 10);

    for (i = 0; i < (MaxGainIndex); i++) {
        if (gain <= sensorGainMapping[i][0]) {
            break;
        }
    }

    *_again = sensorGainMapping[i][1];
    *_dgain = (gain * (256 - again) / 256) >> 2 ;//gain为10位小数  imx179dgain为8位 小数
#else
    /* gain = (gain/1024);  */
    u32 again = 256 - ((256 << 10) / gain);

    if (again > 224) {
        again = 224;
    }
    *_again = again;
    *_dgain = (gain * (256 - again) / 256) >> 2 ;//gain为10位小数  imx179dgain为8位 小数
#endif

}

static void set_dgain(u32 dgain)
{
    wrIMX179_MIPIReg(0x0104, 0x01);
    //dgaing greeR
    wrIMX179_MIPIReg(0x020E, (dgain >> 8) & 0x0f);
    wrIMX179_MIPIReg(0x020F, dgain & 0xff); //
    //dgain red
    wrIMX179_MIPIReg(0x0210, (dgain >> 8) & 0x0f);
    wrIMX179_MIPIReg(0x0211, dgain & 0xff); //
    //dgain blue
    wrIMX179_MIPIReg(0x0212, (dgain >> 8) & 0x0f);
    wrIMX179_MIPIReg(0x0213, dgain & 0xff); //
    //dgain greeB
    wrIMX179_MIPIReg(0x0214, (dgain >> 8) & 0x0f);
    wrIMX179_MIPIReg(0x0215, dgain & 0xff); //

    wrIMX179_MIPIReg(0x0104, 0x00); //

}


static void set_shutter(u32 texp, u32 texp_mck)
{
    /* printf("texp = %x\n", texp); */
    wrIMX179_MIPIReg(0x0104, 0x01);
    wrIMX179_MIPIReg(0x0202, texp >> 8);
    wrIMX179_MIPIReg(0x0203, texp & 0xff); //
    wrIMX179_MIPIReg(0x0104, 0x00); //
}


u32 imx179_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_gain;

    cal_texp_mck(exp_time_us, &texp, &texp_gain);

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = gain * texp_gain >> 10;
    shutter->ae_exp_clk = 0;
    return 0;
}



u32 imx179_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;

    calc_gain(shutter->ae_gain, &again, &dgain);

    /* printf("shutter->ae_gain =0x%x, again 0x%x, dgain 0x%x\n",shutter->ae_gain, again, dgain );  */
    set_again(again);
    set_dgain(dgain);
    set_shutter(shutter->ae_exp_line, shutter->ae_exp_clk);

    return 0;
}


void IMX179_MIPI_sleep()
{

}

void IMX179_MIPI_wakeup()
{

}

void IMX179_MIPI_W_Reg(u16 addr, u16 val)
{
    wrIMX179_MIPIReg((u16) addr, (u8) val);
}

u16 IMX179_MIPI_R_Reg(u16 addr)
{
    u8 val;
    rdIMX179_MIPIReg((u16) addr, &val);
    return val;
}



REGISTER_CAMERA(IMX179_MIPI) = {
    .logo 				= 	"IMX179M",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_RGGB,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	30,

    .sen_size 			= 	{IMX179_MIPI_OUTPUT_W, IMX179_MIPI_OUTPUT_H},
    .isp_size 			= 	{IMX179_MIPI_OUTPUT_W, IMX179_MIPI_OUTPUT_H},

    .cap_fps         		= 	30,
    .sen_cap_size 			= 	{IMX179_MIPI_OUTPUT_W, IMX179_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{IMX179_MIPI_OUTPUT_W, IMX179_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	IMX179_MIPI_check,
        .init 		        = 	IMX179_MIPI_init,
        .set_size_fps 		=	IMX179_MIPI_set_output_size,
        .power_ctrl         =   IMX179_MIPI_power_ctl,

        .get_ae_params 	    =	imx179_mipi_get_ae_params,
        .get_awb_params 	=   imx179_mipi_get_awb_params,
        .get_iq_params 	    =	imx179_mipi_get_iq_params,

        .sleep 		        =	IMX179_MIPI_sleep,
        .wakeup 		    =	IMX179_MIPI_wakeup,
        .write_reg 		    =	IMX179_MIPI_W_Reg,
        .read_reg 		    =	IMX179_MIPI_R_Reg,

    }
};


#endif

