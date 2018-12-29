#ifndef APP_CONFIG_H
#define APP_CONFIG_H





//#define CONFIG_BOARD_MIPI_V3
//#define CONFIG_BOARD_MIPI_V4
#define CONFIG_BOARD_MIPI_DEVELOP_BOARD
//#define CONFIG_BOARD_DEVELOP_20170411
//#define CONFIG_BOARD_MIPI_TOUCH_V1
//#define CONFIG_BOARD_MIPI_TOUCH_20170419
//#define CONFIG_BOARD_MIPI_20170330
//#define CONFIG_BOARD_MIPI_V3_TOUCH
//#define CONFIG_BOARD_MIPI_AC5403_20170906
//#define CONFIG_BOARD_AC54_L86
//#define CONFIG_BOARD_AC56_L86
//#define CONFIG_BOARD_HHF_HANGPAI
// #define CONFIG_BOARD_BBM_SENDER_BOARD
// #define CONFIG_BOARD_BBM_RECEIVE_BOARD
// #define CONFIG_BOARD_FLY_BOARD
// #define CONFIG_BOARD_SOCKET_5606_20180518
// #define CONFIG_BOARD_SOCKET_5602_20180510
// #define CONFIG_BOARD_SOCKET_5601_20180507
// #define CONFIG_BOARD_SOCKET_5603_20180522
//#define CONFIG_BOARD_IPC


#ifdef CONFIG_BOARD_SOCKET_5606_20180518
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
// #define CONFIG_ETH_PHY_ENABLE
// #define CONFIG_WIFI_ENABLE
// #define CONFIG_IOKEY_ENABLE                     [> IO按键开关  <]
#define LCD_AVOUT       /* LCD选择 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
#endif

#ifdef CONFIG_BOARD_SOCKET_5603_20180522
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
// #define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 网络后摄像头使能 <]
// #define CONFIG_VIDEO3_ENABLE 		            [> usb后摄像头使能 <]
#define CONFIG_UI_ENABLE                        /* UI开关 */
// #define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
// #define CONFIG_GT911_B                          [> 触摸屏选择 <]
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_JL02_ENABLE               /* UI风格 */
#define LCD_AVOUT       /* LCD选择 */
// #define CONFIG_ETH_PHY_ENABLE
#define CONFIG_WIFI_ENABLE
#define CONFIG_IOKEY_ENABLE                     /* IO按键开关  */
#endif

#ifdef CONFIG_BOARD_SOCKET_5602_20180510
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
// #define CONFIG_ETH_PHY_ENABLE
// #define CONFIG_WIFI_ENABLE
// #define CONFIG_IOKEY_ENABLE                     [> IO按键开关  <]
#define LCD_AVOUT       /* LCD选择 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define CONFIG_UI_ENABLE                        /* UI开关 */
#endif

#ifdef CONFIG_BOARD_SOCKET_5601_20180507
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
// #define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 网络后摄像头使能 <]
// #define CONFIG_VIDEO3_ENABLE 		            [> usb后摄像头使能 <]
#define CONFIG_UI_ENABLE                        /* UI开关 */
#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
#define CONFIG_GT911_B                          /* 触摸屏选择 */
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_JL02_ENABLE               /* UI风格 */
#define LCD_DSI_VDO_4LANE_MIPI_EK79030       /* LCD选择 */
// #define CONFIG_ETH_PHY_ENABLE
#define CONFIG_WIFI_ENABLE
#define CONFIG_IOKEY_ENABLE                     /* IO按键开关  */
#endif

#ifdef CONFIG_BOARD_FLY_BOARD
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD氓聧隆茅鈧€懊︹€孤?*/
//#define CONFIG_VIDEO1_ENABLE 		            /* 忙篓隆忙鈥古该ヂ惻矫︹€樷€灻テ捖徝ヂぢ疵ぢ铰棵ㄆ捖?*/
//#define CONFIG_VIDEO3_ENABLE 		            /* usb氓聬沤忙鈥樷€灻テ捖徝ヂぢ疵ぢ铰棵ㄆ捖?*/
//#define CONFIG_UI_ENABLE                        /* UI氓录鈧モ€?*/
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 猫搂娄忙鈥樎该ヂ甭徝ヂ尖偓氓鈥?*/
//#define CONFIG_GT911_B                          /* 猫搂娄忙鈥樎该ヂ甭徝┾偓鈥懊︹€孤?*/
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI氓录鈧モ€?*/
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI茅拢沤忙聽录 */
#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD茅鈧€懊︹€孤?*/
#endif

#ifdef CONFIG_BOARD_BBM_RECEIVE_BOARD
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD0_ENABLE                       /* SD卡选择 */
// #define CONFIG_VIDEO1_ENABLE 		            #<{(| 模拟后摄像头使能 |)}>#
// #define CONFIG_VIDEO3_ENABLE 		            #<{(| usb后摄像头使能 |)}>#
//#define CONFIG_VIDEO4_ENABLE 	                	/*RF视频接收使能*/
#define CONFIG_UI_ENABLE                        /* UI开关 */
// #define CONFIG_TOUCH_PANEL_ENABLE               #<{(| 触摸屏开关 |)}>#
// #define CONFIG_GT911_B                          #<{(| 触摸屏选择 |)}>#
// #define CONFIG_TOUCH_UI_ENABLE                  #<{(| TOUCH UI开关 |)}>#

// #define CONFIG_UI_STYLE_LY_ENABLE               #<{(| UI风格 |)}>#
// #define LCD_DSI_VDO_4LANE_720P_RM68200GA1       #<{(| LCD选择 |)}>#
#define LCD_DSI_VDO_4LANE_MIPI_EK79030          /* LCD选择  */
// #define LCD_DSI_VDO_4LANE_MIPI_NT35523          #<{(| LCD选择  |)}>#
// #define LCD_ILI9805C
#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格 */
// #define CONFIG_UI_STYLE_LY_ENABLE               #<{(| UI风格 |)}>#


#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
#define CONFIG_PAP_ENABLE                       /* PAP模块使能  */
// #define CONFIG_VIDEO0_ENABLE 		            #<{(| 前摄像头使能  |)}>#
#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#endif





#ifdef CONFIG_BOARD_BBM_SENDER_BOARD
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
// #define CONFIG_VIDEO1_ENABLE 		            #<{(| 模拟后摄像头使能 |)}>#
// #define CONFIG_VIDEO3_ENABLE 		            #<{(| usb后摄像头使能 |)}>#
// #define CONFIG_UI_ENABLE                        #<{(| UI开关 |)}>#
// #define CONFIG_TOUCH_PANEL_ENABLE               #<{(| 触摸屏开关 |)}>#
// #define CONFIG_GT911_B                          #<{(| 触摸屏选择 |)}>#
// #define CONFIG_TOUCH_UI_ENABLE                  #<{(| TOUCH UI开关 |)}>#
//#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
// #define LCD_DSI_VDO_4LANE_720P_RM68200GA1       #<{(| LCD选择 |)}>#
#define LCD_DSI_VDO_4LANE_MIPI_EK79030          /* LCD选择  */
// #define CONFIG_UI_STYLE_JL02_ENABLE             #<{(| UI风格 |)}>#


#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
// #define CONFIG_PAP_ENABLE                       #<{(| PAP模块使能  |)}>#
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
#endif



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



#ifdef CONFIG_BOARD_MIPI_V4
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
//#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
//#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能 */
//#define CONFIG_UI_ENABLE                        /* UI开关 */
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
//#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
//#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD选择 */
#endif



#ifdef CONFIG_BOARD_MIPI_DEVELOP_BOARD
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
//#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 网络后摄像头使能 <]
 //#define CONFIG_VIDEO3_ENABLE 		            [> usb后摄像头使能 <]
#define CONFIG_UI_ENABLE                        [> UI开关 <]
 //#define CONFIG_TOUCH_PANEL_ENABLE               [> 触摸屏开关 <]
 //#define CONFIG_GT911_B                          [> 触摸屏选择 <]
 //#define CONFIG_TOUCH_UI_ENABLE                  [> TOUCH UI开关 <]
//#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
//#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       [> LCD选择 <]

 #define LCD_ILI8961C          			[> LCD选择  <]
 #define CONFIG_UI_STYLE_LY_ENABLE             [> UI风格 <]
#endif



#ifdef CONFIG_BOARD_IPC
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO1_ENABLE 		            [> 模拟后摄像头使能 <]
// #define CONFIG_VIDEO4_ENABLE 		            [> 网络后摄像头使能 <]
//#define CONFIG_VIDEO3_ENABLE 		            [> usb后摄像头使能 <]
// #define CONFIG_UI_ENABLE                        [> UI开关 <]
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
#define CONFIG_UI_STYLE_LY_ENABLE               [> UI风格 <]
#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD选择 */
//#define LCD_DSI_VDO_4LANE_MIPI_EK79030          /* LCD选择  */
//#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格 */
#endif



#ifdef CONFIG_BOARD_HHF_HANGPAI
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择 */
#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
//#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能 */
//#define CONFIG_UI_ENABLE                        /* UI开关 */
//#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关 */
//#define CONFIG_GT911_B                          /* 触摸屏选择 */
//#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关 */
//#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
//#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD选择 */
#define CONFIG_UI_STYLE_LY_ENABLE               /* UI风格 */
#define LCD_DSI_VDO_4LANE_720P_RM68200GA1       /* LCD选择 */
#endif





#ifdef CONFIG_BOARD_SNC
#define CONFIG_NET_CLIENT                       #<{(|网络客户端选择|)}>#
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD卡选择 */
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





#ifdef CONFIG_BOARD_AC54_L86
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择  */
#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能  */
#define CONFIG_UI_ENABLE                        /* UI开关  */
#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关  */
#define CONFIG_GT911_B                          /* 触摸屏选择  */
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关  */
#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格  */
#define LCD_DSI_VDO_4LANE_1280x400              /* LCD选择  */
#endif


#ifdef CONFIG_BOARD_AC54_L86_NEW
#define __CPU_AC5401__                            /* CPU */
#define CONFIG_SD2_ENABLE                       /* SD卡选择  */
//#define CONFIG_VIDEO1_ENABLE 		            /* 模拟后摄像头使能 */
// #define CONFIG_VIDEO3_ENABLE 		            #<{(| usb后摄像头使能  |)}>#
//#define CONFIG_VIDEO4_ENABLE 		            /* 网络后摄像头使能 */
#define CONFIG_UI_ENABLE                        /* UI开关  */
#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关  */
#define CONFIG_GT911_B                          /* 触摸屏选择  */
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关  */
#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格  */
// #define LCD_DSI_VDO_4LANE_1280x400              #<{(| LCD选择  |)}>#
#define LCD_DSI_VDO_4LANE_MIPI_EK79030          /* LCD选择  */
#endif



#ifdef CONFIG_BOARD_AC56_L86
#define __CPU_AC5601__                            /* CPU */
#define CONFIG_SD1_ENABLE                       /* SD卡选择  */
#define CONFIG_VIDEO3_ENABLE 		            /* usb后摄像头使能  */
#define CONFIG_UI_ENABLE                        /* UI开关  */
#define CONFIG_TOUCH_PANEL_ENABLE               /* 触摸屏开关  */
#define CONFIG_GT911_B                          /* 触摸屏选择  */
#define CONFIG_TOUCH_UI_ENABLE                  /* TOUCH UI开关  */
#define CONFIG_UI_STYLE_JL02_ENABLE             /* UI风格  */
#define LCD_DSI_VDO_4LANE_MIPI_EK79030          /* LCD选择  */
#endif


//#define CONFIG_DATABASE_2_RTC                   /* 系统配置存RTC */
#define CONFIG_DATABASE_2_FLASH                 /* 系统配置存flash */
#define CONFIG_DEBUG_ENABLE                     /* 打印开关 */
//#define CONFIG_PAP_ENABLE                       /* PAP模块使能  */
#define CONFIG_VIDEO0_ENABLE 		            /* 前摄像头使能  */
#define CONFIG_VIDEO_DEC_ENABLE                 /* 视频解码显示  */
#define CONFIG_ADKEY_ENABLE                     /* AD按键开关  */
#define CONFIG_IOKEY_ENABLE                     /* IO按键开关  */
#define CONFIG_GSENSOR_ENABLE                   /* 重力感应开关  */
//#define KEY_MODE_EXIT_MENU                      /* UI菜单MODE键退出  */
#define USE_VE_MOTION_DETECT_MODE_ISP			/* 移动侦测使用MODE_ISP, 否则使用MODE_NORMAL */

// #define CONFIG_FILE_PREVIEW_ENABLE              #<{(| 预览图使能 |)}>#



#if defined CONFIG_VIDEO1_ENABLE
#define     VIDEO_PARK_DECT     1
#elif defined CONFIG_VIDEO3_ENABLE
#define     VIDEO_PARK_DECT     3
#elif defined CONFIG_VIDEO4_ENABLE
#define     VIDEO_PARK_DECT     4
#else
#define     VIDEO_PARK_DECT     0
#endif


#ifdef CONFIG_TOUCH_UI_ENABLE
#define CONFIG_FILE_PREVIEW_ENABLE              [> 预览图使能 <]
#endif

#ifdef CONFIG_VIDEO1_ENABLE
//#define CONFIG_AV10_SPI_ENABLE                  /* AV10 SPI开关  */
#endif


// #define CONFIG_GPS_ENABLE								#<{(| 打开GPS导航功能 |)}>#

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
#define CONFIG_REC_PATH_3       CONFIG_STORAGE_PATH"/C/DCIM/2/"

#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_2
#define CAMERA3_CAP_PATH        CONFIG_REC_PATH_3

#define CONFIG_DEC_PATH_1 	    CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_2 	    CONFIG_REC_PATH_2
#define CONFIG_DEC_PATH_3 	    CONFIG_REC_PATH_3

#define MAX_FILE_NAME_LEN       64
#define FILE_SHOW_NUM           12  /* 一页显示文件数 */

#define THREE_WAY_ENABLE		0


#ifndef __LD__
#include "board.h"
#endif



/*------------网络配置----------*/

// #define CONFIG_ETH_PHY_ENABLE
//#define CONFIG_WIFI_ENABLE
#define ACCESS_NUM 1
#define UUID "f2dd3cd7-b026-40aa-aaf4-f6ea07376490"
#define CONFIG_ENABLE_VLIST
#define WIFI_CAM_PREFIX    "wifi_camera_ac54_"
//#define WIFI_CAM_SUFFIX     "xxxxxxxxxxxxx"
#define WIFI_CAM_WIFI_PWD    "12345678"
//#define IPERF_ENABLE
//#define CONFIG_FORCE_RESET_VM  //檫除vm所有配置


//#define CONFIG_RTS_JPEG_ENABLE
#define CONFIG_RTS_H264_ENABLE
#define CONFIG_NET_TCP_ENABLE
//#define CONFIG_NET_UDP_ENABLE


/*两者只能选一*/
//#define CONFIG_NET_CLIENT  //wifi投屏 发送端
//#define CONFIG_NET_SERVER  //wifi投屏 接收端


//使能WIFI丢包统计，需要跟APP配合
//#define CONFIG_PACKET_LOSS_RATE_ENABLE

/*-------------网络端口----------------*/

#define CTP_CTRL_PORT   3333
#define CDP_CTRL_PORT   2228
#define PROJECTION_PORT 2230
#define VIDEO_PREVIEW_PORT 2226
#define VIDEO_PLAYBACK_PORT 2223
#define HTTP_PORT           8080
#define RTSP_PORT           554




//视频库内使用
#define _DUMP_PORT          2229
#define _FORWARD_PORT    2224
#define _BEHIND_PORT     2225






///////////////////////////
//#define ECHO_CANCELL_ENABLE //是否启用回声消除



/*------------ RF模块配置-------------*/
// #define CONFIG_RTC676X_ENABLE
// #define CONFIG_A7130_ENABLE
//#define CONFIG_P2418_ENABLE
// #define CONFIG_A7196_ENABLE


///////////////////////////




























#endif

