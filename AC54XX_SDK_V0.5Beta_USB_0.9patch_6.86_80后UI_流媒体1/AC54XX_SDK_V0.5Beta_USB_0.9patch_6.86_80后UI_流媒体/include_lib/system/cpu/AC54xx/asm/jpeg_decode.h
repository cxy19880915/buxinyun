/*************************************************************************
	> File Name: lib/include/cpu/dv16/asm/jpeg_decode.h
	> Author:
	> Mail:
	> Created Time: Mon 31 Oct 2016 02:22:35 PM HKT
 ************************************************************************/

#ifndef _JPEG_DECODE_H_
#define _JPEG_DECODE_H_

#include "typedef.h"
#include "jpeg_common.h"
#include "system/includes.h"

extern u16 QT_TBL[0x80];
extern u16 STD_HUFFMAN_TBL[258];

#define QTAB_ADDR       ((s16 *)(QT_TBL))
#define HTAB_DC0_ADDR   ((u16 *)((u8*)STD_HUFFMAN_TBL))                         //  0x30
#define HTAB_AC0_ADDR   ((u16 *)((u8*)STD_HUFFMAN_TBL + 0x30))                  //  0xd2
#define HTAB_DC1_ADDR   ((u16 *)((u8*)STD_HUFFMAN_TBL + 0x30 + 0xd2))           //  0x30
#define HTAB_AC1_ADDR   ((u16 *)((u8*)STD_HUFFMAN_TBL + 0x30 + 0xd2 + 0x30))    //  0xd2

struct jpeg_info {
    u8 *data;
    u8 *data_out;
    u32 cur_pos;
    u16 x_pos;
    u16 y_pos;
    u16 x;
    u16 y;
    u16 old_x;
    u16 old_y;
    u16 old_x_mcu_num;
    u16 old_y_mcu_num;
    u16 x_mcu_num;
    u16 y_mcu_num;
    u16 x_mcu_cnt;
    u16 y_mcu_cnt;
    u8 samp_Y;
    u8 htab_Y;
    u8 htab_Cr;
    u8 htab_Cb;
    u8 qtab_Y;
    u8 qtab_Cr;
    u8 qtab_Cb;
    u8 y_cnt;
};

struct jpeg_dec_fd {
    JPEG_SFR *sfr;
    struct jpeg_info jpg_info;
    struct jpg_yuv_buffer jpg_yuv[2];
    u32 mcu_num;
    u32 mcu_len;
    u32 head_len;
    u32 bits_cnt;
    u8  dec_frame;
    u8  mode;
    u8  old_mode;
    volatile u8 status;
    u8  yuv_type;
    u8  obuf_index;
    u8  manual_en;//control

    u8  frame_end;
    u8  restart_marker;
    u8  restart_flag;
    u32 mcu_interval_cnt;
    u32 mcu_interval;
    u32 frame_num;

    void *priv;

    int (*jpg_yuv_out)(void *priv, void *arg);
    //int (*jpg_wait_data)(void *priv);
    spinlock_t lock;
};


int jpg_parse_JFIF(struct jpeg_dec_fd *hd);
int jpeg_dec_reset_restart(struct jpeg_dec_fd *fd);
void jpeg_dec_get_restart(struct jpeg_dec_fd *fd);
#endif
