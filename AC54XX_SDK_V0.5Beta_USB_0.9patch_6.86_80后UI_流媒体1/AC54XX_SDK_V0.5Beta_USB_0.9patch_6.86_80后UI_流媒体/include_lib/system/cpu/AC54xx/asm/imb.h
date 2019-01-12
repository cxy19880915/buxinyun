#ifndef __IMB_H__
#define __IMB_H__

#include "typedef.h"


//图层索引
enum {
    IMB_LAYER0,
    IMB_LAYER1,
    IMB_LAYER2,
    IMB_LAYER3,
    IMB_LAYER4,
    IMB_LAYER5,
    IMB_LAYER6,
    IMB_LAYER7,
    IMB_MAX_LAYER,
};



struct yuv_addr {
    u8 *y_badr;
    u8 *u_badr;
    u8 *v_badr;
};

//输出格式
enum {
    PACKED_YUV420,
    PACKED_YUV422,
    PACKED_YUV444,
};

//输入格式
enum {
    FMT_SOLID_COLOR,         //
    FMT_YUV420,              //16
    FMT_YUV422,              //16
    FMT_RGB888,              //8
    FMT_RGB565,              //8
    FMT_OSD_16BITS,          //水平8对齐
    FMT_OSD_8BITS,           //水平8对齐
    FMT_OSD_2BITS,           //水平32对齐
    FMT_OSD_1BIT,            //水平64对齐
};

//OSD表类型
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

struct merge_info {
    u8 rotate_en;           //旋转使能
    u8 hori_mirror_en;      //水平镜像使能
    u8 vert_mirror_en;      //垂直镜像使能
    u8 packed_format;       //数据格式
    u32 dest_addr;          //存放地址
};

struct osd_info {
    u8 type;
    u8 index;
    u32 *tab;               //osd表(1/2/8位)
};

struct layer_info {
    u8  en;                 //使能
    u8  index;        		//图层序号
    u8  alpha;              //透明度
    u8  data_fmt;           //数据格式
    u8  osd_tab_sel;        //osd表选择
    u16 x;					//图层起始X坐标
    u16 y;					//图层起始Y坐标
    u16 width;				//图层宽度
    u16 height;				//图层高度
    u16 buf_width;           //buf宽度(以像素为单位)
    u16 buf_height;
    u32 back_color;         //背景颜色
    u8 *baddr;				//数据基地址
    struct yuv_addr yuv;	//不用设置
};


// struct imb_info {
// u16 frame_width;		//16 align,[16-2048]
// u16 frame_height;		//16 align,[16-2048]
// struct osd_info osd[OSD_MAX_TAB];
// struct layer_info layer[IMB_MAX_LAYER];
// [>struct merge_info;<]
// struct imb_opt_reg *reg;
// };

#define LAYER_ENABLE	1
#define LAYER_DISABLE   0

#define write_reg(reg, value)  	*(reg) = value
#define set_reg_bit(reg, b) 	*(reg) = *(reg) | BIT(b)
#define clr_reg_bit(reg,b)      *(reg) = *(reg) & ~BIT(b)
#define read_reg_bit(reg,b)     *(reg)&BIT(b)


void disp_init(void);
void imb_frame_size(u16 width, u16 height);
void imb_osd_tab_conf(u8 type, u8 index, const u32 *tab);
struct layer_info *imb_get_layer_hdl(u8 index);
void imb_layer_conf(struct layer_info *layer);
void imb_merge_conf(struct merge_info *info);
u8 imb_read_pnd();

#endif
