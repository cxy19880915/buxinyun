#ifndef  __PACKET_H__
#define  __PACKET_H__

#include "sock_api/sock_api.h"
#include "os/os_compat.h"
#include "server/net_server.h"
#include "app_config.h"
#include "server/server_core.h"
#include "server/ctp_server.h"
#include "simple_mov_unpkg.h"
#include "rt_stream_pkg.h"
#include "fs/fs.h"
#include "common/common.h"

#include "time.h"
#include "sys/time.h"

struct __packet_info {
    u32 len;
    void *sock;
    u8 *data;
    FILE *fd;
    char file_name[64];
    struct tm time;
    struct vfscan *fs;
    u8 type;
    u8 fast_play_mask;
    struct __mov_unpkg_info info;
};



int get_mov_media_info(struct __packet_info *pinfo);
FILE *play_next(struct __packet_info *pinfo);
FILE *play_prev(struct __packet_info *pinfo);
int send_video_packet(struct __packet_info  *pinfo, u32 i);
int send_audio_packet(struct __packet_info *pinfo, u32 j);
int send_media_packet(struct __packet_info *pinfo);
int send_date_packet(struct __packet_info *pinfo, u32 msec);
int send_end_packet(struct __packet_info *pinfo);
int update_data(struct __packet_info *pinfo);
void incr_date_time(struct tm *tm_time, int incr);
int send_gps_data_packet(struct __packet_info *pinfo);
int find_gps_data(struct __packet_info *pinfo);
int unfind_gps_data(void);

#endif  /*PACKET_H*/

