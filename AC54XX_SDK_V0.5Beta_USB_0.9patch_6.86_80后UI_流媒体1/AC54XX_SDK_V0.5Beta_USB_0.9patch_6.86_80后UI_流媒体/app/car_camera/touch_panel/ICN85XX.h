/*++

 Copyright (c) 2012-2022 ChipOne Technology (Beijing) Co., Ltd. All Rights Reserved.
 This PROPRIETARY SOFTWARE is the property of ChipOne Technology (Beijing) Co., Ltd.
 and may contains trade secrets and/or other confidential information of ChipOne
 Technology (Beijing) Co., Ltd. This file shall not be disclosed to any third party,
 in whole or in part, without prior written consent of ChipOne.
 THIS PROPRIETARY SOFTWARE & ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
 WITH ALL FAULTS, & WITHOUT WARRANTY OF ANY KIND. CHIPONE DISCLAIMS ALL EXPRESS OR
 IMPLIED WARRANTIES.

 File Name:    icn85xx.h
 Abstract:
              input driver.
Author:       Zhimin Tian
Date :        08,14,2013
Version:      1.0
History :
   2012,10,30, V0.1 first version

 --*/


#ifndef __LINUX_ICN85XX_H__
#define __LINUX_ICN85XX_H__


#ifndef __ICN85XX_H__
#define __ICN85XX_H__

    #include "typedef.h"
    //#include "touch_driver.h"
    #define ICN85_MAX_X               1280
    #define ICN85_MAX_Y               480
    #define ICN85_MAX_TOUCH_POINT     5

    #define GTP_CNT_DOWN        2
    #define GTP_CNT_LONG        80
    #define GTP_CNT_HOLD        8

    u8 ICN85XX_read_id(void);
    u8 ICN85XX_init(void);
    u8 ICN85XX_read_point(void);
    u8 ICN85XX_sleep(void);
    void ICN85XX_wakeup(void);
#endif




//tp config

#define COMPILE_FW_WITH_DRIVER      1
#define FORCE_UPDATA_FW             0


#define ICN_X_MIRRORING         0   
#define ICN_Y_MIRRORING         0   

//#define ENABLE_BYTE_CHECK       //do not open anytime

#define POINT_NUM                   ICN85_MAX_TOUCH_POINT



//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
     #define COL_NUM                          128//24
     #define ROW_NUM                          128//36

     #define MD25D40_ID1                      0x514013
     #define MD25D40_ID2                      0xC84013
     #define MD25D20_ID1                      0x514012
     #define MD25D20_ID2                      0xC84012
     #define GD25Q10_ID                       0xC84011
     #define MX25L512E_ID                     0xC22010
     #define MD25D05_ID                       0x514010
     #define MD25D10_ID                       0x514011

	#define ICN85XX_WITHOUT_FLASH              0x11
	#define ICN85XX_WITH_FLASH_85              0x22
	#define ICN85XX_WITH_FLASH_86              0x33
	#define ICN85XX_WITHOUT_FLASH_87           0x55
    #define ICN85XX_WITH_FLASH_87              0x66



#define FLASH_TOTAL_SIZE                0x00010000
#define FLASH_PAGE_SIZE                 0x1000
#define FLASH_AHB_BASE_ADDR             0x00100000
#define FLASH_PATCH_PARA_BASE_ADDR      (FLASH_TOTAL_SIZE - FLASH_PAGE_SIZE)          //  allocate 1 page for patch para, 0xff00
#define FLASH_CODE_INFO_BASE_ADDR       (FLASH_PATCH_PARA_BASE_ADDR - FLASH_PAGE_SIZE)        //  0xfe00,allocate 1 page for system para
#define FLASH_CRC_ADDR                  (FLASH_AHB_BASE_ADDR + FLASH_CODE_INFO_BASE_ADDR + 0x00) //  0xfe00
#define FLASH_CODE_LENGTH_ADDR          (FLASH_AHB_BASE_ADDR + FLASH_CODE_INFO_BASE_ADDR + 0x04) //  0xfe04





#define ICN85XX_NAME                "ctp_icn85xx"

#define CTP_NAME                    ICN85XX_NAME

#define CTP_RESET_LOW_PERIOD        (5)
#define CTP_RESET_HIGH_PERIOD       (100)
#define CTP_WAKEUP_LOW_PERIOD       (20)
#define CTP_WAKEUP_HIGH_PERIOD      (50)
#define CTP_POLL_TIMER              (16)    /* ms delay between samples */
#define CTP_START_TIMER             (100)    /* ms delay between samples */


#define IIC_RETRY_NUM               3
#define POINT_SIZE                  7

#define ICN85xx_REG_PMODE  			0x04
#define PMODE_ACTIVE                0x00
#define PMODE_MONITOR               0x01
#define PMODE_HIBERNATE             0x02

#define MAX_LENGTH_PER_TRANSFER           128
#define B_SIZE                            120//96//32 //128//64//32


//-----------------------------------------------------------------------------
// Struct, Union and Enum DEFINITIONS
//-----------------------------------------------------------------------------
typedef struct _POINT_INFO
{
    unsigned char  u8ID;
    unsigned short u16PosX;     // coordinate X, plus 4 LSBs for precision extension
    unsigned short u16PosY;     // coordinate Y, plus 4 LSBs for precision extension
    unsigned char  u8Pressure;
    unsigned char  u8EventId;
}POINT_INFO;

typedef struct icn85xx_ts_data {
#if 0  //  struct i2c_client        *client;
//    struct input_dev         *input_dev;
//    struct work_struct       pen_event_work;
  //  struct workqueue_struct  *ts_workqueue;
//CONFIG_HAS_EARLYSUSPEND
    struct early_suspend     early_suspend;

   // struct hrtimer           timer;
 //   spinlock_t               irq_lock;
 //   struct semaphore         sem;
   #endif
    int         ictype;
    int         code_loaded_flag;
  //  POINT_INFO  point_info[POINT_NUM+1];
    int         point_num;
    int         irq;
    int         irq_is_disable;
    int         use_irq;
    int         work_mode;
    int         screen_max_x;
    int         screen_max_y;
    int         revert_x_flag;
    int         revert_y_flag;
    int         exchange_x_y_flag;
}icn85xx_ts_data;

typedef enum
{
    R_OK = 100,
    R_FILE_ERR,
    R_STATE_ERR,
    R_ERASE_ERR,
    R_PROGRAM_ERR,
    R_VERIFY_ERR,
}E_UPGRADE_ERR_TYPE;

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//add icnt87xx
//-----------------------------------------------------------------------------
extern unsigned short icnt87_sram_crc;
extern unsigned short icnt87_sram_length;
  ////////////icn87 config///////
#define    SFCTL_BASE_87       (0x0000F600)

#define    CMD_SEL_87          (SFCTL_BASE_87 + 0x0000)
#define    FLASH_ADDR_87       (SFCTL_BASE_87 + 0x0004)
#define    SRAM_ADDR_87        (SFCTL_BASE_87 + 0x0008)
#define    DATA_LENGTH_87      (SFCTL_BASE_87 + 0x000C)
#define    START_DEXC_87       (SFCTL_BASE_87 + 0x0010)
#define    RELEASE_FLASH_87    (SFCTL_BASE_87 + 0x0014)
#define    CLEAR_HW_STATE_87   (SFCTL_BASE_87 + 0x0018)
#define    CRC_RESULT_87       (SFCTL_BASE_87 + 0x001C)
#define    SW_CRC_START_87     (SFCTL_BASE_87 + 0x0020)
#define    SF_BUSY_87          (SFCTL_BASE_87 + 0x0024)
#define    WATCHDOG_CRC_CFG_87 (SFCTL_BASE_87 + 0x0028)

#define FLASH_CMD_FAST_READ                        0x01
#define FLASH_CMD_ERASE_SECTOR                     0x02
#define FLASH_CMD_ERASE_BLOCK                      0x03
#define FLASH_CMD_PAGE_PROGRAM                     0x04
#define FLASH_CMD_READ_STATUS                      0x05
#define FLASH_CMD_READ_IDENTIFICATION              0x06

#define FLASH_EARSE_4K                             0
#define FLASH_EARSE_32K                            1

#define FLASH_STOR_INFO_ADDR                       0xe000
#define SRAM_EXCHANGE_ADDR                         0xd000
#define SRAM_EXCHANGE_ADDR1                        0xd100

#define FIRMWARA_INFO_AT_BIN_ADDR                  0x00f4

typedef unsigned char  U8;
typedef unsigned short  U16;
typedef unsigned int   U32;

/////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

int icn85xx_iic_test(void);
int icn85xx_update(void);
void ICN85XX_reset(void);

int  icn85xx_i2c_rxdata(unsigned short addr, char *rxdata, int length);
int  icn85xx_i2c_txdata(unsigned short addr, char *txdata, int length);
int  icn85xx_write_reg(unsigned short addr, char para);
int  icn85xx_read_reg(unsigned short addr, char *pdata);
int  icn85xx_prog_i2c_rxdata(unsigned int addr, char *rxdata, int length);
int  icn85xx_prog_i2c_txdata(unsigned int addr, char *txdata, int length);
int  icn85xx_prog_write_reg(unsigned int addr, char para);
int  icn85xx_prog_read_reg(unsigned int addr, char *pdata);
int icn85xx_prog_read_page(unsigned int Addr,unsigned char *Buffer, unsigned int Length);

int  icn85xx_readVersion(void);
void icn85xx_rawdatadump(short *mem, int size, char br);
void icn85xx_set_fw(int size, unsigned char *buf);
void icn85xx_memdump(char *mem, int size);
int  icn85xx_checksum(int sum, char *buf, unsigned int size);
int  icn85xx_update_status(int status);
int  icn85xx_get_status(void);
int  icn85xx_open_fw( char *fw);
int  icn85xx_read_fw(int offset, int length, char *buf);
int  icn85xx_close_fw(void);
int  icn85xx_goto_progmode(void);
int  icn85xx_check_progmod(void);
int  icn85xx_read_flashid(void);
int  icn85xx_erase_flash(void);
int  icn85xx_prog_buffer(unsigned int flash_addr,unsigned int sram_addr,unsigned int copy_length,unsigned char program_type);
int  icn85xx_prog_data(unsigned int flash_addr, unsigned int data);
void  icn85xx_read_flash(unsigned int sram_address,unsigned int flash_address,unsigned long copy_length,unsigned char i2c_wire_num);
int  icn85xx_fw_download(unsigned int offset, unsigned char * buffer, unsigned int size);
int  icn85xx_bootfrom_flash(int ictype);
int  icn85xx_bootfrom_sram(void);
int  icn85xx_crc_enable(unsigned char enable);
unsigned int icn85xx_crc_calc(unsigned crc_in, char *buf, int len);

short  icn85xx_read_fw_Ver(char *fw);
int  icn85xx_fw_update(void *fw);
//E_UPGRADE_ERR_TYPE  icn85xx_fw_update(void *fw);

//-------------------------------------------------------------------------------
//icnt87xx Function PROTOTYPES
//-----------------------------------------------------------------------------
int   icn87xx_fw_update(void *fw);
int   icn87xx_boot_sram(void);
int icn87xx_calculate_crc(unsigned short len);
int  icn87xx_prog_i2c_rxdata(unsigned int addr, char *rxdata, int length);
int  icn87xx_prog_i2c_txdata(unsigned int addr, char *txdata, int length);
int  icn87xx_read_flashid(void);
#endif

