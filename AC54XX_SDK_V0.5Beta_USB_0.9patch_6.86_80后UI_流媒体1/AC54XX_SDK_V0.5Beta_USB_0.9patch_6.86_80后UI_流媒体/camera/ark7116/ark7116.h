#ifndef _ARK7116_H_
#define _ARK7116_H_

#include "cpu.h"
#include "typedef.h"
#include "asm/isp_dev.h"

/********************************************
[PannelName]
SAT070AT50DHY0
[PannelType]
D_RGB888
[PannelResolution]
 800
 480
1056
 525
  20
  10
  46
  23
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
  -1
Update date:Thursday, August 27, 2015
Update time:11:37:50
History:
********************************************/

typedef struct _PanlstaticPara {
    unsigned int addr;
    unsigned char dat;
} PanlstaticPara;

typedef struct _PanlPosDynPara {
    unsigned int addr;
    unsigned char dat_posDyn[6];
} PanlPosDynPara;

typedef struct _PanlSysDynPara {
    unsigned int addr;
    unsigned char dat_sysDyn[8];
} PanlSysDynPara;


typedef enum _InputSourceIDType {
    INPUT_AV = 0,
    INPUT_AV1,
    INPUT_AV2,
    INPUT_TV,
    INPUT_CAMERA_DoorBell,
    INPUT_CAMERA_Car,
    INPUT_SVIDEO,
    INPUT_ITU656,
    INPUT_FM,
    INPUT_YPBPR,
    INPUT_VGA,
    MAX_VIDEO_CHANNEL,
    ALL_INPUT_SOURCE = 0XFF,
} InputSourceType;


/*屏参参数相关的结构体*/
typedef struct _PannelPara {
    PanlstaticPara  *pVideoStaicPara;
    PanlPosDynPara *pVideoPosDynPara;
    PanlSysDynPara  *pVideoSysDynPara;
} PannelPara;
typedef struct _VideoChannel {
    unsigned char INPUT_ID;
    PannelPara    VideoPara;
} VideoChannel;

#define STATIC_NUM 136
#define POS_DYN_NUM 0
#define SYS_DYN_NUM 0
#define PAD_MUX_NUM 18


#ifdef _AMT630A_MCU_C_
#define _AMT630A_MCU_C_
#else
#define _AMT630A_MCU_C_ extern
#endif

//typedef unsigned int    UINT;
//typedef unsigned char	  UCHAR;
//typedef unsigned long   ULONG;
//typedef char            CHAR;


//MCU CFG Addr
#define MCU_CFG_ADDR 				0xC6

//BUS Addr
#define BUS_STATUS_ADDR         	0xAF

#define RSTN                        0XFD00

typedef enum _VdeOutputType {
    VDE_CLOSE = 0,
    VDE_RED,
    VDE_GREEN,
    VDE_BLUE,
    VDE_GRAY,
    VDE_WHITE,
    VDE_BLACK,
    MAX_VDECOLOR = VDE_BLACK,
} VdeOutputType;

//===================================================================================
//定义变量
//===================================================================================
#define DISP_16_9	0
#define PAL			0


typedef struct _PowerSettingInfo {
    u8 mode;
    u8 status;
} PowerInfoType;


typedef struct	_VideoSettingInfo {              //OSD参数
    u8 curSource; //AV TV DVD CAMERA ...
    u8 videoType; //cvbs / yc / itu656 / ypbpr / rgb ...
    u8 colorSys;

    u8 brigthness;
    u8 contrast;
    u8 saturation;
    u8 tint;
    u8 palthlenH;
    u8 palthlenL;
    u8 ntscthlenH;
    u8 ntscthlenL;
    u8 nosiceSw;
    u8 ScreeSw;
} VideoInfoType;

typedef struct	_OsdSettingInfo {              //OSD参数
    u8 defLanguage;
    u8 curlanguage;		    //OSD语言
    u32  dispTime;         //OSD消失时间
    u32  storageTime;      //OSD记忆数据时间
    u8 alph;             //OSD透明度
    u8 xPos;	            //OSD菜单水平偏移
    u8 yPos;             //OSD菜单垂直偏移
} OsdInfoType;

typedef struct _DispSettingInfo {
    u8 zoomMode;
    u8 flipMode;
    u8 udAdj;
    u8 lrAdj;
} DispInfoType;

typedef struct _LogoSettingInfo {
    u8 logoID;
    u8 ColorMode;
    u32  FontRamNum;
    u32 StarAddr;
} LogoInfoType;

typedef  struct _sysWorkPara {
    u8           OXAA;

    //PowerInfoType   Power;
    VideoInfoType   Video;
    OsdInfoType     Osd;		       //OSD参数
    DispInfoType    Disp;
    LogoInfoType    Logo;
    u8           OX55;
    u8           xorSum;            //系统参数校验和
} SysWorkParaType;

_AMT630A_MCU_C_ SysWorkParaType g_sysSetting;
_AMT630A_MCU_C_ u8  g_ucbrightness;
_AMT630A_MCU_C_ u8  g_ucContrast;
_AMT630A_MCU_C_ u8  g_ucSaturation;


#define BRIGHT_REG               		0XFFD4
#define CONTRAST_REG            		0XFFD3
#define SATURATION_REG          		0XFFD6
#define TINT_REG                        0XFFD5
#define VDE_REG                    		0XFFD2

u8  check7116_data_valid(void);
s32 ark7116_check(u8 isp_dev);
s32 ark7116_initialize(u8 isp_dev, u16 *width, u16 *height, u8 *format, u8 *frame_freq);
s32 ark7116_set_output_size(u16 *width, u16 *height, u8 *freq);
s32 ark7116_power_ctl(u8 isp_dev, u8 is_work);
void ark7116_reset();
void InitGlobalPara();
void AMT_WriteStaticPara(PanlstaticPara *dataPt, u32 num);
void ConfigPadMuxPara(void);
void InitChip(void);
void InitARKChip(void);
void AMT_WriteColorSysDynPara(PanlSysDynPara *dataPt, unsigned int num, unsigned char currentSys);
void ConfigSlaveMode(void);
void AMT_WriteDispZoomDynPara(PanlPosDynPara *dataPt, unsigned int num, unsigned char currentmode);
void ConfigStaticPara(unsigned char CurretSource);
void ConfigColorSysDynPara(u8 currentSys);
void ConfigDispZoomDynPara(u8 currentmode);

#endif
