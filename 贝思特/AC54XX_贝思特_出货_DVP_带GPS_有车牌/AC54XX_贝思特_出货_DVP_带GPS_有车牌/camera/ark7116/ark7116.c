#include "ark7116.h"
#include "asm/iic.h"
#include "asm/isp_dev.h"
#include "asm/isp_com.h"
#include "asm/isp_sensor_manage.h"

/*S_IIC_FUN ark7116_iic_fun;*/
static void *iic = NULL;
static u32 reset_gpio[2] = {-1, -1};

u8  g_ucbrightness;
u8  g_ucContrast;
u8  g_ucSaturation;

/*==============start===============*/
/*AV1
[VideoChannel]
AV1
[VideoType]
CVBS
[VideoPI]
VIDEO_P
[VideoPicSys]
PAL
[VideoData]
13500000
 690
 280
 864
 312

Update date:Thursday, August 27, 2015
Update time:11:37:50
*/
PanlstaticPara AV1_staticPara[] = {
//GLOBAL
    {0XFD0A, 0X30},
    {0XFD0B, 0X27},
    {0XFD0E, 0X2C},
    {0XFD0F, 0X03},
    {0XFD10, 0X04},
    {0XFD11, 0XFF},
    {0XFD12, 0XFF},
    {0XFD13, 0XFF},
    {0XFD14, 0X02},
    {0XFD15, 0X02},
//DECODER
    {0XFE56, 0X00},
    {0XFED7, 0XF7},
    {0xFEDC, 0X10}, // av input Channel select BIT5\4;
    {0XFE83, 0XF3},
//VP
    {0XFFB0, 0X26}, //46 0XFFC9才起作用 26 0XFFC9不起作用
    {0XFFB1, 0X0F},
    {0XFFB2, 0X08},
    {0XFFB3, 0X08},
    {0XFFB4, 0X08},
    {0XFFC9, 0X40}, //-
    {0XFFD7, 0X00}, //-
    {0XFFD8, 0X00}, //-
    {0XFFCE, 0X10}, //-
    {0XFFCF, 0X80}, //-
    {0XFFD0, 0X80},
    {0XFFF0, 0X13},
    {0XFFF1, 0XE1},
    {0XFFF2, 0XFD},
    {0XFFF3, 0XDD},
    {0XFFF4, 0XFD},
    {0XFFF5, 0X0E},
    {0XFFF6, 0XFD},
    {0XFFF7, 0XE5},
    {0XFFF8, 0XE9},
    {0XFFF9, 0XFD},
    {0XFFFA, 0X20},
    {0XFFFB, 0X81},
    {0XFFD3, 0X90},
    {0XFFD4, 0X7B},
    {0XFFD5, 0X00},
    {0XFFD6, 0X3B}, //调色度 +越浓 -越淡
//SCALER
    {0XFC00, 0X40},
    {0XFCD8, 0X03},
    {0XFF00, 0X03},
    {0XFF01, 0X04},
    {0XFF02, 0X0A},
    {0XFF03, 0X11},
    {0XFF04, 0X1A},
    {0XFF05, 0X24},
    {0XFF06, 0X30},
    {0XFF07, 0X3C},
    {0XFF08, 0X49},
    {0XFF09, 0X56},
    {0XFF0A, 0X62},
    {0XFF0B, 0X6D},
    {0XFF0C, 0X78},
    {0XFF0D, 0X82},
    {0XFF0E, 0X8C},
    {0XFF0F, 0X95},
    {0XFF10, 0X9E},
    {0XFF11, 0XA7},
    {0XFF12, 0XAF},
    {0XFF13, 0XB6},
    {0XFF14, 0XBE},
    {0XFF15, 0XC5},
    {0XFF16, 0XCC},
    {0XFF17, 0XD2},
    {0XFF18, 0XD8},
    {0XFF19, 0XDE},
    {0XFF1A, 0XE4},
    {0XFF1B, 0XE9},
    {0XFF1C, 0XEE},
    {0XFF1D, 0XF2},
    {0XFF1E, 0XF6},
    {0XFF1F, 0XFB},
    {0XFF20, 0X04},
    {0XFF21, 0X0A},
    {0XFF22, 0X11},
    {0XFF23, 0X1A},
    {0XFF24, 0X24},
    {0XFF25, 0X30},
    {0XFF26, 0X3C},
    {0XFF27, 0X49},
    {0XFF28, 0X56},
    {0XFF29, 0X62},
    {0XFF2A, 0X6D},
    {0XFF2B, 0X78},
    {0XFF2C, 0X82},
    {0XFF2D, 0X8C},
    {0XFF2E, 0X95},
    {0XFF2F, 0X9E},
    {0XFF30, 0XA7},
    {0XFF31, 0XAF},
    {0XFF32, 0XB6},
    {0XFF33, 0XBE},
    {0XFF34, 0XC5},
    {0XFF35, 0XCC},
    {0XFF36, 0XD2},
    {0XFF37, 0XD8},
    {0XFF38, 0XDE},
    {0XFF39, 0XE4},
    {0XFF3A, 0XE9},
    {0XFF3B, 0XEE},
    {0XFF3C, 0XF2},
    {0XFF3D, 0XF6},
    {0XFF3E, 0XFB},
    {0XFF3F, 0X04},
    {0XFF40, 0X0A},
    {0XFF41, 0X11},
    {0XFF42, 0X1A},
    {0XFF43, 0X24},
    {0XFF44, 0X30},
    {0XFF45, 0X3C},
    {0XFF46, 0X49},
    {0XFF47, 0X56},
    {0XFF48, 0X62},
    {0XFF49, 0X6D},
    {0XFF4A, 0X78},
    {0XFF4B, 0X82},
    {0XFF4C, 0X8C},
    {0XFF4D, 0X95},
    {0XFF4E, 0X9E},
    {0XFF4F, 0XA7},
    {0XFF50, 0XAF},
    {0XFF51, 0XB6},
    {0XFF52, 0XBE},
    {0XFF53, 0XC5},
    {0XFF54, 0XCC},
    {0XFF55, 0XD2},
    {0XFF56, 0XD8},
    {0XFF57, 0XDE},
    {0XFF58, 0XE4},
    {0XFF59, 0XE9},
    {0XFF5A, 0XEE},
    {0XFF5B, 0XF2},
    {0XFF5C, 0XF6},
    {0XFF5D, 0XFB},
};
PanlPosDynPara AV1_posDynPara[] = {
//dispmode:  16:9  4:3  DM_EX0  DM_EX1  DM_EX2  DM_EX3
//GLOBAL
//PAD MUX
//DECODER
//VP
//SCALER
};
PanlSysDynPara AV1_sysDynPara[] = {
//picSys:   PAL  PAL-N  PAL-M  NTSC SECAM PAL-60 NTSC-J NTSC-4.43
//GLOBAL
//PAD MUX
//DECODER
//VP
//SCALER
};
/*============== end ===============*/



/*点屏 PAD MUX 参数*/
PanlstaticPara AMT_PadMuxStaticPara[] = {
//PAD MUX
    {0XFD31, 0X33},
    {0XFD34, 0X11},
    {0XFD35, 0X41},
    {0XFD36, 0X44},
    {0XFD37, 0X44},
    {0XFD38, 0X44},
    {0XFD39, 0X44},
    {0XFD3A, 0X11},
    {0XFD3B, 0X11},
    {0XFD3C, 0X11},
    {0XFD3D, 0X11},
    {0XFD3E, 0X11},
    {0XFD3F, 0X11},
    {0XFD40, 0X44},
    {0XFD41, 0X11},
    {0XFD42, 0X33},
    {0XFD43, 0X11},
    {0XFD50, 0X0B},
};

/*不同通道屏参的定义、初始化*/
VideoChannel VideoChannelPara[] = {
    { INPUT_AV1, {AV1_staticPara, AV1_posDynPara, AV1_sysDynPara}},
};

static void ARK7116_Delay(u32 cont_time)
{
    while (cont_time--) {
        // _nop_();
        ;;;;;;;;;;;;;;;;;;;;
    }
}

#define I2C_ACCESS_LOOP_TIME 20
unsigned char wrARK7116Reg(u16 regID, unsigned char regDat)
{
    unsigned char ucLoop;
    unsigned char ucDeviceAddr;
    unsigned char uctmpDeviceAddr;
    unsigned char ucSubAddr;

    ucLoop = I2C_ACCESS_LOOP_TIME;
    uctmpDeviceAddr = (unsigned char)((regID >> 8) & 0XFF);
    ucSubAddr = (unsigned char)(regID & 0XFF);

    switch (uctmpDeviceAddr) {
    case 0XF9:
    case 0XFD:
        ucDeviceAddr = 0XB0;
        break;

    case 0XFA:
        ucDeviceAddr = 0XBE;
        break;

    case 0XFB:
        ucDeviceAddr = 0XB6;
        break;

    case 0XFC:
        ucDeviceAddr = 0XB8;
        break;

    case 0XFE:
        ucDeviceAddr = 0XB2;
        break;

    case 0XFF:
        ucDeviceAddr = 0XB4;
        break;

    case 0X00:
        ucDeviceAddr = 0XBE;
        break;

    default:
        ucDeviceAddr = 0XB0;
        break;
    }

    while (ucLoop--) {
        if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, ucDeviceAddr)) {
            continue;
        }

        if (dev_ioctl(iic, IIC_IOCTL_TX, ucSubAddr)) {
            continue;
        }

        if (dev_ioctl(iic, IIC_IOCTL_TX, regDat)) {
            continue;
        }
#if 0
        ark7116_iic_fun.startSCCB();
        if (0 == ark7116_iic_fun.SCCBwriteByte(ucDeviceAddr)) {
            puts("a");
            continue;
        }

        if (0 == ark7116_iic_fun.SCCBwriteByte(ucSubAddr)) {
            puts("b");
            continue;
        }

        if (0 == ark7116_iic_fun.SCCBwriteByte(regDat)) {
            puts("c");
            continue;
        }
        break;
#endif
    }

    dev_ioctl(iic, IIC_IOCTL_TX_STOP_BIT, 0);
    /*ark7116_iic_fun.stopSCCB();*/

    return (1);
}

unsigned char rdARK7116Reg(u16 regID, unsigned char *regDat)
{
    unsigned char ucLoop;
    unsigned char ucDeviceAddr;
    unsigned char uctmpDeviceAddr;
    unsigned char ucSubAddr;

    ucLoop = I2C_ACCESS_LOOP_TIME;
    uctmpDeviceAddr = (unsigned char)((regID >> 8) & 0XFF);
    ucSubAddr = (unsigned char)(regID & 0XFF);

    switch (uctmpDeviceAddr) {
    case 0XF9:
    case 0XFD:
        ucDeviceAddr = 0XB0;
        break;

    case 0XFA:
        ucDeviceAddr = 0XBE;
        break;

    case 0XFB:
        ucDeviceAddr = 0XB6;
        break;

    case 0XFC:
        ucDeviceAddr = 0XB8;
        break;

    case 0XFE:
        ucDeviceAddr = 0XB2;
        break;

    case 0XFF:
        ucDeviceAddr = 0XB4;
        break;

    case 0X00:
        ucDeviceAddr = 0XBE;
        break;

    default:
        ucDeviceAddr = 0XB0;
        break;
    }

    while (ucLoop--) {
        if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, ucDeviceAddr)) {
            continue;
        }

        if (dev_ioctl(iic, IIC_IOCTL_TX, ucSubAddr)) {
            continue;
        }

        if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, ucDeviceAddr | BIT(0))) {
            continue;
        }

        dev_ioctl(iic, IIC_IOCTL_RX, regDat);
        break;
#if 0
        ark7116_iic_fun.startSCCB();
        if (0 == ark7116_iic_fun.SCCBwriteByte(ucDeviceAddr)) {
            puts("d");
            continue;
        }

        if (0 == ark7116_iic_fun.SCCBwriteByte(ucSubAddr)) {
            puts("e");
            continue;
        }

        ark7116_iic_fun.startSCCB();
        if (0 == ark7116_iic_fun.SCCBwriteByte(ucDeviceAddr | 0X01)) {
            puts("f");
            continue;
        }
        *regDat = ark7116_iic_fun.SCCBreadByte();
        ark7116_iic_fun.Ack();
        break;
#endif
    }
    /*ark7116_iic_fun.stopSCCB();*/
    dev_ioctl(iic, IIC_IOCTL_TX_STOP_BIT, 0);

    return (1);
}

#if 1
/********************ATK7116***********************************/

/****************************************************************************
*name:   InitiaGlobalClk(void)
*input:  void
*output: void

*description:
      ARK 初始化模块时钟。

*history:
   1.Jordan.chen   2011/12/19    1.0    buil   this  function
*****************************************************************************/
void InitGlobalPara()
{
    //printfStr("InitGlobalPara\n");

    wrARK7116Reg(0XFD0E, 0X20);
    ConfigPadMuxPara();
    ConfigStaticPara(INPUT_AV1);
    ConfigDispZoomDynPara(DISP_16_9);
    ConfigColorSysDynPara(PAL);
    wrARK7116Reg(0XFD0E, 0X2C);

    delay_2ms(5);
    wrARK7116Reg(0XFEA0, 0X03);
    delay_2ms(5);
    wrARK7116Reg(0XFEA0, 0X02);
}

/****************************************************************************
*name:   AMT_WriteStaticPara(PanlstaticPara * dataPt,u32 num)
*input:
*output:

*description:配置显示模式参数。
*history:   yshuizhou   2013/08/08    1.0    build   this  function
*****************************************************************************/
void AMT_WriteStaticPara(PanlstaticPara *dataPt, u32 num)
{
    unsigned char ucLoop;
    unsigned char ucDeviceAddr;
    unsigned char uctmpDeviceAddr;
    unsigned char ucSubAddr;
    unsigned char ucRegVal;

    while (num--) {
        ucLoop = I2C_ACCESS_LOOP_TIME;
        uctmpDeviceAddr = (u8)((((*dataPt).addr) >> 8) & 0XFF);
        ucSubAddr = (u8)(((*dataPt).addr) & 0XFF);
        ucRegVal = (*dataPt).dat;



        switch (uctmpDeviceAddr) {
        case 0XF9:
        case 0XFD:
            ucDeviceAddr = 0XB0;
            break;

        case 0XFA:
            ucDeviceAddr = 0XBE;
            break;

        case 0XFB:
            ucDeviceAddr = 0XB6;
            break;

        case 0XFC:
            ucDeviceAddr = 0XB8;
            break;

        case 0XFE:
            ucDeviceAddr = 0XB2;
            break;

        case 0XFF:
            ucDeviceAddr = 0XB4;
            break;

        case 0X00:
            ucDeviceAddr = 0XBE;
            break;

        default:
            ucDeviceAddr = 0XB0;
            break;
        }


        while (ucLoop--) {
            if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, ucDeviceAddr)) {
                continue;
            }

            if (dev_ioctl(iic, IIC_IOCTL_TX, ucSubAddr)) {
                continue;
            }

            if (dev_ioctl(iic, IIC_IOCTL_TX, ucRegVal)) {
                continue;
            }
            break;
#if 0
            ark7116_iic_fun.startSCCB();
            if (0 == ark7116_iic_fun.SCCBwriteByte(ucDeviceAddr)) {
                puts("g");
                continue;
            }

            if (0 == ark7116_iic_fun.SCCBwriteByte(ucSubAddr)) {
                puts("h");
                continue;
            }

            if (0 == ark7116_iic_fun.SCCBwriteByte(ucRegVal)) {
                puts("i");
                continue;
            }
            break;
#endif
        }
        dataPt++;
    }
    /*ark7116_iic_fun.stopSCCB();*/
    dev_ioctl(iic, IIC_IOCTL_TX_STOP_BIT, 0);
}

/***********************************************************
*name:    ConfigPadMuxPara()
*input:   non
*output:  non
*Update:  2011-11-18
*state:   allright

*description:
         这个函数是配置PadMux的静态参数。

*History:  yshuizhou   2011/12/06    1.0    build  this  function
************************************************************/
void ConfigPadMuxPara(void)
{
    //int i;
    //printfStr("ConfigPadMuxPara");
    AMT_WriteStaticPara(AMT_PadMuxStaticPara, PAD_MUX_NUM);

//	for(i=0;i<sizeof(AMT_PadMuxStaticPara)/sizeof(PanlstaticPara);i++)
//    {
//        wrARK7116Reg(AMT_PadMuxStaticPara[i].addr,AMT_PadMuxStaticPara[i].value);
//    }
}


/****************************************************************************
*name:   InitChip(void)
*input:  void
*output: void

*description:
      初始化Chip 函数。

*history: yshuizhou  2014/10/29    1.0    buil   this  function
*****************************************************************************/
void InitChip(void)
{
    //printfStr("InitChip");

    unsigned char readVal;

resetAMT630A:
    SET_RESET_OUT_B();
    SET_RESET_H_B();
    ARK7116_Delay(40000);
    SET_RESET_L_B();
    ARK7116_Delay(40000);
    SET_RESET_H_B();
    ARK7116_Delay(40000);
    SET_RESET_L_B();

    ConfigSlaveMode();

    wrARK7116Reg(0XFAC6, 0x40);
    rdARK7116Reg(0XFAC6, &readVal);
    printf("**************readVal = %d****\n", readVal);
    if (0X40 != readVal) {
//        goto resetAMT630A;
//        ark7116_reset();
    }
    //link ok
//   while(1)
    {
        InitGlobalPara();
    }

    //InitOSD();
//
//   OSDsetup(0x68,0x25,10,9,COLOR(GREEN,TRANSPARENCE));
//   OsdDrawStr(0,0,COLOR(YELLOW,RED), "&\x000 BRIGHT     ");
//   OsdDrawStr(1,0,COLOR(YELLOW,TRANSPARENCE), "&\x001 CONTRAST");
//   OsdDrawStr(2,0,COLOR(YELLOW,TRANSPARENCE), "&\x002 COLOR");
//   OsdDrawStr(3,0,COLOR(YELLOW,TRANSPARENCE), "&\x003 RING VOL");
//   OsdDrawStr(4,0,COLOR(YELLOW,TRANSPARENCE), "&\x004 RESET ");
//
//   OsdDrawStr(6,0,COLOR(YELLOW,TRANSPARENCE), "&\x000");
//   OsdDrawNum(6,7,COLOR(YELLOW,TRANSPARENCE), 10);
//   OsdDrawGuage(7, 0, MAX_VALUE, COLOR(YELLOW,TRANSPARENCE),10);
//
//   OsdConfigScaler(H_ZOOM,1);

//   OsdHide();//osd disable



    /*
        OSDsetup(0x58,0x50,11,1,COLOR(GREEN,TRANSPARENCE));
        OsdDrawStr(0, 0, COLOR(RED,TRANSPARENCE), "SUCCESS!");
        OsdConfigScaler(HV_ZOOM, 2);
        OsdConfigBlink(ENABLE, 0X07,0x01, 0x10, 0x00, 0x10);*/

}

/****************************************************************************
*name:   InitARKChip(void)
*input:  void
*output: void

*description:
      初始化ARK CHIP 函数。

*history: Jordan.chen   2011/12/07    0.1    buil   this  function
*****************************************************************************/
void InitARKChip(void)
{

    InitChip();
    ARK7116_Delay(500);
}


/****************************************************************************
*name:   AMT_WriteColorSysDynPara(PanlSysDynPara * dataPt,u32 num,u8 currentSys)
*input:
*output:

*description:配置图像制式参数
*history:   yshuizhou   2013/08/08    1.0    build   this  function
*****************************************************************************/
void AMT_WriteColorSysDynPara(PanlSysDynPara *dataPt, unsigned int num, unsigned char currentSys)
{
    unsigned char ucLoop;
    unsigned char ucDeviceAddr;
    unsigned char uctmpDeviceAddr;
    unsigned char ucSubAddr;
    unsigned char ucRegVal;

    while (num--) {
        ucLoop = I2C_ACCESS_LOOP_TIME;
        uctmpDeviceAddr = (unsigned char)((((*dataPt).addr) >> 8) & 0XFF);
        ucSubAddr = (unsigned char)(((*dataPt).addr) & 0XFF);
        ucRegVal = (*dataPt).dat_sysDyn[currentSys];



        switch (uctmpDeviceAddr) {
        case 0XF9:
        case 0XFD:
            ucDeviceAddr = 0XB0;
            break;

        case 0XFA:
            ucDeviceAddr = 0XBE;
            break;

        case 0XFB:
            ucDeviceAddr = 0XB6;
            break;

        case 0XFC:
            ucDeviceAddr = 0XB8;
            break;

        case 0XFE:
            ucDeviceAddr = 0XB2;
            break;

        case 0XFF:
            ucDeviceAddr = 0XB4;
            break;

        case 0X00:
            ucDeviceAddr = 0XBE;
            break;

        default:
            ucDeviceAddr = 0XB0;
            break;
        }

        while (ucLoop--) {
            if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, ucDeviceAddr)) {
                continue;
            }

            if (dev_ioctl(iic, IIC_IOCTL_TX, ucSubAddr)) {
                continue;
            }

            if (dev_ioctl(iic, IIC_IOCTL_TX, ucRegVal)) {
                continue;
            }
            break;
#if 0
            ark7116_iic_fun.startSCCB();
            if (0 == ark7116_iic_fun.SCCBwriteByte(ucDeviceAddr)) {
                puts("j");
                continue;
            }

            if (0 == ark7116_iic_fun.SCCBwriteByte(ucSubAddr)) {
                puts("k");
                continue;
            }

            if (0 == ark7116_iic_fun.SCCBwriteByte(ucRegVal)) {
                puts("l");
                continue;
            }
            break;
#endif
        }
        dataPt++;
    }
    /*ark7116_iic_fun.stopSCCB();*/
    dev_ioctl(iic, IIC_IOCTL_TX_STOP_BIT, 0);
}


/****************************************************************************
*name:   ConfigSlaveMode(void)
*input:  CtrlMode
*output: void

*description:
*history: yshuizhou   2014/10/29    1.0    transplant   this  function
*****************************************************************************/
void ConfigSlaveMode(void)
{
    u8 AddrBuff[6] = {0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6};
    u8 DataBuff[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    u8 i;

    //printfStr("ConfigSlaveMode");

    DataBuff[0] = 0X55;
    DataBuff[1] = 0xAA;
    DataBuff[2] = 0X03;
    DataBuff[3] = 0X50;  //slave mode
    DataBuff[4] = 0;     // crc val
    DataBuff[5] = DataBuff[2] ^ DataBuff[3] ^ DataBuff[4];

    //AMT_WriteReg(MCU_CFG_ADDR,0X20);
    //AMT_WriteReg(RSTN,0X00);
    //AMT_WriteReg(RSTN,0X5A);

    wrARK7116Reg(BUS_STATUS_ADDR, 0x00);  //I2c Write Start
    printf("************ConfigSlaveMode wrARK7116Reg************\n");
    for (i = 0; i < 6; i++) {
        wrARK7116Reg(AddrBuff[i], DataBuff[i]);
    }
    wrARK7116Reg(BUS_STATUS_ADDR, 0x11);  //I2c Write End
    ARK7116_Delay(50);
}

/****************************************************************************
*name:   AMT_WriteDispZoomDynPara(PanlSysDynPara * dataPt,u32 num,u8 currentSys)
*input:
*output:

*description:配置显示模式参数。
*history:   yshuizhou   2013/08/08    1.0    build   this  function
*****************************************************************************/
void AMT_WriteDispZoomDynPara(PanlPosDynPara *dataPt, unsigned int num, unsigned char currentmode)
{
    unsigned char ucLoop;
    unsigned char ucDeviceAddr;
    unsigned char uctmpDeviceAddr;
    unsigned char ucSubAddr;
    unsigned char ucRegVal;

    while (num--) {
        ucLoop = I2C_ACCESS_LOOP_TIME;
        uctmpDeviceAddr = (unsigned char)((((*dataPt).addr) >> 8) & 0XFF);
        ucSubAddr = (unsigned char)(((*dataPt).addr) & 0XFF);
        ucRegVal = (*dataPt).dat_posDyn[currentmode];



        switch (uctmpDeviceAddr) {
        case 0XF9:
        case 0XFD:
            ucDeviceAddr = 0XB0;
            break;

        case 0XFA:
            ucDeviceAddr = 0XBE;
            break;

        case 0XFB:
            ucDeviceAddr = 0XB6;
            break;

        case 0XFC:
            ucDeviceAddr = 0XB8;
            break;

        case 0XFE:
            ucDeviceAddr = 0XB2;
            break;

        case 0XFF:
            ucDeviceAddr = 0XB4;
            break;

        case 0X00:
            ucDeviceAddr = 0XBE;
            break;

        default:
            ucDeviceAddr = 0XB0;
            break;
        }

        while (ucLoop--) {
            if (dev_ioctl(iic, IIC_IOCTL_TX_WITH_START_BIT, ucDeviceAddr)) {
                continue;
            }

            if (dev_ioctl(iic, IIC_IOCTL_TX, ucSubAddr)) {
                continue;
            }

            if (dev_ioctl(iic, IIC_IOCTL_TX, ucRegVal)) {
                continue;
            }
            break;
#if 0
            ark7116_iic_fun.startSCCB();
            if (0 == ark7116_iic_fun.SCCBwriteByte(ucDeviceAddr)) {
                puts("m");
                continue;
            }

            if (0 == ark7116_iic_fun.SCCBwriteByte(ucSubAddr)) {
                puts("n");
                continue;
            }

            if (0 == ark7116_iic_fun.SCCBwriteByte(ucRegVal)) {
                puts("o");
                continue;
            }
            break;
#endif
        }
        dataPt++;
    }
    /*ark7116_iic_fun.stopSCCB();*/
    dev_ioctl(iic, IIC_IOCTL_TX_STOP_BIT, 0);
}


/***********************************************************
*name:    ConfigStaticPara()
*input:     CurretSource
*output:    non
*Update:    2011-11-18
*state:     allright

*description:
         这个函数是配置不同通道的静态参数。

*History:  yshuizhou   2011/12/06    1.0    build  this  function
************************************************************/
void ConfigStaticPara(unsigned char CurretSource)
{
    //printfStr("ConfigStaticPara");
    CurretSource = CurretSource;
    AMT_WriteStaticPara(VideoChannelPara[0].VideoPara.pVideoStaicPara, STATIC_NUM);
}


/***********************************************************
*name:     	ConfigColorSysDynPara(u8 currentSys)
*input:     currentSys
*output:    non
*update:    2011-11-18
*state:     allright

*description:
         这个函数是实时配置不同图像制式参数。

*History:  yshuizhou   2011/12/06    1.0    build  this  function
************************************************************/
void ConfigColorSysDynPara(u8 currentSys)
{
    //printfStr("ConfigColorSysDynPara");
    AMT_WriteColorSysDynPara(VideoChannelPara[0].VideoPara.pVideoSysDynPara, SYS_DYN_NUM, currentSys);
    rdARK7116Reg(BRIGHT_REG, &g_ucbrightness);
    rdARK7116Reg(CONTRAST_REG, &g_ucContrast);
    rdARK7116Reg(SATURATION_REG, &g_ucSaturation);
}



/***********************************************************
*name:     	ConfigDispZoomDynPara(u8 currentmode)
*input:     currentmode
*output:    non
*update:    2011-11-18
*state:     allright

*description:
         这个函数是配置不同通道16:9/4:3显示的参数。

*History:  yshuizhou   2011/12/06    1.0    build  this  function
************************************************************/
void ConfigDispZoomDynPara(u8 currentmode)
{
    //printfStr("ConfigDispZoomDynPara");
    AMT_WriteDispZoomDynPara(VideoChannelPara[0].VideoPara.pVideoPosDynPara, POS_DYN_NUM, currentmode);
}

/**************************************************************/

#endif

/*void ark7116_iic_set(u8 isp_dev)*/
/*{*/
/*iic_select(&ark7116_iic_fun, isp_dev);*/
/*}*/

void ark7116_reset()
{
    SET_RESET_OUT_B();
    SET_RESET_H_B();
    ARK7116_Delay(40000);
    SET_RESET_L_B();
    ARK7116_Delay(40000);
    SET_RESET_H_B();
    ARK7116_Delay(40000);
}

//void ark7116_iic_init_io(void)
//{
//    SCCB_SID_OUT_B;         //SDA设置成输出
////    PORTG_DIR &= ~BIT(10);PORTG_PU |= BIT(10);PORTG_PD &=~BIT(10);
////    PORTG_DIR &= ~BIT(11);PORTG_PU |= BIT(11);PORTG_PD &=~BIT(11);
//    SCCB_SIC_OUT_B;         	//SCL设置成输出
//	SCCB_SIC_H_B();
//	delay(300);
//    SCCB_SID_H_B();
//    delay(300);
//
//}

s32 ark7116_id_check()
{
//    unsigned char readVal = 0;
//    rdARK7116Reg(0XFAC6, &readVal);
//    //if (readVal == 0x20)
//    {
//        printf ("ARK7116 ID is : %x", readVal);
    return 1;
//    }

    //return 0;
}

void ARK7116_xclk_set(u8 isp_dev)
{
//	if (isp_dev==0)
//	{
//		ISP0_XCLK_IO_ABLE();
//	}
//	else
//	{
    /*ISP1_XCLK_IO_ABLE(clk_gpio);*/
//	}
}

u8 check7116_data_valid(void)
{
    unsigned char readVal;
    rdARK7116Reg(0XFA26, &readVal);
    return (readVal & BIT(1));
}

s32 ark7116_check(u8 isp_dev)
{
    static u8 ID_ok = 0;

    if (!iic) {
        if (isp_dev == ISP_DEV_0) {
            iic = dev_open("iic0", 0);
        } else {
            iic = dev_open("iic1", 0);
        }
        if (!iic) {
            return -1;
        }
    }

    puts("\n ark7116_check \n");
    ARK7116_xclk_set(isp_dev);
    puts("\n ARK7116_xclk_set \n");
    /*ark7116_iic_set(isp_dev);*/
    puts("\n ark7116_id_check \n");
    if (ID_ok) {
        return 0;
    }

//    ark7116_reset();
//    ark7116_iic_init_io();
    puts("\n ark7116_id_check \n");
    if (0 == ark7116_id_check()) {
        dev_close(iic);
        iic = NULL;
        return -1;
    }
    ID_ok = 1;
    return 0;
}

s32 ark7116_set_output_size(u16 *width, u16 *height, u8 *freq)
{
    *width = 720;
    *height = 480;
    *freq = 50 / 2;
    return 0;
}

void ARK7116_config_SENSOR(u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
    static u8 init_ok = 0;
    if (init_ok == 0) {
        InitARKChip();
    }

    printf("***********************InitARKChip***********************************\n");
    init_ok = 1;
    *format = SEN_IN_FORMAT_UYVY;

    ark7116_set_output_size(width, height, frame_freq);
}

s32 ark7116_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq)
{
//	puts("\n\n gm7150_initialize \n\n");

//	if (0 != gm7150_check(isp_dev))
//	{
//		return -1;
//	}

    ARK7116_config_SENSOR(width, height, format, frame_freq);

    return 0;
}

s32 ark7116_power_ctl(u8 isp_dev, u8 is_work)
{
    return 0;
}
