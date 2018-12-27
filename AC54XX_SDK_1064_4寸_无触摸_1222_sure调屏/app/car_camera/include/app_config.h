#ifndef APP_CONFIG_H
#define APP_CONFIG_H


/*----------打印开关----------*/
#define CONFIG_DEBUG_ENABLE


//#define CONFIG_BOARD_FPGA
// #define CONFIG_BOARD_DVP_20170329
// #define CONFIG_BOARD_MIPI_V2
#define CONFIG_BOARD_MIPI_V3
// #define CONFIG_BOARD_MIPI_V4
// #define CONFIG_BOARD_MIPI_DEVELOP_BOARD
// #define CONFIG_BOARD_DEVELOP_20170411
// #define CONFIG_BOARD_LQFP128_V1
//#define CONFIG_BOARD_MIPI_TOUCH_V1
//#define CONFIG_BOARD_MIPI_TOUCH_V1
// #define CONFIG_BOARD_MIPI_20170330

#define JL5401    1
#define JL5601    2
#define SLELE_5XX   JL5601
//#define SERIES4   1 
//#define SERIES2   2 
//#define SERIES1   3 
	
//#define LCD_BACKLIGHT    SERIES4  

#define pmsg(fmt,args...) do{printf("L%d(%s)"fmt, __LINE__,__FILE__,  ##args);}while(0)

 
#define LCD_NAME      "mipi_2lane_mipi_854"
 

#ifdef CONFIG_BOARD_DVP_20170329
#endif

#ifdef CONFIG_BOARD_LQFP128_V1
#endif

#ifdef CONFIG_BOARD_MIPI_DEVELOP_BOARD
#define CONFIG_SD2_ENABLE
#else
#define CONFIG_SD1_ENABLE
#endif
//屏的背光是几并，根据屏背光选择
//#define SERIES4   1 
//#define SERIES2   2 
//#define SERIES1   3 
	
//#define LCD_BACKLIGHT    SERIES2   

#define CONFIG_SD0_PATH 	"storage/sd0"
#define CONFIG_SD1_PATH 	"storage/sd1"
#define CONFIG_SD2_PATH 	"storage/sd2"

#ifdef CONFIG_BOARD_MIPI_DEVELOP_BOARD
#define CONFIG_STORAGE_PATH 	CONFIG_SD2_PATH
#define SDX_DEV					"sd2"
#else
#define CONFIG_STORAGE_PATH 	CONFIG_SD1_PATH
#define SDX_DEV					"sd1"
#endif

#define CONFIG_ROOT_PATH     	 CONFIG_STORAGE_PATH"/C/"
// #define VREC_ROOT_PATH      CONFIG_STORAGE_PATH"/C/"

#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/DCIM/1/"
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/DCIM/2/"

#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_2

#define CONFIG_DEC_PATH_1 	CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_2 	CONFIG_REC_PATH_2
#define CONFIG_DEC_PATH_3 	CONFIG_REC_PATH_1

#define MAX_FILE_NAME_LEN  64

#define FILE_SHOW_NUM  12  //一页显示文件数

//define CONFIG_ETH_PHY_ENABLE
#define CONFIG_PAP_ENABLE  /* PAP模块使能 */

/*-----------摄像头开关------------*/
#define CONFIG_VIDEO0_ENABLE 		/*前摄像头使能*/
//#define CONFIG_VIDEO1_ENABLE 		/*模拟后摄像头使能*/
#define CONFIG_VIDEO3_ENABLE 		/*usb后摄像头使能*/


/*--------------视频解码显示------------*/
#define CONFIG_VIDEO_DEC_ENABLE

/*------------AD按键开关-----------*/
#define CONFIG_ADKEY_ENABLE

/*------------IO按键开关-----------*/
#define CONFIG_IOKEY_ENABLE

/*------------UI开关-----------*/
#define CONFIG_UI_ENABLE

#ifdef CONFIG_UI_ENABLE

#ifdef CONFIG_BOARD_MIPI_V3
#define CONFIG_UI_STYLE_LY_ENABLE
#endif

#ifdef CONFIG_BOARD_MIPI_V4
#define CONFIG_UI_STYLE_LY_ENABLE
#endif

#ifdef CONFIG_BOARD_MIPI_DEVELOP_BOARD
#define CONFIG_UI_STYLE_LY_ENABLE
#endif

#ifdef CONFIG_BOARD_MIPI_TOUCH_V1
/*------------触摸屏开关----------*/
#define CONFIG_TOUCH_PANEL_ENABLE
/*-----------TOUCH UI开关-----------*/
#define CONFIG_TOUCH_UI_ENABLE
#define CONFIG_UI_STYLE_JL02_ENABLE
#endif


#endif // CONFIG_UI_ENABLE


#ifdef CONFIG_TOUCH_UI_ENABLE
#define CONFIG_FILE_PREVIEW_ENABLE
#endif

/*------------UI菜单MODE键退出-----------*/
// #define KEY_MODE_EXIT_MENU

/*------------重力感应开关----------*/
#define CONFIG_GSENSOR_ENABLE


/*------------av10 spi 开关----------*/
#define CONFIG_AV10_SPI_ENABLE


/*-------------avin det and avin parking det-----*/
extern void AV_PCTL();
extern void av_parking_det_init();
extern unsigned char av_parking_det_status();
extern unsigned char PWR_CTL(unsigned char on_off);
extern unsigned char read_power_key();
extern unsigned char usb_is_charging();
extern unsigned int get_usb_wkup_gpio();
extern void key_voice_enable(int enable);


































#endif

