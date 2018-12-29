
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "sc2363_mipi.h"
#include "asm/isp_alg.h"
static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

extern void *sc2363_mipi_get_ae_params();
extern void *sc2363_mipi_get_awb_params();
extern void *sc2363_mipi_get_iq_params();
extern void sc2363_mipi_ae_ev_init(u32 fps);

#define INPUT_CLK  24
#define PCLK  78

#define LINE_LENGTH_CLK     0x820
#define FRAME_LENGTH30        1250
#define FRAME_LENGTH25        1500
#define ROW_TIME_NS         26667

#define LINE_LENGTH_CLK_25FPS     0xb40
#define LINE_LENGTH_CLK_20FPS     0xe10
#define LINE_LENGTH_CLK_15FPS     0x12c0
#define ROW_TIME_NS_25FPS         35556
#define ROW_TIME_NS_20FPS         44445
#define ROW_TIME_NS_15FPS         59260

static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expline = -1;
static u32 cur_line_length_clk = -1;

static u32 line_length_clk = LINE_LENGTH_CLK;

static void *iic = NULL;
static u32 uframelen = 1250;
#define WRCMD 0x60
#define RDCMD 0x61


typedef struct {
    u16 addr;
    u8 value;
} Sensor_reg_ini;

Sensor_reg_ini sc2363_mipi_INI_REG[] = {
    0x0103, 0x01,  //74.25 sysclk 297M cntclk
    0x0100, 0x00,

//close mipi
//0x3018,0x1f,
//0x3019,0xff,
//0x301c,0xb4,

    0x320c, 0x0a,
    0x320d, 0x50, //a80->a50

    0x3e01, 0x23,

    0x363c, 0x05, //04
    0x3635, 0xa8, //c0
    0x363b, 0x0d, //0d

    0x3620, 0x08,
    0x3622, 0x02,
    0x3635, 0xc0, //
    0x3908, 0x10,

    0x3624, 0x08, //count_clk inv  need debug  flash row in one channel

    0x5000, 0x06, //rts column test
    0x3e06, 0x00,
    0x3e08, 0x03,
    0x3e09, 0x10,
    0x3333, 0x10,
    0x3306, 0x7e,

//0x3e08,0x1f,
//0x3e09,0x1f,
//0x3e06,0x03,
    0x3902, 0x05,
//0x3909,0x01,  //auto blc
//0x390a,0xf5,  //auto blc

    0x3213, 0x08,

    0x337f, 0x03, //new auto precharge  330e in 3372   [7:6] 11: close div_rst 00:open div_rst
    0x3368, 0x04,
    0x3369, 0x00,
    0x336a, 0x00,
    0x336b, 0x00,
    0x3367, 0x08,
    0x330e, 0x30,

    0x3366, 0x7c, // div_rst gap


    0x3633, 0x42,

    0x330b, 0xe0,

    0x3637, 0x57,

    0x3302, 0x1f, // adjust the gap betwen first and second cunt_en pos edage to even times the clk
    0x3309, 0xde, // adjust the gap betwen first and second cunt_en pos edage to even times the clk
//0x303f,0x81, // pclk sel pll_sclk_dig_div  20171205



//leage current

    0x3907, 0x00,
    0x3908, 0x61,
    0x3902, 0x45,
    0x3905, 0xb8,
//0x3904,0x06, //10.18
    0x3e01, 0x8c,
    0x3e02, 0x10,
    0x3e06, 0x00,

    0x3038, 0x48,
    0x3637, 0x5d,
    0x3e06, 0x00,


//0921
    0x3908, 0x11,

    0x335e, 0x01, //ana dithering
    0x335f, 0x03,
    0x337c, 0x04,
    0x337d, 0x06,
    0x33a0, 0x05,
    0x3301, 0x04,

    0x3633, 0x4f, //prnu

    0x3622, 0x06, //blksun
    0x3630, 0x08,
    0x3631, 0x84,
    0x3306, 0x30,

    0x366e, 0x08, // ofs auto en [3]
    0x366f, 0x22, // ofs+finegain  real ofs in 0x3687[4:0]

    0x3637, 0x59, // FW to 4.6k //9.22

    0x3320, 0x06, // New ramp offset timing
//0x3321,0x06,
    0x3326, 0x00,
    0x331e, 0x11,
    0x331f, 0xc1,
    0x3303, 0x20,
    0x3309, 0xd0,
    0x330b, 0xbe,
    0x3306, 0x36,

    0x3635, 0xc2, //TxVDD,HVDD
    0x363b, 0x0a,
    0x3038, 0x88,

//9.22
    0x3638, 0x1f, //ramp_gen by sc  0x30
    0x3636, 0x25, //
    0x3625, 0x02,
    0x331b, 0x83,
    0x3333, 0x30,

//10.18
    0x3635, 0xa0,
    0x363b, 0x0a,
    0x363c, 0x05,

    0x3314, 0x13, //preprecharge

//20171101 reduce hvdd pump lighting
    0x3038, 0xc8, // high pump clk,low lighting
    0x363b, 0x0b, //high hvdd ,low lighting
    0x3632, 0x18, //large current,low ligting  0x38 (option)

//20171102 reduce hvdd pump lighting
    0x3038, 0xff, // high pump clk,low lighting

    0x3639, 0x09,
    0x3621, 0x28,
    0x3211, 0x0c,

//20171106
    0x366f, 0x26,

//20171121
    0x366f, 0x2f,
    0x3320, 0x01,
    0x3306, 0x48,
    0x331e, 0x19,
    0x331f, 0xc9,

    0x330b, 0xd3,
    0x3620, 0x28,

//20171122
    0x3309, 0x60,
    0x331f, 0x59,
    0x3308, 0x10,
    0x3630, 0x0c,

//digital ctrl
    0x3f00, 0x07, // bit[2] = 1
    0x3f04, 0x05,
    0x3f05, 0x04, // hts / 2 - 0x24

    0x3802, 0x01,
    0x3235, 0x08,
    0x3236, 0xc8, // vts x 2 - 2

//20171127
    0x3630, 0x1c,

//20171129

    0x320c, 0x08, //2080 hts
    0x320d, 0x20,

    0x320e, 0x04, //1250 vts
    0x320f, 0xe2,

//digital ctrl
    0x3f04, 0x03,
    0x3f05, 0xec, // hts / 2 - 0x24

    0x3235, 0x09,
    0x3236, 0xc2, // vts x 2 - 2

    0x3e01, 0x9c,
    0x3e02, 0x00,

    0x3039, 0x54, //vco 390M
    0x303a, 0xb3, //sysclk  78M
    0x303b, 0x06,
    0x303c, 0x0e,
    0x3034, 0x01, //cunt clk 312M
    0x3035, 0x9b,

//mipi
    0x3018, 0x33, //[7:5] lane_num-1
    0x3031, 0x0a, //[3:0] bitmode
    0x3037, 0x20, //[6:5] bitsel
    0x3001, 0xFE, //[0] c_y

//lane_dis of lane3~8
//0x3018,0x12,
//0x3019,0xfc,

    0x4603, 0x00, //[0] data_fifo mipi mode
    0x4837, 0x19, //[7:0] pclk period * 2
    0x4827, 0x48, //[7:0] hs_prepare_time[7:0]

    0x33aa, 0x10, //save power

//20171208  logical   inter
    0x3670, 0x04, //0x3631 3670[2] enable  0x3631 in 0x3682
    0x3677, 0x84, //gain<gain0
    0x3678, 0x88, //gain0=<gain<gain1
    0x3679, 0x88, //gain>=gain1
    0x367e, 0x08, //gain0 {3e08[4:2],3e09[3:1]}
    0x367f, 0x28, //gain1

    0x3670, 0x0c, //0x3633 3670[3] enable  0x3633 in 0x3683     20171227
    0x3690, 0x34, //gain<gain0
    0x3691, 0x11, //gain0=<gain<gain1
    0x3692, 0x42, //gain>=gain1
    0x369c, 0x08, //gain0{3e08[4:2],3e09[3:1]}
    0x369d, 0x28, //gain1

    0x360f, 0x01, //0x3622 360f[0] enable  0x3622 in 0x3680
    0x3671, 0xc6, //gain<gain0
    0x3672, 0x06, //gain0=<gain<gain1
    0x3673, 0x16, //gain>=gain1
    0x367a, 0x28, //gain0{3e08[4:2],3e09[3:1]}
    0x367b, 0x3f, //gain1

//20171218
    0x3802, 0x00,

//20171225 BLC power save mode
    0x3222, 0x29,
    0x3901, 0x02,
    0x3905, 0x98,

//20171227
    0x3e1e, 0x34, // digital finegain enable

//20180113
    0x3314, 0x08,

//init
    0x3301, 0x06,
    0x3306, 0x48,
    0x3632, 0x08,
    0x3e00, 0x00,
    0x3e01, 0x4d,
    0x3e02, 0xe0,
    0x3e03, 0x03,
    0x3e06, 0x00,
    0x3e07, 0x80,
    0x3e08, 0x03,
    0x3e09, 0x10,

    0x0100, 0x01,
};


unsigned char wrSc2363_mipi_Reg(u16 regID, unsigned char regDat)
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
    if (ret == 0) {
        printf("wreg iic fail\n");
    }
    return ret;

}

unsigned char rdSc2363_mipi_Reg(u16 regID, unsigned char *regDat)
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

void sc2363_mipi_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    u8 v;


    sc2363_mipi_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(sc2363_mipi_INI_REG) / sizeof(Sensor_reg_ini); i++) {
        wrSc2363_mipi_Reg(sc2363_mipi_INI_REG[i].addr, sc2363_mipi_INI_REG[i].value);
    }
    /*for(i=0;i<sizeof(sc2363_mipi_INI_REG)/sizeof(Sensor_reg_ini);i++)
    {
        rdSc2363_mipi_Reg(sc2363_mipi_INI_REG[i].addr,&v);
        printf("0x%4x,0x%2x\n", sc2363_mipi_INI_REG[i].addr, v);
    }*/

    sc2363_mipi_ae_ev_init(*frame_freq);
    if (*frame_freq == 25) {
        wrSc2363_mipi_Reg(0x320e, 0x05);
        wrSc2363_mipi_Reg(0x320f, 0xdc);
        uframelen = 1500;
    } else {
        uframelen = 1250;
    }
    *format = SEN_IN_FORMAT_BGGR;
    return;
}


s32 sc2363_mipi_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    *width = SC2363_MIPI_OUTPUT_W;
    *height = SC2363_MIPI_OUTPUT_H;

    return 0;
}


s32 sc2363_mipi_power_ctl(u8 isp_dev, u8 is_work)
{

    return 0;
}

s32 sc2363_mipi_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    for (i = 0; i < 3; i++) { //
        rdSc2363_mipi_Reg(0x3107, &pid);
        rdSc2363_mipi_Reg(0x3108, &ver);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

    if (pid != 0x22 && ver != 0x32) {
        puts("\n----not sc2363_mipi-----\n");
        return -1;
    }
    puts("\n----hello sc2363_mipi-----\n");
    return 0;
}

void sc2363_mipi_reset(u8 isp_dev)
{
    u32 reset_gpio;
    u32 pwdn_gpio;

    if (isp_dev == ISP_DEV_0) {
        reset_gpio = reset_gpios[0];
        pwdn_gpio = pwdn_gpios[0];
    } else {
        reset_gpio = reset_gpios[1];
        pwdn_gpio = pwdn_gpios[1];
    }
    gpio_direction_output(reset_gpio, 0);
    gpio_direction_output(pwdn_gpio, 0);

    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    delay(40000);

    printf("pwdn_gpio=%d\n", pwdn_gpio);
    gpio_direction_output(pwdn_gpio, 1);
}


static u8 cur_sensor_type = 0xff;

s32 sc2363_mipi_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{

    puts("\n\n sc2363_mipi_check \n\n");
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

    sc2363_mipi_reset(isp_dev);

    if (0 != sc2363_mipi_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

//    strcpy(sensor_logo, "SR013");
    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 sc2363_mipi_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n sc2363_mipi_init22 \n\n");

    sc2363_mipi_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

static void set_again(u32 again)
{
    return;
    if (cur_again == again) {
        return;
    }
    cur_again  = again;
    //printf("again = %04x\n",again);
    wrSc2363_mipi_Reg(0x3e08, ((again >> 6) & 0x1c) | 0x03);
    wrSc2363_mipi_Reg(0x3e09, again & 0xff);
    return;
}

static void set_dgain(u32 dgain, u32 gain)
{
    if (cur_dgain == gain) {
        return;
    }
    cur_dgain  = gain;
    // printf("dgain = %04x\n",gain);
    wrSc2363_mipi_Reg(0x3e08, (gain >> 14) & 0xff);
    wrSc2363_mipi_Reg(0x3e09, (gain >> 6) & 0xff);

    wrSc2363_mipi_Reg(0x3812, 0x00);

    if (cur_expline > 160) {
        wrSc2363_mipi_Reg(0x3314, 0x04);
    }
    if (cur_expline < 80) {
        wrSc2363_mipi_Reg(0x3314, 0x14);
    }
    gain >>= 10;
    if (gain < 2) {
        wrSc2363_mipi_Reg(0x3301, 0x06);
        wrSc2363_mipi_Reg(0x3606, 0x48);
        wrSc2363_mipi_Reg(0x3632, 0x08);
        wrSc2363_mipi_Reg(0x3812, 0x30);
        return;
    }
    if (gain < 4) {
        wrSc2363_mipi_Reg(0x3301, 0x14);
        wrSc2363_mipi_Reg(0x3606, 0x48);
        wrSc2363_mipi_Reg(0x3632, 0x08);
        //wrSc2363_mipi_Reg(0x3314,0x02);
        wrSc2363_mipi_Reg(0x3812, 0x30);
        return;
    }
    if (gain < 8) {
        wrSc2363_mipi_Reg(0x3301, 0x18);
        wrSc2363_mipi_Reg(0x3606, 0x48);
        wrSc2363_mipi_Reg(0x3632, 0x08);
        //wrSc2363_mipi_Reg(0x3314,0x02);
        wrSc2363_mipi_Reg(0x3812, 0x30);
        return;
    }
    if (gain < 15) {
        wrSc2363_mipi_Reg(0x3301, 0x13);
        wrSc2363_mipi_Reg(0x3606, 0x48);
        wrSc2363_mipi_Reg(0x3632, 0x08);
        //wrSc2363_mipi_Reg(0x3314,0x02);
        wrSc2363_mipi_Reg(0x3812, 0x30);
        return;
    }
    if (gain < 31) {
        wrSc2363_mipi_Reg(0x3301, 0xa1);
        wrSc2363_mipi_Reg(0x3606, 0x88);
        wrSc2363_mipi_Reg(0x3632, 0x48);
        //wrSc2363_mipi_Reg(0x3314,0x02);
        wrSc2363_mipi_Reg(0x3812, 0x30);
        return;
    }
    //if (gain<31)
    {
        wrSc2363_mipi_Reg(0x3301, 0xa1);
        wrSc2363_mipi_Reg(0x3606, 0x78);
        wrSc2363_mipi_Reg(0x3632, 0x78);
        //wrSc2363_mipi_Reg(0x3314,0x02);
        wrSc2363_mipi_Reg(0x3812, 0x30);
        return;
    }
}



static u32 convertgain2cfg(u32 val)
{
    u32 course = 1;
    u32 finegain;

    while (val >= 2048) {
        course *= 2;
        val /= 2;
    }
    finegain = 0x10 + val / 128;
    return ((course - 1) << 8) + finegain;
}
static void updateforgain(u32 gain)
{
    wrSc2363_mipi_Reg(0x3903, 0x84);
    wrSc2363_mipi_Reg(0x3903, 0x04);
    wrSc2363_mipi_Reg(0x3812, 0x00);

    if (gain < 2) {
        wrSc2363_mipi_Reg(0x3301, 0x03);
        wrSc2363_mipi_Reg(0x3631, 0x84);
        wrSc2363_mipi_Reg(0x3620, 0x28);
        return;
    }
    if (gain < 4) {
        wrSc2363_mipi_Reg(0x3301, 0x08);
        wrSc2363_mipi_Reg(0x3631, 0x86);
        wrSc2363_mipi_Reg(0x3620, 0x08);
        return;
    }
    if (gain < 8) {
        wrSc2363_mipi_Reg(0x3301, 0x09);
        wrSc2363_mipi_Reg(0x3631, 0x86);
        wrSc2363_mipi_Reg(0x3620, 0x08);
        return;
    }
    wrSc2363_mipi_Reg(0x3301, 0x20);
    wrSc2363_mipi_Reg(0x3631, 0x86);
    wrSc2363_mipi_Reg(0x3620, 0x08);

}
#define MAX_AGAIN (15*1024 +512)
#define MAX_DGAIN 12*1024
static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{
    int i;
    u32 reg0, reg1, reg2;
    u32 dgain, again;

    if (gain < 1024) {
        gain = 1024;
    }
    if (gain > 127 * 1024) {
        gain = 127 * 1024;
    }

    dgain = gain * 1024 / MAX_AGAIN;
    if (dgain < 1024) {
        dgain = 1024;
    }
    if (dgain > MAX_DGAIN) {
        dgain = MAX_DGAIN;
    }

    again = gain * 1024 / dgain;

    if (again < 1024) {
        again = 1024;
    }
    if (again > MAX_AGAIN) {
        again = MAX_AGAIN;
    }

    *_again = convertgain2cfg(again);
    *_dgain = convertgain2cfg(dgain);;

    //printf("gain = %d dg = %d; ag= %d; \n ",gain,dgain,again);
}


static void set_shutter(u32 texp)
{
    if (cur_expline == texp) {
        //return;
    }
    cur_expline  = texp;

    wrSc2363_mipi_Reg(0x3e00, (texp >> 12) & 0xff);
    wrSc2363_mipi_Reg(0x3e01, (texp >> 4) & 0xff);
    wrSc2363_mipi_Reg(0x3e02, (texp << 4) & 0xf0);
}


u32 sc2363_mipi_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
    u32 texp_align;
    u32 ratio;
    u32 fLen;
    static bLow = 0;

    fLen = uframelen;
    if (exp_time_us == 60000) {
        fLen = 2500;
        if (bLow == 0) {
            bLow = 1;
            wrSc2363_mipi_Reg(0x320e, 0x09);
            wrSc2363_mipi_Reg(0x320f, 0xc4);
        }
    } else {
        if (bLow == 1) {
            bLow = 0;
            wrSc2363_mipi_Reg(0x320e, uframelen >> 8);
            wrSc2363_mipi_Reg(0x320f, uframelen & 0x0ff);
        }
    }
    texp = exp_time_us * 78 * 2 / LINE_LENGTH_CLK;

    if (texp < 1) {
        texp = 1;
    }
    if (texp > fLen * 2 - 4) {
        texp = fLen * 2 - 4;
    }
    texp_align = (texp) * LINE_LENGTH_CLK / (78 * 2);

    if (texp_align < exp_time_us) {
        ratio = (exp_time_us) * (1 << 10) / texp_align;
        //printf("ratio = %d\n",ratio);
    } else {
        ratio = (1 << 10);
    }

    shutter->ae_exp_line =  texp;
    shutter->ae_gain = (gain * ratio) >> 10;
    shutter->ae_exp_clk = 0;

//   printf("exp_time_us=%d, texp=%d, gain=%d->%d\n", exp_time_us, texp, gain,shutter->ae_gain);
    return 0;

}

u32 sc2363_mipi_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;

    calc_gain((shutter->ae_gain), &again, &dgain);
    set_shutter(shutter->ae_exp_line);
    set_again(again);
    set_dgain(dgain, shutter->ae_gain);

    return 0;
}


void sc2363_mipi_sleep()
{


}

void sc2363_mipi_wakeup()
{


}

void sc2363_mipi_W_Reg(u16 addr, u16 val)
{
    /*printf("update reg%x with %x\n", addr, val);*/
    wrSc2363_mipi_Reg((u16)addr, (u8)val);
}
u16 sc2363_mipi_R_Reg(u16 addr)
{
    u8 val;
    rdSc2363_mipi_Reg((u16)addr, &val);
    return val;
}

REGISTER_CAMERA(sc2363_mipi) = {
    .logo 				= 	"sc2363",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_BGGR,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_CSI2,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_CSI2_2_LANE,
    .fps         		= 	25,

    .sen_size 			= 	{SC2363_MIPI_OUTPUT_W, SC2363_MIPI_OUTPUT_H},
    .isp_size 			= 	{SC2363_MIPI_OUTPUT_W, SC2363_MIPI_OUTPUT_H},

    .cap_fps         		= 	25,
    .sen_cap_size 			= 	{SC2363_MIPI_OUTPUT_W, SC2363_MIPI_OUTPUT_H},
    .isp_cap_size 			= 	{SC2363_MIPI_OUTPUT_W, SC2363_MIPI_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	sc2363_mipi_check,
        .init 		        = 	sc2363_mipi_init,
        .set_size_fps 		=	sc2363_mipi_set_output_size,
        .power_ctrl         =   sc2363_mipi_power_ctl,

        .get_ae_params  	=	sc2363_mipi_get_ae_params,
        .get_awb_params 	=	sc2363_mipi_get_awb_params,
        .get_iq_params 	    	=	sc2363_mipi_get_iq_params,

        .sleep 		        =	sc2363_mipi_sleep,
        .wakeup 		    =	sc2363_mipi_wakeup,
        .write_reg 		    =	sc2363_mipi_W_Reg,
        .read_reg 		    =	sc2363_mipi_R_Reg,

    }
};



