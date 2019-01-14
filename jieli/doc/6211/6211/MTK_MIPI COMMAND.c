#ifdef BUILD_LK
#include <string.h>
#else
#include <linux/string.h>
#endif
 
#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/upmu_common.h>
#endif
#include "lcm_drv.h"


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH  (768)
#define FRAME_HEIGHT (1024)


#define LCM_DSI_CMD_MODE    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {
    .set_gpio_out = NULL,
};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

int mipi_transfer = 0;
struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

#define REGFLAG_DELAY 										0xFE
#define REGFLAG_END_OF_TABLE								0xFF

#ifdef BUILD_LK
#define I2C_CH                I2C0
#define MT8193_I2C_ADDR       0x5A
#endif
         
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) 
    {
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) 
        {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }	
}

static void lcd_stb(unsigned char enabled){
    if (enabled)
    {
        mt_set_gpio_out(GPIO_LCM_STB, GPIO_OUT_ONE);
    }
    else
    {
        mt_set_gpio_out(GPIO_LCM_STB, GPIO_OUT_ZERO);
    }
}
         
static void lcd_power_en(unsigned char enabled)
{
    if (enabled)
    {      
#ifdef BUILD_LK
        /* VGP2_PMU 3V */
        upmu_set_rg_vgp1_vosel(5);
        upmu_set_rg_vgp1_en(1);

#else
	hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2800, "LCM");
#endif
        mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ONE);
    }
    else
    {      
#ifdef BUILD_LK
        /* VGP2_PMU 3V */
       	upmu_set_rg_vgp1_en(0);
        upmu_set_rg_vgp1_vosel(0);
#else
	hwPowerDown(MT6323_POWER_LDO_VGP1, "LCM");
#endif
        mt_set_gpio_out(GPIO_LCM_PWR, GPIO_OUT_ZERO);
    }
}

#define GPIO_LVDS_RST GPIO82

static void lcm_lvds_sleep_que(char enabled){

    mt_set_gpio_mode(GPIO_LVDS_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LVDS_RST, GPIO_DIR_OUT);

    if (enabled)
    {
#ifdef BUILD_LK
        /* VGP2_PMU 3V */
        upmu_set_rg_vgp3_vosel(3);
        upmu_set_rg_vgp3_en(1);
#else
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1800, "LCM");
#endif
	mt_set_gpio_out(GPIO_LVDS_RST, 0);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LVDS_RST, 1);
	MDELAY(10);
    }
    else
    {
#ifdef BUILD_LK
        /* VGP2_PMU 3V */
        upmu_set_rg_vgp3_en(0);
        upmu_set_rg_vgp3_vosel(0);
#else
        hwPowerDown(MT6323_POWER_LDO_VGP3, "LCM");
#endif
	mt_set_gpio_out(GPIO_LVDS_RST, 0);
    }
}

static void lcd_reset(unsigned char enabled)
{
    if (enabled)
    {
        mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    }
    else
    {	
        mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);    	
    }
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
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type     = LCM_TYPE_DSI;
    params->width    = FRAME_WIDTH;
    params->height   = FRAME_HEIGHT;
    params->dsi.mode = BURST_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;

    // Video mode setting		
    params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;


#if 0
    params->dsi.vertical_sync_active				= 50;
    params->dsi.vertical_backporch					= 30;
    params->dsi.vertical_frontporch 				= 36;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 64;
    params->dsi.horizontal_backporch				= 56;
    params->dsi.horizontal_frontporch				= 60;
    params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;
#else
    params->dsi.vertical_sync_active                            = 10;//1;
    params->dsi.vertical_backporch                                      = 15;//18;
    params->dsi.vertical_frontporch                             = 15;//23;
    params->dsi.vertical_active_line                            = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active                          = 20;//1;
    params->dsi.horizontal_backporch                            = 70;//80;
    params->dsi.horizontal_frontporch                           = 70;//80;
    params->dsi.horizontal_active_pixel                         = FRAME_WIDTH;
#endif


#if 0
    // Bit rate calculation
    params->dsi.pll_div1 = 0;		
    params->dsi.pll_div2 = 1; 		
    params->dsi.fbk_div  = 19;

    params->dsi.cont_clock = 1;
    params->dsi.horizontal_bllp = 0x115; //word count=0x340
#else

#if 0
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.packet_size = 256;
    params->dsi.intermediat_buffer_num = 0;
    params->dsi.word_count = 800*3;
#endif
    params->dsi.PLL_CLOCK                              = 177;//133;//249;//181;//137;//134;//140;//249;
    //params->dsi.LCM_DSI_PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_247;
    params->dsi.ssc_disable = 1;
    params->dsi.cont_clock = 1;
    params->dsi.horizontal_bllp = 0x115; //word count=0x340
#endif

}

static struct LCM_setting_table lcm_initialization_setting[] = {
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/
#if 1
//ping
	        {0x7A,  1,      {0xC1}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x20,  1,      {0x00}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x21,  1,      {0x00}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x22,  1,      {0x43}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x23,  1,      {0x46}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x24,  1,      {0x14}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x25,  1,      {0x46}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x26,  1,      {0x00}},
        {REGFLAG_DELAY, 2,      {}},

        {0x27,  1,      {0x0f}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x28,  1,      {0x0a}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x29,  1,      {0x0f}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x34,  1,      {0x80}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x36,  1,      {0x46}},
        //{REGFLAG_DELAY,       2,      {}},

        {0xB5,  1,      {0xA0}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x5C,  1,      {0xFF}},
        //{REGFLAG_DELAY,       2,      {}},

{0x13,  1,      {0x10}},
		
		{0x56,  1,      {0x90}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x6B,  1,      {0x21}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x69,  1,      {0x20}},
        //{REGFLAG_DELAY,       2,      {}},


//{0x10,  1,      {0x47}},
//{0x2a,  1,      {0x41}},
        {0xB6,  1,      {0x20}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x51,  1,      {0x20}},
        //{REGFLAG_DELAY,       2,      {}},

        {0x09,  1,      {0x10}},
        //{REGFLAG_DELAY,       2,      {}},


    {0x7A,  1,  {0x3E}},
    //{REGFLAG_DELAY, 2,  {}},
#else	//old
	{0x7A,	1,	{0xC1}},
	{REGFLAG_DELAY,	2,	{}},
	
	{0x20,	1,	{0x00}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x21,	1,	{0x00}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x22,	1,	{0x43}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x23,	1,	{0x3C}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x24,	1,	{0x40}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x25,	1,	{0x38}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x26,	1,	{0x00}},
	{REGFLAG_DELAY,	2,	{}},

	{0x27,	1,	{0x24}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x28,	1,	{0x32}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x29,	1,	{0x1E}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x34,	1,	{0x80}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x36,	1,	{0x3C}},
	//{REGFLAG_DELAY,	2,	{}},

	{0xB5,	1,	{0xA0}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x5C,	1,	{0xFF}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x56,	1,	{0x90}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x6B,	1,	{0x21}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x69,	1,	{0x19}},
	//{REGFLAG_DELAY,	2,	{}},
	

//{0x10,  1,      {0x47}},
//{0x2a,  1,      {0x41}},
	{0xB6,	1,	{0x20}},
	//{REGFLAG_DELAY,	2,	{}},
	
	{0x51,	1,	{0x20}},
	//{REGFLAG_DELAY,	2,	{}},

	{0x09,	1,	{0x10}},
	//{REGFLAG_DELAY,	2,	{}},

    {0x7A,  1,  {0x3E}},
    {REGFLAG_DELAY, 2,  {}},

#endif

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

extern void DSI_clk_HS_mode(unsigned char enter);
extern void DSI_Continuous_HS(void);
static void init_lcm_registers(void)
{
    unsigned int data_array[16];
    
#ifdef BUILD_LK
    printf("%s, LK \n", __func__);
#else
    printk("%s, kernel", __func__);
#endif
    
    //mipi_transfer = 1;
    //push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    //mipi_transfer = 0;

    data_array[0] = 0x00010500;  //software reset					 
    //dsi_set_cmdq(data_array, 1, 1);
    MDELAY(50);
    DSI_Continuous_HS();
    DSI_clk_HS_mode(1);
    MDELAY(80);
    
    data_array[0] = 0x00290500;  //display on                        
    //dsi_set_cmdq(data_array, 1, 1);

    mipi_transfer = 1;
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    mipi_transfer = 0;
	
}


static void lcm_init(void)
{
#if 0//def BUILD_LK
    return;//LK donothing
#else
//    lcd_reset(0);
//    lcd_power_en(0);
lcm_lvds_sleep_que(1);
	lcd_stb(1);
	MDELAY(10);//Must > 5ms
    lcd_power_en(1);
    MDELAY(50);//Must > 5ms
    lcd_reset(1);
    MDELAY(200);//Must > 50ms
    init_lcm_registers();
#endif  
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];
    
    data_array[0] = 0x00280500;  //display off                        
    dsi_set_cmdq(data_array, 1, 1);
//    MDELAY(5);

lcm_lvds_sleep_que(0);
    lcd_reset(0);
    lcd_power_en(0);
    DSI_clk_HS_mode(0);
//    MDELAY(10);
}


static void lcm_resume(void)
{
lcm_lvds_sleep_que(1);
    lcd_reset(0);
    lcd_power_en(0);
    lcd_power_en(1);
    MDELAY(50);//Must > 5ms
    lcd_reset(1);
    MDELAY(200);//Must > 50ms
    init_lcm_registers();
}


LCM_DRIVER auo_b079xat02_dsi_vdo_lcm_drv = 
{
    .name           = "kd079d2_32nv_a8",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    //.compare_id    = lcm_compare_id,
};



