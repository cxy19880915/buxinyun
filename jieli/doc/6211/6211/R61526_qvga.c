#ifdef BUILD_LK
#include <platform/mt_gpio.h> 
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#include <platform/upmu_common.h>
#include <debug.h>
#elif (defined BUILD_UBOOT)
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include "icn6211_iic.h"
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#define ICN6211_LOG printf
	#define ICN6211_REG_WRITE(add, data) icn6211_reg_i2c_write(add, data)
	#define ICN6211_REG_READ(add) icn6211_reg_i2c_read(add)
#elif (defined BUILD_UBOOT)
#else
	extern int icn6211_i2c_write(u8 addr, u8 data);
	extern int icn6211_i2c_read(u16 addr, u32 *data);
	#define ICN6211_LOG printk
	#define ICN6211_REG_WRITE(add, data) icn6211_i2c_write(add, data)
	#define ICN6211_REG_READ(add) lcm_icn6211_i2c_read(add)
#endif

#define SPI_CS_H		mt_set_gpio_out(GPIO55,GPIO_OUT_ONE)
#define SPI_CS_L    mt_set_gpio_out(GPIO55,GPIO_OUT_ZERO)

#define SPI_CLK_H		mt_set_gpio_out(GPIO57,GPIO_OUT_ONE)
#define SPI_CLK_L   mt_set_gpio_out(GPIO57,GPIO_OUT_ZERO)

#define SPI_DO_H    mt_set_gpio_out(GPIO56,GPIO_OUT_ONE)
#define SPI_DO_L    mt_set_gpio_out(GPIO56,GPIO_OUT_ZERO)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (320)
#define FRAME_HEIGHT (240)
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

static const u16 powerOnData1[] = { 
					0x0B0,
					0x13F,
					0x13F
};
//160ms	

static const u16 powerOnData2[] = { 
					0x0FE,
					0x100,
					0x100,
					0x100,
					0x121,
					0x1B4,
					
					0x0B3,
					0x100,
					0x110,
					
					0x0E0,
					0x100,
					0x140,			
};		
//200ms					
static const u16 powerOnData3[] = { 
					0x0B3,
					0x100,
					0x100,
					
					0x0FE,
					0x100,
					0x100,
					0x100,
					0x121,
					0x130,
					
					0x0B0,
					0x13F,
					0x13F,
					
					0x0B3,
					0x100,
					0x100,
					0x100,
					0x100,
					
					0x0B4,
					0x100,
					
					0x0C0,
					0x113,
					0x14F,
					0x100,
					0x110,
					0x1A0,
					0x120,
					0x101,
					0x143,
					
					0x0C1,
					0x101,
					0x102,
					0x11f,
					0x108,
					0x108
};		
//200ms		
static const u16 powerOnData4[] = {
					0x0c3,
					0x101,
					0x100,
					0x121,
					0x108,
					0x108
};

//200ms		
static const u16 powerOnData5[] = {
					0x0c4,
					0x110,
					0x107,
					0x110,
					0x101,
					
					0x0C8,
					0x107,
					0x10d,
					0x10d,
					0x111,
					0x116,
					0x10c,
					0x107,
					0x109,
					0x111,
					0x107,
					0x112,
					0x111,
					0x109,
					0x107,
					0x10c,
					0x116,
					0x111,
					0x10d,
					0x10d,
					0x107,
					0x103,
					0x121,
					
					0x0C9,
					0x107,
					0x10d,
					0x10d,
					0x111,
					0x116,
					0x10c,
					0x107,
					0x109,
					0x111,
					0x107,
					0x112,
					0x111,
					0x109,
					0x107,
					0x10c,
					0x116,
					0x111,
					0x10d,
					0x10d,
					0x107,
					0x103,
					0x121,
					
					0x0CA,
					0x107,
					0x10d,
					0x10d,
					0x111,
					0x116,
					0x10c,
					0x107,
					0x109,
					0x111,
					0x107,
					0x112,
					0x111,
					0x109,
					0x107,
					0x10c,
					0x116,
					0x111,
					0x10d,
					0x10d,
					0x107,
					0x103,
					0x121,
					
					0x0D0,
					0x133,
					0x153,
					0x182,
					0x122,
					
					0x0D1,
					0x137,
					0x15d,
					0x110,
					
					0x0D2,
					0x103,
					0x132,
					
					0x0D4,
					0x103,
					0x125,
					
					0x0E2,
					0x13f,
					
					0x035,
					0x100,
					
					0x036,
					0x100,
					
					0x03A,
					0x166,
					
					0x02A,
					0x100,
					0x100,
					0x100,
					0x1EF,
					
					0x02B,
					0x100,
					0x100,
					0x101,
					0x13F,
					
					0x011
};					
//100ms		

static const u16 powerOnData6[] = {
			0x029,
			
			0x0B4,
			0x110,
			
			0x02c
};
//20ms	

static const u16 enter_sleep[] = {
			0x028,
			0x010
};


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;
		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
		// DSI
		/* Command mode setting */		
		params->dsi.LANE_NUM				= LCM_THREE_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_BGR;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666;//LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		    params->dsi.packet_size=256;

		// Video mode setting		
	    params->dsi.intermediat_buffer_num = 2;//2  //because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
   
			params->dsi.PS=LCM_PACKED_PS_18BIT_RGB666;//LCM_PACKED_PS_24BIT_RGB888;	
	    params->dsi.vertical_sync_active     = 10;
	    params->dsi.vertical_backporch   = 3;
	    params->dsi.vertical_frontporch  = 3;
			params->dsi.vertical_active_line				= FRAME_HEIGHT; 
	
	    params->dsi.horizontal_sync_active   = 20;
	    params->dsi.horizontal_backporch     = 50;
	    params->dsi.horizontal_frontporch    = 20;
			params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

  		params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
   		params->dsi.vertical_active_line=FRAME_HEIGHT;
   		params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

	  params->dsi.pll_div1=1;						// div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=2;						// div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =8;	//4				// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
}


#ifdef BUILD_LK
#define I2C_CH                I2C1
#define ICN6211_I2C_ADDR      0x58

u32 _icn6211_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = mt_i2c_write(I2C_CH, chip, cmdBuffer, cmdBufferLen,1);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = mt_i2c_read(I2C_CH, chip, dataBuffer, dataBufferLen,1);

    return ret_code;
}

u32 _icn6211_i2c_write(U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        return 0;
    }

    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmdBuffer[cmdIndex];
        cmdIndex++;
        i++;
    }

    while(dataIndex < dataBufferLen)
    {
        write_data[i] = dataBuffer[dataIndex];
        dataIndex++;
        i++;
    }

    ret_code = mt_i2c_write(I2C_CH, chip, write_data, transfer_len, 1);

    return ret_code;
}

u32 icn6211_reg_i2c_read(u8 addr)
{
    u8 cmd = addr;
    int cmd_len = 1;
    u8 data = 0xFF;
    int data_len = 1;
		u32 result_tmp;
		
    result_tmp = _icn6211_i2c_read(ICN6211_I2C_ADDR, &cmd, cmd_len, &data, data_len);
    return data;
}

int icn6211_reg_i2c_write(u8 addr, u8 value)
{
    u8 cmd = addr;
    int cmd_len = 1;
    u8 data = value;
    int data_len = 1;	
    u32 result_tmp;

    result_tmp = _icn6211_i2c_write(ICN6211_I2C_ADDR, &cmd, cmd_len, &data, data_len);
		return result_tmp;
}


#else
//kernel build
u32 lcm_icn6211_i2c_read(u16 addr)
{
    u32 u4Reg = 0;
    u32 ret_code = 0;

    ret_code = icn6211_i2c_read(addr, &u4Reg);
    if (ret_code != 0)
    {
        return ret_code;
    }

    return u4Reg;
}
#endif

static void init_icn6211_registers()
{
#ifdef BUILD_LK
    printf("[LK/LCM] init_icn6211_registers() ++\n"); 
#elif (defined BUILD_UBOOT)
    // do nothing in uboot 
#else
    printk("[LCM] init_ic6211_registers() ++\n");
#endif


ICN6211_REG_WRITE(0x20, 0xF0);
ICN6211_REG_WRITE(0x21, 0x40);
ICN6211_REG_WRITE(0x22, 0x10);
ICN6211_REG_WRITE(0x23, 0x0A);
ICN6211_REG_WRITE(0x24, 0x0A);
ICN6211_REG_WRITE(0x25, 0x14);
ICN6211_REG_WRITE(0x26, 0x00);
ICN6211_REG_WRITE(0x27, 0x04);
ICN6211_REG_WRITE(0x28, 0x02);
ICN6211_REG_WRITE(0x29, 0x02);
ICN6211_REG_WRITE(0x34, 0x80);
ICN6211_REG_WRITE(0x36, 0x0A);
ICN6211_REG_WRITE(0x86, 0x2A);
ICN6211_REG_WRITE(0xB5, 0xA0);
ICN6211_REG_WRITE(0x5C, 0xFF);
ICN6211_REG_WRITE(0x87, 0x10);
ICN6211_REG_WRITE(0x2A, 0x01);
ICN6211_REG_WRITE(0x56, 0x92);
ICN6211_REG_WRITE(0x6B, 0x61);
ICN6211_REG_WRITE(0x69, 0x10);//10
ICN6211_REG_WRITE(0x10, 0x10);
ICN6211_REG_WRITE(0x11, 0x98);
ICN6211_REG_WRITE(0xB6, 0x20);
ICN6211_REG_WRITE(0x51, 0x20);
ICN6211_REG_WRITE(0x09, 0x10);

//ICN6211_REG_WRITE(0xa3, 0x53);
//ICN6211_REG_WRITE(0x6b, 0x62);
//ICN6211_REG_WRITE(0x69, 0x08);

//ICN6211_REG_WRITE(0x2e, 0x20);//0x2e->0x20 0x2f->0x20 0x2a->0x39 黑白棋格
//ICN6211_REG_WRITE(0x2f, 0x20);

//ICN6211_REG_WRITE(0x6B, 0x71);
//ICN6211_REG_WRITE(0x69, 0x09);
//ICN6211_REG_WRITE(0x56, 0x93);
//ICN6211_REG_WRITE(0x14, 0x43);
//ICN6211_REG_WRITE(0x2a, 0x39);//49              
#ifdef BUILD_LK
    printf("[LK/LCM] init_icn6211_registers() --\n");
#elif (defined BUILD_UBOOT)
#else
    printk("[LCM] init_icn6211_registers() --\n");
#endif
}

static int SspSendData(u16 *pData, u16 length)
{	
	u8 bitCount;
	u16 i,j;
	u16 data;  
	int len;
	 
  bitCount=9;
  
  len = length/sizeof(u16);
  
  SPI_CS_L;
	UDELAY(1);
  
  for(i=0;i<len;i++)
  {
	  	data=pData[i];  	 	
			for(j=bitCount;j!=0;j--)
			{  	
			  	SPI_CLK_L;
			  	if( 0x01 & (data>>(j-1)) )			  	
			  			SPI_DO_H;		  		
			  	else			  	
			  			SPI_DO_L;
		  	
			  	UDELAY(1);	  			
			  	SPI_CLK_H;			  	
			  	UDELAY(1);			  	
			} 	  	  	
  }
  SPI_CS_H;
  UDELAY(1);
  
  return 0;
}

static void r61526_init(void)
{		
		SspSendData(powerOnData1, sizeof(powerOnData1));
		MDELAY(150);
		SspSendData(powerOnData2, sizeof(powerOnData2));
		MDELAY(200);
		SspSendData(powerOnData3, sizeof(powerOnData3));
		MDELAY(200);
		SspSendData(powerOnData4, sizeof(powerOnData4));
		MDELAY(200);
		SspSendData(powerOnData5, sizeof(powerOnData5));
		MDELAY(200);
		SspSendData(powerOnData6, sizeof(powerOnData6));
		MDELAY(50);

}

static void lcm_power(bool on)
{
	mt_set_gpio_mode(GPIO139,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO139,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO139,on?GPIO_OUT_ONE:GPIO_OUT_ZERO); 
}

static void lcm_init(void)
{
		lcm_power(true);
		MDELAY(2);
	
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(100);
    
    init_icn6211_registers();
    MDELAY(2);

    r61526_init();    
    MDELAY(2);    
}

static void lcm_suspend(void)
{	
	 SET_RESET_PIN(1);
   MDELAY(20);
   SET_RESET_PIN(0);
   MDELAY(50);
   SET_RESET_PIN(1);
   MDELAY(100);
    
	 SspSendData(enter_sleep, sizeof(enter_sleep));
	 MDELAY(30);
}

static void lcm_resume(void)
{
	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
    return 1;
}

LCM_DRIVER r61526_qvga_lcm_drv = 
{
   .name			= "r61526_qvga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};
