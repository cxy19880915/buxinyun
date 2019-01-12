#ifndef     _GPS_H_
#define     _GPS_H_

typedef struct{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
}DATE_TIME;

typedef struct{
    int  latitude; //经度
    int  longitude; //纬度
    int latitude_Degree;    //度
    int        latitude_Cent;   //分
    int     latitude_Second; //秒
    int longitude_Degree;    //度
    int        longitude_Cent;  //分
    int     longitude_Second; //秒
    int      speed; //速度
    int      direction; //航向
    int      height; //海拔高度
    int satellite;
    u8     NS;
    u8     EW;
    DATE_TIME D;
    char latitude_buf[10];
    char longitude_buf[10];
}GPS_INFO_1;



/////////////////////////////////////

#define 	GPS_DATA_SIZE			(512)
#define	DATA_OFFSET			(4)

typedef struct
{
	u16 m_dwWrId;
	u16 m_dwRdId;
}FIFO_BUF_STRUCT;
typedef struct
{
	u8 m_bRMCYear;					//
	u8 m_btRMCMonth;				//
	u8 m_btRMCDay;					//
	u8 m_btRMCUTCHour;			//保存的UTC时间
	u8 m_btRMCMinute;				//
	u8 m_btRMCSecond;				//
}GPS_TIME;


typedef struct{
    u8 curSpeed;   //速度
    u32 curLattitude; //纬度
	u32 curLongitude; //经度
    u8 curAngle;     //方向   
    u8 satellite;  //使用恒星数量
    u8 alarmType;  //警示类型
    u8 limitSpeed;  //限速值  
    u16 distance;    //倒数值
    u8 sate_en;
    u8 err;
    struct utime time;
}GPS_INFO;


extern GPS_INFO  gps_info;

void Decode_Uart_Data(GPS_INFO *g_info);
void DecodeUart(GPS_INFO *g_info);
void get_g_info(GPS_INFO * g_info);
u8 DataIn_Decode(u8 *pBuff, u16 nMaxCount,u16 *pLen);
void clear_gps();
void start_gps();

#endif
