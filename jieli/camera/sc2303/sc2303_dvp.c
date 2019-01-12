#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "sc2303_dvp.h"
#include "asm/isp_alg.h"

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

extern void *sc2303_dvp_get_ae_params();
extern void *sc2303_dvp_get_awb_params();
extern void *sc2303_dvp_get_iq_params();
extern void sc2303_dvp_ae_ev_init(u32 fps);


#define LINE_LENGTH_CLK     0x960
#define FRAME_LENGTH        1125
#define ROW_TIME_NS         29630
#define INPUT_CLK  24
#define PCLK  81

#define LINE_LENGTH_CLK_25FPS     0xb40
#define LINE_LENGTH_CLK_20FPS     0xe10
#define LINE_LENGTH_CLK_17FPS     0x10df
//#define LINE_LENGTH_CLK_15FPS     0x12c0
#define ROW_TIME_NS_25FPS         35556
#define ROW_TIME_NS_20FPS         44445
#define ROW_TIME_NS_17FPS         53323
//#define ROW_TIME_NS_15FPS         59260




static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expline = -1;
static u32 cur_line_length_clk = -1;

static u32 line_length_clk = LINE_LENGTH_CLK;

static void *iic = NULL;

#define WRCMD 0x60
#define RDCMD 0x61


typedef struct
{
    u16 addr;
    u8 value;
} sensor_ini_regs_t;

sensor_ini_regs_t sc2303_dvp_ini_regs[] =
{

    0x0103,0x01,
    0x0100,0x00,

    0x3e03,0x03,
    0x3620,0x44,  //gain>2 0x46
    0x3627,0x04,
    0x3621,0x28,
    0x3641,0x03,
    0x3d08,0x01,
    0x3640,0x01,


    0x3300,0x20,
    0x3e03,0x0b,
    0x3635,0x88,


    0x320c,0x0a,
    0x320d,0x50, //2640 hts for 25fps

    0x3e0f,0x05, //11bit
    0x3305,0x00,
    0x3306,0xd0,
    0x330a,0x02,
    0x330b,0x38,

    0x363a,0x06, //NVDD fullwell
    0x3632,0x42, //TXVDD fpn



    0x3622,0x02, //blksun
    0x3630,0x48,
    0x3631,0x80,

    0x3334,0xc0,

    0x3e0e,0x06, //[1] 1:dcg gain in 3e08[5]


    0x3637,0x83,
    0x3638,0x83,
    0x3620,0x46,

    0x3035,0xca, //带状竖条
    0x330b,0x78,
    0x3416,0x44,
    0x363a,0x04,

    0x3e09,0x20, // 1x gain


    0x337f,0x03, //new auto precharge  330e in 3372
    0x3368,0x04,
    0x3369,0x00,
    0x336a,0x00,
    0x336b,0x00,
    0x3367,0x08,
    0x330e,0x80,


    0x3620,0x58,
    0x3632,0x41,
    0x3639,0x04,
    0x363a,0x08,
    0x3333,0x10,

    0x3e01,0x8c,
    0x3e14,0xb0,

    //0314
    0x3038,0x41, //smear

    //
    0x3309,0x30,

    0x331f,0x27,
    0x3321,0x2a,

    0x3620,0x54,
    0x3627,0x03,

    //3.3V 0424
    0x3632,0x40,
    0x363a,0x07,
    0x3635,0x80,
    0x3621,0x28,

    //0426
    0x3626,0x30,

    //0427  light
    0x3633,0xf4,
    0x3632,0x00,
    0x3630,0x4f,

    //full well to 20k
    0x3637,0x84,


    //aec  analog gain: 0x7a0   2xdgain :0xf60  4xdgain : 0x1ed0

    //0x3e1e,0x84, //[7]:1 open DCG function in 0x3e03=0x03 [4]:0 DCG >2   [2] 1: dig_fine_gain_en
    //0x3e0e,0x36, //[7:3] fine gain to compsensate 2.4x DCGgain  5 : 2.3125x  6:2.375x  [2]:1 DCG gain between sa1gain 2~4  [1]:1 dcg gain in 0x3e08[5]
    //0x3e1d,0xb2, //[7]:1 only limit sa1 gain  [5]:1 htem en [4]:1 max_gain_mannual_en
    //0x3e18,0x3f, //max_gain_mannual
    //0x3700,0x03, //[0] 1: dig_fine_gain_apply_en

    //0525
    //auto TXVDD
    0x3670,0x20, //bit[5] for auto 3635 in 0x36a5
    0x3683,0x88, //3635 value <gain0
    0x3684,0x84, //3635 value between gain0 and gain1
    0x3685,0x80, //3635 value > gain1
    0x369a,0x07, //gain0
    0x369b,0x0f, //gain1

    0x330e,0x30, //auto_precharge

    //0728
    0x3637,0x84,
    0x330e,0x20,
    0x3683,0x84,
    0x3338,0x50,

    0x3621,0x08,

    //0728 fpn
    0x3639,0x07,
    0x363a,0x06,
    0x330e,0x30,
    0x3038,0x44,

    0x3670,0x20, //bit[5] for auto 3635 in 0x36a5
    0x3683,0x84, //3635 value <gain0
    0x3684,0x84, //3635 value between gain0 and gain1
    0x3685,0x92, //3635 value > gain1
    0x369a,0x07, //gain0
    0x369b,0x0f, //gain1


    0x3630,0x27, //blksun

    0x3620,0x53,

    //0803
    0x3637,0x83, // 16K Fullwell
    0x3306,0xa0,

    //0x3d08,0x00,
    //0x3641,0x00,

    0x3640,0x00,

    //0808
    0x3637,0x84,

    //0817
    0x3034,0x05,
    0x3e0f,0x07,
    0x3640,0x01,

    0x330b,0x30,

    //0818
    0x3306,0x90,
    0x330b,0x50,  // 0x50  //a8

    //0823
    0x3034,0x05,
    0x3035,0xc2, // 96M

    0x3208,0x07,
    0x3209,0x80,
    0x320a,0x04,
    0x320b,0x40,

    0x320c,0x09,
    0x320d,0x60, // 30fps


    0x3039,0x35,
    0x303a,0x2e, // 81M

    0x3221,0x66,
    //for 25fps
    //0x320c,0x0b
    //0x320d,0x40,

    0x3635,0x84, // SC add for production on 20170928
    0x363a,0x06, // SC add for production on 20170928
    0x3639,0x05, // SC add for production on 20170928
    0x3670,0x00, // SC add for production on 20170928


    0x0100,0x01,

};


unsigned char wr_sc2303_reg(u16 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD))
    {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8))
    {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff))
    {
        ret = 0;
        goto __wend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat))
    {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}

unsigned char rd_sc2303_reg(u16 regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD))
    {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8))
    {
        ret = 0;
        goto __rend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID & 0xff))
    {
        ret = 0;
        goto __rend;
    }

    delay(100);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, RDCMD))
    {
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

void sc2303_dvp_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u8 v;


    sc2303_dvp_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(sc2303_dvp_ini_regs) / sizeof(sensor_ini_regs_t); i++)
    {
        wr_sc2303_reg(sc2303_dvp_ini_regs[i].addr, sc2303_dvp_ini_regs[i].value);
    }

    sc2303_dvp_ae_ev_init(*frame_freq);

    cur_again = -1;
    cur_dgain = -1;
    cur_expline = -1;
    cur_line_length_clk = -1;
    return;
}


s32 sc2303_dvp_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}


s32 sc2303_dvp_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

s32 sc2303_dvp_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++)   //
    {
        rd_sc2303_reg(0x3107, &pid);
        rd_sc2303_reg(0x3108, &ver);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    // if (pid != 0x22 && ver != 0x35)
    if (pid != 0x23 && ver != 0x00)
    {
        puts("\n----not sc2303_dvp-----\n");
        return -1;
    }
    puts("\n----hello sc2303_dvp-----\n");
    return 0;
}

void sc2303_dvp_reset(u8 isp_dev)
{
    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0)
    {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    }
    else
    {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }

    /*printf("pwdn_gpio=%d\n", pwdn_gpio);*/
    gpio_direction_output(pwdn_gpio, 0);

    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    gpio_direction_output(pwdn_gpio, 1);
    delay(40000);
}


static u8 cur_sensor_type = 0xff;

s32 sc2303_dvp_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{

    puts("\n\n sc2303_dvp_check \n\n");
    if (!iic)
    {
        if (isp_dev == ISP_DEV_0)
        {
            iic = dev_open("iic0", 0);
        }
        else
        {
            iic = dev_open("iic1", 0);
        }
        if (!iic)
        {
            return -1;
        }
    }
    else
    {
        if (cur_sensor_type != isp_dev)
        {
            return -1;
        }
    }
    printf("\n\n isp_dev =%d\n\n", isp_dev);

    reset_gpios[isp_dev] = reset_gpio;
    pwdn_gpios[isp_dev] = pwdn_gpio;

    sc2303_dvp_reset(isp_dev);

    if (0 != sc2303_dvp_ID_check())
    {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 sc2303_dvp_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n sc2303_dvp_init22 \n\n");

    sc2303_dvp_config_SENSOR(width, height, format, frame_freq);

    return 0;
}


static void set_again(u32 again)
{
    return;
}

static void set_dgain(u32 dgain)
{
    if (cur_dgain == dgain)
    {
        return;
    }
    cur_dgain  = dgain;

    wr_sc2303_reg(0x3e07, (dgain >> 16)&0xff);
    wr_sc2303_reg(0x3e08, ((dgain >> 8)&0xff)|(0x03));
    wr_sc2303_reg(0x3e09, dgain & 0xff);

}

#if 0
//q10
static void calc_gain(u32 gain, u8 *again, u16 *dgain)
{
    u32 ag;
    *again = 0;
    *dgain = gain * 0x10 / (1 << 10);
}
#else

static u32 fine_gain_lut[][2]=
{
    0x20,1024,
    0x21,1055,
    0x22,1086,
    0x23,1117,
    0x24,1148,
    0x25,1179,
    0x26,1211,
    0x27,1242,
    0x28,1275,
    0x29,1306,
    0x2a,1338,
    0x2b,1368,
    0x2c,1399,
    0x2d,1430,
    0x2e,1462,
    0x2f,1493,
    0x30,1524,
    0x31,1554,
    0x32,1587,
    0x33,1618,
    0x34,1649,
    0x35,1681,
    0x36,1713,
    0x37,1745,
    0x38,1778,
    0x39,1809,
    0x3a,1840,
    0x3b,1871,
    0x3c,1902,
    0x3d,1933,
    0x3e,1965,
    0x3f,1997,
};

static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{
    int i;
    u32 reg0, reg1, reg2;
    u32 dgain, dcg_gain,coarse_gain,fine_gain;
    u32 dgain_val, dcg_gain_val,coarse_gain_val,fine_gain_val;

    if(gain <1024) gain = 1024;
    if(gain > 260*1024) gain = 260*1024;

    if(gain < 33563) //32.776
    {
        dgain = 0;
        dgain_val = 1024;
    }
    else if(gain < 67126) //65.552
    {
        dgain = 1;
        dgain_val = 2048;
    }
    else if(gain < 134251) //131.104
    {
        dgain = 3;
        dgain_val = 4096;
    }
    else
    {
        dgain = 7;
        dgain_val = 8192;
    }

    if(gain < 2462) //2.403
    {
        dcg_gain = 0;
        dcg_gain_val = 1024;
    }
    else
    {
        dcg_gain = 1;
        dcg_gain_val = 2509; //2.45
    }

    if(gain < 1966)  //1.919
    {
        coarse_gain = 0;
        coarse_gain_val = 1024;
    }
    else if(gain < 2462) //2.403
    {
        coarse_gain = 1;
        coarse_gain_val = 1976;
    }
    else if(gain < 4816) //4.7
    {
        coarse_gain = 0;
        coarse_gain_val = 1024;
    }
    else if(gain < 9142) //8.9
    {
        coarse_gain = 1;
        coarse_gain_val = 1976;
    }
    else if(gain < 16962) //16.56
    {
        coarse_gain = 3;
        coarse_gain_val = 3789;
    }
    else
    {
        coarse_gain = 7;
        coarse_gain_val = 7025;
    }

    fine_gain_val = (u32)((u64) gain  *(1<<30)/dgain_val/dcg_gain_val/coarse_gain_val);

    if(fine_gain_val < 1024)
    {
        fine_gain_val  = 1024;
    }

    for(i = 0; i < sizeof(fine_gain_lut)/sizeof(fine_gain_lut[0]); i++)
    {
        if(fine_gain_lut[i][1] > fine_gain_val)
        {
            break;
        }
    }

    i = i - 1;
    if(i < 0) i = 0;

    fine_gain = fine_gain_lut[i][0];

    reg0 = dgain & 0x01; //0x3e07
    reg1 = ((dgain>>1)<<6) | (dcg_gain << 5) | (coarse_gain << 2);
    reg2 = fine_gain; //0x3e09

    *_again = 0;
    *_dgain = (reg0 <<16)|(reg1<<8)|(reg2);

    //printf("gain=%d, %d, %d, %d, 0x%x\n", gain, dgain , dcg_gain, coarse_gain, fine_gain);
}
#endif

static void set_shutter(u32 texp)
{

    if(line_length_clk != cur_line_length_clk)
    {
        cur_line_length_clk = line_length_clk;

        wr_sc2303_reg(0x320c, line_length_clk >> 8);
        wr_sc2303_reg(0x320d, (line_length_clk  & 0xff));
    }

    if (cur_expline == texp)
    {
        return;
    }
    cur_expline  = texp;



    wr_sc2303_reg(0x3e01, texp >> 4);
    wr_sc2303_reg(0x3e02, (texp << 4) & 0xf0);
}


u32 sc2303_dvp_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_align;
    u32 ratio;

#if SC2303_FPS_VARIABLE
    if(exp_time_us <= 30000)
    {
        line_length_clk = LINE_LENGTH_CLK;
        texp = exp_time_us * 1000 * 2 / ROW_TIME_NS;
    }
    else if(exp_time_us <= 40000)
    {
        line_length_clk = LINE_LENGTH_CLK_25FPS;
        texp = exp_time_us * 1000 * 2 / ROW_TIME_NS_25FPS;
    }
    else if(exp_time_us <= 50000)
    {
        line_length_clk = LINE_LENGTH_CLK_20FPS;
        texp = exp_time_us * 1000 * 2 / ROW_TIME_NS_20FPS;
    }
    else
    {
       // line_length_clk = LINE_LENGTH_CLK_15FPS;
        //texp = exp_time_us * 1000 * 2 / ROW_TIME_NS_15FPS;
        line_length_clk = LINE_LENGTH_CLK_17FPS;
        texp = exp_time_us * 1000 * 2 / ROW_TIME_NS_17FPS;

    }
#else


    texp = exp_time_us * 1000 * 2 / ROW_TIME_NS;
#endif // SC2303_FPS_VARIABLE
    if (texp < 1)
    {
        texp = 1;
    }
    if (texp > FRAME_LENGTH * 2 - 4)
    {
        texp = FRAME_LENGTH * 2 - 4;
    }
    texp_align = (texp) * ROW_TIME_NS / (1000 * 2);

    if (texp_align < exp_time_us)
    {
        ratio = (exp_time_us) * (1 << 10) / texp_align;
    }
    else
    {
        ratio = (1 << 10);
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = (gain * ratio) >> 10;
    shutter->ae_exp_clk = 0;

    //printf("exp_time_us=%d, texp=%d, gain=%d\n", exp_time_us, texp, shutter->ae_gain);
    return 0;

}

u32 sc2303_dvp_set_shutter(isp_ae_shutter_t *shutter)
{

    u32 again, dgain;

    calc_gain((shutter->ae_gain), &again, &dgain);

    set_again(again);
    set_dgain(dgain);

    set_shutter(shutter->ae_exp_line);

    if (shutter->ae_gain <= 2048)
    {
        wr_sc2303_reg(0x3622, 0x06);
    }
    else
    {
        wr_sc2303_reg(0x3622, 0x02);
    }

    return 0;
}

void sc2303_dvp_sleep()
{


}

void sc2303_dvp_wakeup()
{


}

void sc2303_dvp_wr_reg(u16 addr, u16 val)
{
    /*printf("update reg%x with %x\n", addr, val);*/
    wr_sc2303_reg((u16)addr, (u8)val);
}
u16 sc2303_dvp_rd_reg(u16 addr)
{
    u8 val;
    rd_sc2303_reg((u16)addr, &val);
    return val;
}

REGISTER_CAMERA(sc2303_dvp) =
{
    .logo 				= 	"sc2303d",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps         		= 	30, // 25

    .sen_size 			= 	{SC2303_DVP_OUTPUT_W, SC2303_DVP_OUTPUT_H},
    .isp_size 			= 	{SC2303_DVP_OUTPUT_W, SC2303_DVP_OUTPUT_H},

    .cap_fps         		= 	30, // 25
    .sen_cap_size 			= 	{SC2303_DVP_OUTPUT_W, SC2303_DVP_OUTPUT_H},
    .isp_cap_size 			= 	{SC2303_DVP_OUTPUT_W, SC2303_DVP_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	sc2303_dvp_check,
        .init 		        = 	sc2303_dvp_init,
        .set_size_fps 		=	sc2303_dvp_set_output_size,
        .power_ctrl         =   sc2303_dvp_power_ctl,

        .get_ae_params  	=	sc2303_dvp_get_ae_params,
        .get_awb_params 	=	sc2303_dvp_get_awb_params,
        .get_iq_params 	    	=	sc2303_dvp_get_iq_params,

        .sleep 		        =	sc2303_dvp_sleep,
        .wakeup 		    =	sc2303_dvp_wakeup,
        .write_reg 		    =	sc2303_dvp_wr_reg,
        .read_reg 		    =	sc2303_dvp_rd_reg,

    }
};


