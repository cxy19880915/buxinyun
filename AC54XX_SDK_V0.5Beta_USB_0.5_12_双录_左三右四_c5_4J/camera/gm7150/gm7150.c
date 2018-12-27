#include "gm7150.h"
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "gpio.h"
#include "generic/jiffies.h"



static u8 avin_fps();

static u32 reset_gpio[2] = {-1, -1};

//0xB8
//0xB9
#define GM7150_PSTA		1
#define GM7150_NSTA		2

/*S_IIC_FUN gm7150_iic_fun;*/
static void *iic = NULL;
static u8 g_pn_status = -1;

#define WRCMD 0xB8
#define RDCMD 0xB9

unsigned char wrGM7150Reg(u16 regID, unsigned char regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __wend;
    }

    delay(50);

    if (dev_ioctl(iic, IIC_IOCTL_TX, regID)) {
        ret = 0;
        goto __wend;
    }

    delay(50);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regDat)) {
        ret = 0;
        goto __wend;
    }

__wend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;

#if 0

    gm7150_iic_fun.startSCCB();
    if (0 == gm7150_iic_fun.SCCBwriteByte(WRCMD)) {
        puts("a");
        gm7150_iic_fun.stopSCCB();
        return (0);
    }
    delay(50);

    if (0 == gm7150_iic_fun.SCCBwriteByte((u8)(regID))) {
        puts("b");
        gm7150_iic_fun.stopSCCB();
        return (0);
    }
    delay(50);
    if (0 == gm7150_iic_fun.SCCBwriteByte(regDat)) {
        puts("c");
        gm7150_iic_fun.stopSCCB();
        return (0);
    }
    gm7150_iic_fun.stopSCCB();

    return (1);
#endif
}

unsigned char rdGM7150Reg(u16 regID, unsigned char *regDat)
{
    u8 ret = 1;

    dev_ioctl(iic, IIC_IOCTL_START, 0);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, WRCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(50);

    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_STOP_BIT, regID)) {
        ret = 0;
        goto __rend;
    }

    delay(50);


    if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, RDCMD)) {
        ret = 0;
        goto __rend;
    }

    delay(50);

    dev_ioctl(iic, IIC_IOCTL_RX_WITH_STOP_BIT, (u32)regDat);

__rend:

    dev_ioctl(iic, IIC_IOCTL_STOP, 0);
    return ret;


#if 0

    gm7150_iic_fun.startSCCB();
    if (0 == gm7150_iic_fun.SCCBwriteByte(WRCMD)) {
        puts("1");
        gm7150_iic_fun.stopSCCB();
        return (0);
    }
    delay(50);
    if (0 == gm7150_iic_fun.SCCBwriteByte((u8)(regID))) {
        puts("2");
        gm7150_iic_fun.stopSCCB();
        return (0);
    }
    gm7150_iic_fun.stopSCCB();
    delay(50);

    //设置寄存器地址后，才是读
    gm7150_iic_fun.startSCCB();
    if (0 == gm7150_iic_fun.SCCBwriteByte(RDCMD)) {
        puts("3");
        gm7150_iic_fun.stopSCCB();
        return (0);
    }
    delay(50);
    *regDat = gm7150_iic_fun.SCCBreadByte();

    gm7150_iic_fun.stopSCCB();

//  	puts("finish");
    return (1);
#endif
}

void gm7150_reset(u8 isp_dev)
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

#if 0
    //puts("\nGM7150 Reset");
    GM7150_NORMAL();

    GM7150_RESET_H();
    //delay_2ms(100);
    delay(50000);
    GM7150_RESET_L();
    //delay_2ms(300);//500ms
    delay(50000);
    GM7150_RESET_H();
#endif
}

s32 gm7150_id_check()
{
    u16 id = 0;
    u16 id1 = 0;
    u16 version = 0;
    u16 version1 = 0;

    //delay_2ms(50);//需要加延时，否则前面几个byte读得不正确
    delay(50000);

    wrGM7150Reg(0x05, 0x01);//Soft Reset

    wrGM7150Reg(0x05, 0x00);

    rdGM7150Reg(0x80, (unsigned char *)&id);
    id <<= 8;
    rdGM7150Reg(0x81, (unsigned char *)&id1);
    id |= id1;
    puts("\nid:");
    put_u16hex(id);

    rdGM7150Reg(0x82, (unsigned char *)&version);
    version <<= 8;
    rdGM7150Reg(0x83, (unsigned char *)&version1);
    version |= version1;
    puts("\nver:");
    put_u16hex(version);

    if (id == 0x7150) {
        puts("\ngm7150_id_check succ\n");
        return 1;
    } else if (id == 0x5150) {
        puts("\n tvp5150_id_check succ\n");
        return 2;
    }

    return 0;
}


/*void GM7150_iic_set(u8 isp_dev)*/
/*{*/
/*iic_select(&gm7150_iic_fun, isp_dev);*/
/*}*/


static u8 cur_sensor_type = 0xff;
static void av_sensor_power_ctrl(u32 _power_gpio, u32 on_off)
{
//    u32 gpio = _power_gpio;
//    gpio_direction_output(gpio, on_off);
//    delay(5000);
}

s32 gm7150_check(u8 isp_dev, u32 _reset_gpio, u32 _power_gpio)
{
    if (isp_dev == ISP_DEV_0) {
        return -1;
    }
    puts("\ngm7150_id check\n");

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
    av_sensor_power_ctrl(_power_gpio, 1);
    gm7150_reset(isp_dev);
    /*GM7150_iic_set(isp_dev);*/

    if (0 == gm7150_id_check()) {
        dev_close(iic);
        iic = NULL;

        puts("\ngm7150_id check fail\n\n");

        return -1;
    }


    cur_sensor_type = isp_dev;

    return 0;
}


s32 gm7150_set_output_size(u16 *width, u16 *height, u8 *freq)
{
    if (avin_fps()) {
        *freq = 25;
        g_pn_status = GM7150_PSTA;
    } else {
        *freq = 30;
        g_pn_status = GM7150_NSTA;
    }
    *width = 720;
    *height = 480;

    return 0;
}

static u8 avin_valid_signal()
{
    u8 status ;

    rdGM7150Reg(0x88, &status);
    if (status & 0x06) {
        return 1;
    }

    return 0;
}

static int wait_signal_valid()
{
    u32 time;

    if (avin_valid_signal()) {
        //信号有效等50ms 待信号稳定
        time = jiffies + msecs_to_jiffies(50);
        while (1) {
            if (time_after(jiffies, time)) {
                puts("\n xxxxx 7150 valid\n");
                return 0;
            }

        }
    } else {
        //信号无效等100ms
        time = jiffies + msecs_to_jiffies(100);
        while (!avin_valid_signal()) {
            if (time_after(jiffies, time)) {
                puts("\n xxxxx 7150 no no no no validxx \n");
                return -1;
            }
        }
    }

    return  0;
}

extern void gm7150_init();
int GM7150_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    gm7150_init();//兼容tvp5150

    if (wait_signal_valid() != 0) {
        return -1;
    }
    *format = SEN_IN_FORMAT_UYVY;
    gm7150_set_output_size(width, height, frame_freq);

    return 0;
}

s32 gm7150_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    puts("\ngm7150_init \n");

    if (GM7150_config_SENSOR(width, height, format, frame_freq) != 0) {
        puts("\ngm7150_init fail\n");
        return -1;
    }
    return 0;
}


void gm7150_init()
{
//    u8 status = 0;
//    u32 cnt=0;
    u16 avid_start, avid_stop;

    wrGM7150Reg(0x00, 0x00); //AIP1A
#ifdef GM7150_656
    wrGM7150Reg(0x03, 0x09);
#else//601
    wrGM7150Reg(0x03, 0x0d); //CVBS输入,输出行场同步信号
#endif
//    iic_write(0xB8,0x03,0x0F);//场消隐开 VBLK开启

#ifndef GM7150_656
    wrGM7150Reg(0x0D, 0x40); //BT601
    wrGM7150Reg(0x0F, 0x00);
#endif

    wrGM7150Reg(0x16, 0x80); //hsync start position
    wrGM7150Reg(0x18, 0x00); //VBLK Start
    wrGM7150Reg(0x19, 0x00); //VBLK_Stop

    avid_start = 0;
    avid_stop = 0;
    wrGM7150Reg(0x11, avid_start >> 2);                  //AVID Start
    wrGM7150Reg(0x12, BIT(2) | (avid_start & 0x03));
    wrGM7150Reg(0x13, avid_stop >> 2);                   //AVID Stop
    wrGM7150Reg(0x14, (avid_stop & 0x03));

//    iic_write(0xB8,0x03,0xcf);//PIN27 输出VBLK信号
//    iic_write(0xB8,0x0f,0x0a);

    wrGM7150Reg(0x08, 0x40); //亮度处理控制
    wrGM7150Reg(0x09, 0x70); //明度
    wrGM7150Reg(0x0a, 0x80); //色度饱和度
    wrGM7150Reg(0x0b, 0x00); //色度色调
    wrGM7150Reg(0x0c, 0x50); //亮度对比度

//    iic_write(0xB8,0x28,0x04);//指定输入格式 (B,D,G,H,I,N)PAL

#if 0
    while (1) {
        rdGM7150Reg(0x88, &status);
        put_u8hex(status);
        if ((status & 0x06) == 0x06) {
            cnt++;
            if (cnt > 200) {
                puts("\n0x88:");
                put_u8hex(status);
                rdGM7150Reg(0x8C, &status);
                puts("\n0x8c:");
                put_u8hex(status);
                break;
            }
        } else {
            cnt = 0;
        }
    }
#endif

    //delay_2ms(50);
    delay(50000);
}

s32 gm7150_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}

static u8 avin_fps()
{
    u8 status ;

    rdGM7150Reg(0x88, &status);
    if (status & BIT(5)) {
        return 1;
    }
    return 0;
}



static u8 avin_mode_det(void *p)
{
    u8 new_status;

    if (avin_fps()) {
        new_status = GM7150_PSTA;
    } else {
        new_status = GM7150_NSTA;
    }

    if (g_pn_status != new_status) {
        return 1;
    }
    return 0;
}


REGISTER_CAMERA(GM7150) = {
    .logo 				= 	"GM7150",
    .isp_dev 			= 	ISP_DEV_NONE,
    .in_format 			= 	SEN_IN_FORMAT_UYVY,
    .out_format 		= 	ISP_OUT_FORMAT_YUV,
    .mbus_type          =   SEN_MBUS_BT656,
    .mbus_config        =   SEN_MBUS_DATA_WIDTH_8B | SEN_MBUS_FIELD_EVEN_HIGH | SEN_MBUS_PCLK_SAMPLE_FALLING,
    .fps         		= 	0,

    .sen_size 			= 	{720, 480},
    .isp_size 			= 	{0, 0},

    .ops                =   {
        .avin_fps           =   avin_fps,
        .avin_valid_signal  =   avin_valid_signal,
        .avin_mode_det      =   avin_mode_det,
        .sensor_check 		= 	gm7150_check,
        .init 		        = 	gm7150_initialize,
        .set_size_fps 		=	gm7150_set_output_size,
        .power_ctrl         =   gm7150_power_ctl,


        .sleep 		        =	NULL,
        .wakeup 		    =	NULL,
        .write_reg 		    =	NULL,
        .read_reg 		    =	NULL,
    }
};


