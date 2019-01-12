
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "sc2143.h"
#include "asm/isp_alg.h"

static u32 reset_gpio[2] = {-1, -1};

extern void *sc2143_get_ae_params();
extern void *sc2143_get_awb_params();
extern void *sc2143_get_iq_params();
extern void sc2143_ae_ev_init(u32 fps);

#define ROW_CLK  0x960
#define INPUT_CLK  24
#define PCLK  36


typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini sc2143_INI_REG[] = {
    0x0103, 0x01,
    0x0100, 0x00,

    0x3c00, 0x45, //FIFO RESET


    0x3907, 0x01, //RNC BLC
    0x3908, 0xc0,
    0x3416, 0x10,

    0x3200, 0x00,
    0x3201, 0x00,
    0x3204, 0x07,
    0x3205, 0x9f,
    0x322c, 0x07,
    0x322d, 0xa8,
    0x322e, 0x07,
    0x322f, 0xff,
    0x3400, 0x53,
    0x3401, 0x1e,
    0x3402, 0x04,
    0x3403, 0x30,

    0x3637, 0x87, //RAMP

    0x3623, 0x02, //analog
    0x3620, 0xc4,
    0x3621, 0x18,

    0x3635, 0x03,

    //Timing

    0x3300, 0x10, //EQ
    0x3306, 0xe0,
    0x330a, 0x01,
    0x330b, 0xa0,


    0x3333, 0x00,
    0x3334, 0x20,



    0x3039, 0x00, //74.25M pclk  74.25=27*(64-0x35)/4
    0x303a, 0x3a,
    0x303b, 0x0c,
    0x3035, 0xca,
    0x320c, 0x08, //0x898 for 30fps
    0x320d, 0x98,
    0x3211, 0x10,
    0x3213, 0x10,

    0x301c, 0xa4, //close mipi
    0x3018, 0xff,

    0x3d08, 0x00, //pclk inv


    0x337f, 0x03, //new auto precharge  330e in 3372
    0x3368, 0x04,
    0x3369, 0x00,
    0x336a, 0x00,
    0x336b, 0x00,
    0x3367, 0x08,
    0x330e, 0x40,

    //0x3630/0x3635 auto ctrl
    0x3670, 0x0b, //bit[3] for 3635 in 3687, bit[1] for 3630 in 3686,bit[0] for 3620 in 3685
    0x3674, 0xa0, //3630 value <gain0
    0x3675, 0x90, //3630 value between gain0 and gain1
    0x3676, 0x40, //3630 value > gain1
    0x367c, 0x07, //gain0
    0x367d, 0x0f, //gain1

    0x3677, 0x0a, //3635 value <gain0
    0x3678, 0x07, //3635 value between gain0 and gain1
    0x3679, 0x07, //3635 value > gain1
    0x367e, 0x07, //gain0
    0x367f, 0x1f, //gain1

    0x3671, 0xc2, //3620 value <gain0   11.23
    0x3672, 0xc2, //3620 value between gain0 and gain1  11.23
    0x3673, 0x63, //3620 value > gain1  11.23
    0x367a, 0x07, //gain0
    0x367b, 0x1f, //gain1

    0x3e03, 0x03, //AE
    0x3e01, 0x46,
    0x3e08, 0x00,
    0x3e09, 0x20,


    0x3401, 0x1e,
    0x3402, 0x00,
    0x3403, 0x48, //increase rnc col num to 72+16=88


    0x5781, 0x08, //dpc
    0x5782, 0x08,
    0x5785, 0x20,
    0x57a2, 0x01,
    0x57a3, 0xf1,

    //fullwell
    0x3637, 0x86,
    0x3635, 0x03, //  gain <16 0a
    0x3622, 0x0e,
    0x3630, 0x00,
    0x3631, 0x80, //gain <2  0x81
    0x3633, 0x54,

    //fullwell adjust 0907

    0x3637, 0x87,
    0x3674, 0xd0,
    0x3677, 0x07,
    0x3633, 0x74,
    0x330b, 0xa8,

    0x3333, 0x80, //col fpn
    0x3334, 0xa0,
    0x3300, 0x20, //shading
    0x3632, 0x40, //gain >8 0x42

    //0908 update rnc num
    0x3403, 0x58,
    0x3416, 0x11,

    0x3302, 0x28, //rst go low point to cancel left column fpn from rst
    0x3309, 0x20, //ramp gap to cancel right column fpn from tx
    0x331f, 0x17,
    0x3321, 0x1a,

    0x3677, 0x0b, //high txvdd when gain<2
    0x3678, 0x08, //3635 value between gain0 and gain1
    0x3679, 0x06, //3635 value > gain1

    0x3306, 0xd0,

    //ECO
    0x322e, 0x08, //rnc
    0x322f, 0x37,
    0x3403, 0x78,



    0x3679, 0x08,

    //1118
    0x3679, 0x06,
    0x3620, 0xc4, //0x64

    //1122
    0x3637, 0x84,
    0x3638, 0x84,

    //1230
    0x3e01, 0x00,
    0x3e02, 0x10,
    0x3039, 0x00,
    0x303a, 0x34,
    0x303b, 0x15, //36M
    0x320c, 0x09,
    0x320d, 0x60,
    0x320e, 0x04,
    0x320f, 0xe2, //2400*1250
    0x330a, 0x00,
    0x330b, 0xc0,


    0x0100, 0x01,

};


static void *iic = NULL;

#define WRCMD 0x60
#define RDCMD 0x61

unsigned char wrSc2143Reg(u16 regID, unsigned char regDat)
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

unsigned char rdSc2143Reg(u16 regID, unsigned char *regDat)
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


/*************************************************************************************************
    sensor api
*************************************************************************************************/

void sc2143_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u8 v;


    sc2143_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(sc2143_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrSc2143Reg(sc2143_INI_REG[i].addr, sc2143_INI_REG[i].value);
    }
    /*for(i=0;i<sizeof(sc2143_INI_REG)/sizeof(Sensor_reg_ini);i++)
    {
        rdSc2143Reg(sc2143_INI_REG[i].addr,&v);
        printf("0x%4x,0x%2x\n", sc2143_INI_REG[i].addr, v);
    }*/

    sc2143_ae_ev_init(*frame_freq);

    *format = SEN_IN_FORMAT_BGGR;
    return;
}


s32 sc2143_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    *width = SC2143_OUTPUT_W;
    *height = SC2143_OUTPUT_H;

    return 0;
}


s32 sc2143_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 sc2143_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdSc2143Reg(0x3107, &pid);
        rdSc2143Reg(0x3108, &ver);
    }
    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    if (pid != 0x21 && ver != 0x45) {
        puts("\n----not sc2143-----\n");
        //return -1;
    }
    puts("\n----hello sc2143-----\n");
    return 0;
}

void sc2143_reset(u8 isp_dev)
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
}


static u8 cur_sensor_type = 0xff;

s32 sc2143_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    puts("\n\n sc2143_check \n\n");
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

    reset_gpio[isp_dev] = _reset_gpio;

    sc2143_reset(isp_dev);

    if (0 != sc2143_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 sc2143_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n sc2143_init22 \n\n");

    sc2143_config_SENSOR(width, height, format, frame_freq);

    return 0;
}


static void set_again(u32 again)
{
    return;
}

static void set_dgain(u32 dgain)
{

    wrSc2143Reg(0x3e08, dgain >> 8);
    wrSc2143Reg(0x3e09, dgain & 0xff);
}

//q10
static void calc_gain(u32 gain, u8 *again, u16 *dgain)
{
    u32 ag;
    *again = 0;
    *dgain = gain * 0x10 / (1 << 10);
}

static void set_shutter(u32 texp)
{
    wrSc2143Reg(0x3e01, texp >> 4);
    wrSc2143Reg(0x3e02, (texp << 4) & 0xf0);
}


u32 sc2143_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_align;
    u32 ratio;

    texp = exp_time_us * PCLK * 2 / ROW_CLK;
    texp_align = (texp) * ROW_CLK / (PCLK * 2);
    //printf("exp_time_us=%d, texp=%d, texp_align=%d\n", exp_time_us, texp, texp_align);
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



u32 sc2143_set_shutter(isp_ae_shutter_t *shutter)
{

    u32 again, dgain;


    calc_gain((shutter->ae_gain), &again, &dgain);

    set_again(again);
    set_dgain(dgain);

    set_shutter(shutter->ae_exp_line);

    return 0;
}

void sc2143_sleep()
{


}

void sc2143_wakeup()
{


}

void sc2143_W_Reg(u16 addr, u16 val)
{
    printf("update reg%x with %x\n", addr, val);
    wrSc2143Reg((u16)addr, (u8)val);
}
u16 sc2143_R_Reg(u16 addr)
{
    u8 val;
    rdSc2143Reg((u16)addr, &val);
    return val;
}



REGISTER_CAMERA(sc2143) = {
    .logo 				= 	"sc2143x",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B,
    .fps                =   30,

    .sen_size           =   { SC2143_OUTPUT_W, SC2143_OUTPUT_H },
    .isp_size           =   { SC2143_OUTPUT_W, SC2143_OUTPUT_H },

    .cap_fps            =   30,
    .sen_cap_size       =   { SC2143_OUTPUT_W, SC2143_OUTPUT_H },
    .isp_cap_size       =   {SC2143_OUTPUT_W, SC2143_OUTPUT_H },

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	sc2143_check,
        .init 		        = 	sc2143_init,
        .set_size_fps 		=	sc2143_set_output_size,
        .power_ctrl         =   sc2143_power_ctl,

        .get_ae_params  	=	sc2143_get_ae_params,
        .get_awb_params 	=	sc2143_get_awb_params,
        .get_iq_params 	    	=	sc2143_get_iq_params,

        .sleep 		        =	sc2143_sleep,
        .wakeup 		    =	sc2143_wakeup,
        .write_reg 		    =	sc2143_W_Reg,
        .read_reg 		    =	sc2143_R_Reg,

    }
};


