#ifndef _GET_YUV_DATA_H_
#define _GET_YUV_DATA_H_

#define YUV_DATA_SOURCE      "video1"
#define YUV_DATA_WIDTH       640
#define YUV_DATA_HEIGHT      480

typedef struct {

    int pid;
    int exit_state;
    void (*cb)(u8 *data) ;
} get_yuv_cfg;

void get_yuv_init(void (*cb)(u8 *data));
void get_yuv_uninit(void);

#endif //_GET_YUV_DATA_H__
