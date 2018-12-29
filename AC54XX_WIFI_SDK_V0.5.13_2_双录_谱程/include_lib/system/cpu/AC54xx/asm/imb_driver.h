#ifndef IMB_DRIVER_H
#define IMB_DRIVER_H

#include "video/fb.h"
#include "typedef.h"
#include "asm/imd.h"


//IMAGE图层颜色定义
#define IMAGE_COLOR_WHITE   0xeb8080
#define IMAGE_COLOR_RED     0x515aef
#define IMAGE_COLOR_GREEN   0x903522
#define IMAGE_COLOR_BLUE    0x28ef6d
#define IMAGE_COLOR_YELLOW  0xd21092
#define IMAGE_COLOR_PURPLE  0x6acadd
#define IMAGE_COLOR_CYAN    0xa9a510
#define IMAGE_COLOR_BLACK   0x108080

enum {
    FMT_SOLID_COLOR,
    FMT_YUV420,              //水平16对齐
    FMT_YUV422,              //水平16对齐
    FMT_RGB888,              //水平8对齐
    FMT_RGB565,              //水平8对齐
    FMT_OSD_16BITS,          //水平8对齐
    FMT_OSD_8BITS,           //水平8对齐
    FMT_OSD_2BITS,           //水平32对齐
    FMT_OSD_1BIT,            //水平64对齐
};

enum {
    OSD_TAB_1BIT,
    OSD_TAB_2BITS,
    OSD_TAB_8BITS,
};

//OSD表选择
enum {
    OSD_TAB0,
    OSD_TAB1,
    OSD_MAX_TAB,
};

enum {
    IMG_STA_CLOSED,
    IMG_STA_WAIT_OPEN,
    IMG_STA_OPEN,
};

enum {
    PACKED_YUV420,
    PACKED_YUV422,
    PACKED_YUV444,
};

struct imb_layer_merge {
    u8 rotate_en;           //旋转使能
    u8 hori_mirror_en;      //水平镜像使能
    u8 vert_mirror_en;      //垂直镜像使能
    u8 packed_format;       //数据格式
    u32 dest_addr;          //存放地址
    u16 width;
    u16 height;
    u8 mode;
    u8 status;
};


struct imb_layer_reg {
    volatile u32 con;
    volatile u32 h_cfg0;
    volatile u32 h_cfg1;
    volatile u32 v_cfg0;
    volatile u32 v_cfg1;
    volatile u32 h_with;
    volatile u32 aph;
    volatile u32 badr0;
    volatile u32 badr1;
    volatile u32 badr2;
    volatile u32 sc_dat;
    volatile u32 hw_with;
};


struct bufrect {
    u16 x;
    u16 y;
    u16 width;
    u16 height;
};

struct imbreq {
    u8  *src;
    u8  *dst;
    u32 src_x;
    u32 src_y;
    u32 src_width;
    u32 src_height;
    u32 dst_x;
    u32 dst_y;
    u32 dst_width;
    u32 dst_height;
    u8  buf_index;
    u16 scale;
    u8  sta;
    u8  busy;
    u8  flags;
    struct imb_layer *layer;
    void *fd;
};

struct imb_layer {
    u8  layer_dis;
    u8  status;
    u8  alpha;         //只对图层1有效
    u8  data_fmt;
    u8  osd_tab_sel;
    u8  sw_en;
    u8  pending;
    u8  block;
    u16  scale;
    struct bufrect src;
    struct bufrect dst;
    struct imbreq ireq;
    u16 x;            //图层x坐标
    u16 y;            //图层y坐标
    u16 width;        //图层宽度
    u16 height;       //图层高度
    u16 buf_x;		  //显示buf起始x坐标
    u16 buf_y;        //显示buf起始y坐标
    u16 buf_width;    //显示buf宽度
    u16 buf_height;   //显示buf高度
    u8  buffer_num;
    u8  buffer_status;
    u8  inused;
    u8  layer_num;
    u32 buffer_size;
    u32 baddr;
    u32 baddr1; 	/* buf0 图层软件插值使用 */
    u32 baddr0;     /* buf1 图层软件插值使用 */
    u32 baddr_t;    /* 临时buf */
    int back_color;
    struct fb_info fb;
    struct imb_layer_reg *reg;

    u32 convert_baddr;
    int convert_begin;
};


#define IMB_LAYER_NUM 8
enum {
    MERGED_IDLE,
    MERGED_LEFT,
    MERGED_RIGHT,
};

enum {
    MERGED_MODE_NORMAL,
    MERGED_MODE_SPECIAL,
};

struct dvxxfb_info {
    void *dev;
    struct imb_layer layer[IMB_LAYER_NUM];
    struct imb_layer_merge merge;
    int start;
};

struct layer_buf {
    u8  num;
    u8  total_num;
    u8  format;
    u16 width;
    u16 height;
    u8  *baddr;
    u32 size;
};

struct lcd_screen_info {
    u16 xres;
    u16 yres;
};

struct imb_update {
    u8 re_update;
    u8 imb_busy;
    struct imb_layer layer;
    u32 baddr;
    int (*refresh)(struct imb_layer *layer, u32 baddr);
};


struct fb_platform_data {
    u8 z_order[8];
};

#define RATIO(r) ((r==FB_COLOR_FORMAT_YUV420)?12:\
				 ((r==FB_COLOR_FORMAT_YUV422)?16:\
				 ((r==FB_COLOR_FORMAT_RGB888)?24:\
				 ((r==FB_COLOR_FORMAT_RGB565)?16:\
				 ((r==FB_COLOR_FORMAT_16K)?16:\
				 ((r==FB_COLOR_FORMAT_256)?8:\
				 ((r==FB_COLOR_FORMAT_2BIT)?2:\
				 ((r==FB_COLOR_FORMAT_1BIT)?1:0))))))))

#define REGISTER_LAYER_BUF(layer,f,w,h,n) \
	u8 layer##_##f##_##w##x##h##_##n[(w*h*n*RATIO(f)/8+31)/32*32] ALIGNE(32);\
	static struct layer_buf layer##_##f##_##w##x##h##_##n##_t sec(.layer_buf_t) = {\
		.format = f,\
		.num = n,\
		.total_num = n,\
		.width = w,\
		.height = h,\
		.baddr = layer##_##f##_##w##x##h##_##n,\
		.size = (w*h*n*RATIO(f)/8+31)/32*32,\
	}\

extern struct layer_buf layer_buf_begin[];
extern struct layer_buf layer_buf_end[];

#define list_for_each_layer_buf(p) \
	for (p=layer_buf_begin; p < layer_buf_end; p++)

int imb_init();
void imb_layer_set_baddr(struct imb_layer *layer, u8 index);

#endif


