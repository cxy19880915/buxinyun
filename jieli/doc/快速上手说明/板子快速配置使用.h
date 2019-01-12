#ifndef APP_CONFIG_H
#define APP_CONFIG_H


/*----------打印开关----------*/
#define CONFIG_DEBUG_ENABLE

/*----------选择对应板子的配置文件----------*/
//#define CONFIG_BOARD_FPGA
// #define CONFIG_BOARD_DVP_20170329
// #define CONFIG_BOARD_MIPI_20170330
// #define CONFIG_BOARD_MIPI_V2
#define CONFIG_BOARD_MIPI_V3
// #define CONFIG_BOARD_DEVELOP_20170411
// #define CONFIG_BOARD_LQFP128_V1


#ifdef CONFIG_BOARD_DVP_20170329
#endif

#ifdef CONFIG_BOARD_LQFP128_V1
#endif

/*----------使能sd卡 此处使用sd1----------*/
#define CONFIG_SD1_ENABLE

/*----------配置sd卡相关文件路径----------*/
#define CONFIG_SD0_PATH 	"storage/sd0"
#define CONFIG_SD1_PATH 	"storage/sd1"
#define CONFIG_SD2_PATH 	"storage/sd2"

#define CONFIG_STORAGE_PATH 	CONFIG_SD1_PATH
#define CONFIG_ROOT_PATH     	 CONFIG_STORAGE_PATH"/C/"
// #define VREC_ROOT_PATH      CONFIG_STORAGE_PATH"/C/"

#define CONFIG_REC_PATH_1       CONFIG_STORAGE_PATH"/C/DCIM/1/"
#define CONFIG_REC_PATH_2       CONFIG_STORAGE_PATH"/C/DCIM/2/"

#define CAMERA0_CAP_PATH        CONFIG_REC_PATH_1
#define CAMERA1_CAP_PATH        CONFIG_REC_PATH_2

#define CONFIG_DEC_PATH_1 	CONFIG_REC_PATH_1
#define CONFIG_DEC_PATH_2 	CONFIG_REC_PATH_2
#define CONFIG_DEC_PATH_3 	CONFIG_REC_PATH_1

/*----------文件名长度最长----------*/
#define MAX_FILE_NAME_LEN  64

//define CONFIG_ETH_PHY_ENABLE
#define CONFIG_PAP_ENABLE  /* PAP模块使能 */

/*-----------摄像头开关------------*/
#define CONFIG_VIDEO0_ENABLE 		/*前摄像头使能*/
#define CONFIG_VIDEO1_ENABLE 		/*后摄像头使能*/



/*--------------视频解码显示------------*/
#define CONFIG_VIDEO_DEC_ENABLE


/*------------AD按键开关-----------*/
#define CONFIG_ADKEY_ENABLE

/*------------IO按键开关-----------*/
#define CONFIG_IOKEY_ENABLE

/*------------UI开关-----------*/
#define CONFIG_UI_ENABLE

/*------------UI菜单MODE键退出-----------*/
// #define KEY_MODE_EXIT_MENU

/*------------重力感应开关----------*/
#define CONFIG_GSENSOR_ENABLE


/*------------av10 spi 开关----------*/
#define CONFIG_AV10_SPI_ENABLE


/*-------------avin det and avin parking det-----*/
extern void AV_PCTL();
extern void AV_PARKING_DET_INIT();
extern unsigned char AV_PARKING_DET_STATUS();
extern unsigned char PWR_CTL(unsigned char on_off);
extern unsigned char read_power_key();
extern unsigned char USB_IS_CHARGING();





































#endif
