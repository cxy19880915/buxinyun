
/*******************************************************************************************
   reis 201508
*******************************************************************************************/
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "h42.h"
#include "asm/isp_alg.h"

#define H42_IIC_R_PORT 0x61
#define H42_IIC_W_PORT 0x60

extern void *h42_get_ae_params();
extern void *h42_get_awb_params();
extern void *h42_get_iq_params();

extern void h42_ae_ev_init(u32 fps);

static s32 H42_set_output_size(u16 *width, u16 *height, u8 *frame_freq);

static u32 reset_gpio[2] = {-1, -1};

// ===============================
// default config
// ===============================
#define H42_MCLK                       24
#define H42_PCLK_DIV2_MINUS1           0x9  //  PLL2, PLL Clock divider
#define H42_PCLK_DIV1_MINUS1           0x1  // PLL_pre_Ratio
#define H42_PCLK_MUL                   0x1E // PLL3, VCO mulitplier

#define H42_PCLK                       ((H42_MCLK*H42_PCLK_MUL/(1+H42_PCLK_DIV1_MINUS1))/(H42_PCLK_DIV2_MINUS1+1))
#define H42_CLK_PER_ROW                0x640//0x640
#define H42_ROW_TIME_US                (H42_CLK_PER_ROW / H42_PCLK)   //us
#define H42_GET_SHUTTER(exp_time)      (exp_time*(H42_PCLK))/(H42_CLK_PER_ROW)

#define H42_DEFAULT_FPS_FACTOR          0x2EE
// ===============================
// ===============================

typedef struct {
    u8 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini H42_INI_REG[] = {
//[JXH42(CM)_1280x720x30_Fx3_DVP_10b.reg]
//;;INI Start
    {0x12, 0x40},
//;;DVP Setting
//{0x0C,0x01}, // test pattern
    {0x0D, 0x40},
    {0x1F, 0x04}, //{0x1F,0x06},//{0x1F,0x04},
//;;PLL Setting
    {0x0E, 0x1c | H42_PCLK_DIV1_MINUS1},
    {0x0F, H42_PCLK_DIV2_MINUS1},
    {0x10, H42_PCLK_MUL},
    {0x11, 0x80},
//;;Frame/Window
    {0x20, H42_CLK_PER_ROW & 0xff},
    {0x21, H42_CLK_PER_ROW >> 8},
    {0x22, H42_DEFAULT_FPS_FACTOR & 0xff}, //降帧率，则提高FrameHeightTime
    {0x23, H42_DEFAULT_FPS_FACTOR >> 8},
    {0x24, H42_OUTPUT_W & 0xff},
    {0x25, H42_OUTPUT_H & 0xff},
    {0x26, ((H42_OUTPUT_H & 0xf00) >> 4) | ((H42_OUTPUT_W >> 8) & 0x0f)},
//{0x24,0x00},
//{0x25,0xD0},
//{0x26,0x25},
    {0x27, 0x45}, //{0x27,0x34},//{0x27,0x45},
    {0x28, 0x0D},
    {0x29, 0x01},
    {0x2A, 0x24},
    {0x2B, 0x29},
    {0x2C, 0x00},
    {0x2D, 0x00},
    {0x2E, 0xB9},
    {0x2F, 0x00},
//;;Sensor Timing
    {0x30, 0x92},
    {0x31, 0x0A},
    {0x32, 0xAA},
    {0x33, 0x14},
    {0x34, 0x38},
    {0x35, 0x54},
    {0x42, 0x41},
    {0x43, 0x50},
//;;Interface
    {0x1D, 0xFF},
    {0x1E, 0x1F},
    {0x6C, 0x90},
    {0x73, 0xB3},
    {0x70, 0x68},
    {0x76, 0x40},
    {0x77, 0x06},
//;;Array/AnADC/PWC
    {0x48, 0x40},
    {0x60, 0xA4},
    {0x61, 0xFF},
    {0x62, 0x40},
    {0x65, 0x00},
    {0x66, 0x20},
    {0x67, 0x30},
    {0x68, 0x04},
    {0x69, 0x74},
    {0x6F, 0x04},
//;0x;Black Sun
    {0x63, 0x19},
    {0x6A, 0x09},
//;;AE/AG/ABLC
    {0x13, 0x87}, // 0:
    {0x14, 0x80},
    {0x16, 0xC0},
    {0x17, 0x40},
    {0x18, 0xC2},
    {0x19, 0x20}, //added to avoid gain & shutter aysnc
    {0x38, 0x35},
    {0x39, 0x98},
    {0x4a, 0x03},
    {0x49, 0x00}, //{0x49,0x10}, //OB
//;;INI End
    {0x12, 0x00},
//;;PWDN Setting
};

/*S_IIC_FUN H42_iic_fun;*/
/*static u8 H42_iic_sel;*/
static void *iic = NULL;

//static void my_delay(u32 clocks)
//{
//    return;
//    delay(clocks);
//}

unsigned char wrH42Reg(unsigned char regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H42_IIC_W_PORT)) {
        ret = 0;
        goto __wend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        goto __wend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }
__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;
#if 0
    if (H42_iic_sel) {
        iic_tx(H42_IIC_W_PORT);
        delay(10);
        iic_tx(regID);
        delay(10);
        iic_tx_we(regDat);
        delay(10);
        return 1 ;
    }

    H42_iic_fun.startSCCB();
    if (0 == H42_iic_fun.SCCBwriteByte(H42_IIC_W_PORT)) {
//		puts("a");
        H42_iic_fun.stopSCCB();
        return (0);
    }
    my_delay(100);
    if (0 == H42_iic_fun.SCCBwriteByte(regID)) {
//		puts("b");
        H42_iic_fun.stopSCCB();
        return (0);
    }
    my_delay(100);
    if (0 == H42_iic_fun.SCCBwriteByte(regDat)) {
//		puts("c");
        H42_iic_fun.stopSCCB();
        return (0);
    }
    H42_iic_fun.stopSCCB();

    return (1);
#endif
}

unsigned char rdH42Reg(unsigned char regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H42_IIC_W_PORT)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, H42_IIC_R_PORT)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);

    return ret;

#if 0
    if (H42_iic_sel) {
        iic_tx(H42_IIC_W_PORT);
        delay(10);
        iic_tx_we(regID);
        delay(10);

        iic_tx(H42_IIC_R_PORT);
        delay(10);
        *regDat = iic_rx_we();

        return *regDat ;
    }

    //¨a?§1yD??2¨′?á??¨|¨¨??????????|ì???¤
    H42_iic_fun.startSCCB();
    if (0 == H42_iic_fun.SCCBwriteByte(H42_IIC_W_PORT)) {
//		puts("1");
        H42_iic_fun.stopSCCB();
        return (0);
    }
    my_delay(100);
    if (0 == H42_iic_fun.SCCBwriteByte(regID)) {
//		puts("2");
        H42_iic_fun.stopSCCB();
        return (0);
    }
    H42_iic_fun.stopSCCB();
    my_delay(100);

    //¨|¨¨??????????|ì???¤o¨??ê?2?¨o??¨￠
    H42_iic_fun.startSCCB();
    if (0 == H42_iic_fun.SCCBwriteByte(H42_IIC_R_PORT)) {
//		puts("3");
        H42_iic_fun.stopSCCB();
        return (0);
    }
    my_delay(100);
    *regDat = H42_iic_fun.SCCBreadByte();
    H42_iic_fun.noAck();
    H42_iic_fun.stopSCCB();

    return (1);
#endif
}


/*************************************************************************************************
    sensor api
*************************************************************************************************/
static u32 h42_pclk = 0;
static u32 h42_row_time_us = 0;
static u32 h42_pre_pll_ratio = 0;

static u32 h42_fps = 30;


static u32 h42_get_shutter(u32 exp_time_us)
{
    return exp_time_us * h42_pclk / H42_CLK_PER_ROW;
}

void H42_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u32 frame_exp_rows;
    u8 tmp;

    H42_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(H42_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrH42Reg(H42_INI_REG[i].addr, H42_INI_REG[i].value);
    }

    if ((*frame_freq) == 60) {
        h42_pre_pll_ratio = 0;
        h42_fps = 60;

    } else if ((*frame_freq) == 30) {
        h42_pre_pll_ratio = 1;
        h42_fps = 30;

    } else if ((*frame_freq) == 50) {
        h42_pre_pll_ratio = 0;
        h42_fps = 50;

    } else {
        h42_pre_pll_ratio = 1;
        h42_fps = 25;
    }



    h42_pclk = ((H42_MCLK * H42_PCLK_MUL / (1 + h42_pre_pll_ratio)) / (H42_PCLK_DIV2_MINUS1 + 1));
    h42_row_time_us = (H42_CLK_PER_ROW / h42_pclk);
    frame_exp_rows = (1000000 * h42_pclk) / (H42_CLK_PER_ROW * (*frame_freq));
    wrH42Reg(0x0e, 0x1c | h42_pre_pll_ratio);
    wrH42Reg(0x22, (frame_exp_rows >> 0) & 0xff);
    wrH42Reg(0x23, (frame_exp_rows >> 8) & 0xff);

    printf("H42 Frame Rate Preset : (%dfps, 0x%x)\n", *frame_freq, frame_exp_rows);

    // h42 bug;
    rdH42Reg(0x09, &tmp);
    if (tmp == 0x81) {
        wrH42Reg(0x27, 0x3b);
        wrH42Reg(0x0c, 0x04);
        wrH42Reg(0x48, 0x60);
        wrH42Reg(0x63, 0x51);
    } else if (tmp == 0x00 || tmp == 0x80) {
        wrH42Reg(0x27, 0x45);
        wrH42Reg(0x0c, 0x00);
        wrH42Reg(0x48, 0x40);
        wrH42Reg(0x63, 0x19);
    }

    wrH42Reg(0x12, 0x00);



    //*format = SEN_IN_FORMAT_BGGR;
    puts("\n----hello H42 222 -----\n");

    return;
}


static s32 H42_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    *width = H42_OUTPUT_W;//640;
    *height = H42_OUTPUT_H;//480;

    //*frame_freq = 30;//30;

    return 0;
}


s32 H42_power_ctl(u8 isp_dev, u8 is_work)
{
#if 0
    if (is_work) {
        if (isp_dev == ISP_DEV_0) {
            ISP0_XCLK_IO_ABLE();
        } else {
            ISP1_XCLK_IO_ABLE();
        }
    } else {
        if (isp_dev == ISP_DEV_0) {
            ISP0_XCLK_IO_DISABLE();
        } else {
            ISP1_XCLK_IO_DISABLE();
        }
    }
#endif

    return 0;
}


//void H42_xclk_set(u8 isp_dev)
//{
////	IOMC1 |= 0X0A<<16;//output PWM 24MHz
//    ISP_XCLK_MAPOUT(); //output 24MHz
//
//	if (isp_dev==ISP_DEV_0)
//	{
//		ISP0_XCLK_IO_ABLE();
//	}
//	else
//	{
//		ISP1_XCLK_IO_ABLE();
//	}
//}



s32 H42_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdH42Reg(0x0A, &pid);
        rdH42Reg(0x0B, &ver);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    if (pid != 0xA0 || ver != 0x42) {
        puts("\n----not H42-----\n");
        return -1;
    }
    puts("\n----hello H42-----\n");
    return 0;
}

void H42_reset(u8 isp_dev)
{

    u32 gpio;

    if (isp_dev == ISP_DEV_0) {
        gpio = reset_gpio[0];
    } else {
        gpio = reset_gpio[1];
    }

    gpio_direction_output(gpio, 0);
    delay(40000);
    gpio_direction_output(gpio, 1);
    delay(40000);

#if 0

    sensor_puts("reset \n");

    if (isp_dev == ISP_DEV_0) {
        SET_RESET_OUT_A();
        SET_RESET_L_A();
        delay(40000);
        SET_RESET_H_A();
        delay(40000);
    } else {
        SET_RESET_OUT_B();
        SET_RESET_L_B();
        delay(40000);
        SET_RESET_H_B();
        delay(40000);
    }
#endif

    wrH42Reg(0x12, 0x80);

}

/*void H42_iic_set(u8 isp_dev)*/
/*{*/
/*iic_select(&H42_iic_fun, isp_dev);*/
/*}*/

static u8 cur_sensor_type = 0xff;

s32 H42_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    if (isp_dev == ISP_DEV_1) {
        return -1;
    }
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

    reset_gpio[isp_dev] = _reset_gpio;

    /*if(H42_iic_sel)*/
    /*{*/
    /*extern void  iic_ini(void);*/
    /*iic_ini();*/
    /*}*/

    //H42_xclk_set(isp_dev);
    /*H42_iic_set(isp_dev);*/
    H42_reset(isp_dev);

    if (0 != H42_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}


s32 H42_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n H42_init 22 \n\n");


//	if (0 != H42_check(isp_dev))
//	{
//		return -1;
//	}

    H42_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

u32 h42_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 exp_line;
    u32 ratio;
    u32 line_time;

    exp_line = h42_get_shutter(exp_time_us);
    line_time = exp_line * H42_CLK_PER_ROW / h42_pclk;
    ratio = (exp_time_us) * (1 << 10) / line_time;
    shutter->ae_exp_line = h42_get_shutter(exp_time_us);

    shutter->ae_gain = ((gain * ratio) >> 10);
    shutter->ae_exp_clk = 0;

    //printf("ae_exp_line=%d, ae_gain=0x%x, gain=0x%x, ratio=0x%x\n", shutter->ae_exp_line, shutter->ae_gain, gain, ratio);

    return 0;
}

u32 h42_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 exp_line = shutter->ae_exp_line;
    u32 gain = shutter->ae_gain;
    u32 again;
    u32 tmp;
    u32 shift;

    tmp = gain >> 10;

    if (tmp < 2) {
        shift = 0;
        again = (gain - 1024) >> 6;
    } else if (tmp < 4) {
        shift = 1;
        again = ((gain >> 1) - 1024) >> 6;
    } else if (tmp < 8) {
        shift = 2;
        again = ((gain >> 2) - 1024) >> 6;
    } else if (tmp < 16) {
        shift = 3;
        again = ((gain >> 3) - 1024) >> 6;
    } else if (tmp < 32) {
        shift = 4;
        again = ((gain >> 4) - 1024) >> 6;
    } else {
        shift = 5;
        again = ((gain >> 5) - 1024) >> 6;
    }

    again = (again & 0x0f) | (shift << 4);

    if (h42_pre_pll_ratio == 1) {
        if (h42_fps == 30) {
            if (exp_line > 0x2ee) {
                wrH42Reg(0x22, (exp_line >> 0) & 0xff);
                wrH42Reg(0x23, (exp_line >> 8) & 0xff);
            } else {
                wrH42Reg(0x23, 0x2ee >> 8);
                wrH42Reg(0x22, 0x2ee & 0xff);
            }
        }
        if (h42_fps == 25) {
            if (exp_line > 0x384) {
                wrH42Reg(0x22, (exp_line >> 0) & 0xff);
                wrH42Reg(0x23, (exp_line >> 8) & 0xff);
            } else {
                wrH42Reg(0x23, 0x384 >> 8);
                wrH42Reg(0x22, 0x384 & 0xff);
            }
        }

    } else {
        if (h42_fps == 60) {
            if (exp_line < 0x2ee) {
                wrH42Reg(0x23, 0x2ee >> 8);
                wrH42Reg(0x22, 0x2ee & 0xff);
            } else {
                wrH42Reg(0x22, (exp_line >> 0) & 0xff);
                wrH42Reg(0x23, (exp_line >> 8) & 0xff);
            }
        }
        if (h42_fps == 50) {
            if (exp_line < 0x384) {
                wrH42Reg(0x23, 0x384 >> 8);
                wrH42Reg(0x22, 0x384 & 0xff);
            } else {
                wrH42Reg(0x22, (exp_line >> 0) & 0xff);
                wrH42Reg(0x23, (exp_line >> 8) & 0xff);
            }
        }
    }

    wrH42Reg(0xC2, 0x01);
    wrH42Reg(0xC3, exp_line & 0xff);
    wrH42Reg(0xC4, 0x02);
    wrH42Reg(0xC5, exp_line >> 8);

    wrH42Reg(0xC0, 0x00);
    wrH42Reg(0xC1, again);

    wrH42Reg(0xC6, 0x0d);
    wrH42Reg(0xC7, 0x40);


    wrH42Reg(0x12, 0x8);

    return 0;
}



void H42_sleep()
{
}

void H42_wakeup()
{
}

void H42_W_Reg(u16 addr, u16 val)
{
    wrH42Reg((u8)addr, (u8)val);
}
u16 H42_R_Reg(u16 addr)
{
    u8 val;
    rdH42Reg((u8)addr, &val);
    return val;
}
//#endif




void reset_H42()
{
    h42_pclk = 0;
    h42_row_time_us = 0;
    h42_pre_pll_ratio = 0;
}



REGISTER_CAMERA(H42) = {
    .logo 				= 	"H42",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B,
    .fps         		= 	30,

    .sen_size 			= 	{H42_OUTPUT_W, H42_OUTPUT_H},
    .isp_size 			= 	{H42_OUTPUT_W, H42_OUTPUT_H},
    .cap_fps                =   15,
    .sen_cap_size             =   {H42_OUTPUT_W, H42_OUTPUT_H},
    .isp_cap_size             =   {H42_OUTPUT_W, H42_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	H42_check,
        .init 		        = 	H42_init,
        .set_size_fps 		=	H42_set_output_size,
        .power_ctrl         =   H42_power_ctl,

        .get_ae_params      =   h42_get_ae_params,
        .get_awb_params     =   h42_get_awb_params,
        .get_iq_params      =   h42_get_iq_params,

        .sleep 		        =	H42_sleep,
        .wakeup 		    =	H42_wakeup,
        .write_reg 		    =	H42_W_Reg,
        .read_reg 		    =	H42_R_Reg,
    }
};
