#ifndef IMC_DRIVER_H
#define IMC_DRIVER_H



#include "typedef.h"
#include "system/includes.h"



//#define IMC_CHANNEL_NUM  	3
#define IMC_ENC_CH_NUM  	2
#define IMC_DISP_CH_NUM  	3

enum imc_src_type {
    IMC_SRC0 = 0,
    IMC_SRC1 = 1,
    IMC_SRC_REP = 2,
};



enum imc_isp_sel {
    IMC_SRC_SEL_ISP0,
    IMC_SRC_SEL_ISP1,
    IMC_SRC_SEL_MANUAL,

};

enum imc_yuv_mode {
    IMC_YUV420 = 0,
    IMC_YUV422,
};

enum imc_axi_max_len {
    IMC_512B = 0,
    IMC_256B,
};




//
//struct imc_osd_info {
//
//    u16 x ;//起始地址
//    u16 y ;//结束地址
//    u32 osd_yuv;//osd颜色
//
////注意：下面的字符串地址必须是全局的,然后年是yyyy，月是nn，日是dd，时是hh，分是mm，秒是ss,其他字符是英文字母&&符号&&汉字
//    char *osd_str; //用户自定义格式，例如 "yyyy-nn-dd\hh:mm:ss" 或者 "hh:mm:ss"
//    u8 osd_w;//用户自定义字体大小,8的倍数
//    u8 osd_h;//8的倍数
//    char *osd_matrix_str; //用户自定义字模字符串,例如“abcd....0123..”
//    u8 *osd_matrix_base; //用户自定义字模的起始地址
//    u32 osd_matrix_len;//用户自定义字模数组的长度,no str len!!!
//
//} ;

























#endif

