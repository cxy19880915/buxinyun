#ifndef __LCD_CONFIG_H__
#define __LCD_CONFIG_H__

#include "app_config.h"




/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (defined LCD_AVOUT)

#define NTSC_720		0
#define NTSC_896		1
#define PAL_720			2
#define PAL_896  		3

#define AVOUT_MODE NTSC_720

#if (AVOUT_MODE == NTSC_720)
#define LCD_DEV_WIDTH	720
#define LCD_DEV_HIGHT	480
#elif (AVOUT_MODE == NTSC_896)
#define LCD_DEV_WIDTH	896
#define LCD_DEV_HIGHT	480
#elif (AVOUT_MODE == PAL_720)
#define LCD_DEV_WIDTH	720
#define LCD_DEV_HIGHT	576
#elif (AVOUT_MODE == PAL_896)
#define LCD_DEV_WIDTH	896
#define LCD_DEV_HIGHT	576
#endif

#endif


#if (defined LCD_DSI_DCS_1LANE_16BIT) ||\
    (defined LCD_DSI_DCS_1LANE_24BIT)
#define LCD_DEV_WIDTH	640
#define LCD_DEV_HIGHT	360
#endif


#if (defined LCD_DSI_DCS_2LANE_24BIT)
#define LCD_DEV_WIDTH	854
#define LCD_DEV_HIGHT	480
#endif

#if (defined LCD_RGB_DX0860EBAA5)
#define LCD_DEV_WIDTH	720
#define LCD_DEV_HIGHT	480
#endif

#if (defined LCD_ILI8961C)
#define LCD_DEV_WIDTH	320
#define LCD_DEV_HIGHT	240
#endif



#if (defined LCD_DSI_DCS_4LANE_1080P)
/*插值*/
#define LCD_DEV_WIDTH	1280//1920
#define LCD_DEV_HIGHT	720//1080
#endif

#if (defined LCD_DSI_DCS_4LANE_720P)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif

#if (defined LCD_DSI_VDO_4LANE_720P_RM68200GA1)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif

#if (defined LCD_480x272) || \
    (defined LCD_480x272_8BITS)
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	272
#endif

#if (defined LCD_DSI_VDO_4LANE_MIPI) || \
    (defined LCD_DSI_VDO_2LANE_MIPI_EK79030) || \
    (defined LCD_DSI_VDO_4LANE_MIPI_EK79030) || \
    (defined LCD_DSI_VDO_4LANE_MIPI_ICN9706)

#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	480
#endif

#if (defined LCD_HX8369A_MCU)|| \
    (defined LCD_HX8369A_SPI_RGB)
#define LCD_DEV_WIDTH	800
#define LCD_DEV_HIGHT	480
#endif

#if (defined LCD_ST7789S_MCU)
#define LCD_DEV_WIDTH	320
#define LCD_DEV_HIGHT	240
#endif

#ifdef LCD_ILI9805C
#define LCD_DEV_WIDTH	800
#define LCD_DEV_HIGHT	480

#endif

#if (defined LCD_MIPI_4LANE_S8500L0)
/*插值*/
#define LCD_DEV_WIDTH	864//1296(被16整除)
#define LCD_DEV_HIGHT	224//336（被16整除）
#endif


#if (defined LCD_DSI_VDO_4LANE_1280x400)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	400
#endif

#if (defined LCD_DSI_VDO_4LANE_1600x400_WTL098802G01_1)
#define LCD_DEV_WIDTH	1600
#define LCD_DEV_HIGHT	400
#endif

#ifdef LCD_DSI_VDO_4LANE_MIPI_NT35523
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif


#define LCD_DEV_SAMPLE	SAMP_YUV444
#define LCD_DEV_BNUM	2//同步显示BUF数量
#define LCD_DEV_BUF     (u32)lcd_dev_buf_addr

#ifndef LCD_DEV_WIDTH
#error LCD_DEV_WIDTH isn't definition!!!
#endif
#ifndef LCD_DEV_HIGHT
#error LCD_DEV_HIGHT isn't definition!!!
#endif


#endif // __LCD_CONFIG_HH

