
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "j1021_dvp.h"
#include "asm/isp_alg.h"

static u32 reset_gpios[2] = {-1, -1};
static u32 pwdn_gpios[2] = {-1, -1};

extern void *j1021_dvp_get_ae_params();
extern void *j1021_dvp_get_awb_params();
extern void *j1021_dvp_get_iq_params();
extern void j1021_dvp_ae_ev_init(u32 fps);

unsigned char wr_j1021_reg(u8 regID, unsigned char regDat);
unsigned char rd_j1021_reg(u8 regID, unsigned char *regDat);
extern void delay_2ms(int cnt);



#define ROWTIME     0xc18 //3096
#define ROW_TIME_NS         43010
#define INPUT_CLK  24
#define PCLK  72000


#define VBLANK_30FPS    0x2f // 0x25
#define VBLANK_25FPS    0xca
#define VBLANK_20FPS    0x1b2
#define VBLANK_16FPS    0x2d5

#define MAX_EXP_LINE_30FPS  775
#define MAX_EXP_LINE_25FPS  930
#define MAX_EXP_LINE_20FPS  1162
#define MAX_EXP_LINE_16FPS  1453



static u32 cur_again = -1;
static u32 cur_dgain = -1;
static u32 cur_expline = -1;

static u32 gRowTime = ROWTIME;
static u32 gVblank = VBLANK_25FPS;// VBLANK_30FPS;
static u32 gVblank_pre = 0;// VBLANK_30FPS;
static u32 gExplineMax = MAX_EXP_LINE_25FPS;
static u32 gTexp_pre = 0;
static u32 gTexp_mck_pre = 0;

static void *iic = NULL;

#define WRCMD 0x64
#define RDCMD 0x65

#define FD_GAIN_SWITCH  0

//fd_gain contex
#define FD_TH  		 	5457  //5.33x
#define FD_TL  		 	5400  //4x

//Limit AGain to 1-13
#define VREFH_LIMIT_H 	0x7f
#define VREFH_LIMIT_L 	0x0c


unsigned int blcc_coef;
unsigned int blcc_base;
unsigned int fd_gain;
unsigned int min_dgain;
unsigned int vthe;


const unsigned short sensor_info[][8]=
{//index 0                       1               2                    3                          4                               5                             6                             7
	//vid_h,   type   pid,   FD_gain,  min_dgain     blcc_coef,  blcc_base,	 vthc
	{0x0701,   'C',   0x08,  4408,     0x0133,       48,         11,		 0x0f},//BG0701C
	{0x0701,   'D',   0x09,  4408,     0x0133,       48,         11,		 0x0f},//BG0701D
	{0x0701,   'E',   0x0a,  4408,     0x0133,       58,         11,		 0x0f},//BG0701E
	{0x0701,   'F',   0x0b,  4408,     0x0133,       48,         11,		 0x0f},//BG0701F
	{0x0701,   'G',   0x0c,  4408,     0x0160,       48,         11,		 0x0c},//BG0701G
	{0x0701,   'H',   0x0d,  4408,     0x0160,       48,         11,		 0x0e},//BG0701H
	{0x0701,   ' ',   0xff,  4408,     0x0160,       48,         11,		 0x0e},//BG0701其他型号


	{0x0707,   'C',   0x08,  3480,     0x0133,       91,         6,			 0x0f},//BG0703C
	{0x0707,   'D',   0x09,  3480,     0x0133,       91,         6,		 	 0x0f},//BG0703D
	{0x0707,   'E',   0x0a,  3480,     0x0133,       91,         6,		 	 0x0f},//BG0703E
	{0x0707,   'F',   0x0b,  3480,     0x0133,       91,         6,		 	 0x0f},//BG0703F
	{0x0707,   'G',   0x0c,  3480,     0x0160,       91,         6,		 	 0x0c},//BG0703G
	{0x0707,   ' ',   0xff,  3480,     0x0160,       91,         6,		 	 0x0c},//BG0703其他型号

	{0x55AA,   'X',   0xFF,  0000,     0xFFFF,       00,         00,		 0xff} //结束标志行
};

void sensor_para_updata(void)
{

	unsigned char tmp,tmp1;
	unsigned short sensor_vid;
	unsigned char  sensor_pid;
	unsigned char  n;
	unsigned char  page;
	static unsigned char para_updata_flag=0;

	//函数只第一次执行有效
	if(para_updata_flag != 0){
		return;
	}
	rd_j1021_reg(0xF0,&page);

	// Read VID & PID
	wr_j1021_reg(0xF0, 0x00);
	rd_j1021_reg(0x00,&tmp);
	rd_j1021_reg(0x01,&tmp1);

	sensor_vid = tmp << 8 | tmp1;

	rd_j1021_reg(0x45,&sensor_pid);
	//sensor_pid = 0x0a;
//	puts("\n sensor_init() : VID=0x%04X PID=0x%02X\n", sensor_vid, sensor_pid);

	n = 0;
	while(sensor_info[n][0] != 0x55AA){
		if((sensor_vid == sensor_info[n][0]) && (sensor_pid == (sensor_info[n][2]&0xFF))){
			fd_gain   = sensor_info[n][3];
			min_dgain = sensor_info[n][4];
			blcc_coef = sensor_info[n][5];
			blcc_base = sensor_info[n][6];
			if(sensor_info[n][7] != 0xff){
				wr_j1021_reg(0xF0, 0x00);
		  		wr_j1021_reg(0x1E, (sensor_info[n][7]&0xFF));
				vthe = sensor_info[n][7];
			}
			para_updata_flag = 1;
			break;
		}
		n++;
	}
	//如果未找到主ID和子ID都匹配的sensor，则使用主ID默认配置
	n = 0;
	if(para_updata_flag == 0){
		while(sensor_info[n][0] != 0x55AA){
			if((sensor_vid == sensor_info[n][0]) && (0xff == (sensor_info[n][2]&0xFF))){
				fd_gain   = sensor_info[n][3];
				min_dgain = sensor_info[n][4];
				blcc_coef = sensor_info[n][5];
				blcc_base = sensor_info[n][6];
				if(sensor_info[n][7] != 0xff){
					wr_j1021_reg(0xF0, 0x00);
		  			wr_j1021_reg(0x1E, (sensor_info[n][7]&0xFF));
					vthe = sensor_info[n][7];
				}
				para_updata_flag = 1;
				break;
			}
			n++;
		}
	}
	if(para_updata_flag == 0){
//		puts("Sensor updata paradata Failure!!!,Pleace check\r\n");
	}
//	puts("FD_Gain=%d,min_dgain=0x%x,blcc_coef=%d,blcc_base=%d,vthe=0x%x\r\n",fd_gain,min_dgain,blcc_coef,blcc_base,vthe);
	wr_j1021_reg(0xF0, page);
}


typedef struct {
    u8 addr;
    u8 value;
} sensor_ini_regs_t;

sensor_ini_regs_t j1021_dvp_ini_regs[] = {

        0xf0, 0x00,// //select page0
        0x1c, 0x01,
        0x50, 0x00,
        0x89, 0x21,//internal vddpix off
        0xb9, 0x21,// //MGC//{0xb9 ,0x22},// //MGC

        0x03, 0x49, //{0x03, 0x47},  // remove red line

        0x06, 0x05,//
        0x07, 0x00,// Size W
        0x08, 0x02,//
        0x09, 0xD0,// Size W

        0x0e, 0x04,//
        0x0f, 0x08,//17,// //row time
        0x14, 0x03,// //TXB ON *
        0x1E, 0x0f,// //VTH 3.8V please check the voltage
        0x20, 0x02,// //row reverse
        0x21, 0x00,// //Vblank
        0x22, 0x2f,// //Vblank // 0x24
        0x28, 0x00,// //RAMP1 ONLY
        0x29, 0x18,// //RSTB =1us
        0x2a, 0x18,// //TXB = 1us
        0x2d, 0x01,// //
        0x2e, 0xB0,// //ibias_cnten_gap=17u
        0x30, 0x18,// //rstb_cmprst_gap=1u
        0x34, 0x20,// //tx_ramp2=32 CLKIN cycle*
        0x36, 0x01,// //read start position
        0x38, 0x03,//
        0x39, 0xfd,//
        0x3a, 0x03,//
        0x3b, 0xfa,//
        0x50, 0x00,//

        0x53, 0x76,//  PLM = reg53<6:0>
        0x54, 0x03,//  PLN = reg54<5:0>
        0x52, 0xdd,//  PLK = reg52<1:0>
        0x60, 0x00,// //row refresh mode
        0x6d, 0x01,// //pll=288M pclk=72M  (when clkin=24M), PCLK=PLLCLK>>(2-reg6d)
        0x64, 0x02,//
        0x65, 0x00,// //RAMP1 length=200
        0x67, 0x05,//
        0x68, 0xff,// //RAMP1 length=5ff
        0x87, 0xaf,// //votlgate of vbg-i
        0x1d, 0x01,// //restart

        0xf0, 0x01,//
        0xc8, 0x04,//
        0xc7, 0xaa,//
        0xe0, 0x01,//
        0xe1, 0x04,//
        0xe2, 0x03,//
        0xe3, 0x02,//
        0xe4, 0x01,//
        0xe5, 0x01,// //vcm_comp =2.56V
        0xb4, 0x01,// //row noise remove on*
        0x20, 0x00,////blcc off
        0x31, 0x00,////blcc target upper high
        0x32, 0x38,//
        0x33, 0x00,////blcc target upper low
        0x34, 0x35,//
        0x35, 0x00,////blcc target lower high
        0x36, 0x33,//
        0x37, 0x00,////blcc target lower low
        0x38, 0x30,//
        0x20, 0x00,// //blcc on
        0x4e, 0x00,//
        0x4f, 0x00,// //digital offset
        0xf1, 0x07,// //dpc off
        0xf0, 0x00,//
        0x7f, 0x00,// //cmp current
        0x81, 0x09,// //dot_en=1,vrst=vth,vtx=vth
        0x82, 0x11,// //bandgap current & ramp current
        0x83, 0x01,// //pixel current
        0x84, 0x07,// //check rst voltage
        0x89, 0x21,// //internal vddpix off
        0x88, 0x03,// //pclk phase  0-7
        0x8a, 0x05,// //pclk drv 0-7
        0x8c, 0x05,// //data drv 0-7

        0x0c, 0x00,
        0x0d, 0x32,
        0x26, 0x00,//
        0x27, 0x00,//

        0xb0, 0x01,//
        0xb1, 0x7f,//
        0xb2, 0x01,//
        0xb3, 0x7f,// //analog gain=1X
        0xb4, 0x11,//
        0xb5, 0x11,//
        0xb6, 0x11,//
        0xb7, 0x01,//
        0xb8, 0x00,// //digital gain=1X
        0xbf, 0x0c,//
        0x8e, 0x00,// //OEN
        0x8d, 0x00,// //OEN

#if 0
        0xf0, 0x00,
        0xb9, 0x21,
        0xf0, 0x01,
        0xc8, 0x04,
#endif
        0x1d, 0x01,//

};


unsigned char wr_j1021_reg(u8 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(100);
//
//    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
//        ret = 0;
//        goto __wend;
//    }
//
//    delay(100);

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

unsigned char rd_j1021_reg(u8 regID, unsigned char *regDat)
{
    u8 ret = 1;
    dev_ioctl(iic, IIC_IOCTL_START, 0);
    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(100);

//    if (dev_ioctl(iic, IIC_IOCTL_TX, regID >> 8)) {
//        ret = 0;
//        goto __rend;
//    }

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

s32 j1021_dvp_set_output_size(u16 *width, u16 *height, u8 *frame_freq)
{
    return 0;
}

void j1021_dvp_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    u32 i;
    //u8 v;


    j1021_dvp_set_output_size(width, height, frame_freq);

    for (i = 0; i < sizeof(j1021_dvp_ini_regs) / sizeof(sensor_ini_regs_t); i++) {
        wr_j1021_reg(j1021_dvp_ini_regs[i].addr, j1021_dvp_ini_regs[i].value);
    }

    j1021_dvp_ae_ev_init(*frame_freq);

    cur_again = -1;
    cur_dgain = -1;
    cur_expline = -1;
    return;
}



s32 j1021_dvp_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

s32 j1021_dvp_ID_check(void)
{
    u8 pid = 0x00;
    u8 ver = 0x00;
    u8 i ;

    wr_j1021_reg(0xf0,0); // ppk add on 20171121
    for (i = 0; i < 3; i++) { //
        rd_j1021_reg(0x00, &pid);
        rd_j1021_reg(0x01, &ver);
    }

    puts("Sensor PID \n");
    put_u8hex(pid);
    put_u8hex(ver);
    puts("\n");

   // if (pid != 0x22 && ver != 0x35)
    if (pid != 0x07 && ver != 0x07) {
        puts("\n----not j1021_dvp-----\n");
        return -1;
    }
    puts("\n----hello j1021_dvp-----\n");
    return 0;
}

void j1021_dvp_reset(u8 isp_dev)
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

    /*printf("pwdn_gpio=%d\n", pwdn_gpio);*/
    gpio_direction_output(pwdn_gpio, 0);

    gpio_direction_output(reset_gpio, 0);
    delay(40000);
    gpio_direction_output(reset_gpio, 1);
    gpio_direction_output(pwdn_gpio, 1);
    delay(40000);
}


static u8 cur_sensor_type = 0xff;

s32 j1021_dvp_check(u8 isp_dev, u32 reset_gpio, u32 pwdn_gpio)
{

    puts("\n\n j1021_dvp_check \n\n");
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

    j1021_dvp_reset(isp_dev);

    if (0 != j1021_dvp_ID_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }

    cur_sensor_type = isp_dev;

    return 0;
}

void resetStatic();
s32 j1021_dvp_init(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n\n j1021_dvp_init22 \n\n");

    j1021_dvp_config_SENSOR(width, height, format, frame_freq);

	sensor_para_updata();

    return 0;
}


static void set_again(u32 again)
{
	wr_j1021_reg(0xb1, (again&0xff)); // ppk add on 20171121

    return;
}

static void set_dgain(u32 dgain)
{

    if (cur_dgain == dgain) {
        return;
    }
    cur_dgain  = dgain;

    wr_j1021_reg(0xb7, (dgain >> 8)&0xff);
    wr_j1021_reg(0xb8, (dgain)&0xff);


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

static void cal_texp_mck(u32 exp_time, u32 *texp, u32 *mck)
{
	  u32 tmp2 = 0;
    s32 remain_ns = 0;

	#if J1021_VARIABLE_FPS
    if (exp_time > 50000) {
        gVblank = VBLANK_16FPS;
        gExplineMax = MAX_EXP_LINE_16FPS;
    } else if (exp_time > 40000) {
        gVblank = VBLANK_20FPS;
        gExplineMax = MAX_EXP_LINE_20FPS;
    } else if (exp_time > 30000) {
        gVblank = VBLANK_25FPS;
        gExplineMax = MAX_EXP_LINE_25FPS;
    } else {
        gVblank = VBLANK_30FPS;
        gExplineMax = MAX_EXP_LINE_30FPS;
    }

   #endif

    *texp = exp_time * 1000 / ROW_TIME_NS;

    if (*texp >= gExplineMax) {
        *texp = gExplineMax;
    } else if (*texp < 1) {
        *texp = 1;
    }

    tmp2 = (*texp) * ROW_TIME_NS;
    remain_ns = exp_time * 1000 - tmp2;
    if (remain_ns > 0) {
        if (remain_ns >= ROW_TIME_NS) {
            *mck = gRowTime;
        } else {
            *mck = (remain_ns * gRowTime / ROW_TIME_NS);
        }
    } else {
        *mck = 0;
    }

    return;


}

static void calc_gain(u32 gain, u32 *_again, u32 *_dgain)
{
    int i;
	u8 fd_state,fd_sel;
	u8 vrefh;
	int dgain,SensorDGainSet;

#if FD_GAIN_SWITCH
	//fd select
	if(gain > FD_TH){
		fd_state = 1;
		fd_sel = 0xAA;
	}else if(gain < FD_TL){
		fd_state = 0;
		fd_sel = 0x55;
	}

	if(fd_state==1)
		gain = gain*1024/fd_gain;
#endif

	// A Gain
	if(gain<1024)gain=1024;
	vrefh = 128*1024/gain;
	if(vrefh>VREFH_LIMIT_H)vrefh=VREFH_LIMIT_H;
	if(vrefh<VREFH_LIMIT_L)vrefh=VREFH_LIMIT_L;
	gain = (gain*vrefh)/128;

	// D Gain
	if(gain<1024)
		gain=1024;
	dgain = (0x100*gain)/1024;

	if(dgain>256*3)
		dgain=256*3;//max dgain = 12*min_dgain

		//if(dgain>256*12)
		//dgain=256*12;//max dgain = 12*min_dgain

	SensorDGainSet = dgain*min_dgain/256;//min dgain=1.35x

	*_again = 0;
 #if FD_GAIN_SWITCH
	*_again = fd_sel*256;
 #endif
    *_again |= vrefh;
    *_dgain = SensorDGainSet;

    //puts("gain=%d, %d, %d, %d, 0x%x\n", gain, dgain , dcg_gain, coarse_gain, fine_gain);
}
#endif

static void set_shutter(u32 texp, u32 texp_mck, u32 again)
{

	if (gVblank != gVblank_pre) {
    	wr_j1021_reg(0x21, gVblank >> 8);
    	wr_j1021_reg(0x22, gVblank & 0xff);
	}
	gVblank_pre = gVblank;

	if (texp != gTexp_pre) {
    	wr_j1021_reg(0x0c, texp >> 8);
    	wr_j1021_reg(0x0d, (texp) & 0xff);
	}
	gTexp_pre = texp;

	if (texp_mck != gTexp_mck_pre) {
    	wr_j1021_reg(0x26, texp_mck >> 8);
    	wr_j1021_reg(0x27, texp_mck & 0xff);
	}
	gTexp_mck_pre = texp_mck;

#if FD_GAIN_SWITCH
	wr_j1021_reg(0xf0, 0x01); // ppk add on 20171121
	wr_j1021_reg(0xc7, (again>>8)&0xff); // ppk add on 20171121
	wr_j1021_reg(0xf0, 0x00); // ppk add on 20171121
#endif

    wr_j1021_reg(0x1d, 0x02);
    delay_2ms(14);
}


u32 j1021_dvp_calc_shutter(isp_ae_shutter_t *shutter, u32 exp_time_us, u32 gain)
{
    u32 texp;
     u32 mck;
    //u32 ratio;


    cal_texp_mck(exp_time_us, &texp, &mck);


    shutter->ae_exp_line =  texp;
    shutter->ae_gain = gain;//(gain * ratio) >> 10;
    shutter->ae_exp_clk = mck;//0;

   // puts("exp_time_us=%d, texp=%d, gain=%d\n", exp_time_us, texp, shutter->ae_gain);
    return 0;

}

u32 j1021_dvp_set_shutter(isp_ae_shutter_t *shutter)
{
    u32 again, dgain;

    calc_gain((shutter->ae_gain), &again, &dgain);

    set_shutter(shutter->ae_exp_line,shutter->ae_exp_clk, again);
    set_again(again);
    set_dgain(dgain);

    return 0;

}

void j1021_dvp_sleep()
{


}

void j1021_dvp_wakeup()
{


}

void j1021_dvp_wr_reg(u16 addr, u16 val)
{
    /*printf("update reg%x with %x\n", addr, val);*/
    wr_j1021_reg((u8)addr, (u8)val);
}
u16 j1021_dvp_rd_reg(u16 addr)
{
    u8 val;
    rd_j1021_reg((u8)addr, &val);
    return val;
}

REGISTER_CAMERA(j1021_dvp) = {
    .logo 				= 	"j1021d",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_RGGB,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_PARALLEL,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_10B | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps         		= 	30, // 25

    .sen_size 			= 	{J1021_DVP_OUTPUT_W, J1021_DVP_OUTPUT_H},
    .isp_size 			= 	{J1021_DVP_OUTPUT_W, J1021_DVP_OUTPUT_H},

    .cap_fps         		= 	30, // 25
    .sen_cap_size 			= 	{J1021_DVP_OUTPUT_W, J1021_DVP_OUTPUT_H},
    .isp_cap_size 			= 	{J1021_DVP_OUTPUT_W, J1021_DVP_OUTPUT_H},

    .ops                =   {
        .avin_fps           =   NULL,
        .avin_valid_signal  =   NULL,
        .avin_mode_det      =   NULL,
        .sensor_check 		= 	j1021_dvp_check,
        .init 		        = 	j1021_dvp_init,
        .set_size_fps 		=	j1021_dvp_set_output_size,
        .power_ctrl         =   j1021_dvp_power_ctl,

        .get_ae_params  	=	j1021_dvp_get_ae_params,
        .get_awb_params 	=	j1021_dvp_get_awb_params,
        .get_iq_params 	    	=	j1021_dvp_get_iq_params,

        .sleep 		        =	j1021_dvp_sleep,
        .wakeup 		    =	j1021_dvp_wakeup,
        .write_reg 		    =	j1021_dvp_wr_reg,
        .read_reg 		    =	j1021_dvp_rd_reg,

    }
};


