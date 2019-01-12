#ifndef __IMD_H__
#define __IMD_H__

#include "typedef.h"
#include "asm/cpu.h"
#include "generic/ioctl.h"

#define 	IMD_SET_COLOR_CFG 			_IOW('F', 0, sizeof(int))
#define 	IMD_GET_COLOR_CFG 			_IOR('F', 0, sizeof(int))
#define 	IMD_SET_COLOR_VALID 		_IOW('F', 1, sizeof(int))
#define 	IMD_GET_COLOR_CFG_SIZE 		_IOR('F', 1, sizeof(int))
#define 	IMD_SET_DEFAULT_COLOR_CFG 	_IOW('F', 2, sizeof(int))

extern const struct device_operations imd_dev_ops;

//<时钟数>
enum NCYCLE {
    CYCLE_ONE,      //RGB-24BIT、MCU-16BIT
    CYCLE_TWO,      //MCU-8BIT
    CYCLE_THREE,    //RGB-8BIT
    CYCLE_FOUR,     //RGB-8BIT+dummy
};

//<输出数据格式>
enum OUT_FORMAT {
    FORMAT_RGB333 = 2,
    FORMAT_RGB666,
    FORMAT_RGB888,
    FORMAT_RGB565,
    FORMAT_YUV888,
    FORMAT_YUV422,
    FORMAT_ITU656, //YUV422同时使能
};

//<扫描方式>
enum INTERLACED {
    INTERLACED_NONE,		//非隔行
    INTERLACED_TIMING,		//时序隔行
    INTERLACED_DATA,		//数据隔行
    INTERLACED_ALL,			//(INTERLACED_DATA|INTERLACED_TIMING)时序数据隔行
};

#define CLK_NEGATIVE    BIT(0)
#define CLK_EN          BIT(1)
#define SIGNAL_DEN      (0<<2)
#define SIGNAL_HSYNC    (1<<2)
#define SIGNAL_VSYNC    (2<<2)
#define SIGNAL_FIELD    (3<<2)

//<位宽>
enum LCD_PORT {
    PORT_1BIT   = 0x00,
    PORT_3BITS  = 0x01,
    PORT_6BITS  = 0x03,
    PORT_8BITS  = 0x07,
    PORT_9BITS  = 0x0F,
    PORT_16BITS = 0x1F,
    PORT_18BITS = 0x3F,
    PORT_24BITS = 0x7F,
};

//<lcd port select>
enum LCD_GROUP {
    PORT_GROUP_AA,
    PORT_GROUP_AB,
    PORT_GROUP_B,
};

//<color type>
enum {
    IMD_COLOR_RGB,
    IMD_COLOR_YUV,
};

//<input format>
enum IN_FORMAT {
    SAMP_YUV420,
    SAMP_YUV422,
    SAMP_YUV444,
};

//<奇/偶行相位>
enum PHASE {
    PHASE_R,
    PHASE_G,
    PHASE_B,
};


//<RAW模式>
enum RAW_MODE {
    RAW_STR_MODE,//B->R->G(default)
    RAW_DLT_MODE,//R->G->B
};

//<模式>
enum LCD_MODE {
    MODE_RGB_SYNC,			// 无显存 连续帧 接HSYNC VSYNC信号
    MODE_RGB_DE_SYNC,		// 无显存 连续帧 按DE HSYNC VSYNC信号
    MODE_RGB_DE,			// 无显存 连续帧 接DE信号
    MODE_MCU,				// 有显存 单帧 SmartPanel 接DE信号
    MODE_AVOUT,				// AV OUT
    MODE_CCIR656,			// 嵌入同步信号(8BITS+DCLK、不需要HSYNC、VSYNC信号,00 FF为同步信号,传输数据不能为00 FF)
};

//<时钟选择以及分频>
#define IMD_INTERNAL    	0x8000
#define IMD_EXTERNAL    	0x0000 //[默认,可省略] 外部PLL时钟目标频率为480MHz,需要通过分频得到所需的频率
//<晶振选择:IMD_INTERNAL模式有效>
#define OSC_12M				0x4000
#define OSC_32K				0x0000 //[默认,可省略]

/*
 * 注: IMD_EXTERNAL,OSC_32K可省略
 * IMD_EXTERNAL|PLL2_CLK|DIVA_1|DIVB_2|DIVC_3         等同于 PLL2_CLK|DIVA_1|DIVB_2|DIVC_3
 * IMD_INTERNAL|OSC_32K|PLL2_CLK|DIVA_1|DIVB_2|DIVC_3 等同于 IMD_INTERNAL|PLL2_CLK|DIVA_1|DIVB_2|DIVC_3
 */

#define PLL_SEL   			2
#define PLL_DIV1  			6
#define PLL_DIV2  			8
#define CVE_DIV   			10

/*
 * @brief 时钟源选择
 */
#define PLL0_CLK        	(0<<PLL_SEL)
#define PLL1_CLK        	(1<<PLL_SEL)
#define PLL2_CLK        	(2<<PLL_SEL)
#define PLL3_CLK        	(3<<PLL_SEL)


/**
 * @brief  时钟分频系统
 */
#define DIVA_1          	(0<<PLL_DIV1)
#define DIVA_3         		(1<<PLL_DIV1)
#define DIVA_5          	(2<<PLL_DIV1)
#define DIVA_7          	(3<<PLL_DIV1)

#define DIVB_1          	(0<<PLL_DIV2)
#define DIVB_2          	(1<<PLL_DIV2)
#define DIVB_4          	(2<<PLL_DIV2)
#define DIVB_8          	(3<<PLL_DIV2)

#define DIVC_1          	(0<<CVE_DIV)
#define DIVC_2          	(1<<CVE_DIV)
#define DIVC_3          	(2<<CVE_DIV)
#define DIVC_4          	(3<<CVE_DIV)

/**
 * @brief  颜色校正
 */

//二级命令										参数说明
#define ADJUST_EN       0x31//使能调试          不带参数
#define ADJUST_DIS      0x32//禁止调试			不带参数
#define GET_ALL         0x33//					struct color_correct
#define SET_ALL         0x34//					struct color_correct

#define SET_Y_GAIN      0x01//					u8
#define SET_U_GAIN      0x02//					u8
#define SET_V_GAIN      0x03//					u8
#define SET_Y_OFFS      0x04//					s8
#define SET_U_OFFS      0x05//					s8
#define SET_V_OFFS      0x06//					s8
#define SET_R_GAIN      0x07//					u8
#define SET_G_GAIN      0x08//					u8
#define SET_B_GAIN      0x09//					u8
#define SET_R_OFFS      0x0a//					s8
#define SET_G_OFFS      0x0b//					s8
#define SET_B_OFFS      0x0c//					s8
#define SET_R_COE0      0x0d//					u8
#define SET_R_COE1      0x0e//					s8
#define SET_R_COE2      0x0f//					s8
#define SET_G_COE0      0x10//					s8
#define SET_G_COE1      0x11//					u8
#define SET_G_COE2      0x12//					s8
#define SET_B_COE0      0x13//					s8
#define SET_B_COE1      0x14//					s8
#define SET_B_COE2      0x15//					u8
#define SET_R_GMA       0x16//					u16
#define SET_G_GMA       0x17//					u16
#define SET_B_GMA       0x18//					u16
#define SET_R_GMA_TBL   0x19//					256 bytes
#define SET_G_GMA_TBL   0x1a//					256 bytes
#define SET_B_GMA_TBL   0x1b//					256 bytes
#define SET_ISP_SCENE   0x1c//					s8
#define SET_CAMERA   	0x1d//					u8


#define FORE_CAMERA	0x01
#define BACK_CAMERA 	0x02
#define BOTH_CAMERA	0x03


struct color_correct {
    u8 y_gain;
    u8 u_gain;
    u8 v_gain;
    s8 y_offs;
    s8 u_offs;
    s8 v_offs;

    u8 r_gain;
    u8 g_gain;
    u8 b_gain;
    s8 r_offs;
    s8 g_offs;
    s8 b_offs;

    u8 r_coe0;
    s8 r_coe1;
    s8 r_coe2;

    s8 g_coe0;
    u8 g_coe1;
    s8 g_coe2;

    s8 b_coe0;
    s8 b_coe1;
    u8 b_coe2;

    u8 rev;			//结构体对齐

    u16 r_gma;
    u16 g_gma;
    u16 b_gma;

    u8 r_gamma_tab[256];
    u8 g_gamma_tab[256];
    u8 b_gamma_tab[256];
};

struct color_effect_cfg {
    const char *fname;
    struct color_correct *adj;
};

/**
 * 硬件时序
 */
struct dev_timing {
    u16 hori_interval; 		/*水平时钟总数(Horizontal Line/HSD period)*/
    u16 hori_sync_clk;		/*水平同步时钟(HSYNC pulse width/HSD pulse width)*/
    u16 hori_start_clk;		/*水平起始时钟(HSYNC blanking/HSD back porth)*/
    u16 hori_pixel_width;	/*水平像素(Horizontal display area/HSD display period)*/

    u16 vert_interval;		/*垂直同步总数(Vertical display area/VSD period time)*/
    u16 vert_sync_clk;		/*垂直同步时钟(VSYNC pulse width)*/
    u16 vert_start_clk;		/*垂直起始时钟(VSYNC Blanking Odd field/VSD back porch Odd field)*/
    u16 vert_start1_clk;	/*垂直起始时钟(隔行模式)(VSYNC Blanking Even field/VSD back porch Even field)*/
    u16 vert_pixel_width;	/*垂直像素(Vertical display area)*/
};

/*
 * 屏接口类型
 */
enum LCD_IF {
    LCD_MIPI,
    LCD_DVP_MCU,
    LCD_DVP_RGB,
};

enum LEN_CFG {
    LEN_256,
    LEN_512,
};

////显示相关////
struct imd_dmm_info {
    u16 xres;					// 屏水平分辨率
    u16 yres;					// 屏垂直分辨率

    u8  buf_num;				// 显示缓存数量
    u32 buf_addr;				// 显示缓存

    u8  test_mode;				// 测试模式(纯色)使能
    u32 color;					// 测试模式颜色设置

    u8  itp_mode_en;            // 插值模式使能，用于720P图源插值到1080P显示，输入不支持YUV444，输出不支持隔行扫描
    enum IN_FORMAT sample;		// 采样方式 YUV420/YUV422/YUV444
    enum OUT_FORMAT format;     // 输出数据格式
    struct color_correct adjust;// 颜色校正参数

    enum LEN_CFG len;			// 访问瞬时长度

    u8  rotate_en;				// 旋转使能
    u8  hori_mirror_en;			// 水平镜像使能
    u8  vert_mirror_en;			// 垂直镜像使能
};

struct imd_dev {
    struct imd_dmm_info info;
    enum LCD_MODE drive_mode;   // 驱动模式
    enum NCYCLE ncycle;         // 每像素时钟数
    enum INTERLACED interlaced_mode;         // 隔行模式
    enum PHASE raw_odd_phase;	// 奇行相位
    enum PHASE raw_even_phase;  // 偶行相位
    enum RAW_MODE raw_mode;     // RAW模式选择
    enum LCD_PORT data_width;   // 数据位宽
    enum LCD_GROUP port_sel;    // 输出端口选择
    u8 avout_mode;				// AVOUT制式(PAL/NTSC/TESTMODE)
    u8 dclk_set;                // dclk使能以及极性配置
    u8 sync0_set;               // AA:PH3/AB:PG2/B:PD3 (DE/HSYNC/VSYNC)
    u8 sync1_set;               // AA:PH4/AB:PG4/B:PD4 (DE/HSYNC/VSYNC)
    u8 sync2_set;               // AA:PH5/AB:PG5/B:PD5 (DE/HSYNC/VSYNC)
    u8 sync3_set;               // Reversed
    u32 clk_cfg;                // clk时钟分频
    u16 pll1_nf;				// 选IMD_INTERNAL时配置
    u16 pll1_nr;				// 选IMD_INTERNAL时配置
    struct dev_timing timing;   // 时序参数
};


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                  MIPI屏相关                                */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @Synopsis  dsi时序设置
 */
struct dsi_timing {
    u16 freq;			// mipi pll总线频率
    u16 tval_lpx;		// LowPower state period 传输时间
    u32 tval_wkup;      // 总线从ulps模式唤醒的时间
    u16 tval_c_pre;		// clk lane HS transition prepare time
    u16 tval_c_sot;     // clk lane HS transition sot time:LP-00
    u16 tval_c_eot;     // clk lane HS transition eot time
    u16 tval_c_brk;		// clk lane总线离开回到LP-11的时间
    u16 tval_d_pre;		// data lane HS transition prepare time
    u16 tval_d_sot;		// data lane HS transition sot time:LP-00
    u16 tval_d_eot;		// data lane HS transition eot time
    u16 tval_d_brk;		// data lane总线离开回到LP-11的时间
};

/**
 * @Synopsis  lane重映射
 */
struct dsi_lane_mapping {
    u8 x0_lane;
    u8 x1_lane;
    u8 x2_lane;
    u8 x3_lane;
    u8 x4_lane;
};
/*
 * 视频/命令模式
 */
enum VIDEO_STREAM_MODE {
    VIDEO_STREAM_COMMAND,   //命令模式
    VIDEO_STREAM_VIDEO,		//视频模式
};
/*
 * 同步事件
 */
enum SYNC_MODE {
    SYNC_PULSE_MODE,
    SYNC_EVENT_MODE,
};
/*
 * 高速模式
 */
enum BURST_MODE {
    NON_BURST_MODE,
    BURST_MODE,
};

enum COLOR_FORMAT {
    COLOR_FORMAT_RGB565,
    COLOR_FORMAT_RGB666,
    COLOR_FORMAT_RGB666_LOOSELY,
    COLOR_FORMAT_RGB888,
};
/*
 *  数据包类型
 */
enum PIXEL_TYPE {
    PIXEL_RGB565_COMMAND,
    PIXEL_RGB565_VIDEO,
    PIXEL_RGB666,
    PIXEL_RGB666_LOOSELY,
    PIXEL_RGB888,
};

/**
 * @Synopsis  时序
 */
struct dsi_video_timing {
    u8 video_mode;			//command/video mode
    u8 sync_event;			//sync pulse/event mode
    u8 burst_mode;			//non-burst/burst mode
    u8 vfst_lp_en;			//enable enter LowPower mode at first line
    u8 vlst_lp_en;			//enable enter LowPower mode at last line
    u8 vblk_lp_en;			//enable enter LowPower mode at blanking line
    u8 color_mode;
    u8 virtual_ch;		    //virtual channel(video mode)
    u8 hs_eotp_en;			//enable EoT packet
    u8 lpdt_psel0;			//enable insert LPDT at first line
    u8 lpdt_psel1;			//enable insert LPDT at last line
    u8 lpdt_psel2;			//enalbe insert LPDT at non-video line
    u8 pixel_type;

    u16 dsi_vdo_vsa_v;		//vertical sync pluse
    u16 dsi_vdo_vbp_v;		//vertical back porch
    u16 dsi_vdo_vact_v; 	//vertical active line
    u16 dsi_vdo_vfp_v;		//vertical front porch

    u16 dsi_vdo_hsa_v;		//horizontal sync pulse
    u16 dsi_vdo_hbp_v;		//horizontal back porch
    u16 dsi_vdo_hact_v;		//horizontal active pixel
    u16 dsi_vdo_hfp_v;		//horizontal front porch

    u16 dsi_vdo_null_v;		//horizontal null dummy
    u16 dsi_vdo_bllp_v;		//BLLP period width
};
/*
 * MIPI PLL时钟分频
 */
enum MIPI_PLL_DIV {
    MIPI_PLL_DIV1,
    MIPI_PLL_DIV2,
    MIPI_PLL_DIV4,
    MIPI_PLL_DIV8,
};

/*
 * 包类型
 */
enum PACKET_TYPE {
    PACKET_DCS,		 			/* DCS包 */
    PACKET_DCS_WITHOUT_BTA,		/* DCS包不带响应 */
    PACKET_GENERIC,  			/* 通用包 */
    PACKET_GENERIC_WITHOUT_BTA, /* 通用包不带响应 */
};

struct mipi_dev {
    struct imd_dmm_info info;
    struct dsi_lane_mapping lane_mapping;
    struct dsi_video_timing video_timing;
    struct dsi_timing timing;

    unsigned int reset_gpio;
    void (*lcd_reset)(unsigned int reset_gpio);
    void (*lcd_reset_release)(unsigned int reset_gpio);

    const u8 *cmd_list;
    u16 cmd_list_item;

    u8 debug_mode;								//调试使能
    /* 配置PLL的频率范围为600MHz~1.2GHz,少于600MHz的频率通过分频获得 */
    u16 target_freq;							//目标频率(MHz)
    enum MIPI_PLL_DIV  pll_division;			//分频
};
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct imd_dev *imd_init();
void imd_start();
void *imd_dmm_dev_open();
u32 *imd_dmm_get_update_flags();
void imd_dmm_color_adj(struct color_correct *adj, u8 init);
struct color_correct *imd_dmm_get_color_adj();
void imd_dmm_r_gamma_set(u8 *r_tab);
void imd_dmm_g_gamma_set(u8 *g_tab);
void imd_dmm_b_gamma_set(u8 *b_tab);
void imd_dmm_r_gamma_update();
void imd_dmm_g_gamma_update();
void imd_dmm_b_gamma_update();
void imd_dmm_r_gamma_cfg();
void imd_dmm_g_gamma_cfg();
void imd_dmm_b_gamma_cfg();

#define REGISTER_LCD_DEVICE(dev) \
	static const struct imd_dev dev sec(.lcd_device)

#define LCD_BUF_SIZE(w,h,s,n) (w*h*n*(((s)==SAMP_YUV420)?(3):(((s)==SAMP_YUV422)?4:6))/2)

extern u8 lcd_dev_buf_addr[];
#define REGISTER_LCD_DEVICE_BUF(w,h,s,n) \
	u8 lcd_dev_buf[LCD_BUF_SIZE(w, h, s, n)] sec(.lcd_dev_buf);

extern struct imd_dev lcd_device_begin[];
extern struct imd_dev lcd_device_end[];

#define list_for_each_lcd_device(p) \
	for (p=lcd_device_begin; p < lcd_device_end; p++)

enum {
    ESD_NOACTION,
    ESD_INTERNAL,/*此方法不允许在推屏过程中向屏发送命令*/
    ESD_EXTERNAL,
};

struct esd_deal {
    u8  method;
    /*内部处理的相关参数*/
    u8  deal_per_frames;
    /*外部处理的相关参数*/
    u8  en;
    u16 count;
    u16 interval;
    void (*esd_check_isr)(struct esd_deal *esd);
    int timer;
};

enum {
    BL_CTRL_BACKLIGHT_AND_CONTROLER,
    BL_CTRL_BACKLIGHT,
};

struct lcd_dev_drive {
    const char *logo;
    u8 enable;
    enum LCD_IF type;
    int (*init)(void *_data);
    int (*uninit)(void *_data);
    void (*bl_ctrl)(void *_data, u8 onoff);
    u8 bl_ctrl_flags;
    void (*setxy)(int x0, int x1, int y0, int y1);
    struct esd_deal esd;
    void *dev;
    u8 bl_sta;//背光状态
};

#define REGISTER_LCD_DEVICE_DRIVE(dev) \
	static const struct lcd_dev_drive dev##_drive sec(.lcd_device_drive)

extern struct lcd_dev_drive lcd_device_drive_begin[];
extern struct lcd_dev_drive lcd_device_drive_end[];

#define list_for_each_lcd_device_drive(p) \
	for (p=lcd_device_drive_begin; p < lcd_device_drive_end; p++)

#endif
