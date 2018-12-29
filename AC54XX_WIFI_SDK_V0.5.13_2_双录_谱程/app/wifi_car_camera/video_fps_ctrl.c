

#include "video_fps_ctrl.h"





int fps_ctrl_init(fps_ctrl_t *h,
                  int init_fps,
                  int high_th,
                  int low_th,
                  int min_fps,
                  int max_fps)
{
    h->init_fps = init_fps;
    h->cur_fps = init_fps;
    h->frame_num = 0;
    h->prev_frame_num = 0;
    h->min_fps = min_fps;
    h->max_fps = max_fps;

    h->high_th = high_th;
    h->low_th = low_th;

    h->fps_denom = 256;

    h->next_action = 0;
    return 0;
}

int fps_ctrl_update(fps_ctrl_t *h, int frame_num, int *fps, int *fps_denom)
{
    int new_fps = h->cur_fps;
    h->next_action--;
    if (h->next_action < 0) {
        h->next_action = 0;
    }

    h->frame_num = frame_num;

    //if (h->next_action <= 0)
    if (1) {
        if (h->frame_num > h->high_th) {
            new_fps = h->cur_fps * 9 / 10;
            h->next_action = new_fps;
            if (h->next_action < 5) {
                h->next_action = 5;
            }
        } else if (h->frame_num < h->low_th) {
            if (h->cur_fps < h->fps_denom) {
                new_fps = h->fps_denom;
            } else {
                new_fps = (h->cur_fps * 11 + 5) / 10;

                if (new_fps <= h->cur_fps) {
                    new_fps += 1;
                }
            }
            h->next_action = new_fps;
            if (h->next_action < 5) {
                h->next_action = 5;
            }
        }
    }
    if (new_fps < h->min_fps) {
        new_fps = h->min_fps;
    } else if (new_fps > h->max_fps) {
        new_fps = h->max_fps;
    }

    h->cur_fps = new_fps;

    h->prev_frame_num = h->frame_num;
    *fps = new_fps;
    *fps_denom = h->fps_denom;

    return 0;
}

int fps_ctrl_deinit(fps_ctrl_t *h)
{
    return 0;
}
