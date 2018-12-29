

#include "video_bitrate_ctrl.h"


int bitrate_ctrl_init(bitrate_ctrl_t *h,
                      int init_bitrate,
                      int high_th,
                      int low_th,
                      int min_bitrate,
                      int max_bitrate)
{
    h->init_bitrate = init_bitrate;
    h->cur_bitrate = init_bitrate;
    h->frame_num = 0;
    h->prev_frame_num = 0;
    h->min_bitrate = min_bitrate;
    h->max_bitrate = max_bitrate;

    h->high_th = high_th;
    h->low_th = low_th;

    return 0;
}

int bitrate_ctrl_update(bitrate_ctrl_t *h, int frame_num, int *bitrate)
{
    int new_bitrate = h->cur_bitrate;

    h->frame_num = frame_num;

    if (h->frame_num > h->high_th) {
        new_bitrate = h->cur_bitrate * 9 / 10;
    } else if (h->frame_num < h->low_th) {
        new_bitrate = (h->cur_bitrate * 11 + 5) / 10;
    }

    if (new_bitrate < h->min_bitrate) {
        new_bitrate = h->min_bitrate;
    } else if (new_bitrate > h->max_bitrate) {
        new_bitrate = h->max_bitrate;
    }

    h->cur_bitrate = new_bitrate;

    h->prev_frame_num = h->frame_num;
    *bitrate = new_bitrate;
    return 0;
}

int bitrate_ctrl_deinit(bitrate_ctrl_t *h)
{
    return 0;
}

