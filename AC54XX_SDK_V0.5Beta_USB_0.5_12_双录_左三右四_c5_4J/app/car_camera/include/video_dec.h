#ifndef __VIDEO_DEC_H_
#define __VIDEO_DEC_H_


#include "system/includes.h"
#include "server/video_server.h"
#include "app_config.h"
#include "storage_device.h"



#define DEC_PATH_NUM  2

enum {
    VIDEO_DEC_STOP,
    VIDEO_DEC_PLAYING,
    VIDEO_DEC_PAUSE,
    VIDEO_DEC_FF,
    VIDEO_DEC_FR,
};

#define FILE_TYPE_MOVE      0
#define FILE_TYPE_JPG       1





struct video_dec_hdl {
    u8 status;
    u8 curr_dir;
    u8 file_type;
    u8 ff_fr_times;
    u8 fname[MAX_FILE_NAME_LEN];
    u8 dir_type;
    int wait;
    int timer;
    int timeout;
    struct server *ui;
    struct vfscan *fs[DEC_PATH_NUM];
    struct server *video_dec;
    union video_dec_req req;
};

#ifdef CONFIG_TOUCH_UI_ENABLE
enum FILE_TYPE {
    VIDEO_FILE = 0,
    PHOTO_FILE,
    LOCK_FILE,
};
int dec_open_file();
#endif
extern const char *dec_path[DEC_PATH_NUM][2];

void video_dec_post_msg(const char *msg, ...);

void video_dec_post_file_info_to_ui();

int video_dec_file(int mode);

int video_dec_play_pause();

void video_dec_scan_dir();

#endif
