#ifndef C2392_MIPI_C
#define  C2392_MIPI_C


#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "c2392_mipi.h"
#include "asm/isp_alg.h"


static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};
static u8 cur_sensor_type = 0xff;

extern void *c2392_mipi_get_ae_params();
extern void *c2392_mipi_get_awb_params();
extern void *c2392_mipi_get_iq_params();
extern void c2392_mipi_ae_ev_init(u32 fps);

typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;


static Sensor_reg_ini C2392_MIPI_INI_REG[] = {

    //[Stream,0xoff]

    0x0309, 0x5e,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0100, 0x00, //streaming off
    0x0003, 0x00,
    0x0003, 0x00,

    //[INIT]

    0x0103, 0x01,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x3000, 0x80,
    0x3080, 0x01,
    0x3081, 0x14,
    0x3082, 0x01,
    0x3083, 0x4b,
    /* 0x3087,0xd0, */
    0x3087, 0x90,
    0x3089, 0x10,
    0x3180, 0x10,
    0x3182, 0x30,
    0x3183, 0x10,
    0x3184, 0x20,
    0x3185, 0xc0,
    0x3189, 0x50,
    0x3c03, 0x00,
    0x3f8c, 0x00,
    0x320f, 0x48,
    0x3023, 0x00,
    0x3d00, 0x33,
    0x3d05, 0x2c,
    0x3c9d, 0x01,
    0x3f08, 0x00,
    0x0309, 0x5e,
    0x0303, 0x01,
    0x0304, 0x01,
    0x0307, 0x56,
    0x3508, 0x00,
    0x3509, 0xcc,
    0x3292, 0x28,
    0x350a, 0x22,
    0x3209, 0x05,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    0x3209, 0x04,
    0x3108, 0xc9,
    0x3109, 0x7f,
    0x310a, 0x42,
    0x310b, 0x02,
    0x3112, 0x74,
    0x3113, 0x00,
    0x3114, 0xc0,
    0x3115, 0x80,
    0x3905, 0x01,
    0x3980, 0x01,
    /* 0x3980,0x81, */
    0x0101, 0x03, //mirror
    0x3881, 0x04,
    0x3882, 0x11, //lane num[5,4]
    0x328b, 0x03,
    0x328c, 0x00,
    0x3981, 0x57,
    0x3180, 0x10,
    0x3213, 0x00,
    0x3205, 0x40,
    0x3208, 0x8d,
    0x3210, 0x12,
    0x3211, 0x40,
    0x3212, 0x50,
    0x3215, 0xc0,
    0x3216, 0x70,
    0x3217, 0x08,
    0x3218, 0x20,
    0x321a, 0x80,
    0x321b, 0x00,
    0x321c, 0x1a,
    0x321e, 0x00,
    0x3223, 0x20,
    0x3224, 0x88,
    0x3225, 0x00,
    0x3226, 0x08,
    0x3227, 0x00,
    0x3228, 0x00,
    0x3229, 0x08,
    0x322a, 0x00,
    0x322b, 0x44,
    0x308a, 0x00,
    0x308b, 0x00,
    0x3280, 0x06,
    0x3281, 0x30,
    0x3282, 0x0f,
    0x3283, 0x51,
    0x3284, 0x0d,
    0x3285, 0x6a,
    0x3286, 0x3b,
    0x3287, 0xa0,
    0x3288, 0x00,
    0x3289, 0x00,
    0x328a, 0x08,
    0x328d, 0x01,
    0x328e, 0x20,
    0x328f, 0x0d,
    0x3290, 0x10,
    0x3291, 0x00,
    0x3292, 0x28,
    0x3293, 0x00,
    0x3216, 0x50,
    0x3217, 0x04,
    0x3205, 0x20,
    0x3215, 0x50,
    0x3223, 0x10,
    0x3280, 0x06,
    0x3282, 0x0f,
    0x3283, 0x50,
    0x308b, 0x0f,
    0x3184, 0x20,
    0x3185, 0xc0,
    0x3189, 0x50,
    0x3280, 0x86,
    0x0003, 0x00,
    0x3280, 0x06,
    0x0383, 0x01,
    0x0387, 0x01,
    0x0340, 0x04, //frame length Hi
    0x0341, 0x50, //frame length Lo
    0x0342, 0x08, //line length Hi
    0x0343, 0x1c, //line length Lo
    0x034c, 0x07, //x width Hi
    0x034d, 0x80, //x width Lo
    0x034e, 0x04, //y height Hi
    0x034f, 0x38, //y height Lo
    0x3b80, 0x42,
    0x3b81, 0x10,
    0x3b82, 0x10,
    0x3b83, 0x07,
    0x3b84, 0x07,
    0x3b85, 0x07,
    /* 0x3b83, 0x07, */
    /* 0x3b84, 0x04, */
    /* 0x3b85, 0x04, */

    0x3b86, 0x80,
    0x3021, 0x11,
    0x3022, 0x22,
    0x3209, 0x04,
    0x3584, 0x12,
    0x3805, 0x05, //hs trial
    0x3806, 0x03, //hs prepare
    0x3807, 0x03, //clk prepare
    0x3808, 0x0c, //clk zero
    0x3809, 0x64, //[7:4]hs zero [3:0]tlpx
    0x380a, 0x5b, //[7:4]clk trail [3:0]clk post
    0x380b, 0xe6, //[7:6]clk pre [5]bclko re [4:0]hs exit
    0x380c, 0x01, //add by lt
    0x3500, 0x10,
    0x308c, 0x20,
    0x308d, 0x31,
    0x3403, 0x00,
    0x3407, 0x01,
    0x3410, 0x04,
    0x3414, 0x01,
    0xe000, 0x32,
    0xe001, 0x87,
    0xe002, 0xa0,
    0xe030, 0x32,
    0xe031, 0x87,
    0xe032, 0xa0,
    0x3500, 0x00,
    0x3a87, 0x02,
    0x3a88, 0x08,
    0x3a89, 0x30,
    0x3a8a, 0x01,
    0x3a8b, 0x90,
    0x3a80, 0x88,
    0x3a81, 0x02,
    0x3009, 0x08,
    0x300b, 0x08,
    0x034b, 0x47,

    0x0202, 0x04, //shutter
    /* 0x0203,0x50, */
    0x0203, 0x40,
    0x0205, 0x30,
    //[Stream,0xon]

    0x0003, 0x00,
    0x0003, 0x00,
    0x0309, 0x56, //turn on pll
    0x0100, 0x01, //streaming on
    0x0003, 0x00,
    0x0003, 0x00,
    0x0003, 0x00,
    //test
    /* 0x3c00,0x04,///color bar on */

};


static void *iic = NULL;

unsigned char wrC2392_MIPIReg(unsigned short regID, unsigned char regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x6c)) {
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

unsigned char rdC2392_MIPIReg(unsigned short regID, unsigned char *regDat)
{
    u8 ret = 1;
    u8 high, low;

    high = regID >> 8;
    low = regID & 0xff;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x6c)) {
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

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, 0x6d)) {
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
void C2392_MIPI_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u16 dsc_addr;
    u8 pid;
    u8 value;

    /* printf("c2392 pid = %x\n", pid); */
    for (i = 0; i < sizeof(C2392_MIPI_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrC2392_MIPIReg(C2392_MIPI_INI_REG[i].addr, C2392_MIPI_INI_REG[i].value);
    }
    *format = SEN_IN_FORMAT_RGGB;

    c2392_mipi_ae_ev_init(*frame_freq);
    return;
}

s32 C2392_MIPI_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

s32 C2392_MIPI_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void C2392_MIPI_xclk_set(u8 isp_dev)
{

}

s32 C2392_MIPI_ID_check(void)
{

    u8 id_hi;
    u8 id_lo;
    u8 i;

    for (i = 0; i < 3; i++) {
        rdC2392_MIPIReg(0x0000, &id_hi);
        rdC2392_MIPIReg(0x0001, &id_lo);
        if ((id_hi == 0x02) && (id_lo == 0x03)) {
            puts("\n----hello C2392_MIPI-----\n");
            return 0;
        }
    }
    puts("\n----not C2392_MIPI-----\n");
    return -1;
}

void C2392_MIPI_reset(u8 isp_dev)
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
    gpio_direction_output(pwdn_gpio, 0);
    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    delay(40000);
}

s32 C2392_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    printf("\n\n C2392_MIPI_check reset pin :%d\n\n", reset_gpio);
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

    C2392_MIPI_reset(isp_dev);

    if (0 != C2392_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;
    return 0;
}


s32 C2392_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n C2392_MIPI_init \n\n");
    C2392_MIPI_config_SENSOR(width, height, format, frame_freq);
    return 0;
}



static void set_again(u32 again)
{
    /* printf("gain = %x\n", again); */
    wrC2392_MIPIReg(0x0205, again);
    return;
}

//exp_time us
//
static void cal_texp_mck(u32 exp_time, u32 *texp, u32 *gain)
{
    u32 tmp2;

    *texp = exp_time * 100 / 3017;
    tmp2 = (*texp) * 3017 / 100;

    *gain = exp_time * (1 << 10) / tmp2;

    return;
}

//q10
static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{
    if (gain > 16 * 1024) {
        gain = 16 * 1024;
    } else if (gain < 1024) {
        gain = 1024;
    }

    u32 temp_h = (gain / 1024 - 1) & 0xf;
    u32 temp_l = ((gain % 1024) >> 6) & 0xf;

    *_again = ((temp_h << 4) | temp_l);
    *_dgain = 0 ;

}

static void set_shutter(u32 texp, u32 texp_mck)
{
    /* printf("texp = %x\n", texp); */
    wrC2392_MIPIReg(0x0202, texp >> 8);
    wrC2392_MIPIReg(0x0203, texp & 0xff); //
}


u32 c2392_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_gain;

    cal_texp_mck(exp_time_us, &texp, &texp_gain);

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = gain * texp_gain >> 10;
    shutter->ae_exp_clk = 0;
    return 0;
}



u32 c2392_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;
    calc_gain(shutter->ae_gain, &again, &dgain);

    set_again(again);

    set_shutter(shutter->ae_exp_line, shutter->ae_exp_clk);

    return 0;
}


void C2392_MIPI_sleep()
{

}

void C2392_MIPI_wakeup()
{

}

void C2392_MIPI_W_Reg(u16 addr, u16 val)
{
    wrC2392_MIPIReg((u16) addr, (u8) val);
}

u16 C2392_MIPI_R_Reg(u16 addr)
{
    u8 val;
    rdC2392_MIPIReg((u16) addr, &val);
    return val;
}



REGISTER_CAMERA(C2392_MIPI) = {
    .logo 				= 	"C2392M",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_RGGB,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	30,

    .sen_size 			= 	{C2392_MIPI_OUTPUT_W, C2392_MIPI_OUTPUT_H},
    .isp_size 			= 	{C2392_MIPI_OUTPUT_W, C2392_MIPI_OUTPUT_H},

    .cap_fps         		= 	30,
    .sen_cap_size 			= 	{C2392_MIPI_OUTPUT_W, C2392_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{C2392_MIPI_OUTPUT_W, C2392_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	C2392_MIPI_check,
        .init 		        = 	C2392_MIPI_init,
        .set_size_fps 		=	C2392_MIPI_set_output_size,
        .power_ctrl         =   C2392_MIPI_power_ctl,

        .get_ae_params 	    =	c2392_mipi_get_ae_params,
        .get_awb_params 	=   c2392_mipi_get_awb_params,
        .get_iq_params 	    =	c2392_mipi_get_iq_params,

        .sleep 		        =	C2392_MIPI_sleep,
        .wakeup 		    =	C2392_MIPI_wakeup,
        .write_reg 		    =	C2392_MIPI_W_Reg,
        .read_reg 		    =	C2392_MIPI_R_Reg,

    }
};


#endif

