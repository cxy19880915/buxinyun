
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "asm/isp_alg.h"
#include "ar0330_mipi.h"

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};
//=====================================================================================//


static void *iic = NULL;

#define WRCMD   0x20
#define RDCMD   0x21

unsigned char wrAR0330Reg(u16 regID, u8 regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}

unsigned char rdAR0330Reg(u16 regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __rend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID & 0xff)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, RDCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(1000);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}

unsigned char wrAR0330Reg_u16(u16 regID, u16 regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regDat >> 8)) {
        ret = 0;
        goto __wend;
    }


    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat & 0xff)) {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}


unsigned char rdAR0330Reg_u16(u16 regID, u16 *regDat)
{
    u8 ret = 1;

    u8 dat = 0;

    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __rend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID & 0xff)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, RDCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_ACK, (u32)&dat);
    printf("1st:%x\n", dat);
    *regDat = (dat << 8);
    delay(100);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)&dat);
    printf("2nd:%x\n", dat);
    *regDat |= dat;

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}

void ar0330_wr16(u16 adr, u16 dat)
{
    wrAR0330Reg_u16(adr, dat);
}


u16 ar0330_rd16(u16 adr)
{
    u16 dat;
    rdAR0330Reg_u16(adr, &dat);
    return dat;
}

void ar0330_wbit(u16 adr, u16 pha, u8 val)
{
    u16 buf;

    rdAR0330Reg_u16(adr, &buf);

    if (val) {
        buf |=  pha;
    } else {
        buf &= ~pha;
    }
    wrAR0330Reg_u16(adr, buf);
}

static s32 read_id(void)
{
    u16 dat;
    u8 rx_dat0;
    u8 rx_dat1;

    dat = ar0330_rd16(0x3000);

    rx_dat0 = dat >> 8;
    rx_dat1 = dat & 0xff;

    puts("\n\nAR0330 read ID : ");
    put_u8hex(rx_dat0);
    put_u8hex(rx_dat1);
    puts("\n\n");
    if (rx_dat0 == 0x26 && rx_dat1 == 0x04) {
        return 0;
    } else {
        return -1;
    }
}

void AR0330_ini(void)
{
    //output config
    ar0330_wbit(0x301A, 0x0001, 1); // Reset Sensor
    delay(0xffff);                  // delay 10ms
    ar0330_wbit(0x301A, 0x0004, 0); // Disable Streaming
    delay(0xffff);                  // delay 10ms
    ar0330_wbit(0x301A, 0x0040, 0); // Drive Pins
    ar0330_wbit(0x301A, 0x0080, 0); // Parallel Enable
    ar0330_wbit(0x301A, 0x1000, 0); // SMIA Serializer 0:Enable 1:Disable
    ar0330_wbit(0x3064, 0x0100, 0); // Disable Embedded Data

    //pll config : mipi 2-lane
    //input      : 24M
    //Fvco       : 588M
    //pix rate   : 98M
    ar0330_wr16(0x302E,  4);        // pre_pll_clk_div
    ar0330_wr16(0x3030, 98);        // pll_multiplier
    ar0330_wr16(0x302C,  2);        // vt_sys_clk_div
    ar0330_wr16(0x302A,  6);        // vt_pix_clk_div
    ar0330_wr16(0x3038,  1);        // op_sys_clk_div
    ar0330_wr16(0x3036, 12);        // op_pix_clk_div
    ar0330_wr16(0x31AC, 0x0c0c);    // data_format is 12-bit
    ar0330_wr16(0x31AE, 0x0202);    // mipi 2-lane mode

    //mipi timing
    ar0330_wr16(0x31B0, 36);        // FRAME PREAMBLE
    ar0330_wr16(0x31B2, 12);        // LINE  PREAMBLE
    ar0330_wr16(0x31B4, 0x2643);    // MIPI TIMING 0
    ar0330_wr16(0x31B6, 0x114E);    // MIPI TIMING 1
    ar0330_wr16(0x31B8, 0x2048);    // MIPI TIMING 2
    ar0330_wr16(0x31BA, 0x0186);    // MIPI TIMING 3
    ar0330_wr16(0x31BC, 0x8005);    // MIPI TIMING 4
    ar0330_wr16(0x31BE, 0x2003);    // MIPI CONFIG STATUS

#if 1
    //ARRAY READOUT SETTINGS : 2304 x 1296
    ar0330_wr16(0x3004,  198);      // X_ADDR_START
    ar0330_wr16(0x3008, 2117);      // X_ADDR_END
    ar0330_wr16(0x3002,  228);      // Y_ADDR_START
    ar0330_wr16(0x3006, 1315);      // Y_ADDR_END
#else

    ar0330_wr16(0x3004,  198);      // X_ADDR_START
    ar0330_wr16(0x3008, 1477);      // X_ADDR_END
    ar0330_wr16(0x3002,  228);      // Y_ADDR_START
    ar0330_wr16(0x3006, 947);      // Y_ADDR_END
#endif

    //Sub-sampling
    ar0330_wr16(0x30A2, 1);         // X_ODD_INCREMENT
    ar0330_wr16(0x30A6, 1);         // Y_ODD_INCREMENT
    ar0330_wbit(0x3040, 0x1000, 0); // Row Bin
    ar0330_wbit(0x3040, 0x2000, 0); // Column Bin
    ar0330_wbit(0x3040, 0x0200, 0); // Column SF Bin

    //Frame-Timing
    ar0330_wr16(0x300C, 1248);      // LINE_LENGTH_PCK
    ar0330_wr16(0x300A, 1308);      // FRAME_LENGTH_LINES
    ar0330_wr16(0x3014,    0);      // FINE_INTEGRATION_TIME
    ar0330_wr16(0x3012, 1307);      // Coarse_Integration_Time
    ar0330_wr16(0x3042,  949);      // EXTRA_DELAY
    ar0330_wbit(0x30BA, 0x0040, 0); // Digital_Ctrl_Adc_High_Speed

    //ISO config
#define ar0330_ISO_133()  {ar0330_wr16(0x305E,0x0080); ar0330_wr16(0x3060,0x0000);}
#define ar0330_ISO_200()  {ar0330_wr16(0x305E,0x00C0); ar0330_wr16(0x3060,0x0000);}
#define ar0330_ISO_400()  {ar0330_wr16(0x305E,0x00C0); ar0330_wr16(0x3060,0x0010);}
#define ar0330_ISO_800()  {ar0330_wr16(0x305E,0x00C0); ar0330_wr16(0x3060,0x0020);}
#define ar0330_ISO_1600() {ar0330_wr16(0x305E,0x00C0); ar0330_wr16(0x3060,0x0030);}
#define ar0330_ISO_3200() {ar0330_wr16(0x305E,0x0180); ar0330_wr16(0x3060,0x0030);}
    ar0330_ISO_800();

    //RGB gain
    u16 x_gain = 0x110;
    u16 r_gain = x_gain * 1.5;
    u16 g_gain = x_gain * 1.0;
    u16 b_gain = x_gain * 1.5;
    ar0330_wr16(0x3056, g_gain);    // Gb gain
    ar0330_wr16(0x3058, b_gain);    // B  gain
    ar0330_wr16(0x305A, r_gain);    // R  gain
    ar0330_wr16(0x305C, g_gain);    // Gr gain

    puts("\nAR0330 initial done");

    //sensor start
    //ar0330_wbit(0x301A, 0x0004,1);  // Enable Streaming
}

void AR0330_run(void)
{
    //sensor start
    ar0330_wbit(0x301A, 0x0004, 1); // Enable Streaming
    puts("\nAR0330 go go go");
}




/*************************************************************************************************
    sensor api
*************************************************************************************************/

static u8 cur_sensor_type = 0xff;

void AR0330_MIPI_reset(u8 isp_dev)
{
    puts("BG0806_MIPI reset \n");

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
static s32 AR0330_MIPI_ID_check()
{
    return read_id();
}


s32 AR0330_MIPI_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{
    puts("\n\n AR0330_MIPI_check \n\n");

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

    AR0330_MIPI_reset(isp_dev);

    puts("gc1004_id_check\n");
    if (0 != AR0330_MIPI_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}


s32 AR0330_MIPI_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    AR0330_ini();
    AR0330_run();

    *width = 1920;
    *height = 1088;
    *format = SEN_IN_FORMAT_GRBG;
    *frame_freq = 30;
    return 0;
}


s32 set_size_fps(u16 *width, u16 *height, u8 *freq)  /*sensor size control*/
{
    return 0;
}

static void set_again(u32 again)
{
    ar0330_wr16(0x3060, again);
    return;
}

static void set_dgain(u32 dgain)
{
    ar0330_wr16(0x305E, dgain);
}

static int ar0330_analog_gain_value(u32 gain, u32 *again, u32 *again_reg)
{
    u32 gain_val = gain * 100 / 1024;

    if (100 <= gain_val && gain_val < 103) {
        *again = 100;
        *again_reg = 0x00;
    } else if (103 <= gain_val && gain_val < 107) {
        *again = 107;
        *again_reg = 0x01;
    } else if (107 <= gain_val && gain_val < 110) {
        *again = 107;
        *again_reg = 0x02;
    } else if (110 <= gain_val && gain_val < 114) {
        *again = 110;
        *again_reg = 0x03;
    } else if (114 <= gain_val && gain_val < 119) {
        *again = 114;
        *again_reg = 0x04;
    } else if (119 <= gain_val && gain_val < 123) {
        *again = 119;
        *again_reg = 0x05;
    } else if (123 <= gain_val && gain_val < 128) {
        *again = 123;
        *again_reg = 0x06;
    } else if (128 <= gain_val && gain_val < 133) {
        *again = 128;
        *again_reg = 0x07;
    } else if (133 <= gain_val && gain_val < 139) {
        *again = 133;
        *again_reg = 0x08;
    } else if (139 <= gain_val && gain_val < 145) {
        *again = 139;
        *again_reg = 0x09;
    } else if (145 <= gain_val && gain_val < 152) {
        *again = 145;
        *again_reg = 0x0a;
    } else if (152 <= gain_val && gain_val < 160) {
        *again = 152;
        *again_reg = 0x0b;
    } else if (160 <= gain_val && gain_val < 168) {
        *again = 160;
        *again_reg = 0x0c;
    } else if (168 <= gain_val && gain_val < 178) {
        *again = 168;
        *again_reg = 0x0d;
    } else if (178 <= gain_val && gain_val < 188) {
        *again = 178;
        *again_reg = 0x0e;
    } else if (188 <= gain_val && gain_val < 200) {
        *again = 188;
        *again_reg = 0x0f;
    } else if (200 <= gain_val && gain_val < 213) {
        *again = 200;
        *again_reg = 0x10;
    } else if (213 <= gain_val && gain_val < 229) {
        *again = 213;
        *again_reg = 0x12;
    } else if (229 <= gain_val && gain_val < 246) {
        *again = 229;
        *again_reg = 0x14;
    } else if (246 <= gain_val && gain_val < 267) {
        *again = 246;
        *again_reg = 0x16;
    } else if (267 <= gain_val && gain_val < 291) {
        *again = 267;
        *again_reg = 0x18;
    } else if (291 <= gain_val && gain_val < 320) {
        *again = 291;
        *again_reg = 0x1a;
    } else if (320 <= gain_val && gain_val < 356) {
        *again = 320;
        *again_reg = 0x1c;
    } else if (356 <= gain_val && gain_val < 400) {
        *again = 356;
        *again_reg = 0x1e;
    } else if (400 <= gain_val && gain_val < 457) {
        *again = 400;
        *again_reg = 0x20;
    } else if (457 <= gain_val && gain_val < 533) {
        *again = 457;
        *again_reg = 0x24;
    } else if (533 <= gain_val && gain_val < 640) {
        *again = 533;
        *again_reg = 0x28;
    } else if (640 <= gain_val && gain_val < 800) {
        *again = 640;
        *again_reg = 0x2c;
    } else if (800 <= gain_val) {
        *again = 800;
        *again_reg = 0x30;
    }

    *again = *again * 1024 / 100;

    return -1;

}
//q10
static void calc_gain(u32 gain, u32 *again, u32 *dgain)
{
    u32 _again;
    u32 _dgain;
    u32 xx;
    u32 yy;
    //printf("gain = %x\n", gain);
    ar0330_analog_gain_value(gain, &_again, again);
    //printf("_again = %x, again_val=%x\n",_again, *again);

    _dgain = gain * (1 << 10) / _again;

    xx = _dgain >> 10;
    yy = _dgain % (1024);
    yy = (yy * 128 + 512) >> 10;

    *dgain = (xx << 7) | yy;

    //printf("dgain = %x\n", *dgain);

}

static void set_shutter(u32 texp)
{
    ar0330_wr16(0x3012, texp);
}


u32 ar0330_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;

    texp = exp_time_us * 98 / 1248 / 2;
    //printf("texp = %d\n", texp);

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = gain ;
    shutter->ae_exp_clk = 0;

    return 0;

}

u32 ar0330_mipi_set_shutter(isp_ae_shutter_t *shutter)
{

    u32 again, dgain;

#if 1

    calc_gain((shutter->ae_gain), &again, &dgain);

    set_again(again);
    set_dgain(dgain);

    set_shutter(shutter->ae_exp_line);
#endif

    return 0;
}


s32 ar0330_mipi_get_mipi_clk(u32 *mipi_clk,  u32 *tval_hstt, u32 *tval_stto, u16 width, u16 height, u8 frame_freq)
{
    *mipi_clk = 0;
    *tval_hstt = 12;
    *tval_stto = 12;
    return 0;
}

extern void *AR0330_MIPI_get_iq_params();
extern void *ar0330_mipi_get_awb_params();
extern void *ar0330_mipi_get_ae_params();

REGISTER_CAMERA(AR0330) = {
    .logo 				= 	"AR0330",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_GRBG,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_12B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	30,

    .sen_size 			= 	{1920, 1088},
    .isp_size 			= 	{1920, 1088},
    //.sen_size 			= 	{1280, 720},
    //.isp_size 			= 	{1280, 720},

    .cap_fps         		= 	30,
    .sen_cap_size 			= 	{1920, 1088},
    .isp_cap_size 			= 	{1920, 1088},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	AR0330_MIPI_check,
        .init 		        = 	AR0330_MIPI_init,
        .set_size_fps 		=	set_size_fps,
        .power_ctrl         =   NULL,

        .get_mipi_clk       =   ar0330_mipi_get_mipi_clk,

        .get_ae_params  	=	ar0330_mipi_get_ae_params,
        .get_awb_params 	=	ar0330_mipi_get_awb_params,
        .get_iq_params 	    =	AR0330_MIPI_get_iq_params,

        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	NULL,
        .read_reg 		    =	NULL,

    }
};


