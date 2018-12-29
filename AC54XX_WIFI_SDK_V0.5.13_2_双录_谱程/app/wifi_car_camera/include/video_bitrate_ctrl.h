#ifndef __BITRATE_CTRL_H__
#define __BITRATE_CTRL_H__

typedef struct _bitrate_ctrl {
    int init_bitrate;
    int cur_bitrate;
    int frame_num;
    int prev_frame_num;
    int min_bitrate;
    int max_bitrate;
    int high_th;
    int low_th;
} bitrate_ctrl_t;

int bitrate_ctrl_init(bitrate_ctrl_t *h,
                      int init_bitrate,
                      int high_th,
                      int low_th,
                      int min_bitrate,
                      int max_bitrate);

int bitrate_ctrl_update(bitrate_ctrl_t *h, int frame_num, int *bitrate);
int bitrate_ctrl_deinit(bitrate_ctrl_t *h);

#endif

