#ifndef _DEV_GPS_H
#define _DEV_GPS_H

#include "device/device.h"
#include "generic/typedef.h"
#include "os/os_api.h"
#include "generic/ioctl.h"


/**************************************************************************************
*    GPRMC 最小定位信息：
*    数据详解：$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh
*　　<1> UTC 时间，hhmmss(时分秒)格式
*　　<2> 定位状态，A=有效定位，V=无效定位
*　　<3>纬度ddmm.mmmm(度分)格式(前面的0也将被传输)
*　　<4> 纬度半球N(北半球)或S(南半球)
*　　<5>经度dddmm.mmmm(度分)格式(前面的0也将被传输)
*　　<6> 经度半球E(东经)或W(西经)
*　　<7>地面速率(000.0~999.9节，前面的0也将被传输)
*　　<8>地面航向(000.0~359.9度，以真北为参考基准，前面的0也将被传输)
*　　<9> UTC 日期，ddmmyy(日月年)格式
*　　<10>磁偏角(000.0~180.0度，前面的0也将被传输)
*　　<11> 磁偏角方向，E(东)或W(西)
*　　<12>模式指示(仅NMEA01833.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效)
**************************************************************************************/

#define GPS_DEBUG		1


/**********************************************************************************************************
app_main()调用mov_pkg_gps_read_thread_create()创建GPS串口接收线程
调用创建mov_pkg_gps_read_thread_create线程前先定义宏：CONFIG_GPS_ENABLE。
CONFIG_GPS_ENABLE宏定义在app_config.h配置
MOV_GPS_SUPORT宏定义用于MOV文件是否加GPS数据，在gps.h配置
gps.h配置里面有GPS_TEST_DATA宏，该宏用于是否用GPS测试数据，定义则用本地测试数据，不定义则接受串口数据。
**********************************************************************************************************/
/*#define GPS_TEST_DATA	1	//不接入GPS模块时，用本地数据测试*/


/*
开启使用nmea库：
通过dev_read读取年月日精度等数据
数据格式:$GPSINFO,年,月,日,时,分,秒,位置精度,水平精度,位置精度,纬度,经度,海拔高度,速度,航向,磁变度\r\n
通过ioctl读取年月日等各个数据

不开启nmea库：
dev_read读取返回GPS源数据格式，需解析！
ioctl同样可读取返回GPS源数据格式，需解析！
*/

/*#define GPS_RECV_USE_NMEA_LIB_EN	1	//使用nmea库*/


/********************* IO cmd *************************/
#define GPS_MAGIC                        'G'

#ifdef GPS_RECV_USE_NMEA_LIB_EN

#define GPS_GET_YEAR                   	_IOR(GPS_MAGIC,1,u32)
#define GPS_GET_MONTH                   _IOR(GPS_MAGIC,2,u32)
#define GPS_GET_DAY                   	_IOR(GPS_MAGIC,3,u32)
#define GPS_GET_HOUR                  	_IOR(GPS_MAGIC,4,u32)
#define GPS_GET_MIN                  	_IOR(GPS_MAGIC,5,u32)
#define GPS_GET_SEC		                _IOR(GPS_MAGIC,6,u32)

#define GPS_GET_PDOP                   	_IOR(GPS_MAGIC,7,u32)
#define GPS_GET_HDOP                  	_IOR(GPS_MAGIC,8,u32)
#define GPS_GET_VDOP                   	_IOR(GPS_MAGIC,9,u32)
#define GPS_GET_LAT                  	_IOR(GPS_MAGIC,10,u32)
#define GPS_GET_LON                   	_IOR(GPS_MAGIC,11,u32)
#define GPS_GET_ELV                 	_IOR(GPS_MAGIC,12,u32)
#define GPS_GET_SPEED                   _IOR(GPS_MAGIC,13,u32)
#define GPS_GET_DIREC                 	_IOR(GPS_MAGIC,14,u32)
#define GPS_GET_DECLI                   _IOR(GPS_MAGIC,15,u32)

#else

#define GPS_READ						_IOR(GPS_MAGIC,16,u32)

#endif


#define GPS_BUF_LEN 	1024
#define GPS_FRAME_MAX	512

struct gps_location_info {
    double	PDOP;		/*位置精度因子*/
    double	HDOP;		/*水平精度因子*/
    double	VDOP;		/*位置精度因子*/
    double	lat;		/*纬度*/
    double	lon;		/*经度*/
    double	elv;		/*海拔高度*/
    double	speed;		/*速度km/h*/
    double	direction;	/*航向*/
    double	declination; /*磁变度*/
};

struct gps_time_info {
    int year;
    char month;
    char week;
    char day;
    char hour;
    char min;
    char sec;
};

struct gps_data_info {
    char *uart_name;
    char *recv_buf;
    char *read_buf_addr;
    char locat_flag;//成功定位标志
    int recv_len;
    OS_SEM rw_sem;
    OS_SEM locat_sem;
    struct gps_time_info time_info;
    struct gps_location_info locat_info;
};


int gps_cfg_open(struct gps_data_info *gps_info);
int gps_cfg_close(struct gps_data_info *gps_info);
int gps_cfg_send(struct gps_data_info *gps_info, char *buf, int len);
int gps_cfg_recv(struct gps_data_info *gps_info, char *buf, int len);


struct gps_platform_data;

struct gps_operations {
    int (*init)(struct gps_platform_data *gps_data);
    int (*open)(struct gps_platform_data *gps_data);
    int (*read)(struct gps_platform_data *gps_data, void *buf, int len);
    int (*write)(struct gps_platform_data *gps_data, void *buf, int len);
    int (*close)(struct gps_platform_data *gps_data);
    int (*ioctl)(struct gps_platform_data *gps_data, u32 cmd, u32 arg);
};

struct gps_device {
    char *name;
    const struct gps_operations *ops;
    struct device dev;
    void *priv;
};


struct gps_platform_data {
    char *uart_name;
    struct gps_data_info gps_info;
    OS_SEM sem;
};

#define GPS_PLATFORM_DATA_BEGIN(data) \
	struct gps_platform_data data = {

#define GPS_GPS_PLATFORM_DATA_END() \
	.gps_info = {\
		.uart_name = NULL,\
		.recv_buf  = NULL,\
		.read_buf_addr = NULL,\
		.locat_flag = 0,\
		.recv_len = 0,\
		.rw_sem = 0,\
		.locat_sem = 0,\
		.time_info = {0},\
		.locat_info = {0},\
	}\
};

#define REGISTER_GPS_DEVICE(dev) \
	struct gps_device dev sec(.gps)

extern const struct gps_device gps_device_begin[];
extern const struct gps_device gps_device_end[];

#define list_for_each_gps_device(dev) \
	for(dev=gps_device_begin;dev<=gps_device_end;dev++)

extern const struct device_operations gps_dev_ops;

extern int  mov_pkg_gps_read_thread_create(void);
extern void mov_pkg_gps_read_thread_kill(void);


#endif

