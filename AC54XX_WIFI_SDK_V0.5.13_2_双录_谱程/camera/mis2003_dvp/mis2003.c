#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "mis2003.h"
#include "asm/isp_alg.h"



static u32 reset_gpio[2] = {-1, -1};

extern void *mis2003_get_ae_params();
extern void *mis2003_get_awb_params();
extern void *mis2003_get_iq_params();
extern void mis2003_ae_ev_init(u32 fps);

static void *iic = NULL;

#define MIS2003_WRCMD 0x60
#define MIS2003_RDCMD 0x61

#define INPUT_CLK  24//摄像头外部晶振的频率,根据使用修改
#define MIS2003_FPS 25

#if MIS2003_FPS==30
#define FRAME_W  2200
#define FRAME_H  1125
#else
//25fps
#define FRAME_W  2640
#define FRAME_H  1125
#endif

#define  M_DIV  33*27*2/INPUT_CLK
#define USE_MIS2003_BLC   true// false //是否使用mis2003自带的暗电平校正功能
#define USE_MIS2003_RNC   true//是否使用mis2003自带的RNC使能

#define APC_EDGE_MAP1      0x720 //摄像头厂家提供的经验参数 代表斜率
#define IE_CFG0            0x06  //摄像头厂家提供的经验参数 代表截距

typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini mis2003_INI_REG[] = {

    //*****************MIS2003_1080P30**************//
    0x3006, 0x04,
    /* 0x3009, 0x03, */
#if USE_MIS2003_BLC==true
    0x3700, 0xDB, // BLC =  自动单帧，全局
#else
    0x3700, 0xDA,
#endif
    0x370C, 0x03,
    0x370E, 0x07,
#if USE_MIS2003_RNC==true
    0x3800, 0x00,// DRNC  ARNC全开
#else
    0x3800, 0x05,// DRNC  ARNC全关
#endif
    0x3B00, 0x01, // no overflow

    //  0x3B02, 0x10,

    //  0x3B04, 0x10,

    //  0x3B06, 0x10,

    //  0x3B08, 0x10,


    0x3400, 0x0b, //下降沿采样


    0x410c, 0x42,                    /////////////////////

    0x400E, 0x24,

    0x4018, 0x18,

    0x4020, 0x14,

    0x4026, 0x1E,

    0x402A, 0x26,

    0x402C, 0x3C,

    0x4030, 0x34,

    0x4034, 0x34,

    0x4036, 0xE0,  // sharding消除、第一个斜波拉长500ns、保持STA_DA与EN_RAMP间隔200ns

    0x4111, 0x0f,

    0x4110, 0x48,

    0x410E, 0x02, // 太阳黑子 LM模式



    0x3a00, 0x00,

    0x3a01, 0xfa,

    0x3a06, 0x00,

    0x3a07, 0xfa,

    0x3100, 0x04, //曝光时间调整寄存器

    0x3101, 0x64,

    //0x3102, 0x00,    //PGA调整寄存器 1x,2x,4x,8x

    //0x3103, 0x00,  //ADC 调整寄存器 1x～8(1+31/32)x

    //*******CIS_IN=27Mhz,PCLK=74.25Mhz******//
    0x3300, M_DIV, //  0x4a,
    0X3301, 0X02,
    0X3302, 0x02,
    0X3303, 0X04,
    //1080P30帧帧长度配置(2200*1125);
    0X3200, (FRAME_H >> 8) & 0xff, //0X04,
    0X3201,  FRAME_H & 0xff, // 0X65,

    0X3202, (FRAME_W >> 8) & 0xff, // 0x08,
    0X3203,  FRAME_W & 0xff, // 0X98,
    //1080P30帧有效像素画幅(1920*1080)
    0X3204, 0X00,
    0X3205, 0X04,
    0X3206, 0X04,
    0X3207, 0X3B,
    0X3208, 0X00,
    0X3209, 0X04,
    0X320A, 0X07,
    0X320B, 0X83,
    0x3D00, 0x01,//开启温度传感器
    /* 0x3c00,0x03, */
    /* 0x3006,0x00, */


};




unsigned char wrMis2003Reg(u16 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, MIS2003_WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __wend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID & 0xff)) {
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

}

unsigned char rdMis2003Reg(u16 regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, MIS2003_WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
        ret = 0;
        goto __rend;
    }

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID & 0xff)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, MIS2003_RDCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(10);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

}


/*************************************************************************************************
  sensor api
 *************************************************************************************************/

void mis2003_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;

    for (i = 0; i < sizeof(mis2003_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrMis2003Reg(mis2003_INI_REG[i].addr, mis2003_INI_REG[i].value);
    }

    return;
}


s32 mis2003_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    *width = MIS2003_OUTPUT_W;
    *height = MIS2003_OUTPUT_H;
    *frame_freq = MIS2003_FPS;
    return 0;
}


s32 mis2003_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 mis2003_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdMis2003Reg(0x3000, &pid);
        rdMis2003Reg(0x3001, &ver);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    if (pid != 0x20 && ver != 0x03) {
        puts("\n----not mis2003-----\n");
        return -1;
    }
    puts("\n----hello mis2003-----\n");
    return 0;
}

void mis2003_reset(u8 isp_dev)
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

s32 mis2003_check(u8 isp_dev, u32 _reset_gpio, u32 pwdn_gpio)
{
    puts("\n\n mis2003_check \n\n");
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

    mis2003_reset(isp_dev);

    if (0 != mis2003_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 mis2003_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n mis2003_init22 \n\n");

    mis2003_config_SENSOR(width, height, format, frame_freq);

    return 0;
}


static void set_again(u32 again)
{
    static u32 again_old = -1;
    if (again_old == again) {
        return ;
    }
    /* printf("again=%d\n ",again); */
    wrMis2003Reg(0x3102, again & (BIT(3) - 1));
    again_old = again;
}

static void set_dgain(u32 dgain)
{

    static u32 dgain_old = -1;
    if (dgain_old == dgain) {
        return ;
    }
    /* printf("dgain=%d\n ",dgain); */
    wrMis2003Reg(0x3103, dgain & 0x1f);
    dgain_old = dgain;
}

//q10
static void calc_gain(u32 gain, u8 *again, u16 *dgain)
{
    char i = 0;
    u16 temp = 0;
    temp = (gain) >> 10;


    if (temp == 0) {
        *dgain = temp;
        *again = temp;
        return;
    }
    if (temp >= 16) {
        temp = 15;
        *again = 3;
        *dgain = 32 - 1;
        return;
    }

    for (i = 3; i >= 0; i--) {
        if (temp & BIT(i)) {
            *again = i;
            /* *dgain=(gain-BIT(i)*1024)/BIT(i)/32;  */
            /* *dgain=((temp-BIT(i))*1024)>>(i+5);  */

            *dgain = (gain - BIT(i) * 1024) >> (i + 5);
            /* printf("isp gain =%d,again=%d dgain=%d add=%d \n",gain,BIT(i),*dgain,BIT(i)*1024+32*(*dgain)*BIT(i));  */
            return ;
        }

    }
}

static void set_shutter(u32 texp)
{
    static u32 ae_exp_line_old = -1;
    if (texp > FRAME_H - 1) {
        texp = FRAME_H - 1;
    }
    if (ae_exp_line_old == texp) {
        return ;
    }
    /* printf("texp=%d\n ",texp); */
    wrMis2003Reg(0x3100, (texp >> 8) & 0xff);
    wrMis2003Reg(0x3101, texp & 0xff);
    ae_exp_line_old = texp;
}

u32 mis2003_set_blc_k(void)
{
    u8 temper = 0;
    static u8 temper_old = -1;
    rdMis2003Reg(0x3d02, &temper);
    if (temper == temper_old) {
        return 0;
    } else {
        temper_old = temper;
    }
    /*  FB = AVG * float( ( (-6.2 * 0x3D02)+2822 )/2048) */
    u16 Ratio;
    if (temper <= 100) {
        Ratio = 2048;
    } else {
        // Ratio = (((2822 * 8) - (temper * 49))) >> 11 >> 3;
        // Ratio = (((2822 * 8) - (temper * 49))) >> 3; // by SONGBO. 2017.10.29.21:52

        Ratio = (((APC_EDGE_MAP1 * 8) - (temper * IE_CFG0))) >> 3; // by SONGBO. 2017.10.30.16:25
    } // 寄存器1表示截距，寄存器2表示斜率，寄存器2为u.3

    wrMis2003Reg(0x3701, (Ratio & 0xff00) >> 8);
    wrMis2003Reg(0x3702, Ratio & 0xff);
    wrMis2003Reg(0x3703, (Ratio & 0xff00) >> 8);
    wrMis2003Reg(0x3704, Ratio & 0xff);
    wrMis2003Reg(0x3705, (Ratio & 0xff00) >> 8);
    wrMis2003Reg(0x3706, Ratio & 0xff);
    wrMis2003Reg(0x3707, (Ratio & 0xff00) >> 8);
    wrMis2003Reg(0x3708, Ratio & 0xff);
    return 0;
}



u32 mis2003_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_align;
    u32 ratio;

    texp = exp_time_us * FRAME_H * MIS2003_FPS / 1000000;
    texp_align = (texp) * 1000 * 1000 / (FRAME_H * MIS2003_FPS);
    /* printf("exp_time_us=%d, texp=%d, texp_align=%d\n", exp_time_us, texp, texp_align); */
    if (texp_align < exp_time_us) {
        ratio = (exp_time_us) * (1 << 10) / texp_align;

    } else {
        ratio = (1 << 10);
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = (gain * ratio) >> 10;
    shutter->ae_exp_clk = 0;
    mis2003_set_blc_k();
    return 0;

}
const u32 m_pga_map[] = {
    /* 1, 2, 3.7, 6.8, 11.7 */
    64, 128, 246, 454 /* U.6 */
};


u32 mis2003_set_dc(isp_ae_shutter_t *shutter, u32 again, u32 dgain)
{
    static int dc_old = -1;
    int curr_sensor_total_gain = 0;
    /* curr_sensor_total_gain=shutter->ae_gain>>10; */
    curr_sensor_total_gain = ((m_pga_map[again] << ((dgain >> 5) & 0x3)) * (0x20 + (dgain & 0x1f))) * 256 /*u.11*/  /* U.8 */ >> 13;
    /* ADC和dgain都是定点小数，ADC小数位数是5位，dgain小数位数为8位 */
    s16 dc = ((~((curr_sensor_total_gain - 64) * 0xb8) + 1) >> 11) + 0x10;
    //t: 调整系数，经验值是0xb8#if 0
#if 1
    if (dc != dc_old) {
        dc_old = dc;
        wrMis2003Reg(0x3b01, (dc & 0xff00) >> 8);
        wrMis2003Reg(0x3b02, dc & 0xff);
        wrMis2003Reg(0x3b03, (dc & 0xff00) >> 8);
        wrMis2003Reg(0x3b04, dc & 0xff);
        wrMis2003Reg(0x3b05, (dc & 0xff00) >> 8);
        wrMis2003Reg(0x3b06, dc & 0xff);
        wrMis2003Reg(0x3b07, (dc & 0xff00) >> 8);
        wrMis2003Reg(0x3b08, dc & 0xff);
    }
    /* printf("mis_dc ==%d \n",dc);  */


#endif
    return 0;
}

u32 mis2003_set_shutter(isp_ae_shutter_t *shutter)
{

    u32 again, dgain;

    calc_gain((shutter->ae_gain), &again, &dgain);
    set_again(again);
    set_dgain(dgain);

    /* mis2003_set_dc(shutter,again,dgain); */
    set_shutter(shutter->ae_exp_line);
    mis2003_set_dc(shutter, again, dgain);
    return 0;
}

void mis2003_sleep()
{


}

void mis2003_wakeup()
{


}

void mis2003_W_Reg(u16 addr, u16 val)
{
    /* printf("update reg%x with %x\n", addr, val); */
    wrMis2003Reg((u16)addr, (u8)val);
}
u16 mis2003_R_Reg(u16 addr)
{
    u8 val;
    rdMis2003Reg((u16)addr, &val);
    return val;
}

u8 mis2003_fps()
{
    return 0;//30fps
}
u8 mis2003_valid_signal(void)
{
    return 1;//valid
}

static u8 mis2003_mode_det(void)
{
    return 0;
}


REGISTER_CAMERA(mis2003) = {
    .logo 				= 	"mis2003",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			=   SEN_IN_FORMAT_GRBG,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps                =   MIS2003_FPS,
    .sen_size 			= 	{MIS2003_OUTPUT_W, MIS2003_OUTPUT_H},
    .isp_size 			= 	{MIS2003_OUTPUT_W, MIS2003_OUTPUT_H},


    .ops                =   {
        .avin_fps           = NULL, // mis2003_fps,
        .avin_valid_signal  = NULL, //  mis2003_valid_signal,
        .avin_mode_det      = NULL, //  mis2003_mode_det,
        .sensor_check 		= 	mis2003_check,
        .init 		        = 	mis2003_init,
        .set_size_fps 		=	mis2003_set_output_size,
        .power_ctrl         =   mis2003_power_ctl,

        .get_ae_params  	=	mis2003_get_ae_params,
        .get_awb_params 	=	mis2003_get_awb_params,
        .get_iq_params 	   	=	mis2003_get_iq_params,

        .sleep 		        = NULL, //	mis2003_sleep,
        .wakeup 		    = NULL, //	mis2003_wakeup,
        .write_reg 		    =	mis2003_W_Reg,
        .read_reg 		    =	mis2003_R_Reg,

    }
};

