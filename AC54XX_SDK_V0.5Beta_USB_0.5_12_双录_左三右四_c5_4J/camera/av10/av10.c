#include "av10.h"
#include "device/av10_spi.h"


//static void dly(u16 tmp)
//{
//    while (tmp--)
//    {
//        asm("nop");
//    }
//}
//
static void wait(void)
{
    delay(0xffff);
}
static u32 reset_gpio[2] = {-1, -1};
void av10_reset(u8 isp_dev)
{

    u32 gpio;
    if (isp_dev == ISP_DEV_0) {
        gpio = reset_gpio[0];
    } else {
        gpio = reset_gpio[1];
    }

    gpio_direction_output(gpio, 1);
    delay(50000);
    gpio_direction_output(gpio, 0);
    delay(50000);
    gpio_direction_output(gpio, 1);
    delay(50000);

}

s32 av10_id_check()
{
    u16 id = 0;
    u16 id1 = 0;
    u16 version = 0;
    u16 version1 = 0;

    delay(500);

    id = av10_drv_rx(AV10_CHIP_ID);
    if (id == 0x34) {
        puts("\n av10 init succes !\n");
        return 1;
    }

    puts("\n av10 id check fail !\n");
    return 0;

}


static void *spi = NULL;
static u8 cur_sensor_type = 0xff;

void *get_spi()
{
    return spi;
}


static void av_sensor_power_ctrl(u32 _power_gpio, u32 on_off)
{
//    u32 gpio = _power_gpio;
//    gpio_direction_output(gpio, on_off);
//    delay(5000);
}

s32 av10_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
{
    if (isp_dev == ISP_DEV_0) {
        return -1;
    }
    puts("\n av10 id check\n");

    if (!get_spi()) {
        spi = dev_open("avin_spi", 0);
        if (!spi) {
            return -1;
        }
    } else {
        if (cur_sensor_type != isp_dev) {
            return -1;
        }
    }

    reset_gpio[isp_dev] = _reset_gpio;
    av_sensor_power_ctrl(_power_gpio, 1);
    av10_reset(isp_dev);

    if (0 == av10_id_check()) {
        dev_close(get_spi());
        spi = NULL;
        return -1;
    }


    cur_sensor_type = isp_dev;
    return 0;
}

static u16 av_width = 720;
static u16 av_height = 576;
static u8 av_frame_num = 25;

static void avin_para_set(u16 width, u16 height, u16 frame_n)
{
    av_width        = width;
    av_height       = height;
    av_frame_num    = frame_n;
}

s32 av10_set_output_size(u16 *width, u16 *height, u8 *freq)
{

    *width = av_width;
    *height = av_height;
    *freq = av_frame_num;

    return 0;
}
static u8 p_n_mode;
static u8 avin_fps()
{
    if (p_n_mode == PAL_MODE) {
        return 1;
    }
    return 0;
    /*     u8 status ; */
    /* status = av10_drv_rx(BIT(8) | 60); */
    /* if (status & BIT(0)) { //1:PAL(25FPS) 0:NTSC(30FPS) */
    /* return 1; */
    /* } */
    /* return 0; */
}


static u8 avin_valid_signal(void *p)
{
    u8 status ;
    status = av10_drv_rx(BIT(8) | 58);
//    printf("status0: %8x \n",status);
    if ((status & 0x07) == 0x06) {
        return 1;
    }

    return 0;
}

static int wait_signal_valid()
{
    u32 i;

    if (avin_valid_signal(NULL)) {
        //信号有效等500ms 待信号稳定
        u32 time = jiffies + msecs_to_jiffies(500);
        while (1) {
            if (time_after(jiffies, time)) {
                puts("\n av10 signal valid\n");
                return 0;
            }
        }
    } else {
        //信号无效等800ms
        u32 time = jiffies + msecs_to_jiffies(800);
        while (!avin_valid_signal(NULL)) {
            if (time_after(jiffies, time)) {
                puts("\n av10 signal invalid\n");
                return -1;
            }
        }
    }

    if (avin_valid_signal(NULL)) {
        return 0;
    }
    puts("\n av10 signal invalid 222\n");
    return -1;
}

#define CYCLE_CNT 200
#define FLITER_CNT 30

static u8 last_mode = OTHER_MODE;
static u8 this_mode;

u8 p_n_mode_fliter(u8 mode)
{
    static u32 cnt = 0;
    if (cnt++ > FLITER_CNT) { //filter
        cnt = 0;
        last_mode = mode;
        p_n_mode = mode;
        if (p_n_mode == PAL_MODE) {
            puts("\n =============== PAL_MODE \n");
        } else {
            puts("\n ================= NTSC_MODE ff\n");
        }
        return 1;
    }

    return 0;
}

static u8 avin_mode_det(void *p)
{
    u32 i;
    u32 need_change = 0;
    for (i = 0; i < FLITER_CNT; i++) {
        static u32 cnt = 0;
        if (!avin_valid_signal(NULL)) {
            if (cnt++ > FLITER_CNT * 3 / 4) {
                cnt = 0;
                need_change = 0;
                puts("\n no validcccc\n");
                return need_change;//no valid signal
            }
        }
    }

    for (i = 0; i < CYCLE_CNT; i++) {
        need_change = 0;
        this_mode = av10_mode_det();

        if (this_mode == PAL_MODE) {
            if (last_mode ==  OTHER_MODE) {
                if (p_n_mode_fliter(this_mode)) {
                    need_change = 1;
                    return need_change;//need remount
                }
            } else if (last_mode ==  NTSC_MODE) {
                if (p_n_mode_fliter(this_mode)) {
                    need_change = 1;
                    return need_change;//need remount
                }
            }
        } else if (this_mode == NTSC_MODE) {
            if (last_mode ==  OTHER_MODE) {
                if (p_n_mode_fliter(this_mode)) {
                    need_change = 1;
                    return need_change;//need remount
                }
            } else {
                if (last_mode ==  PAL_MODE) {
                    if (p_n_mode_fliter(this_mode)) {
                        need_change = 1;
                        return need_change;//need remount
                    }
                }
            }
        }
    }
    return need_change;
}

extern void av10_mode_cfg(u8 mode_type);
extern void av10_init();

extern u8 av10_mode_det();
int av10_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{

    av10_init();
    if (wait_signal_valid() != 0) {
        return -1;
    }

    u8 av10_mode;
    static u8 flag = 0;
    if (!flag) {
        //first time
        flag = 1;
        avin_mode_det(NULL);
    }
    av10_mode = p_n_mode;
    if (av10_mode == PAL_MODE) {
        puts("\n avin pal\n");
        avin_para_set(720, 576, 25);
    } else {
        puts("\n avin ntsc \n");
        avin_para_set(720, 480, 30);
    }
    av10_mode_cfg(av10_mode);

    *format = SEN_IN_FORMAT_UYVY;
    av10_set_output_size(width, height, frame_freq);

    return 0;
}

s32 av10_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\n av10_init \n");
    if (av10_config_SENSOR(width, height, format, frame_freq) != 0) {
        puts("\n av10_init fail\n");
        return -1;
    }
    return 0;
}


void av10_init()
{
    av10_f_or(AV10_PORTA_DIR, 0xf0);
    av10_f_or(AV10_PORTC_DIR, BIT(1));
    av10_f_or(AV10_PORTA_PD, 0xf0);
    av10_f_or(AV10_PORTC_PD, BIT(1));

    av10_f_and(AV10_PORTA_DIR, ~(u8)BIT(3));
    av10_f_mov(AV10_PORTB_DIR, 0X00);
//        av10_f_or (AV10_PORTA_HD,BIT(3));  //test
//        av10_f_or (AV10_PORTB_HD,0xff);  //test
#if 1
    av10_f_or(AV10_PORTC_DIR, BIT(3) | BIT(4));
    av10_f_and(AV10_PORTC_PD, ~(u8)(BIT(3) | BIT(4)));
    av10_f_and(AV10_PORTC_PU, ~(u8)(BIT(3) | BIT(4)));
    av10_f_and(AV10_PORTC_DIE, ~(u8)(BIT(3) | BIT(4)));
    av10_f_mov(AV10_CLK_CON0, (u8)(BIT(0) | BIT(1) | (3 << 2) | BIT(4)));//27M
#else
    av10_f_mov(AV10_CLK_CON0, (1 << 2) | BIT(4) | BIT(5));//54M
#endif
    wait();

    //================analog configue=================//
    //clear
    av10_f_mov(AV10_AFE_CON0, 0);
    av10_f_mov(AV10_AFE_CON1, 0);
    av10_f_mov(AV10_AFE_CON2, 0);
    av10_f_mov(AV10_AFE_CON3, 0);
    av10_f_mov(AV10_AFE_CON4, 0);
    av10_f_mov(AV10_AFE_CON5, 0);
    av10_f_mov(AV10_AFE_CON6, 0);

    //config
    av10_f_or(AV10_AFE_CON1, (0 << 5)); //VCM_GEN_SEL 0
    av10_f_or(AV10_AFE_CON1, (0 << 6)); //VCM_SEL 00

    av10_f_or(AV10_AFE_CON2, (0 << 4)); //SYNCTIP_CTRL 00

    av10_f_or(AV10_AFE_CON2, (0 << 3)); //CP_ISEL 0
    av10_f_or(AV10_AFE_CON2, (4 << 0)); //CPCCUR_CTRL 100

    av10_f_or(AV10_AFE_CON2, (0 << 6)); //SUB_CTRL 0

    av10_f_or(AV10_AFE_CON2, (0 << 7)); //PGA_MUTE 0
    av10_f_or(AV10_AFE_CON3, (0 << 4)); //PGA_ISEL 0
    av10_f_or(AV10_AFE_CON3, (0 << 0)); //PGA_GAIN_MODE 0
//        av10_f_or (AV10_AFE_CON3,(4<<1));   //PGA_CAPSEL 100
    av10_f_or(AV10_AFE_CON3, (7 << 1)); //PGA_CAPSEL 100


    av10_f_or(AV10_AFE_CON4, (4 << 5)); //ADC_ASEL 100
    av10_f_or(AV10_AFE_CON5, (1 << 2)); //ADC_DSEL 01

    av10_f_or(AV10_AFE_CON5, (0 << 6)); //ADC_IBIAS_SEL 00
    av10_f_or(AV10_AFE_CON4, (0 << 2)); //ADC_VREF 000

    av10_f_or(AV10_AFE_CON5, (0 << 0)); //ADC_VMILER_SEL 0
    av10_f_or(AV10_AFE_CON5, (1 << 1)); //ADC_VCMREF_SEL 1
    av10_f_or(AV10_AFE_CON4, (1 << 0)); //VNBIAS_SEL 1
    av10_f_or(AV10_AFE_CON4, (1 << 1)); //VPBIAS_SEL 1
    av10_f_or(AV10_AFE_CON5, (2 << 4)); //ADC_CMP_RETDLY_SEL 10

    //enable
    av10_f_or(AV10_AFE_CON0, BIT(0));   //BIAS_EN
    av10_f_or(AV10_AFE_CON0, BIT(1));   //CVBS0_EN
    av10_f_or(AV10_AFE_CON1, BIT(0));   //VCM_EN
    av10_f_or(AV10_AFE_CON1, BIT(1));   //SYNCTIP_EN
    av10_f_or(AV10_AFE_CON1, BIT(4));   //CHAGE_PUMP_EN
    av10_f_or(AV10_AFE_CON0, BIT(4));   //PGA_EN
    av10_f_or(AV10_AFE_CON0, BIT(5));   //PGA_CH0_EN
    av10_f_or(AV10_AFE_CON1, BIT(3));   //ADC_CLK_EN
    av10_f_or(AV10_AFE_CON1, BIT(2));   //ADC_EN
    wait();

    //==============sample configue===================//

    av10_f_mov(AV10_CVD_CON0, BIT(0) | BIT(1) | BIT(3) | (2 << 6));
    av10_f_mov(AV10_CVD_CON1, BIT(4) | BIT(7));

//        av10_f_and(AV10_PORTA_PD,~(BIT(4)|BIT(5)|BIT(6)|BIT(7)));
//        av10_f_and(AV10_PORTA_DIR,~(BIT(4)|BIT(5)|BIT(6)|BIT(7)));

    av10_f_mov(AV10_CVD_CON4, 0);
    av10_f_or(AV10_CVD_CON4, (6 << 0)); //sync_dly
    av10_f_mov(AV10_GCVT_GAIN, 0xaa);   //gain cvt gain

    av10_f_mov(AV10_CVD_CON5, 0x80);    //u_xtune
    av10_f_mov(AV10_CVD_CON6, 0x60);    //v_xtune

    {
        u16 black_level = 2048;
        av10_f_mov(AV10_SMP_BLVL_H, (black_level >> 8));
        av10_f_mov(AV10_SMP_BLVL_L, (black_level >> 0));
    }

    {
//            u16 smp_gain = 0x800;
        u16 smp_gain = 0x4c0;
        av10_f_mov(AV10_SMP_GAIN_H, (smp_gain >> 8));
        av10_f_mov(AV10_SMP_GAIN_L, (smp_gain >> 0));
    }

    //==============cvd configue===================//

    av10_f_or(BIT(8) | 0XB2, BIT(6));
    //av10_f_or (BIT(8)|0XB2,BIT(7)); //line buffer reset
    //v10_f_mov(BIT(8)|0XB2,~BIT(7));

    //PAL
    av10_f_mov(BIT(8) | 0X00, 0X32);
    av10_f_mov(BIT(8) | 0X01, 0X00);
    av10_f_mov(BIT(8) | 0X02, 0X4f);
    av10_f_mov(BIT(8) | 0X03, 0x06);
    av10_f_mov(BIT(8) | 0X04, 220);
    av10_f_mov(BIT(8) | 0X05, 50);
    av10_f_mov(BIT(8) | 0x0c, 0x80);
    av10_f_and(BIT(8) | 0x0f, ~(u8)BIT(5));

    av10_f_mov(BIT(8) | 0x07, 0x20 | 0x01); //yc delay
    //av10_f_mov(BIT(8)|0X08,144);        //contrast
    //av10_f_mov(BIT(8)|0X09,16);         //brightness
    //av10_f_mov(BIT(8)|0X0a,160);        //saturation

    av10_f_mov(BIT(8) | 0X18, 0X2A); //CDTO 27m
    av10_f_mov(BIT(8) | 0X19, 0X09);
    av10_f_mov(BIT(8) | 0X1A, 0X8a);
    av10_f_mov(BIT(8) | 0X1B, 0Xcb);

    av10_f_mov(BIT(8) | 0X2E, 134); //hactive start 130
    av10_f_mov(BIT(8) | 0X2F, 80);  //hactive width 80

    av10_f_mov(BIT(8) | 0X30, 46);  //vertical start 42
    av10_f_mov(BIT(8) | 0X31, 193); //vertical width 193

//        av10_f_mov(BIT(8)|0X80,0x0b);   //y peak
//        av10_f_mov(BIT(8)|0Xb5,0xc0);   //c peak
//        av10_f_mov(BIT(8)|0Xb7,0x06);

    //cvd status reset
    av10_f_mov(BIT(8) | 0X3f, 0x00);

    //==============color configue===================//

    //av10_f_mov(AV10_CBS_U_COS,0xf0);
    //av10_f_mov(AV10_CBS_U_SIN,0x90);
    //av10_f_mov(AV10_CBS_V_COS,0xf0);
    //av10_f_mov(AV10_CBS_V_SIN,0x90);

    av10_f_mov(AV10_CBS_Y_GAIN, 0x78);
    av10_f_mov(AV10_CBS_U_GAIN, 0x98);
    av10_f_mov(AV10_CBS_V_GAIN, 0x98);

    av10_f_mov(AV10_CBS_Y_OFFS, 0xf8);
    av10_f_mov(AV10_CBS_U_OFFS, 0x00);
    av10_f_mov(AV10_CBS_V_OFFS, 0x00);
}

s32 av10_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

void av10_mode_cfg(u8 mode_type)
{
    av10_f_mov(AV10_CVD_CON0, 0);
    av10_f_mov(AV10_CVD_CON0, BIT(0));

    if (mode_type == PAL_MODE) {
        av10_f_mov(AV10_CVD_CON0, BIT(0) | BIT(1) | BIT(3) | (2 << 6));
        av10_f_mov(AV10_CVD_CON1, BIT(4) | BIT(7));

        av10_f_mov(AV10_CVD_CON4, 0);
        av10_f_or(AV10_CVD_CON4, (6 << 0)); //sync_dly
        av10_f_mov(AV10_GCVT_GAIN, 0xaa);   //gain cvt gain

        av10_f_mov(AV10_CVD_CON5, 0x80);    //u_xtune
        av10_f_mov(AV10_CVD_CON6, 0x60);    //v_xtune

        {
            u16 black_level = 2048;
            av10_f_mov(AV10_SMP_BLVL_H, (black_level >> 8));
            av10_f_mov(AV10_SMP_BLVL_L, (black_level >> 0));
        }

        {
//            u16 smp_gain = 0x800;
            u16 smp_gain = 0x4c0;
            av10_f_mov(AV10_SMP_GAIN_H, (smp_gain >> 8));
            av10_f_mov(AV10_SMP_GAIN_L, (smp_gain >> 0));
        }

        av10_f_or(BIT(8) | 0XB2, BIT(6));
        //av10_f_or (BIT(8)|0XB2,BIT(7)); //line buffer reset
        //v10_f_mov(BIT(8)|0XB2,~BIT(7));

        //PAL
        av10_f_mov(BIT(8) | 0X00, 0X32);
        av10_f_mov(BIT(8) | 0X01, 0X00);
        av10_f_mov(BIT(8) | 0X02, 0X4f);
        av10_f_mov(BIT(8) | 0X03, 0x06);
        av10_f_mov(BIT(8) | 0X04, 220);
        av10_f_mov(BIT(8) | 0X05, 50);
        av10_f_mov(BIT(8) | 0x0c, 0x80);
        av10_f_and(BIT(8) | 0x0f, ~(u8)BIT(5));
        av10_f_mov(BIT(8) | 0x07, 0x20 | 0x01); //yc delay
//        av10_f_mov(BIT(8)|0x07,0x00|0x01);  //yc delay
        //av10_f_mov(BIT(8)|0X08,144);        //contrast
        //av10_f_mov(BIT(8)|0X09,16);         //brightness
        //av10_f_mov(BIT(8)|0X0a,160);        //saturation

        av10_f_mov(BIT(8) | 0X18, 0X2A); //CDTO 27m
        av10_f_mov(BIT(8) | 0X19, 0X09);
        av10_f_mov(BIT(8) | 0X1A, 0X8a);
        av10_f_mov(BIT(8) | 0X1B, 0Xcb);

        av10_f_mov(BIT(8) | 0X2E, 134); //hactive start 130
        av10_f_mov(BIT(8) | 0X2F, 80);  //hactive width 80

        av10_f_mov(BIT(8) | 0X30, 46);  //vertical start 42
        av10_f_mov(BIT(8) | 0X31, 193); //vertical width 193

        av10_f_mov(BIT(8) | 0X3f, 0x00);

    } else if (mode_type == NTSC_MODE) {

        //==============sample configue===================//

        av10_f_mov(AV10_CVD_CON0, 0);
        av10_f_mov(AV10_CVD_CON1, 0);
        av10_f_mov(AV10_CVD_CON4, 0);
        av10_f_mov(AV10_CVD_CON5, 0);
        av10_f_mov(AV10_CVD_CON6, 0);

        av10_f_mov(AV10_CVD_CON0, BIT(0) | BIT(1) | BIT(3) | (2 << 6));
        av10_f_mov(AV10_CVD_CON1, BIT(4) | BIT(7));

        av10_f_mov(AV10_CVD_CON4, 0);
        av10_f_or(AV10_CVD_CON4, (6 << 0)); //sync_dly
        av10_f_mov(AV10_GCVT_GAIN, 0xaa);   //gain cvt gain

        av10_f_mov(AV10_CVD_CON5, 0x80);    //u_xtune
        av10_f_mov(AV10_CVD_CON6, 0x60);    //v_xtune

        {
            u16 black_level = 2048;
            av10_f_mov(AV10_SMP_BLVL_H, (black_level >> 8));
            av10_f_mov(AV10_SMP_BLVL_L, (black_level >> 0));
        }

        {
            u16 smp_gain = 0x4c0;
            av10_f_mov(AV10_SMP_GAIN_H, (smp_gain >> 8));
            av10_f_mov(AV10_SMP_GAIN_L, (smp_gain >> 0));
        }

        //==============cvd configue===================//
        //NTSC
        av10_f_or(BIT(8) | 0XB2, BIT(7));
        av10_f_mov(BIT(8) | 0XB2, 0x7f);
        av10_f_or(BIT(8) | 0XB2, BIT(6));

        av10_f_mov(BIT(8) | 0x07, 0x20 | 0x01); //yc delay
        //av10_f_mov(BIT(8)|0X08,144);      //contrast
        //av10_f_mov(BIT(8)|0X09,16);       //brightness
        //av10_f_mov(BIT(8)|0X0a,160);      //saturation

        av10_f_mov(BIT(8) | 0X80, 0X19);    //y  peak
        //av10_f_mov(BIT(8)|0XB5,0XC3);
        //av10_f_mov(BIT(8)|0Xaf,BIT(8)|0x50);
        //av10_f_mov(BIT(8)|0Xb4,BIT(8)|0x20);

        av10_f_mov(BIT(8) | 0x29, 0x38);

        //cvd status reset
        av10_f_mov(BIT(8) | 0X3f, 0x00);
    }
}



u8 av10_mode_det()
{
    u8 status ;
    u8 tmp_mode = OTHER_MODE;
    status = av10_drv_rx(BIT(8) | 58);
//    printf("status1 : %d \n",status);
    if ((status & 0x07) == 0x06) {
        status = av10_drv_rx(BIT(8) | 60);
        if ((status & BIT(2))  || (status & BIT(0))) {
            tmp_mode = PAL_MODE;
        } else {
            tmp_mode = NTSC_MODE;
        }
    } else {
        tmp_mode = OTHER_MODE;
    }

    return tmp_mode;
}


REGISTER_CAMERA(AV10) = {
    .logo 				= 	"AV10",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_UYVY,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_BT656,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B | SEN_MBUS_FIELD_EVEN_HIGH | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps         		= 	0,

    .sen_size 			= 	{0, 0},
    .isp_size 			= 	{0, 0},

    .ops                    = {
        .avin_fps           =   avin_fps,
        .avin_valid_signal  =   avin_valid_signal,
        .avin_mode_det      =   avin_mode_det,
        .sensor_check 		= 	av10_check,
        .init 		        = 	av10_initialize,
        .set_size_fps 		=	av10_set_output_size,
        .power_ctrl         =   av10_power_ctl,


        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	NULL,
        .read_reg 		    =	NULL,
    }
};
