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


#define GPIO_LCM_CLK (GPIO71|0x80000000)

#define GPIO_LCM_DATA (GPIO125| 0x80000000)

#define GPIO_LCM_CS (GPIO121| 0x80000000)

#define GPIO_LCM_RST (GPIO146| 0x80000000)


#define SPI_CS_H		mt_set_gpio_mode(GPIO_LCM_CS,GPIO_MODE_00);\
	 				mt_set_gpio_dir(GPIO_LCM_CS,GPIO_DIR_OUT);\
					mt_set_gpio_out(GPIO_LCM_CS,GPIO_OUT_ONE)
					
#define SPI_CS_L       mt_set_gpio_mode(GPIO_LCM_CS,GPIO_MODE_00);\
	 				mt_set_gpio_dir(GPIO_LCM_CS,GPIO_DIR_OUT);\
				      mt_set_gpio_out(GPIO_LCM_CS,GPIO_OUT_ZERO)

#define SPI_CLK_H	mt_set_gpio_mode(GPIO_LCM_CLK,GPIO_MODE_00);\
	 				mt_set_gpio_dir(GPIO_LCM_CLK,GPIO_DIR_OUT);\
	 				mt_set_gpio_out(GPIO_LCM_CLK,GPIO_OUT_ONE)

#define SPI_CLK_L     mt_set_gpio_mode(GPIO_LCM_CLK,GPIO_MODE_00);\
	 				mt_set_gpio_dir(GPIO_LCM_CLK,GPIO_DIR_OUT);\
	 				mt_set_gpio_out(GPIO_LCM_CLK,GPIO_OUT_ZERO)

#define SPI_DO_H    mt_set_gpio_mode(GPIO_LCM_DATA,GPIO_MODE_00);\
	 				mt_set_gpio_dir(GPIO_LCM_DATA,GPIO_DIR_OUT);\
	 				mt_set_gpio_out(GPIO_LCM_DATA,GPIO_OUT_ONE)

#define SPI_DO_L       mt_set_gpio_mode(GPIO_LCM_DATA,GPIO_MODE_00);\
	 				mt_set_gpio_dir(GPIO_LCM_DATA,GPIO_DIR_OUT);\
	 				mt_set_gpio_out(GPIO_LCM_DATA,GPIO_OUT_ZERO)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (240)
#define FRAME_HEIGHT (320)
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))





#define SPI_DO_IN   mt_set_gpio_mode(GPIO_LCM_DATA,GPIO_MODE_00);\
                              mt_set_gpio_dir(GPIO_LCM_DATA,GPIO_DIR_IN)
                           
#define SPI_DO_OUT   mt_set_gpio_mode(GPIO_LCM_DATA,GPIO_MODE_00);\
                              mt_set_gpio_dir(GPIO_LCM_DATA,GPIO_DIR_OUT)



void read_idd(char data1){
char i,data;
char tp=4;//4;

ulong id=0;

//return;

  SPI_CS_L;
 // UDELAY(1);
 SPI_DO_L;

 SPI_CLK_L;
 
 SPI_CLK_H;


for (i=0;i<8;i++){

 	SPI_CLK_L;

	  if(tp&0x80)  {SPI_DO_H;}
	  else  {SPI_DO_L;}
	  SPI_CLK_H;
	  tp<<=1;

}


SPI_DO_IN;

SPI_CLK_L;
UDELAY(1);
SPI_CLK_H;

id=0;

for(i=0;i<24;i++){

	 id<<=1 ; 
	SPI_CLK_L;
      UDELAY(1);
	SPI_CLK_H;
       if(mt_get_gpio_in(GPIO_LCM_DATA))  id|=1;
	

}







SPI_CS_H;

//SPI_DO_OUT;
//SPI_DO_H;
//SPI_CLK_H;
/*

mt_set_gpio_mode(GPIO_LCM_DATA,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_DATA,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_DATA,0); 

mt_set_gpio_mode(GPIO_LCM_CLK,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_CLK,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_CLK,GPIO_OUT_ONE); 


  mt_set_gpio_mode(GPIO_LCM_CS,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_CS,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_CS,GPIO_OUT_ONE); 



	  mt_set_gpio_mode(GPIO_LCM_RST,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST,GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCM_RST,0); 
	
*/

#ifdef BUILD_LK
    printf("111111www1ddddd11VVV1id====%lx \n",id); 
#else
    printk("0000000000000000000000id====%lx \n",id);
#endif



}



void SPI_WriteComm(unsigned int i)
{ 
    unsigned char n,m;

	
	SPI_CS_L;	
	
	SPI_CLK_L;
	SPI_DO_L;
	SPI_CLK_H;
	
    //SPI_CS = 0;    
    //SPI_CLK = 0;
    //SPI_DI = 0;
    //SPI_CLK = 1;
     
    m=i;
    for(n=0; n<8; n++)				
    {     	SPI_CLK_L;      
           if((m&0x80)!=0)

            {    
               // SPI_CLK = 0;
               // SPI_DI = 1;
               // SPI_CLK = 1;
             
		SPI_DO_H;
	//	SPI_CLK_L;
	//	SPI_CLK_H;
	
            }
            else
            {     
               // SPI_CLK = 0;
               // SPI_DI = 0;
                //SPI_CLK = 1;
    
	SPI_DO_L;
//	SPI_CLK_L;
//	SPI_CLK_H;
	
            }            
SPI_CLK_H;
            m=m<<1;

           
     }  
     
     //SPI_CS = 1;
     
	SPI_CS_H;
} 



void SPI_WriteData(unsigned int i)
{ 
    unsigned char n,m;
    
 //   SPI_CS = 0;    
  //  SPI_CLK = 0;
  //  SPI_DI = 1;
  //  SPI_CLK = 1;
  SPI_CS_L;	

	SPI_CLK_L;
	SPI_DO_H;
	SPI_CLK_H;
	
     
    m=i;
    for(n=0; n<8; n++)						
    {    	SPI_CLK_L;       
           if((m&0x80)!=0)

            {    
                //SPI_CLK = 0;
               // SPI_DI = 1;
                //SPI_CLK = 1;

		SPI_DO_H;
	//	SPI_CLK_L;
//		SPI_CLK_H;
            }
            else
            {     
              //  SPI_CLK = 0;
             //   SPI_DI = 0;
             //   SPI_CLK = 1;

			  SPI_DO_L;
//	SPI_CLK_L;
//	SPI_CLK_H;
            }  
				SPI_CLK_H;          
            m=m<<1;

           
     }  
     
     SPI_CS_H;
} 




void ST7789SHANNS24panelinitialcode(void) 
{ 
	//-----------------------------------ST7789S reset sequence------------------------------------// 
    SPI_CS_H;    
    SPI_CLK_H;
    SPI_DO_H;
	//LCD_RESET=1; 

	//SET_RESET_PIN(1);
	mt_set_gpio_mode(GPIO_LCM_RST,GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCM_RST,GPIO_DIR_OUT);
	
	mt_set_gpio_out(GPIO_LCM_RST,1); 

	
	MDELAY(1); 											   //Delay 1ms 
//	LCD_RESET=0; 
	
  
  
	mt_set_gpio_out(GPIO_LCM_RST,0); 

	//SET_RESET_PIN(0);
	MDELAY(1);											   //Delay 10ms 
	//LCD_RESET=1; 
	//SET_RESET_PIN(0);
		
	mt_set_gpio_out(GPIO_LCM_RST,1); 

	//Delay 120ms 
	MDELAY(10);	


	//read_idd(4);

SPI_WriteComm(0x11); 
MDELAY(120);    

                                          //Delay 120ms 
//------------------------------display and color format setting--------------------------------// 
SPI_WriteComm(0x36); 
SPI_WriteData(0x00); 
SPI_WriteComm(0x3a); 
SPI_WriteData(0x66); 
//--------------------------------ST7789S Frame rate setting----------------------------------//  

SPI_WriteComm(0xb1);
SPI_WriteData(0x40);	//40  00   //4e
SPI_WriteData(0x18);		//18
SPI_WriteData(0x1a);		 //	1a

SPI_WriteComm(0xb0);
SPI_WriteData(0x11);	//11 00
SPI_WriteData(0xF0);	//f0 C0

SPI_WriteComm(0xb2); 
SPI_WriteData(0x0c); 
SPI_WriteData(0x0c); 
SPI_WriteData(0x00); 
SPI_WriteData(0x33); 
SPI_WriteData(0x33); 
SPI_WriteComm(0xb7); 
SPI_WriteData(0x35); 
//---------------------------------ST7789S Power setting--------------------------------------// 
SPI_WriteComm(0xbb); 
SPI_WriteData(0x35); 
SPI_WriteComm(0xc0); 
SPI_WriteData(0x2c); 
SPI_WriteComm(0xc2); 
SPI_WriteData(0x01); 
SPI_WriteComm(0xc3); 
SPI_WriteData(0x10); 
SPI_WriteComm(0xc4); 
SPI_WriteData(0x20); 
SPI_WriteComm(0xc6); 
SPI_WriteData(0x0f); 
SPI_WriteComm(0xd0); 
SPI_WriteData(0xa4); 
SPI_WriteData(0xa1); 
//--------------------------------ST7789S gamma setting---------------------------------------// 
SPI_WriteComm(0xe0); 
SPI_WriteData(0xd0); 
SPI_WriteData(0x00); 
SPI_WriteData(0x02); 
SPI_WriteData(0x07); 
SPI_WriteData(0x0a); 
SPI_WriteData(0x28); 
SPI_WriteData(0x32); 
SPI_WriteData(0x44); 
SPI_WriteData(0x42); 
SPI_WriteData(0x06); 
SPI_WriteData(0x0e); 
SPI_WriteData(0x12); 
SPI_WriteData(0x14); 
SPI_WriteData(0x17); 
SPI_WriteComm(0xe1); 
SPI_WriteData(0xd0); 
SPI_WriteData(0x00); 
SPI_WriteData(0x02); 
SPI_WriteData(0x07); 
SPI_WriteData(0x0a); 
SPI_WriteData(0x28); 
SPI_WriteData(0x31); 
SPI_WriteData(0x54); 
SPI_WriteData(0x47); 
SPI_WriteData(0x0e); 
SPI_WriteData(0x1c); 
SPI_WriteData(0x17); 
SPI_WriteData(0x1b); 
SPI_WriteData(0x1e); 


SPI_WriteComm(0x21); 


MDELAY(10);    


SPI_WriteComm(0x29); 	

//	SPI_WriteComm(0x29); 


}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{

#if 1
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;
		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
		// DSI
		/* Command mode setting */		
		params->dsi.LANE_NUM				= LCM_ONE_LANE;
		//The following defined the fomat for data coming from LCD engine.
		//params->dsi.data_format.color_order = LCM_COLOR_ORDER_BGR;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666;//LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		    params->dsi.packet_size=256;

		// Video mode setting		
	    params->dsi.intermediat_buffer_num = 2;//2  //because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
   
			params->dsi.PS=LCM_PACKED_PS_18BIT_RGB666;//LCM_PACKED_PS_24BIT_RGB888;	
	    params->dsi.vertical_sync_active     = 4;
	    params->dsi.vertical_backporch   = 4;
	    params->dsi.vertical_frontporch  = 8;
			params->dsi.vertical_active_line				= FRAME_HEIGHT; 
	
	    params->dsi.horizontal_sync_active   = 10;
	    params->dsi.horizontal_backporch     = 10;
	    params->dsi.horizontal_frontporch    = 38;
			params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

  		params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
   		params->dsi.vertical_active_line=FRAME_HEIGHT;
   		params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

	params->dsi.PLL_CLOCK =68;//64;// 27; //234; //240;
	  params->dsi.pll_div1=1;						// div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=2;						// div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =8;	//4				// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#else



#ifdef BUILD_LK
    printf("--m16--LK--hx8394a--%s--\n", __func__);
#else
    printk("--m16--KERNEL--hx8394a--%s--\n", __func__);
#endif
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

    #if 0// (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
    #else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE ;//BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
    #endif

	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM				= LCM_ONE_LANE;//LCM_THREE_LANE ;//LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666;

	// Video mode setting
	params->dsi.PS=LCM_LOOSELY_PS_18BIT_RGB666;

	params->dsi.vertical_sync_active				= 4;// 3    2
	params->dsi.vertical_backporch					= 4;// 20   1
	params->dsi.vertical_frontporch					= 8; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;// 50  2
	params->dsi.horizontal_backporch				= 10;
	params->dsi.horizontal_frontporch				= 38;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    //params->dsi.LPX=8;

	// Bit rate calculation
	params->dsi.PLL_CLOCK =64; //54;//108 ;//180;//34;// 234; //240;
	//1 Every lane speed
	params->dsi.pll_div1=0; // div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	params->dsi.pll_div2=0; // div2=0,1,2,3;div1_real=1,2,4,4
	params->dsi.fbk_div =9; // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#endif


}


#ifdef BUILD_LK
#define I2C_CH                I2C2
#define ICN6211_I2C_ADDR      0x58>>1

/*
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

*/



u32 icn6211_reg_i2c_read(u8 addr)
{
   /* u8 cmd = addr;
    int cmd_len = 1;
    u8 data = 0xFF;
    int data_len = 1;
		u32 result_tmp;
		
    result_tmp = _icn6211_i2c_read(ICN6211_I2C_ADDR, &cmd, cmd_len, &data, data_len);
    return data;
*/
 kal_uint32 ret_code = I2C_OK;
    kal_uint8 len;
    struct mt_i2c_t i2c;

   char  dataBuffer[2];
   dataBuffer[0]= addr;

    i2c.id = I2C_CH;
    i2c.addr = ICN6211_I2C_ADDR;
    i2c.mode = ST_MODE;
    i2c.speed = 100;
    len = 1;
    
    
    
    	#ifndef BUILD_LK
	   printk("8912===========iiiiiiii=========\n");
	 #else
	  printf("8912==========iiiiiiiiiiii====\n");
	 #endif  
	 
	 

    ret_code = i2c_write_read(&i2c, dataBuffer, len, len);



   	#ifndef BUILD_LK
	   printk("8912===========iiiiiiii==11111111===%x\n",dataBuffer[0]);
	 #else
	  printf("8912==========iiiiiiiiiiii==222222222==%x,bbb2========%x \n",dataBuffer[0],dataBuffer[1]);
	 #endif  
	 
	 

      return  *dataBuffer;


}

int icn6211_reg_i2c_write(u8 addr, u8 value)
{
   /* u8 cmd = addr;
    int cmd_len = 1;
    u8 data = value;
    int data_len = 1;	
    u32 result_tmp;

    result_tmp = _icn6211_i2c_write(ICN6211_I2C_ADDR, &cmd, cmd_len, &data, data_len);
		return result_tmp;
		*/
		
		
		 kal_uint32 ret_code = I2C_OK;
      kal_uint8 write_data[I2C_FIFO_SIZE], len;
    struct mt_i2c_t i2c;

    i2c.id = I2C_CH;
    i2c.addr = ICN6211_I2C_ADDR;
    i2c.mode = ST_MODE;
    i2c.speed = 100;

    write_data[0]= addr;
      write_data[1] = value;
    len = 2;

    #if 1 //def lt8912_DEBUG
      /* dump write_data for check */
    printf("[lt8912_i2c_write] dev_addr = 0x%x, write_data[0x%x] = 0x%x \n", addr, write_data[0], write_data[1]);
    #endif

    ret_code = i2c_write(&i2c, write_data, len);


    #ifndef BUILD_LK
	   printk("8912===========ret_code=======%d==\n",ret_code);
	 #else
	 
	 
	  printf("8912==========ret_code===%d=\n",ret_code);
	 #endif  
      return ret_code;
      
      
      
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
/*

0x20 = 0xF0
0x21 = 0x40
0x22 = 0x10
0x23 = 0x26
0x24 = 0x0A
0x25 = 0x0A
0x26 = 0x00
0x27 = 0x0F
0x28 = 0x04
0x29 = 0x04
0x34 = 0x80
0x36 = 0x26
0x86 = 0x29
0xB5 = 0xA0
0x5C = 0xFF
0x2A = 0x01
0x56 = 0x92
0x6B = 0x71
0x69 = 0x07
0x10 = 0x40
0x11 = 0x88
0xB6 = 0x20
0x51 = 0x20
0x09 = 0x10




0x20 = 0xF0
0x21 = 0x40
0x22 = 0x10
0x23 = 0x26
0x24 = 0x0A
0x25 = 0x0A
0x26 = 0x00
0x27 = 0x08
0x28 = 0x04
0x29 = 0x04
0x34 = 0x80
0x36 = 0x26
0x86 = 0x28
0xB5 = 0xA0
0x5C = 0xFF
0x2A = 0x01
0x56 = 0x90
0x6B = 0x71
0x69 = 0x09
0x10 = 0x00
0x11 = 0x88
0xB6 = 0x20
0x51 = 0x20
0x09 = 0x10


*/

ICN6211_REG_WRITE(0x20, 0xF0);
ICN6211_REG_WRITE(0x21, 0x40);
ICN6211_REG_WRITE(0x22, 0x10);
ICN6211_REG_WRITE(0x23, 0x26);
ICN6211_REG_WRITE(0x24, 0x0A);
ICN6211_REG_WRITE(0x25, 0x0a);
ICN6211_REG_WRITE(0x26, 0x00);
ICN6211_REG_WRITE(0x27, 0x08);
ICN6211_REG_WRITE(0x28, 0x04);
ICN6211_REG_WRITE(0x29, 0x04);
ICN6211_REG_WRITE(0x34, 0x80);
ICN6211_REG_WRITE(0x36, 0x26);
ICN6211_REG_WRITE(0x86, 0x28);
ICN6211_REG_WRITE(0xB5, 0xA0);
ICN6211_REG_WRITE(0x5C, 0xFF);
ICN6211_REG_WRITE(0x2A, 0x01);
ICN6211_REG_WRITE(0x56, 0x93);
ICN6211_REG_WRITE(0x6B, 0x71);
ICN6211_REG_WRITE(0x69, 0x09);//10
ICN6211_REG_WRITE(0x10, 0x05);
ICN6211_REG_WRITE(0x11, 0x88);
ICN6211_REG_WRITE(0xB6, 0x20);
ICN6211_REG_WRITE(0x51, 0x20);
ICN6211_REG_WRITE(0x09, 0x10);

//ICN6211_REG_WRITE(0x14, 0x43);
//ICN6211_REG_WRITE(0x2a, 0x49);


/*
icn6211_reg_i2c_read(0x20);
icn6211_reg_i2c_read(0x21);
icn6211_reg_i2c_read(0x22);
icn6211_reg_i2c_read(0x23);
icn6211_reg_i2c_read(0x24);
icn6211_reg_i2c_read(0x25);
icn6211_reg_i2c_read(0x26);
icn6211_reg_i2c_read(0x27);
icn6211_reg_i2c_read(0x28);
icn6211_reg_i2c_read(0x29);
icn6211_reg_i2c_read(0x34);
icn6211_reg_i2c_read(0x36);
icn6211_reg_i2c_read(0x86);
icn6211_reg_i2c_read(0xB5);
icn6211_reg_i2c_read(0x5C);
icn6211_reg_i2c_read(0x87);
icn6211_reg_i2c_read(0x2A);
icn6211_reg_i2c_read(0x56);
icn6211_reg_i2c_read(0x6B);
icn6211_reg_i2c_read(0x69);
icn6211_reg_i2c_read(0x10);
icn6211_reg_i2c_read(0x11);
icn6211_reg_i2c_read(0xB6);
icn6211_reg_i2c_read(0x51);
icn6211_reg_i2c_read(0x09);
*/

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
/*
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
*/



#define ICN6211_EN GPIO120

#define ICN6211_VCC_EN GPIO78

static void lcm_power(bool on)
{
	
	
	#ifdef BUILD_LK
    printf("[LK/LCM] init_icn6211_registers() - lcm_power -\n");
#elif (defined BUILD_UBOOT)
#else
    printk("[LCM] init_icn6211_registers() - lcm_power-\n");
#endif


	mt_set_gpio_mode(ICN6211_VCC_EN,GPIO_MODE_00);
	mt_set_gpio_dir(ICN6211_VCC_EN,GPIO_DIR_OUT);
	mt_set_gpio_out(ICN6211_VCC_EN,GPIO_OUT_ONE); 
	
	MDELAY(20);
		
	mt_set_gpio_mode(ICN6211_EN,GPIO_MODE_00);
	mt_set_gpio_dir(ICN6211_EN,GPIO_DIR_OUT);
	mt_set_gpio_out(ICN6211_EN,on?GPIO_OUT_ONE:GPIO_OUT_ZERO); 
		
		
		
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
    MDELAY(120);
    
    init_icn6211_registers();
    MDELAY(2);

   // r61526_init();    
    //MDELAY(2);   
	ST7789SHANNS24panelinitialcode();
}

static void lcm_suspend(void)
{	
	 SET_RESET_PIN(1);
   MDELAY(20);
   SET_RESET_PIN(0);
   MDELAY(50);
   SET_RESET_PIN(1);
   MDELAY(100);
    
//	 SspSendData(enter_sleep, sizeof(enter_sleep));
//	 MDELAY(30);
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
