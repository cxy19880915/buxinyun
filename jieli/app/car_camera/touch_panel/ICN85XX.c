/*++

 Copyright (c) 2012-2022 ChipOne Technology (Beijing) Co., Ltd. All Rights Reserved.
 This PROPRIETARY SOFTWARE is the property of ChipOne Technology (Beijing) Co., Ltd.
 and may contains trade secrets and/or other confidential information of ChipOne
 Technology (Beijing) Co., Ltd. This file shall not be disclosed to any third party,
 in whole or in part, without prior written consent of ChipOne.
 THIS PROPRIETARY SOFTWARE & ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
 WITH ALL FAULTS, & WITHOUT WARRANTY OF ANY KIND. CHIPONE DISCLAIMS ALL EXPRESS OR
 IMPLIED WARRANTIES.

 File Name:    icn85xx.c
 Abstract:
               input driver.
 Author:       Zhimin Tian
 Date :        08,14,2013
 Version:      1.0
 History :
     2012,10,30, V0.1 first version
 --*/

#include "icn85xx.h"
#include "icn85xx_fw.h"
//#include "touch_iic.h"
#include "touch_panel_manager.h"
#include "event.h"

//
icn85xx_ts_data icn85xx_ts;
static struct sw_touch_panel_platform_data *_touch_panel_data = NULL;

static int _touch_panel_timer_reg;


static u8 icn85xx_event = 0xff;
static u8 icn85xx_Pressure = 0xff;

#define TOUCH_PANEL_EVENT_NONE   0xff

typedef struct {
    u8 fresh;
    u16 x;
    u16 y;
    u16 size;
} POINT_t;

static POINT_t point[ICN85_MAX_TOUCH_POINT];

#if 1
    #define ICN85XX_IIC_PROG_WADDR 0x60
    #define ICN85XX_IIC_PROG_RADDR 0x61
    #define ICN85XX_IIC_WADDR 0x90
    #define ICN85XX_IIC_RADDR 0x91
    #define ICN87XX_IIC_PROG_WADDR       0x60
    #define ICN87XX_IIC_PROG_RADDR       0x61
#else
    #define ICN85XX_IIC_WADDR 0xB0
    #define ICN85XX_IIC_RADDR 0xB1
#endif

//icn85xx_ts_data *icn85xx_ts;
//////////////////////////////

extern void delay_2ms(int cnt);


#define ICN85XX_CNT_DOWN        3
#define ICN85XX_CNT_LONG        40
#define ICN85XX_CNT_HOLD        0

#if 1//COMPILE_FW_WITH_DRIVER
       static char firmware[128] = "icn85xx_firmware";
#else
    #if SUPPORT_SENSOR_ID
        static char firmware[128] = {0};
    #else
       static char firmware[128] = {"/misc/modules/ICN8505.BIN"};
    #endif
#endif


u8 icn85xx_report_point_info(void)
{

    u8 buf[ICN85_MAX_TOUCH_POINT*POINT_SIZE+3]={0};
    static unsigned char finger_last[ICN85_MAX_TOUCH_POINT + 1]={0};
    unsigned char  finger_current[ICN85_MAX_TOUCH_POINT + 1] = {0};
    unsigned int i,position = 0;
    int temp = 0;
    int ret = -1;
    static u8 fresh_comm = 0;


    //printf("1\n");

  
    ret = icn85xx_i2c_rxdata(0x1000, buf, 2);
    if (ret < 0) {
         printf(" icn85xx_report_value_B read_data i2c_rxdata failed\n" );
          return 0;
      }
    icn85xx_ts.point_num = buf[1];

    //printf("icn85xx_ts.point_num = %d\n", icn85xx_ts.point_num);
    if (icn85xx_ts.point_num > ICN85_MAX_TOUCH_POINT || icn85xx_ts.point_num==0)
     {
     	  printf("==point_num error ==\n");
          //printf("icn85xx_ts.point_num = %d\n", icn85xx_ts.point_num);
         return 0;
     }
     else
	{
	    ret = icn85xx_i2c_rxdata(0x1002, buf+2, icn85xx_ts.point_num*POINT_SIZE);
	    //printf_buf(buf+2,POINT_SIZE);
		if (ret < 0)
		{
			printf(" icn85xx_report_value_B read_data i2c_rxdata failed\n" );
			return 0;
		}

        for(position = 0; position<icn85xx_ts.point_num; position++)
        {
    		temp = buf[2 + POINT_SIZE*position] ;
    		//point[temp].x_last = point[temp].x_pos;
    		//point[temp].y_last = point[temp].y_pos;
    		icn85xx_Pressure = buf[7 + POINT_SIZE*position];
    		icn85xx_event = buf[POINT_SIZE*position+8];

    //			tch_touch_info[temp].cnt++ ;
if(_touch_panel_data->_X_MIRRORING)
            point[temp].x= _touch_panel_data->_MAX_X -((buf[4 + POINT_SIZE*position]<<8) + buf[3 + POINT_SIZE*position]);
else
            point[temp].x= (buf[4 + POINT_SIZE*position]<<8) + buf[3 + POINT_SIZE*position];


if(_touch_panel_data->_Y_MIRRORING)
            point[temp].y = _touch_panel_data->_MAX_Y-((buf[6 + POINT_SIZE*position]<<8) + buf[5 + POINT_SIZE*position]);
else
            point[temp].y = (buf[6 + POINT_SIZE*position]<<8) + buf[5 + POINT_SIZE*position];
            
        }
		return icn85xx_ts.point_num;


	}

    return 0;
}


///////////********add*****////////////

/***********************************************************************************************
Name    :   icn85xx_report_value_B
Input   :   void
Output  :
function    : reprot touch ponit
***********************************************************************************************/


void icn85xx_read_point(void *arg)
{
    int i;
    static u8 touch_on_flag = 0;
    static u32 points_cnt = 0;

    if (gpio_read(_touch_panel_data->int_pin)) {
        
        _touch_panel_data->points.point_num = icn85xx_report_point_info();
    }
    else
    {
        _touch_panel_data->points.point_num = 0;
    }

    _touch_panel_data->points.p[i].e = TOUCH_PANEL_EVENT_NONE;
    #if 1
    if(icn85xx_event != 4)
    {
        points_cnt++;
        if (points_cnt == ICN85XX_CNT_DOWN)
        {
            _touch_panel_data->points.p[i].x = point[i].x;
            _touch_panel_data->points.p[i].y = point[i].y;
            _touch_panel_data->points.p[i].e = TOUCH_EVENT_DOWN;
            touch_on_flag = 1;
        }
        else if (points_cnt > ICN85XX_CNT_DOWN)
        {
                if (point[i].x != _touch_panel_data->points.p[i].x ||
                    point[i].y != _touch_panel_data->points.p[i].y)
                {
                    if (points_cnt > ICN85XX_CNT_DOWN + ICN85XX_CNT_DOWN)
                    {

                        _touch_panel_data->points.p[i].x = point[i].x;
                        _touch_panel_data->points.p[i].y = point[i].y;
                        _touch_panel_data->points.p[i].e = TOUCH_EVENT_MOVE;
                        points_cnt = ICN85XX_CNT_DOWN;
                    }
                }

                if (points_cnt == ICN85XX_CNT_LONG + ICN85XX_CNT_HOLD) {
                    _touch_panel_data->points.p[i].x = point[i].x;
                    _touch_panel_data->points.p[i].y = point[i].y;
                    _touch_panel_data->points.p[i].e = TOUCH_EVENT_HOLD;
                }

                if (points_cnt > ICN85XX_CNT_LONG + ICN85XX_CNT_HOLD) {
                    points_cnt = ICN85XX_CNT_LONG;
                }
         }
    }
    else
    {
        if(touch_on_flag == 1)
        {
            touch_on_flag = 0;
             _touch_panel_data->points.p[i].x = point[i].x;
             _touch_panel_data->points.p[i].y = point[i].y;
            _touch_panel_data->points.p[i].e = TOUCH_EVENT_UP;
        }
        else
        {
            _touch_panel_data->points.p[i].e = TOUCH_PANEL_EVENT_NONE;
        }
         points_cnt = 0;
    }

    if (_touch_panel_data->points.p[i].e != TOUCH_PANEL_EVENT_NONE)
    {
            
            struct sys_event eve;
      
            eve.type = SYS_TOUCH_EVENT;
            eve.u.touch.event = _touch_panel_data->points.p[i].e;
            eve.u.touch.pos.x = _touch_panel_data->points.p[i].x;
            eve.u.touch.pos.y = _touch_panel_data->points.p[i].y;
  
            
            sys_event_notify(&eve);
       
            if (_touch_panel_data->_DEBUGE)
            {
                printf("\n [%d](event = %d) (%d,%d)", i, _touch_panel_data->points.p[i].e, _touch_panel_data->points.p[i].x, _touch_panel_data->points.p[i].y);
            }
    }
    #endif
}



/**
功能： prog 模式下IIC写寄存器操作
输入： addr - 寄存器地址3byte
       buf  - 寄存器值数组指针
       len  - 数组长度
输出： TRUE - 成功
       FALSE- 失败
**/
u8 icn_iic_prog_write(u32 addr, u8 *buf, u32 len)   //u24 DEFINE?
{
    return _touch_panel_write(ICN85XX_IIC_PROG_WADDR, addr, buf, len);
}

u8 icn_iic_prog_write_32BIT(u32 addr, u8 *buf, u32 len)   //u24 DEFINE?
{
    return _touch_panel_write_32BIT(ICN85XX_IIC_PROG_WADDR, addr, buf, len);
}

/**
功能： IIC读寄存器操作
输入： addr - 寄存器地址
        buf  - 寄存器值数组指针
        len  - 数组长度
输出： TRUE - 成功
       FALSE- 失败
**/
u8 icn_iic_prog_read(u32 addr, u8 *buf, u32 len)//u24 DEFINE?
{
    return _touch_panel_read(ICN85XX_IIC_PROG_WADDR, ICN85XX_IIC_PROG_RADDR, addr, buf, len);
}


u8 icn_iic_prog_read_32BIT(u32 addr, u8 *buf, u32 len)//u24 DEFINE?
{
    return _touch_panel_read_32BIT(ICN85XX_IIC_PROG_WADDR, ICN85XX_IIC_PROG_RADDR, addr, buf, len);
}


////////////////////////add icn87xx//////////////////

u8 icn_iic_prog_write_87(u32 addr, u8 *buf, u32 len)   //u24 DEFINE?
{
    return _touch_panel_write(ICN87XX_IIC_PROG_WADDR, addr, buf, len);
}

u8 icn_iic_prog_read_87(u32 addr, u8 *buf, u32 len)//u24 DEFINE?
{
    return _touch_panel_read(ICN87XX_IIC_PROG_WADDR, ICN87XX_IIC_PROG_RADDR, addr, buf, len);
}
static void TCH_INT_H()
{
    if (_touch_panel_data)
    {
        //pmsg("TCH_INT_H\n");
        gpio_direction_output(_touch_panel_data->int_pin, 1);
    }
}

static void TCH_INT_L()
{
    if (_touch_panel_data) {
        //pmsg("TCH_INT_L\n");
        gpio_direction_output(_touch_panel_data->int_pin, 0);
    }
}

static void TCH_RST_H()
{
    if (_touch_panel_data) {
        gpio_direction_output(_touch_panel_data->rst_pin, 1);
    }
}

static void TCH_RST_L()
{
    if (_touch_panel_data) {
        gpio_direction_output(_touch_panel_data->rst_pin, 0);
    }
}

static void TCH_INT_IN()
{
    if (_touch_panel_data) {
        gpio_direction_input(_touch_panel_data->int_pin);
    }
}

void ICN85XX_reset(void)
{
    TCH_INT_H();
    delay_2ms(10);

    TCH_RST_L();
    delay_2ms(20);


/*    TCH_INT_L();
    delay_2ms(10);
*/
    TCH_RST_H();
    delay_2ms(60);

    TCH_INT_H();
    delay_2ms(10);

    TCH_INT_IN();
    delay_2ms(10);

}



/**********************************/
/*********以下为接口函数**********/
/**********************************/


/**
功能： ICN85XX读ID
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/

u8 ICN85XX_read_id(void)
{
    u8 ret = FALSE;


    u32 i;

 // static  u8 tmp;
   // static u32 buf[256];
	 ICN85XX_reset();
	  delay_2ms(10);
	ret=icn85xx_iic_test();
    if (ret < 0)
    {
        printf("icn85xx_iic_test  failed.\n");
        return FALSE;

    }
    else
    {
        printf("iic communication ok: 0x%x\n", icn85xx_ts.ictype);
		return TRUE;
    }

    return ret;
}

char icn85xx_check(struct sw_touch_panel_platform_data *data)
{
    _touch_panel_data = data;
    return ICN85XX_read_id();
}

u8 ICN85XX_init(void)
{
    u8 ret = FALSE;
//  struct icn85xx_ts_data *icn85xx_ts;

    ret=icn85xx_update();

    ret = TRUE;
    if (ret == TRUE) {
        if (_touch_panel_data->enable) {
            _touch_panel_timer_reg = sys_timer_add((void *)0, icn85xx_read_point, 20);
        }
    }

    if(ret<0)
    {
	 printf("ICN85XX_init fail =\n");
	 return FALSE;
    }
    else
    	{
		 printf("ICN85XX_init ok =\n");
		 return TRUE;
	}
	return ret;



}


/**
功能： ICN85XX读坐标及数据处理
输入： void
输出： touch_num 触点个数
**/
u8 ICN85XX_read_point(void)
{
    u8 touchnumber=0;
	touchnumber=icn85xx_report_point_info();
//	printf("\n touchnumber %d\n",touchnumber);
	return touchnumber;

}


/**
功能： ICN85XX进入睡眠模式
输入： void
输出： TRUE - 成功
       FALSE- 失败
**/
u8 ICN85XX_sleep(void)
{
    int ret = FALSE;
    ret=icn85xx_write_reg(ICN85xx_REG_PMODE, PMODE_HIBERNATE);
    if(ret<0)
	return FALSE;
     else
    	return TRUE;
}


/**
功能： ICN85XX唤醒
输入： void
输出： void
**/
void ICN85XX_wakeup(void)
{
	ICN85XX_reset();

//    TCH_INT_IN();
}

/////////////////////////////////
/***********************************************************************************************
Name    :   icn85xx_prog_i2c_rxdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : read data from icn85xx, prog mode
***********************************************************************************************/
int icn85xx_prog_i2c_rxdata(unsigned int addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;
	while(retries < IIC_RETRY_NUM)
	{
		ret=icn_iic_prog_read_32BIT(addr, rxdata, length);
		if(ret == TRUE) break;
		retries++;
	}
	if(retries==3)	ret=-1;
	return 0;

}
/***********************************************************************************************
Name    :   icn85xx_prog_i2c_txdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : send data to icn85xx , prog mode
***********************************************************************************************/
int icn85xx_prog_i2c_txdata(unsigned int addr, char *txdata, int length)
{
    int ret = -1;
    //char tmp_buf[128];
    int retries = 0;
	while(retries < IIC_RETRY_NUM)
	{
		ret=icn_iic_prog_write_32BIT(addr, txdata, length);
		if(ret == TRUE)
            return 0;
		retries++;
	}
	if(retries==3)
        ret=-1;
	return 0;

}
/***********************************************************************************************
Name    :   icn85xx_prog_write_reg
Input   :   addr -- address
            para -- parameter
Output  :
function    :   write register of icn85xx, prog mode
***********************************************************************************************/
int icn85xx_prog_write_reg(unsigned int addr, char para)
{
    char buf[3];
    int ret = -1;

    buf[0] = para;
    ret = icn85xx_prog_i2c_txdata(addr, buf, 1);
    if (ret < 0) {
//        icn85xx_error("%s write reg failed! %#x ret: %d\n", __func__, buf[0], ret);
        return -1;
    }

    return ret;
}


/***********************************************************************************************
Name    :   icn85xx_prog_read_reg
Input   :   addr
            pdata
Output  :
function    :   read register of icn85xx, prog mode
***********************************************************************************************/
int icn85xx_prog_read_reg(unsigned int addr, char *pdata)
{
    int ret = -1;
    ret = icn85xx_prog_i2c_rxdata(addr, pdata, 1);
    return ret;
}

/***********************************************************************************************
Name    :   icn85xx_i2c_rxdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : read data from icn85xx, normal mode
***********************************************************************************************/
int icn85xx_i2c_rxdata(unsigned short addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;
   // unsigned char tmp_buf[2];
	while(retries < IIC_RETRY_NUM)
	{
		//ret=icn_iic_read(addr, rxdata, length);
		ret = _touch_panel_read(ICN85XX_IIC_WADDR, ICN85XX_IIC_RADDR, addr, rxdata, length);
		if(ret == TRUE) break;
		retries++;
	}
	if(ret==FALSE)	ret=-1;
	return ret;

}
/***********************************************************************************************
Name    :   icn85xx_i2c_txdata
Input   :   addr
            *rxdata
            length
Output  :   ret
function    : send data to icn85xx , normal mode
***********************************************************************************************/
int icn85xx_i2c_txdata(unsigned short addr, char *txdata, int length)
{
    int ret = -1;
  //  unsigned char tmp_buf[128];
    int retries = 0;
	while(retries < IIC_RETRY_NUM)
	{
		//ret=icn_iic_write(addr, txdata, length);
        ret = _touch_panel_write(ICN85XX_IIC_WADDR, addr, txdata, length);
		if(ret == TRUE) break;
		retries++;
	}
	if(ret==FALSE)	ret=-1;
	return ret;

}

/***********************************************************************************************
Name    :   icn85xx_write_reg
Input   :   addr -- address
            para -- parameter
Output  :
function    :   write register of icn85xx, normal mode
***********************************************************************************************/
int icn85xx_write_reg(unsigned short addr, char para)
{
    char buf[3];
    int ret = -1;

    buf[0] = para;
    ret = icn85xx_i2c_txdata(addr, buf, 1);
    if (ret < 0) {
        printf("write reg failed! %#x ret: %d\n", buf[0], ret);
        return -1;
    }
    return ret;
}


/***********************************************************************************************
Name    :   icn85xx_read_reg
Input   :   addr
            pdata
Output  :
function    :   read register of icn85xx, normal mode
***********************************************************************************************/
int icn85xx_read_reg(unsigned short addr, char *pdata)
{
    int ret = -1;
    ret = icn85xx_i2c_rxdata(addr, pdata, 1);
    if(ret < 0)
    {
        printf("addr: 0x%x: 0x%x\n", addr, *pdata);
	return -1;
    }
    return ret;
}

int icn87xx_prog_i2c_rxdata(unsigned int addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;
	while(retries < IIC_RETRY_NUM)
	{
		ret=icn_iic_prog_read_87(addr, rxdata, length);
		if(ret == TRUE) break;
		retries++;
	}
	if(retries==3)	ret=-1;
	return 0;
}
int icn87xx_prog_i2c_txdata(unsigned int addr, char *rxdata, int length)
{
    int ret = -1;
    int retries = 0;
	while(retries < IIC_RETRY_NUM)
	{
		ret=icn_iic_prog_write_87(addr, rxdata, length);
		if(ret == TRUE) break;
		retries++;
	}
	if(retries==3)	ret=-1;
	return 0;
}
/***********************************************************************************************
Name    :   icn85xx_iic_test
Input   :   void
Output  :
function    : 0 success,
***********************************************************************************************/
int icn85xx_iic_test(void)
{

    int  ret = -1;
    u8 value = 0;
    u8 buf[3];
    int  retry = 0;
    int  flashid;
    icn85xx_ts.ictype = ICN85XX_WITH_FLASH_85;

    while(retry++ < 3)
    {
        ret = icn85xx_read_reg(0xa, &value);
        if(ret > 0)
        {
            if(value == 0x85)
            {
                icn85xx_ts.ictype = ICN85XX_WITH_FLASH_85;
                return ret;
            }
            else if((value == 0x86)||(value == 0x88) )
            {
                icn85xx_ts.ictype = ICN85XX_WITH_FLASH_86;
                return ret;
            }
            else if(value == 0x87)
            {
                icn85xx_ts.ictype = ICN85XX_WITH_FLASH_87;
                return ret;
            }
        }

        delay_2ms(3);
    }

    // add junfuzhang 20131211
    // force ic enter progmode
    ret =icn85xx_goto_progmode();
	if(ret<0)
	{
		printf("icn85xx_goto_progmod error! \n");
		return -1;
	}
    delay_2ms(10);

    retry = 0;
    while(retry++ < 3)
    {
        buf[0] = buf[1] = buf[2] = 0x0;
        ret = icn85xx_prog_i2c_txdata(0x040000,buf,3);
        if (ret < 0)
        {
        	printf("write prog reg failed! ret: %d\n", ret);
        	return ret;
        }
           ret = icn85xx_prog_i2c_rxdata(0x040000, buf, 3);
           //pmsg("buf[0]: %d,buf[1]: %d,buf[2]: %d\n",buf[0],buf[1],buf[2]);
           //pmsg("ret = %d\n", ret);
        if(ret ==0)
        {
            if((buf[2] == 0x85) && (buf[1] == 0x05))
            {
                flashid = icn85xx_read_flashid();
                printf("icn85xx_flashid: 0x%x \n", flashid);
                if((MD25D40_ID1 == flashid) || (MD25D40_ID2 == flashid)
                    ||(MD25D20_ID1 == flashid) || (MD25D20_ID2 == flashid)
                    ||(GD25Q10_ID == flashid) || (MX25L512E_ID == flashid)
                    || (MD25D05_ID == flashid)|| (MD25D10_ID == flashid))
                {
                    icn85xx_ts.ictype = ICN85XX_WITH_FLASH_85;
                }
                else
                {
                    icn85xx_ts.ictype = ICN85XX_WITHOUT_FLASH;
                }
                return ret;
            }
            else if((buf[2] == 0x85) && (buf[1] == 0x0e))
            {
                flashid = icn85xx_read_flashid();
                printf("icn85xx_flashid2: 0x%x \n", flashid);
                if((MD25D40_ID1 == flashid) || (MD25D40_ID2 == flashid)
                    ||(MD25D20_ID1 == flashid) || (MD25D20_ID2 == flashid)
                    ||(GD25Q10_ID == flashid) || (MX25L512E_ID == flashid)
                    || (MD25D05_ID == flashid)|| (MD25D10_ID == flashid))
                {
                    icn85xx_ts.ictype = ICN85XX_WITH_FLASH_86;
                }
                else
                {
                    icn85xx_ts.ictype = ICN85XX_WITHOUT_FLASH;
                }
                return ret;
            }
            else  //for ICNT87
            {
    			ret = icn87xx_prog_i2c_rxdata(0xf001, buf, 2);

    			if(ret > 0)
    			{
    				if(buf[1] == 0x87)
    				{
                        flashid = icn87xx_read_flashid();
                        printf("icnt87 flashid: 0x%x\n",flashid);
                        if(0x114051 == flashid)
                        {
                            icn85xx_ts.ictype = ICN85XX_WITH_FLASH_87;

                        }
                        else
                        {
    				        icn85xx_ts.ictype = ICN85XX_WITHOUT_FLASH_87;
                        }
    					return ret;
    				}
    			}
	        }

        }

        delay_2ms(3);
    }
    if(retry==3)
    {
        return -1;
    }
    return 0;
}


int icn85xx_update(void)
{
    unsigned short fwVersion = 0;
    unsigned short curVersion = 0;
    int retry = 0;
	int ret=-1;

    if(icn85xx_ts.ictype == ICN85XX_WITHOUT_FLASH)
    {

	    icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);
        if(R_OK == icn85xx_fw_update(firmware))
        {
            icn85xx_ts.code_loaded_flag = 1;
            printf("ICN85XX_WITHOUT_FLASH, update default fw ok\n");
        }
        else
        {
            icn85xx_ts.code_loaded_flag = 0;
            printf("ICN85XX_WITHOUT_FLASH, update error\n");
	     return -1;

        }
	    return 	R_OK;
    }
    else if((icn85xx_ts.ictype == ICN85XX_WITH_FLASH_85) || (icn85xx_ts.ictype == ICN85XX_WITH_FLASH_86))
    {
        icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);
        fwVersion = icn85xx_read_fw_Ver(firmware);
        curVersion = icn85xx_readVersion();


        #if FORCE_UPDATA_FW
            retry = 3;
            while(retry > 0)
            {
                if(icn85xx_goto_progmode() != 0)
                {
                    printf("\n icn85xx_goto_progmode() != 0 error\n");
                    return -1;
                }
                icn85xx_read_flashid();
                if(R_OK == icn85xx_fw_update(firmware))
                {
                    break;
                }
                retry--;
                printf("\n icn85xx_fw_update failed. \n");
            }
		if(!retry)
			return -1;

        #elif COMPILE_FW_WITH_DRIVER
           if(fwVersion > curVersion)
           {
                retry = 3;
                while(retry > 0)
                    {
                        if(R_OK == icn85xx_fw_update(firmware))
                        {
                            break;
                        }
                        retry--;
                        printf("icn85xx_fw_update failed.\n");
                    }
			if(!retry)
				return -1;
           }
        #endif
	return 	R_OK;
    }
     else if(icn85xx_ts.ictype  == ICN85XX_WITHOUT_FLASH_87)
    {
        printf("\n icn85xx_update  87 without flash\n");

            icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);

        fwVersion = icn85xx_read_fw_Ver(firmware);
        printf("\n fwVersion : 0x%x\n", fwVersion);


        if(R_OK == icn87xx_fw_update(firmware))
        {
            icn85xx_ts.code_loaded_flag = 1;
            printf("ICN87XX_WITHOUT_FLASH, update default fw ok\n");
	     return R_OK;
        }
        else
        {
            icn85xx_ts.code_loaded_flag = 0;
            printf("ICN87XX_WITHOUT_FLASH, update error\n");
	     return -1;
        }

    }
	  else if(icn85xx_ts.ictype == ICN85XX_WITH_FLASH_87)
    {
        printf("icn85xx_update 87 with flash\n");
        icn85xx_set_fw(sizeof(icn85xx_fw), &icn85xx_fw[0]);


        fwVersion = icn85xx_read_fw_Ver(firmware);
        curVersion = icn85xx_readVersion();


        #if FORCE_UPDATA_FW
            if(R_OK == icn87xx_fw_update(firmware))
            {
                icn85xx_ts.code_loaded_flag = 1;
                printf("ICN87XX_WITH_FLASH, update default fw ok\n");
		  return R_OK;
            }
            else
            {
                icn85xx_ts.code_loaded_flag = 0;
                printf("ICN87XX_WITH_FLASH, update error\n");
		  return -1;
            }

        #elif COMPILE_FW_WITH_DRIVER
            if(fwVersion > curVersion)
            {
                retry = 3;
                while(retry > 0)
                {
                    if(R_OK == icn87xx_fw_update(firmware))
                    {
                        return R_OK;
                        break;
                    }
                    retry--;
                    printf("icn87xx_fw_update failed.\n");
                }
		   if(retry==0)
		   	return -1;
            }
        #endif
    }
	else
	{
		printf("ictype error, ictype: %x .\n",icn85xx_ts.ictype);
		return -1;
	}
    return 0;
}
char icn85xx_ioctl(u32 cmd, u32 arg)
{
    switch (cmd) {
    case TOUCH_PANEL_CMD_SLEEP:
        ICN85XX_sleep();
        break;
    case TOUCH_PANEL_CMD_WAKEUP:
        ICN85XX_wakeup();
        break;
    case TOUCH_PANEL_CMD_RESET:
        ICN85XX_init();
        break;
    case TOUCH_PANEL_CMD_DISABLE:
        if (_touch_panel_data->enable) {
            _touch_panel_data->enable = 0;
            sys_timer_del(_touch_panel_timer_reg);
        }
        break;
    case TOUCH_PANEL_CMD_ENABLE:
        if (!_touch_panel_data->enable) {
            _touch_panel_data->enable = 1;
            _touch_panel_timer_reg = sys_timer_add((void *)0, icn85xx_read_point, 20);
        }
        break;
    default:
        return 0;
    }
    return 1;
}

#if 0
_TOUCH_PANEL_INTERFACE icn85xx_ops = {
    .logo 				 = 	"icn85xx",
    .touch_panel_check   =   icn85xx_check,
    .touch_panel_init    =   ICN85XX_init,
    .touch_panel_ioctl   =   icn85xx_ioctl,
};

REGISTER_TOUCH_PANEL(icn85xx)

.touch_panel_ops = &icn85xx_ops,

};
#endif

