#ifndef APP_CONFIG_H
#define APP_CONFIG_H




#define CONFIG_BOARD_MIPI_V3
//#define CONFIG_BOARD_MIPI_V3_LONGSCREEN
//#define CONFIG_BOARD_MIPI_V4
//#define CONFIG_BOARD_MIPI_DEVELOP_BOARD
//#define CONFIG_BOARD_DEVELOP_20170411
//#define CONFIG_BOARD_MIPI_TOUCH_V1
//#define CONFIG_BOARD_MIPI_TOUCH_20170419
//#define CONFIG_BOARD_MIPI_20170330
//#define CONFIG_BOARD_MIPI_V3_TOUCH
//#define CONFIG_BOARD_MIPI_AC5403_20170906


#ifdef CONFIG_BOARD_MIPI_V3_LONGSCREEN
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define LCD_DSI_VDO_4LANE_1600x400_WTL098802G01_1       /* LCD选择 */
#define CONFIG_UI_STYLE_LY_LONGSCREEN_ENABLE
#ifdef CONFIG_VIDEO3_ENABLE
#error "VIDEO3 NOT SUPPORT IN THIS CASE"
#endif
#endif


#ifdef CONFIG_BOARD_MIPI_V3
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
//#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define LCD_ILI8961C       /* LCD选择 */
//#define LCD_ST7789S_MCU

//#define GSENSOR_UP   //猿人，接ACC的特殊操作，停车开机后，录像一分钟后再关机，还需要快捷键操作 
//#define LCD_9341_360x640
#endif



#ifdef CONFIG_BOARD_MIPI_V4
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
//#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD选择 */
#endif



#ifdef CONFIG_BOARD_MIPI_DEVELOP_BOARD
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
//#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD选择 */
#endif



#ifdef CONFIG_BOARD_MIPI_TOUCH_V1
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择  */
#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
//#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
#define CONFIG_GT911                            /* 触摸屏选择 */
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格 */
#define LCD_DSI_VDO_4LANE_MIPI_ICN9706          /* LCD选择 */
#endif



#ifdef CONFIG_BOARD_MIPI_AC5403_20170906
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择  */
#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能  */
//#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能  */
#define CONFIG_UI_ENABLE                        /* UI开关  */
#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关  */
#define CONFIG_GT911                            /* 触摸屏选择  */
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关  */
#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格  */
#define LCD_DSI_VDO_4LANE_MIPI_ICN9706          /* LCD选择  */
#endif





#define CONFIG_DATABASE_2_RTC                   /* 系统配置存RTC */
//#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
#define CONFIG_PAP_ENABLE                       /* PAP模块使能  */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#define CONFIG_ADKEY_ENABLE                     /* AD按键开关  */
#define CONFIG_IOKEY_ENABLE                     /* IO按键开关  */
#define CONFIG_GSENSOR_ENABLE                   /* 重力感应开关  */
//#define KEY_MODE_EXIT_MENU                      /* UI菜单MODE键退出  */
//#define USE_VE_MOTION_DETECT_MODE_ISP			[>移动侦测使用MODE_ISP, 否则使用MODE_NORMAL<]

#ifdef CONFIG_TOUCH_UI_ENABLE
#define CONFIG_FILE_PREVIEW_ENABLE              /* 预览图使能 */
#endif

#ifdef CONFIG_VIDEO1_ENABLE
#define CONFIG_AV10_SPI_ENABLE                  /* AV10 SPI开关  */
#endif


#ifdef CONFIG_SD0_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd0"
#define SDX_DEV					"sd0"
#endif

#ifdef CONFIG_SD1_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd1"
#define SDX_DEV					"sd1"
#endif

#ifdef CONFIG_SD2_ENABLE
#define CONFIG_STORAGE_PATH 	"storage/sd2"
#define SDX_DEV					"sd2"
#endif

#define CONFIG_ROOT_PATH     	CONFIG_STORAGE_PATH"/C/"
#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/DCIM/1/"
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/DCIM/2/"

#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_2

#define CONFIG_DEC_PATH_1 	    CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_2 	    CONFIG_REC_PATH_2
#define CONFIG_DEC_PATH_3 	    CONFIG_REC_PATH_1

#define MAX_FILE_NAME_LEN       64
#define FILE_SHOW_NUM           12  /* 一页显示文件数 */




#ifndef __LD__
#include "board.h"
#endif































#endif

