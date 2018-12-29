#ifndef __FPS_CTRL_H__
#define __FPS_CTRL_H__

typedef struct _fps_ctrl {
    int init_fps;
    int cur_fps;
    int frame_num;
    int prev_frame_num;
    int min_fps;
    int max_fps;
    int high_th;
    int low_th;
    int fps_denom;
    int next_action;
} fps_ctrl_t;

int fps_ctrl_init(fps_ctrl_t *h,
                  int init_fps,
                  int high_th,
                  int low_th,
                  int min_fps,
                  int max_fps);

int fps_ctrl_update(fps_ctrl_t *h, int frame_num, int *fps, int *fps_denom);
int fps_ctrl_deinit(fps_ctrl_t *h);


#endif

