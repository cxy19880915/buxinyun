#ifndef HM2131_MIPI_C
#define  HM2131_MIPI_C


#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "hm2131_mipi.h"
#include "asm/isp_alg.h"


#define FRAME_LENGTH_PCK        0x500  //1280

#define FRAME_LENGTH_LINES_30FPS      0x452  //1106
#define MAX_EXP_LINES_30FPS           (FRAME_LENGTH_LINES_30FPS - 2)
#define FRAME_LENGTH_LINES_25FPS      0x52f  //1327
#define MAX_EXP_LINES_25FPS           (FRAME_LENGTH_LINES_25FPS - 2)
#define FRAME_LENGTH_LINES_20FPS      0x67b  //1659
#define MAX_EXP_LINES_20FPS           (FRAME_LENGTH_LINES_20FPS - 2)
#define FRAME_LENGTH_LINES_15FPS      0x8a4  //2212
#define MAX_EXP_LINES_15FPS           (FRAME_LENGTH_LINES_15FPS - 2)

static u32 cur_frame_length = -1;
static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_exp = -1;

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};
static u8 cur_sensor_type = 0xff;

extern void *hm2131_mipi_get_ae_params();
extern void *hm2131_mipi_get_awb_params();
extern void *hm2131_mipi_get_iq_params();
extern void hm2131_mipi_ae_ev_init(u32 fps);


typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;




static Sensor_reg_ini HM2131_MIPI_INI_REG[] = {
    0x0103, 0x00, //soft reset
    0x0304, 0x2A,
    0x0305, 0x0C,
    0x0307, 0x55,
    0x0303, 0x04,
    0x0309, 0x00,
    0x030A, 0x0A,
    0x030D, 0x02,
    0x030F, 0x14,
    0x5268, 0x01,
    0x5264, 0x24,
    0x5265, 0x92,
    0x5266, 0x23,
    0x5267, 0x07,
    0x5269, 0x02,
    0x0100, 0x02,
    0x0100, 0x02,
    0x0111, 0x01,
    0x0112, 0x0A,
    0x0113, 0x0A,
    0x4B20, 0xCE,
    0x4B18, 0x12,
    0x4B02, 0x05,
    0x4B43, 0x07,
    0x4B05, 0x1C,
    0x4B0E, 0x00,
    0x4B0F, 0x0D,
    0x4B06, 0x06,
    0x4B39, 0x0B,
    0x4B42, 0x07,
    0x4B03, 0x0C,
    0x4B04, 0x07,
    0x4B3A, 0x0B,
    0x4B51, 0x80,
    0x4B52, 0x09,
    0x4B52, 0xC9,
    0x4B57, 0x07,
    0x4B68, 0x6B,
    0x0350, 0x37,
    0x5030, 0x10,
    0x5032, 0x02,
    0x5033, 0xD1,
    0x5034, 0x01,
    0x5035, 0x67,
    0x5229, 0x90,
    0x5061, 0x00,
    0x5062, 0x94,
    0x50F5, 0x06,
    0x5230, 0x00,
    0x526C, 0x00,
    0x520B, 0x41,
    0x5254, 0x08,
    0x522B, 0x00,
    0x4144, 0x08,
    0x4148, 0x03,
    0x4024, 0x40,
    0x4B66, 0x00,
    0x4B31, 0x06,
    0x0202, 0x04,
    0x0203, 0x50,
    0x0340, 0x04,
    0x0341, 0x52,
    0x0342, 0x05,
    0x0343, 0x00,
    0x034C, 0x07,
    0x034D, 0x88,
    0x034E, 0x04,
    0x034F, 0x40,
    0x0101, 0x00,
    0x4020, 0x10,
    0x50DD, 0x01,
    0x0350, 0x37,
    0x4131, 0x01,
    0x4132, 0x20,
    0x5011, 0x00,
    0x5015, 0x00,
    0x501D, 0x1C,
    0x501E, 0x00,
    0x501F, 0x20,
    0x50D5, 0xF0,
    0x50D7, 0x12,
    0x50BB, 0x14,
    0x5040, 0x07,
    0x50B7, 0x00,
    0x50B8, 0x10,
    0x50B9, 0xFF,
    0x50BA, 0xFF,
    0x5200, 0x26,
    0x5201, 0x00,
    0x5202, 0x00,
    0x5203, 0x00,
    0x5217, 0x01,
    0x5219, 0x01,
    0x5234, 0x01,
    0x526B, 0x03,
    0x4C00, 0x00,
    0x0310, 0x00,
    0x4B31, 0x06,
    0x4B3B, 0x02,
    0x4B44, 0x0C,
    0x4B45, 0x01,
    0x50A1, 0x00,
    0x50AA, 0x2E,
    0x50AC, 0x44,
    0x50AB, 0x04,
    0x50A0, 0xB0,
    0x50A2, 0x1B,
    0x50AF, 0x00,
    0x5208, 0x55,
    0x5209, 0x03,
    0x520D, 0x40,
    0x5214, 0x18,
    0x5215, 0x03,
    0x5216, 0x00,
    0x521A, 0x10,
    0x521B, 0x24,
    0x5232, 0x04,
    0x5233, 0x03,
    0x5106, 0xF0,
    0x510E, 0xC1,
    0x5166, 0xF0,
    0x516E, 0xC1,
    0x5196, 0xF0,
    0x519E, 0xC1,
    0x51C0, 0x80,
    0x51C4, 0x80,
    0x51C8, 0x80,
    0x51CC, 0x80,
    0x51D0, 0x80,
    0x51D4, 0x80,
    0x51D8, 0x80,
    0x51DC, 0x80,
    0x51C1, 0x03,
    0x51C5, 0x13,
    0x51C9, 0x17,
    0x51CD, 0x27,
    0x51D1, 0x27,
    0x51D5, 0x2B,
    0x51D9, 0x2B,
    0x51DD, 0x2B,
    0x51C2, 0x4B,
    0x51C6, 0x4B,
    0x51CA, 0x4B,
    0x51CE, 0x49,
    0x51D2, 0x49,
    0x51D6, 0x49,
    0x51DA, 0x49,
    0x51DE, 0x49,
    0x51C3, 0x10,
    0x51C7, 0x18,
    0x51CB, 0x10,
    0x51CF, 0x08,
    0x51D3, 0x08,
    0x51D7, 0x08,
    0x51DB, 0x08,
    0x51DF, 0x00,
    0x51E0, 0x94,
    0x51E2, 0x94,
    0x51E4, 0x94,
    0x51E6, 0x94,
    0x51E1, 0x00,
    0x51E3, 0x00,
    0x51E5, 0x00,
    0x51E7, 0x00,
    0x5264, 0x23,
    0x5265, 0x07,
    0x5266, 0x24,
    0x5267, 0x92,
    0x5268, 0x01,
    0xBAA2, 0xC0,
    0xBAA2, 0x40,
    0xBA90, 0x01,
    0xBA93, 0x02,
    0x3110, 0x0B,
    0x373E, 0x8A,
    0x373F, 0x8A,
    0x3701, 0x05,
    0x3709, 0x05,
    0x3703, 0x04,
    0x370B, 0x04,
    0x3713, 0x00,
    0x3717, 0x00,
    0x5043, 0x01,
    0x5040, 0x05,
    0x5044, 0x07,
    0x6000, 0x0F,
    0x6001, 0xFF,
    0x6002, 0x1F,
    0x6003, 0xFF,
    0x6004, 0xC2,
    0x6005, 0x00,
    0x6006, 0x00,
    0x6007, 0x00,
    0x6008, 0x00,
    0x6009, 0x00,
    0x600A, 0x00,
    0x600B, 0x00,
    0x600C, 0x00,
    0x600D, 0x20,
    0x600E, 0x00,
    0x600F, 0xA1,
    0x6010, 0x01,
    0x6011, 0x00,
    0x6012, 0x06,
    0x6013, 0x00,
    0x6014, 0x0B,
    0x6015, 0x00,
    0x6016, 0x14,
    0x6017, 0x00,
    0x6018, 0x25,
    0x6019, 0x00,
    0x601A, 0x43,
    0x601B, 0x00,
    0x601C, 0x82,
    0x0000, 0x00,
    0x0104, 0x01,
    0x0104, 0x00,
    0x0100, 0x03,
    0x0100, 0x03,
};


static void *iic = NULL;

unsigned char wrHM2131_MIPIReg(unsigned short regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x68)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        ret = 0;
        goto __wend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, low)) {
        ret = 0;
        goto __wend;
    }
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

unsigned char rdHM2131_MIPIReg(unsigned short regID, unsigned char *regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x68)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, high)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, low)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x69)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat)) {
        ret = 0;
        goto __rend;
    }
__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}


/*************************************************************************************************
 sensor api
 *************************************************************************************************/
void HM2131_MIPI_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u16 dsc_addr;
    u8 pid;
    u8 value;

    /* printf("hm2131 pid = %x\n", pid); */
    for (i = 0; i < sizeof(HM2131_MIPI_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrHM2131_MIPIReg(HM2131_MIPI_INI_REG[i].addr, HM2131_MIPI_INI_REG[i].value);
    }
    *format = SEN_IN_FORMAT_RGGB;
    return;
}

s32 HM2131_MIPI_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

s32 HM2131_MIPI_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void HM2131_MIPI_xclk_set(u8 isp_dev)
{

}

s32 HM2131_MIPI_ID_check(void)
{

    u8 id_hi;
    u8 id_lo;
    u8 i;

    for (i = 0; i < 3; i++) {
        rdHM2131_MIPIReg(0x0000, &id_hi);
        rdHM2131_MIPIReg(0x0001, &id_lo);
        printf("hm2131 id = %2x, %2x\n", id_hi, id_lo);
        if ((id_hi == 0x21) && (id_lo == 0x31)) {
            puts("\n----hello HM2131_MIPI-----\n");
            return 0;
        }
    }
    puts("\n----not HM2131_MIPI-----\n");
    return -1;
}

void HM2131_MIPI_reset(u8 isp_dev)
{
    puts("HM2131_MIPI reset \n");

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
    gpio_direction_output(pwdn_gpio, 0);
    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(pwdn_gpio, 1);
    gpio_direction_output(reset_gpio, 1);
    delay(40000);
}

s32 HM2131_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    printf("\n\n HM2131_MIPI_check reset pin :%d\n\n", reset_gpio);
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

    HM2131_MIPI_reset(isp_dev);

    if (0 != HM2131_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;
    return 0;
}


s32 HM2131_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n HM2131_MIPI_init \n\n");

    HM2131_MIPI_config_SENSOR(width, height, format, frame_freq);

    cur_frame_length = -1;
    cur_again = -1;
    cur_dgain = -1;
    cur_exp = -1;


    return 0;
}
s32 hm2131_mipi_get_mipi_clk(u32 *mipi_clk,  u32 *tval_hstt, u32 *tval_stto, u16 width, u16 height, u8 frame_freq)
{
    *mipi_clk = 480;
    *tval_hstt = 0;
    *tval_stto = 30;
    return 0;
}



static void set_again(u32 again)
{

    if (cur_again == again) {
        return;
    }
    cur_again = again;
    wrHM2131_MIPIReg(0x0205, again);
    return;
}
static void set_dgain(u32 dgain)
{
    if (cur_dgain == dgain) {
        return;
    }
    cur_dgain = dgain;
    wrHM2131_MIPIReg(0x020e, dgain >> 6);
    wrHM2131_MIPIReg(0x020f, (dgain & 0x3f) << 2);
    return;
}

//exp_time us
//
static void cal_texp_mck(u32 exp_time, u32 *texp, u32 *gain)
{
    u32 tmp2;


    *texp = exp_time * 1000 / 30118;

    if (*texp < 2) {
        *texp = 2;
    }

#if HM2131_VARIABLE_FPS
    if (*texp >= MAX_EXP_LINES_15FPS) {
        *texp = MAX_EXP_LINES_15FPS;
    }
#else
    if (*texp >= MAX_EXP_LINES_30FPS) {
        *texp = MAX_EXP_LINES_30FPS;
    }
#endif

    tmp2 = (*texp) * 30118 / 1000;

    *gain = exp_time * (1 << 10) / tmp2;
    if (*gain < 1024) {
        *gain = 1024;
    }

    return;
}

//q10
static void calc_gain(u32 gain, u8 *_again, u16 *_dgain)
{
    u32 temp_h;
    u32 temp_l;

    if (gain > 16 * 1024) {
        gain = 16 * 1024;
    } else if (gain < 1024) {
        gain = 1024;
    }

    if (gain >= 8 * 1024) {
        temp_h = 3;
    } else if (gain >= 4 * 1024) {
        temp_h = 2;
    } else if (gain >= 2 * 1024) {
        temp_h = 1;
    } else {
        temp_h = 0;
    }

    temp_l = (gain / (1 << temp_h) - 1024) >> 6;

    *_again = ((temp_h << 4) | temp_l);
    *_dgain = (gain / ((1 << temp_h) * (16 + temp_l))) ;
    if (*_dgain > 255) {
        *_dgain = 255;
    }

}

static void set_shutter(u32 texp, u32 texp_mck)
{
    u32 frame_length = FRAME_LENGTH_LINES_30FPS;
    if (cur_exp == texp) {
        return;
    }

#if HM2131_VARIABLE_FPS
    if (texp > MAX_EXP_LINES_20FPS) {
        frame_length = FRAME_LENGTH_LINES_15FPS;
    } else if (texp > MAX_EXP_LINES_25FPS) {
        frame_length = FRAME_LENGTH_LINES_20FPS;
    } else if (texp > MAX_EXP_LINES_30FPS) {
        frame_length = FRAME_LENGTH_LINES_25FPS;
    } else {
        frame_length = FRAME_LENGTH_LINES_30FPS;
    }

    if (frame_length != cur_frame_length) {

        cur_frame_length = frame_length;
        wrHM2131_MIPIReg(0x0340, frame_length >> 8);
        wrHM2131_MIPIReg(0x0341, frame_length & 0xff); //
    }

#endif
    cur_exp = texp;

    wrHM2131_MIPIReg(0x0202, texp >> 8);
    wrHM2131_MIPIReg(0x0203, texp & 0xff); //
}


u32 hm2131_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_gain;
    u32 new_gain;

    cal_texp_mck(exp_time_us, &texp, &texp_gain);
    new_gain = gain * texp_gain >> 10;

    if (new_gain > 16 * 1024) {
        new_gain = 16 * 1024;
    } else if (new_gain < 1024) {
        new_gain = 1024;
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = new_gain;
    shutter->ae_exp_clk = 0;
    return 0;
}

u32 hm2131_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;

    calc_gain(shutter->ae_gain, &again, &dgain);
    set_shutter(shutter->ae_exp_line, 0);
    set_again(again);
    set_dgain(dgain);

    wrHM2131_MIPIReg(0x0104, 0x01);
    wrHM2131_MIPIReg(0x0104, 0x00);

    return 0;
}


void HM2131_MIPI_sleep()
{

}

void HM2131_MIPI_wakeup()
{

}

void HM2131_MIPI_W_Reg(u16 addr, u16 val)
{
    wrHM2131_MIPIReg((u16) addr, (u8) val);
}

u16 HM2131_MIPI_R_Reg(u16 addr)
{
    u8 val;
    rdHM2131_MIPIReg((u16) addr, &val);
    return val;
}



REGISTER_CAMERA(HM2131_MIPI) = {
    .logo 				= 	"HM2131M",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	30,

    .sen_size 			= 	{HM2131_MIPI_OUTPUT_W, HM2131_MIPI_OUTPUT_H},
    .isp_size 			= 	{HM2131_MIPI_OUTPUT_W, HM2131_MIPI_OUTPUT_H},

    .cap_fps         		= 	30,
    .sen_cap_size 			= 	{HM2131_MIPI_OUTPUT_W, HM2131_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{HM2131_MIPI_OUTPUT_W, HM2131_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	HM2131_MIPI_check,
        .init 		        = 	HM2131_MIPI_init,
        .set_size_fps 		=	HM2131_MIPI_set_output_size,
        .power_ctrl         =   HM2131_MIPI_power_ctl,
        .get_mipi_clk       =   hm2131_mipi_get_mipi_clk,

        .get_ae_params 	    =	hm2131_mipi_get_ae_params,
        .get_awb_params 	=   hm2131_mipi_get_awb_params,
        .get_iq_params 	    =	hm2131_mipi_get_iq_params,

        .sleep 		        =	HM2131_MIPI_sleep,
        .wakeup 		    =	HM2131_MIPI_wakeup,
        .write_reg 		    =	HM2131_MIPI_W_Reg,
        .read_reg 		    =	HM2131_MIPI_R_Reg,

    }
};


#endif

