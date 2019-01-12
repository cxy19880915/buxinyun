#ifndef __DDR_FILE_H__
#define __DDR_FILE_H__

#include "typedef.h"

enum {
    YUV444,//0
    YUV422,//1
    YUV420,//2
    YUV411,//3
    RGB565,//4
    RGB888,//5
    OSD16,//6
    OSD8,//7
    OSD2,//8
    OSD1,//9
    INVALID_FORMAT,
};


struct filehead {
    u32 version;
    u32 total_file;
    u32 total_size;
    u8  reserved[4];
};

struct fileinfo {
    u8 fname[30];
    u8 format;
    u8 alpha;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u32 addr;
    u32 len;
};

struct filehead *ddr_get_filehead();
u8 ddr_fopen(u8 *filename, struct fileinfo **inf);
u8 get_format(u8 fmt);
u8 *ddr_get_baddr();

#define DATA_START_ADDR 0x5C00000
//#define DATA_START_ADDR 0x5100000

#endif // __SDFILE_H__
