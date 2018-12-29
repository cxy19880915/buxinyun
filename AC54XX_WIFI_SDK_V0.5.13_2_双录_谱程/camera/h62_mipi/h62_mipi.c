#ifndef H62_MIPI_C
#define  H62_MIPI_C


#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "h62_mipi.h"
#include "asm/isp_alg.h"

#define VBLANK_30FPS    0x27 // 0x25
#define VBLANK_25FPS    0xfe
#define VBLANK_20FPS    0x250
#define VBLANK_16FPS    0x3f5

#define FRAME_W  1600// 2200
#define FRAME_H  900// 1125
#define H62_FPS  25// 30//25


#define INPUT_CLK       24
static u8  H62_WRCMD =      0x60;
static u8  H62_RDCMD =      0x61;




static u32 cur_expline = -1;
static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expclk = -1;
static u32 cur_reg_idx = -1;
static u32 cur_vblank = -1;
static u32 next_reg_idx = -1;



static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

static u8 cur_sensor_type = 0xff;


extern void *h62_mipi_get_ae_params();
extern void *h62_mipi_get_awb_params();
extern void *h62_mipi_get_iq_params();
extern void h62_mipi_ae_ev_init(u32 fps);

typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_ini;

struct stVrefh {
    u16 index;
    u16 th_low;
    u16 th_high;
    u8 vrefh;
};
#define EXTERNAL_LDO_1V5 1

//如果系统VDDIO使用3.3V请定义此宏定义值为1
//如果系统VDDIO使用1.8V请定义此宏定义值为0
#define	VDDIO_VOLTAGE_3V3 0
#define DSC_ADDR_BASE 0x0400

static Sensor_reg_ini H62_MIPI_INI_REG[] = {

    {0x12, 0x40},
    {0x0E, 0x11},
    {0x0F, 0x09},
    {0x10, 0x1E},
    {0x11, 0x80},
    {0x19, 0x68},




#if 0
#if 1 //25fp
    {0x20, 0x00},
    {0x21, 0x09},
#else //30fp

    {0x20, 0x80},
    {0x21, 0x07},
#endif

    {0x22, 0xEE},
    {0x23, 0x02},
#endif



#if 0       // 20 fps
    {0x20, 0x40},
    {0x21, 0x06},
    {0x22, 0x6B},
    {0x23, 0x04},
#elif 1     //25fps
    {0x20, 0x40},
    {0x21, 0x06},
    {0x22, 0x84},
    {0x23, 0x03},
#endif
    //win out size
    {0x24, 0x04},
    {0x25, 0xD4},
    {0x26, 0x25},

    //start offset
#if SENSOR_FLIP
    {0x27, 0x19},
    {0x28, 0x12},
#else
    {0x27, 0x10},
    {0x28, 0x15},
#endif
    {0x29, 0x02},
    {0x2A, 0x01},
    {0x2B, 0x21},
    {0x2C, 0x08},
    {0x2D, 0x01},
    {0x2E, 0xBB},
    {0x2F, 0xC0},
    {0x41, 0x88},
    {0x42, 0x12},
    {0x39, 0x90},
    {0x1D, 0x00},
    {0x1E, 0x04},
    {0x7A, 0x4C},
    {0x70, 0x49},
    {0x71, 0x2A},
    {0x72, 0x48},
    {0x73, 0x33},
    {0x74, 0x52},
    {0x75, 0x2B},
    {0x76, 0x40},
    {0x77, 0x06},
    {0x78, 0x18},
    {0x66, 0x38},
    {0x1F, 0x20},
    {0x30, 0x90},
    {0x31, 0x0C},
    {0x32, 0xFF},
    {0x33, 0x0C},
    {0x34, 0x4B},
    {0x35, 0xA3},
    {0x36, 0x06},
    {0x38, 0x40},
    {0x3A, 0x08},
    {0x56, 0x02},
    {0x60, 0x01},
    {0x0D, 0x50},
    {0x57, 0x80},
    {0x58, 0x33},
    {0x5A, 0x04},
    {0x5B, 0xB6},
    {0x5C, 0x08},
    {0x5D, 0x67},
    {0x5E, 0x04},
    {0x5F, 0x08},
    {0x66, 0x28},
    {0x67, 0xF8},
    {0x68, 0x00},
    {0x69, 0x74},
    {0x6A, 0x1F},
    {0x63, 0x82},
    {0x6C, 0xC0},
    {0x6E, 0x5C},
    {0x82, 0x01},
    {0x0C, 0x00},
    {0x46, 0xC2},
    {0x48, 0x7E},
    {0x62, 0x40},
    {0x7D, 0x57},
    {0x7E, 0x28},
    {0x80, 0x00},
    {0x4A, 0x05},
    {0x49, 0x10},
    {0x13, 0x81},
    {0x59, 0x97},

#if 0//ENSOR_FLIP
    {0x12, 0x30},
#else
    {0x12, 0x00},
#endif

    {0x47, 0x47},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},
    {0x0B, 0x62},


    {0x47, 0x44},
    {0x1F, 0x21},
    {0xFF, 0xFF}
};


static void *iic = NULL;

unsigned char wrH62_MIPIReg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H62_WRCMD)) {
        ret = 0;
        //  puts("1 err\n");
        goto __wend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        //   puts("2 err\n");

        goto __wend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        //    puts("3 err\n");

        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
}

unsigned char rdH62_MIPIReg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H62_WRCMD)) {
        ret = 0;
        //puts("4 \n");

        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        //puts("5\n");

        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H62_RDCMD)) {
        ret = 0;
        // puts("6\n");

        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat)) {
        //puts("7\n");

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
void H62_MIPI_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u16 dsc_addr;
    u8 pid;

    /* rdH62_MIPIReg(0x0045, &pid); */
    /* pid = pid & 0x3f; */

    printf("h62 pid = %x\n", pid);

    for (i = 0; i < sizeof(H62_MIPI_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrH62_MIPIReg(H62_MIPI_INI_REG[i].addr, H62_MIPI_INI_REG[i].value);
    }


    /* *format = SEN_IN_FORMAT_GRBG; */

    h62_mipi_ae_ev_init(*frame_freq);


    /* wrH62_MIPIReg(0x001d, 0x02); */

    return;
}

s32 H62_MIPI_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

s32 H62_MIPI_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void H62_MIPI_xclk_set(u8 isp_dev)
{

}

s32 H62_MIPI_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i;
    if (!iic) {
        iic = dev_open("iic0", 0);
        if (!iic) {
            return -1;
        }

    }
    for (i = 0; i < 3; i++) {
        rdH62_MIPIReg(0x0a, &pid);
        rdH62_MIPIReg(0x0b, &ver);
        puts("Sensor ID \n");
        put_u8hex(pid);
        put_u8hex(ver);
        puts("\n");
        //printf("ADD = %x\n", H62_WRCMD);
        /* H62_WRCMD=0x60+i*4; */
        /* H62_RDCMD =H62_WRCMD+1;  */
    }


    if (pid != 0xa0 || ver != 0x62) {
        puts("\n----notH62_MIPI-----\n");
        return -1;
    }
    puts("\n----helloH62_MIPI-----\n");
    return 0;
}

void H62_MIPI_reset(u8 isp_dev)
{
    puts("H62_MIPI reset \n");

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

s32 H62_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    printf("\n\nH62_MIPI_check reset pin :%d\n\n", reset_gpio);
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

    H62_MIPI_reset(isp_dev);

    if (0 != H62_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;


    return 0;
}


s32 H62_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\nH62_MIPI_init \n\n");

    cur_expline = -1;
    cur_again = -1;
    cur_dgain = -1;
    cur_expclk = -1;
    cur_reg_idx = -1;
    cur_vblank = -1;
    next_reg_idx = -1;

    H62_MIPI_config_SENSOR(width, height, format, frame_freq);
    return 0;
}



static void set_again(u32 again)
{
    if (cur_again == again) {
        return;
    }
    cur_again = again;
#if 0
    wrH62_MIPIReg(0x00, again);
#else
    wrH62_MIPIReg(0xC4, 0x00);
    wrH62_MIPIReg(0xC5, again);

    wrH62_MIPIReg(0x1F, 0x80);   //group write enable
#endif
    return;
}

static void set_dgain(u32 dgain)
{
    if (cur_dgain == dgain) {
        return;
    }
    cur_dgain = dgain;

    /* wrH62_MIPIReg(0x00bc, dgain >> 8); */
    /* wrH62_MIPIReg(0x00bd, dgain & 0xff); */
}

//q10
static void calc_gain(u32 gain, u32 *again, u32 *dgain)
{
    u16 temp = 0;
    u8 pga_h = 0;
    u8 pga_l = 0;
    u8 i = 0;
    temp = (gain) >> 10;
    if (temp == 0) {
        *dgain = 0;
        *again =  pga_h << 4 | pga_l;
        return ;
    }
    if (temp >= 128 * 2) {
        pga_h = 7;
        pga_l = 16 - 1;
        *dgain = 0;
        *again =  pga_h << 4 | pga_l;
        return;
    }

    for (i = 7; i >= 0; i--) {
        if (temp & BIT(i)) {
            pga_h = i;
            pga_l = (gain - BIT(i) * 1024) / BIT(i) / 64;
            *dgain = 0;
            *again =  pga_h << 4 | pga_l;
            /* printf("isp gain =%d, add=%d ,pag_h=%d ,pag_l=%d  \n",gain,BIT(i)*1024+BIT(i)* pga_l*64, BIT(i),pga_l);  */
            return ;
        }
    }

}





static void set_shutter(u32 texp, u32 texp_mck)
{
    if (cur_expline == texp) {
        return ;
    }
    /* printf("texp %d \n",texp ); */
    if (texp > FRAME_H - 1) {
        texp = FRAME_H - 1;
    }
#if 0
    wrH62_MIPIReg(0x01, texp & 0xff);

    wrH62_MIPIReg(0x02, (texp >> 8) & 0xff);
#else//使用预加载功能

    wrH62_MIPIReg(0xC0, 0x01);
    wrH62_MIPIReg(0xC1, texp & 0xff);

    wrH62_MIPIReg(0xC2, 0x02);
    wrH62_MIPIReg(0xC3, (texp >> 8) & 0xff);

    wrH62_MIPIReg(0x1F, 0x80);   //group write enable

#endif
    cur_expline = texp;

}


u32 h62_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp =  0;
    u32 texp_align = 0;
    u32 ratio = 0;
    texp = exp_time_us * FRAME_H * H62_FPS / 1000000;
    texp_align = (texp) * 1000 * 1000 / (FRAME_H * H62_FPS);
    if (texp_align < exp_time_us) {
        ratio = (exp_time_us) * (1 << 10) / texp_align;

    } else {
        ratio = (1 << 10);
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = (gain * ratio) >> 10;
    shutter->ae_exp_clk = 0;
    return 0;

}



u32 h62_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;
    calc_gain(shutter->ae_gain, &again, &dgain);

    set_again(again);
    set_dgain(dgain);

    set_shutter(shutter->ae_exp_line, shutter->ae_exp_clk);

    return 0;
}


void H62_MIPI_sleep()
{

}

void H62_MIPI_wakeup()
{

}

void H62_MIPI_W_Reg(u16 addr, u16 val)
{
    wrH62_MIPIReg((u8) addr, (u8) val);
}

u16 H62_MIPI_R_Reg(u16 addr)
{
    u8 val;
    rdH62_MIPIReg((u8) addr, &val);
    return val;
}



REGISTER_CAMERA(H62_MIPI) = {
    .logo 				= 	"H62M",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			=   SEN_IN_FORMAT_BGGR,  //	SEN_IN_FORMAT_GBRG,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_1_LANE,
    .fps         		= 	H62_FPS, // 30,

    .sen_size 			= 	{H62_MIPI_OUTPUT_W, H62_MIPI_OUTPUT_H},
    .isp_size 			= 	{H62_MIPI_OUTPUT_W, H62_MIPI_OUTPUT_H},

    .cap_fps         		= 	H62_FPS,
    .sen_cap_size 			= 	{H62_MIPI_OUTPUT_W, H62_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{H62_MIPI_OUTPUT_W, H62_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	H62_MIPI_check,
        .init 		        = 	H62_MIPI_init,
        .set_size_fps 		=	H62_MIPI_set_output_size,
        .power_ctrl         =  H62_MIPI_power_ctl,

        .get_ae_params 	    =	h62_mipi_get_ae_params,
        .get_awb_params 	=	h62_mipi_get_awb_params,
        .get_iq_params 	    =	h62_mipi_get_iq_params,

        .sleep 		        =	H62_MIPI_sleep,
        .wakeup 		    =	H62_MIPI_wakeup,
        .write_reg 		    =	H62_MIPI_W_Reg,
        .read_reg 		    =	H62_MIPI_R_Reg,
    }
};


#endif


