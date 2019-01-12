#ifndef __JPG_CODEC_H__
#define __JPG_CODEC_H__




#define JPEG_ENC_CMD_BASE                0x00300000
#define JPEG_ENC_SET_Q_VAL               (JPEG_ENC_CMD_BASE + 1)
#define JPEG_ENC_SET_ABR                 (JPEG_ENC_CMD_BASE + 2)
#define SET_IMAGE_ZOOM_BUF               (JPEG_ENC_CMD_BASE + 3)

struct img_zoom_mem {
    u8 *addr;
    u32 size;
};



#endif
