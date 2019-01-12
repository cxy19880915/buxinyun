#ifndef _JPEG_IOCMDS_H__
#define _JPEG_IOCMDS_H__




#define JPEG_ENC_CMD_BASE                0x00300000
#define JPEG_ENC_SET_Q_VAL               (JPEG_ENC_CMD_BASE + 1)
#define JPEG_ENC_SET_ABR                 (JPEG_ENC_CMD_BASE + 2)
#define SET_IMAGE_AUXILIARY_BUF          (JPEG_ENC_CMD_BASE + 3)
#define JPEG_ENC_SET_CYC_TIME            (JPEG_ENC_CMD_BASE + 4)
#define JPEG_ENC_SET_Q_TABLE             (JPEG_ENC_CMD_BASE + 5)

struct jpeg_yuv {
    u8 *y;
    u8 *u;
    u8 *v;
};

struct icap_auxiliary_mem {
    u8 *addr;
    u32 size;
};

struct jpg_q_table {
    u16  YQT_DCT[0x40] ;
    u16  UVQT_DCT[0x40];
    u8   DQT[138]; //file header
};


#endif
