#ifndef SYS_TIME_H
#define SYS_TIME_H

#include "typedef.h"


struct sys_time {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

typedef struct DATE_TIME {
    u16 year1;
    u8 month1;
    u8 day1;
    u8 hour1;
    u8 minute1;
    u8 second1;
};
 typedef  struct data{
    double  latitude; //经度
    double  longitude; //纬度
    int  latitude_Degree;    //度
    int         latitude_Cent;   //分
    int     latitude_Second; //秒
    int  longitude_Degree;    //度
    int        longitude_Cent;  //分
    int     longitude_Second; //秒
    float     speed; //速度
    float     direction; //航向
    float     height; //海拔高度
    int satellite;
   u8     NS;
    u8     EW;
	    u16 year1;
    u8 month1;
    u8 day1;
    u8 hour1;
    u8 minute1;
    u8 second1;
    struct DATE_TIME D;
}GPS_INFO;













#endif
