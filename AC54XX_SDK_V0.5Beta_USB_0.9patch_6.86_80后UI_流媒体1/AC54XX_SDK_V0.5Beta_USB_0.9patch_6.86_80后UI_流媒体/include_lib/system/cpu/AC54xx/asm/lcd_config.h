#ifndef __LCD_CONFIG_H__
#define __LCD_CONFIG_H__

#include "app_config.h"

/**
 * dvp lcd panel
*/
#define LCD_HX8369A_MCU			0
#define LCD_HX8369A_SPI_RGB		1
#define LCD_ILI8960				2
#define LCD_ILI9806				3
#define LCD_LQ026B3UX01         4
#define LCD_OTA5182A            5
#define LCD_LYWS030GQ           6
#define LCD_LX50FWB4001_V2      7
#define LCD_AVOUT               8
#define LCD_480x272             9
#define LCD_480x272_8BITS       10
#define LCD_ST7789S_MCU			11
/**
 * mipi lcd panel
*/

#define MIPI_BASE_NUM		20
#define LCD_DSI_DCS_1LANE_16BIT				(MIPI_BASE_NUM+1)
#define LCD_DSI_DCS_1LANE_24BIT				(MIPI_BASE_NUM+2)
#define LCD_DSI_DCS_2LANE_24BIT				(MIPI_BASE_NUM+3)
#define LCD_DSI_DCS_4LANE_1080P				(MIPI_BASE_NUM+4)
#define LCD_DSI_DCS_4LANE_720P				(MIPI_BASE_NUM+5)
#define LCD_DSI_VDO_4LANE_720P_RM68200GA1	(MIPI_BASE_NUM+6)
#define LCD_DSI_VDO_4LANE_MIPI      		(MIPI_BASE_NUM+7)  //??
#define LCD_MIPI_4LANE_S8500L0				(MIPI_BASE_NUM+8)
#define LCD_DSI_VDO_4LANE_1280x480_EK79030				(MIPI_BASE_NUM+9)   //??????
#define LCD_DSI_VDO_4LANE_1280x480_LX68HD				(MIPI_BASE_NUM+10)  //????
#define LCD_DSI_VDO_4LANE_1280x480_HX8394				(MIPI_BASE_NUM+11)  //?????

#define LCD_DSI_VDO_4LANE_1280x800				(MIPI_BASE_NUM+12)  
#define LCD_DSI_VDO_4LANE_HX839D_BOE686         (MIPI_BASE_NUM+13) 
#define LCD_DSI_VDO_4LANE_1280x720_SSD2201         (MIPI_BASE_NUM+14) 
#define LCD_DSI_VDO_4LANE_1280x480_WT686     (MIPI_BASE_NUM+15)
//#define LCD_DSI_VDO_2LANE_864x480               
#ifdef CONFIG_TOUCH_UI_ENABLE
#define LCD_TYPE   LCD_DSI_VDO_4LANE_1280x480_WT686
#else
#define LCD_TYPE   LCD_DSI_VDO_4LANE_1280x480_WT686
#endif

#define IF_ENABLE(LCD) (LCD_TYPE == LCD)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_AVOUT)

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


#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x480_WT686)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	480

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_DSI_DCS_1LANE_16BIT)||(LCD_TYPE == LCD_DSI_DCS_1LANE_24BIT)
#define LCD_DEV_WIDTH	640
#define LCD_DEV_HIGHT	360
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_DSI_DCS_2LANE_24BIT)
#define LCD_DEV_WIDTH	854
#define LCD_DEV_HIGHT	480
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_DSI_DCS_4LANE_1080P)
/*插值*/
#define LCD_DEV_WIDTH	1280//1920
#define LCD_DEV_HIGHT	720//1080
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_DSI_DCS_4LANE_720P)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_DSI_VDO_4LANE_720P_RM68200GA1 || LCD_DSI_VDO_4LANE_1280x720_SSD2201 == LCD_TYPE)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	720
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_480x272) || (LCD_TYPE == LCD_480x272_8BITS)
#define LCD_DEV_WIDTH	480
#define LCD_DEV_HIGHT	272
#endif

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_MIPI)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	480
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_HX8369A_MCU)||(LCD_TYPE == LCD_HX8369A_SPI_RGB)
#define LCD_DEV_WIDTH	800
#define LCD_DEV_HIGHT	480
#endif

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x480_LX68HD)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	480
#endif

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x480_EK79030 || LCD_DSI_VDO_4LANE_HX839D_BOE686 == LCD_TYPE)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	480
#endif

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x800)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	800
#endif

#if (LCD_TYPE == LCD_DSI_VDO_4LANE_1280x480_HX8394)
#define LCD_DEV_WIDTH	1280
#define LCD_DEV_HIGHT	480
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_ST7789S_MCU)
#define LCD_DEV_WIDTH	320
#define LCD_DEV_HIGHT	240
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if (LCD_TYPE == LCD_MIPI_4LANE_S8500L0)
/*插值*/
#define LCD_DEV_WIDTH	864//1296(被16整除)
#define LCD_DEV_HIGHT	224//336（被16整除）
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define LCD_DEV_SAMPLE	SAMP_YUV420
#define LCD_DEV_BNUM	2//同步显示BUF数量
#define LCD_DEV_BUF     (u32)lcd_dev_buf_addr


void ui_lcd_light_on(void);
void ui_lcd_light_on_low(void);
void ui_lcd_light_off(void);

#ifndef LCD_DEV_WIDTH
#error LCD_DEV_WIDTH isn't definition!!!
#endif
#ifndef LCD_DEV_HIGHT
#error LCD_DEV_HIGHT isn't definition!!!
#endif

extern void delay_2ms(u32 cnt);

#endif // __LCD_CONFIG_HH
