#include "sock_api/sock_api.h"
#include "os/os_compat.h"
#include "server/net_server.h"
/* #include "app_config.h" */
#include "server/server_core.h"
#include "server/ctp_server.h"
#include "server/simple_mov_unpkg.h"
#include "server/rt_stream_pkg.h"
#include "fs/fs.h"
#include "common/common.h"
#include "generic/list.h"
#include "server/packet.h"
#define VIDEO_PLAYBACK_TASK_NAME "video_playback"
#define VIDEO_PLAYBACK_TASK_STK  0x300
#define VIDEO_PLAYBACK_TASK_PRIO 22



#define IMAGE_SIZE 800*1024

struct __playback {
    u32 id;
    u8 state;
    void *video_playback_sock_hdl;
    int (*cb)(void *priv, u8 *data, size_t len);
    u8 kill_flag;
    OS_MUTEX mutex;
    struct list_head cli_head;
};


struct __playback playback_info;
#define playback_info_hander (&playback_info)
static u32 cout = 0;//thread_fork name;
struct __playback_cli_info {
    struct list_head entry;
    u32 msec;
    u32 state;
    struct sockaddr_in remote_addr;
    int pid;
    u8 stop;
    u8 kill_flag;
    u16 fast_ctrl;
    u32 tmp_is_30fps;
    struct __packet_info pinfo;
    OS_SEM   sem;

};
//找I帧第2方式，根据帧数
static int find_idr_frame2(struct __packet_info *pinfo, u32 offset)
{
    int i = 0;
    char buf[5];
    int  sample_offset;
    int sample_size;
    int ret;

    i = offset;

    while (1) {

        if (i >= pinfo->info.video_sample_count) {
            return pinfo->info.video_sample_count - 1;
        }

        sample_size = get_sample_size(pinfo->info.stsz_tab, i);
        if (sample_size == -1) {
            return -1;
        }
        sample_offset = get_chunk_offset(pinfo->info.stco_tab, i);
        if (sample_offset == -1) {
            return -1;
        }
        if (false == fseek(pinfo->fd, sample_offset, SEEK_SET)) {
            return -1;
        }
        ret = fread(pinfo->fd, buf, 5);
        if (ret != 5) {
            return -1;
        }
        if (buf[4] != 0x67) {

            i++;
        } else {
            return i;
        }

    }

}

static int find_idr_frame(struct __packet_info *pinfo, u32 offset)
{
    int i = 0;
    char buf[5];
    int sample_offset;
    int sample_size;
    int ret;

    float  frame_per_ts = 0 ;

    /* printf(" pinfo->info.durition=%d   pinfo->info.scale=%d \n", pinfo->info.durition, pinfo->info.scale); */
    /* frame_per_ts = (pinfo->info.durition * 1000) / (pinfo->info.video_sample_count * pinfo->info.scale); */

    /* i = offset / frame_per_ts; */

    i = offset * pinfo->info.scale / (pinfo->info.sample_duration * 1000);
    while (1) {

        if (i >= pinfo->info.video_sample_count) {
            return pinfo->info.video_sample_count - 1;
        }

        sample_size = get_sample_size(pinfo->info.stsz_tab, i);
        if (sample_size == -1) {
            return -1;
        }

        sample_offset = get_chunk_offset(pinfo->info.stco_tab, i);
        if (sample_offset == -1) {
            return -1;
        }

        if (false == fseek(pinfo->fd, sample_offset, SEEK_SET)) {
            return -1;
        }
        ret = fread(pinfo->fd, buf, 5);
        if (ret != 5) {
            return -1;
        }

        if (buf[4] != 0x67) {
            i--;
        } else {
            return i;
        }

    }

}


















static int send_date_per_fps(struct __playback_cli_info *cli, u32 i)
{
    int ret = 0;

    u32  is_30fps = 0;
    u32 sec = 0;

    sec = (i - cli->tmp_is_30fps) / (cli->pinfo.info.scale / cli->pinfo.info.sample_duration);

    if (sec) {
        printf("sec->%d\n", sec);

        ret = send_date_packet(&cli->pinfo, sec);
        send_gps_data_packet(&cli->pinfo);

        cli->tmp_is_30fps = i;
    }

    return ret;
}
static void video_playback_cli_thread(void *arg)
{
    int ret;
    struct vfscan *fs;
    FILE *fd;
    u8 name[32];
    char buf[64];
    int i = 0, j = 0;
    u32 msec = 0;
    struct __playback_cli_info *cli = (struct __playback_cli_info *)arg;

    puts("start  video_playback_cli_thread\n");
    cli->pinfo.fd = fopen(cli->pinfo.file_name, "r");
    cli->fast_ctrl = 0;

    if (cli->pinfo.fd == NULL) {
        puts("open fail\n");
        fget_name(cli->pinfo.fd, name, 32);
        sprintf(buf, "path:%s", name);
        CTP_CMD_COMBINED(NULL, CTP_OPEN_FILE, "TIME_AXIS_PLAY", "NOTIFY", buf);
        goto err1;
    }

    if (!is_vaild_mov_file(cli->pinfo.fd)) {
        puts("open fail\n");
        fget_name(cli->pinfo.fd, name, 32);
        sprintf(buf, "path:%s", name);
        CTP_CMD_COMBINED(NULL, CTP_OPEN_FILE, "TIME_AXIS_PLAY", "NOTIFY", buf);
        goto err1;
    }
    if (find_gps_data(&cli->pinfo) < 0) {
        puts("mov file no gps data\n");
    }

    cli->pinfo.type = H264_TYPE_VIDEO;
    //get mov media info
    if (get_mov_media_info(&cli->pinfo)) {
        puts("get mov media fail\n");
        goto err1;
    }
    i = find_idr_frame(&cli->pinfo, cli->msec);
    printf("sample start at %d sample_duration=%d\n", i, cli->pinfo.info.scale);



    if (send_media_packet(&cli->pinfo) <= 0) {
        printf("send media packet fail \n\n");
        CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "TIME_AXIS_PLAY", "NOTIFY", CTP_NET_ERR_MSG);
        goto err1;

    }

    /* send_date_packet(&cli->pinfo, 0); */


    incr_date_time(&cli->pinfo.time, cli->msec / 1000);

    cli->tmp_is_30fps = i;
    j = i / (cli->pinfo.info.video_sample_count / cli->pinfo.info.audio_chunk_num);

    while (1) {
        if (cli->stop) {
            os_sem_pend(&cli->sem, 0);
        }

        if (cli->kill_flag) {
            goto err1;
        }

        if (i >= cli->pinfo.info.video_sample_count - 1) {
            cli->pinfo.fd = play_next(&cli->pinfo);

            send_end_packet(&cli->pinfo);
            if (cli->pinfo.fd == NULL) {
                goto err1;
            }

            update_data(&cli->pinfo);
            i = 0;
            cli->tmp_is_30fps = 0;
            j = 0;
            if (find_gps_data(&cli->pinfo) < 0) {
                printf("mov gps data err \n");
            }
            send_media_packet(&cli->pinfo);
        }

        if (cli->pinfo.info.video_sample_count && cli->pinfo.info.audio_chunk_num) {
            if ((i % (cli->pinfo.info.video_sample_count / cli->pinfo.info.audio_chunk_num) == 0)
                && j < cli->pinfo.info.audio_chunk_num) {
                if (!cli->fast_ctrl) {
                    ret = send_audio_packet(&cli->pinfo, j);
                    if (ret <= 0) {
                        goto err1;
                    }
                }

                j++;
            }
        }

        ret = send_video_packet(&cli->pinfo, i);

        if (ret <= 0) {
            printf("send video packet fail\n");
            CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "TIME_AXIS_PLAY", "NOTIFY", CTP_NET_ERR_MSG);
            goto err1;
        }

        /* printf("cli->fast_ctrl=%d    cli->pinfo.fast_play_mask=%d\n", cli->fast_ctrl, cli->pinfo.fast_play_mask); */
        if (!cli->fast_ctrl) {
            /* cli->pinfo.fast_play_mask = 0;//快播标记位 */
            i++;
        } else {
            /* i += (cli->fast_ctrl * 30);  //fps = 30; */

            i += cli->fast_ctrl;
            /* cli->pinfo.fast_play_mask = FAST_PLAY_MAKER;//快播标记位 */

//           cli->pinfo.fast_play_mask = 0;//快播标记位
            /* printf("video playback speed1 %d index:%d\n", cli->fast_ctrl, i); */
            i = find_idr_frame2(&cli->pinfo, i);
            /* printf("video playback speed2 %d index:%d\n", cli->fast_ctrl, i); */
        }

        send_date_per_fps(cli, i);
    }

err1:
    unfind_gps_data();
    if (cli->pinfo.info.stco_tab != NULL) {
        free(cli->pinfo.info.stco_tab);
        cli->pinfo.info.stco_tab = NULL;
    }

    if (cli->pinfo.info.stsz_tab != NULL) {
        free(cli->pinfo.info.stsz_tab);
        cli->pinfo.info.stsz_tab = NULL;
    }

    if (cli->pinfo.info.audio_stco_tab != NULL) {
        free(cli->pinfo.info.audio_stco_tab);
        cli->pinfo.info.audio_stco_tab = NULL;
    }

    if (cli->pinfo.fd != NULL) {
        fclose(cli->pinfo.fd);
        cli->pinfo.fd = NULL;
    }

    os_mutex_pend(&playback_info_hander->mutex, 0);
    list_del(&cli->entry);
    /* puts("third\n"); */
    sock_unreg(cli->pinfo.sock);
    /* puts("four\n"); */
    free(cli->pinfo.data);

    free(cli);
    os_mutex_post(&playback_info_hander->mutex);

    /* puts("end  video_playback_cli_thread\n"); */
}




#if 0
int video_playback_req_handler(void *msg)
{
    char buf[128];
    struct sockaddr_in remote_addr;
    void *ret;
    char name[32];
    int ret1 = 0;
    if (msg == NULL) {
        return -1;
    }

    struct net_req *req = (struct net_req *)msg;

    socklen_t addrlen = sizeof(remote_addr);
    sprintf(buf, "path:%s,offset:%s", req->playback.file_name, req->playback.msec);
    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "TIME_AXIS_PLAY", "NOTIFY", buf);

    ret = sock_accept(playback_info_hander->video_playback_sock_hdl, (struct sockaddr *)&remote_addr, &addrlen, NULL, NULL);

    if (ret == NULL) {
        printf("%s ::%d sock_accept \n", __func__, __LINE__);
        goto err1;
    }


    //malloc cli info res
    struct __playback_cli_info *cli = (struct __playback_cli_info *)calloc(1, sizeof(struct __playback_cli_info));

    if (cli == NULL) {
        printf("%s ::%d calloc \n", __func__, __LINE__);
        goto err1;
    }

    printf("cli->pinfo.sock : 0x%x \n", ret);
    //get req
    strcpy(cli->pinfo.file_name, req->playback.file_name);
    cli->msec = req->playback.msec;
    cli->pinfo.sock = ret;
    //cpu_write_range_limit(&cli->pinfo.sock,1);
    printf("cli->pinfo.sock : %d \n", cli->pinfo.sock);
    printf("req->playback.filename : %s sec:%d\n", cli->pinfo.file_name, cli->msec);
    //free req

    free(req);
    //malloc data's bufffer
    cli->pinfo.data = (u8 *)calloc(IMAGE_SIZE, 1);

    if (cli->pinfo.data == NULL) {
        printf("%s ::%d malloc \n", __func__, __LINE__);
        free(cli);
        goto err1;
    }

    cli->pinfo.len = IMAGE_SIZE;

    memcpy(&cli->remote_addr, &remote_addr, sizeof(struct sockaddr_in));
    os_sem_create(&cli->sem, 0);
    os_mutex_create(&playback_info_hander->mutex);
    os_mutex_pend(&playback_info_hander->mutex, 0);
    list_add_tail(&cli->entry, &playback_info_hander->cli_head);
    os_mutex_post(&playback_info_hander->mutex);

    printf("create cli thread\n");
    sprintf(name, "playback_cli_%x", remote_addr.sin_addr.s_addr);
    ret1 = thread_fork(name, 21, 0x600, 0, (void *)0, video_playback_cli_thread, (void *)cli);
    if (ret1 != OS_NO_ERR) {
        printf("thread fork err =%d\n", ret1);
        goto err1;
    }
    return 0;
err1:
    sock_unreg(playback_info_hander->video_playback_sock_hdl);
    return -1;
}
#endif
int video_playback_req_handler2(void *msg)
{
    char buf[128];
    struct sockaddr_in remote_addr;
    void *ret;
    fd_set rdset;
    struct timeval tv = {5, 0};

    char name[32];
    int ret1 = 0;
    if (msg == NULL) {
        return -1;
    }

    struct net_req *req = (struct net_req *)msg;

    socklen_t addrlen = sizeof(remote_addr);
    /* sprintf(buf, "path:%s,offset:%d", req->playback.file_name, req->playback.msec); */

    sprintf(buf, "status:%d", 0);
    CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "TIME_AXIS_PLAY", "NOTIFY", buf);
    while (1) {
        FD_ZERO(&rdset);
        FD_SET(sock_get_socket(playback_info_hander->video_playback_sock_hdl), &rdset);

        ret1 = sock_select(playback_info_hander->video_playback_sock_hdl, &rdset, NULL, NULL, &tv);
        if (ret1 < 0) {
            goto err1;
        } else if (ret1 == 0) {
            puts("accept time out\n");
            CTP_CMD_COMBINED(NULL, CTP_REQUEST, "TIME_AXIS_PLAY", "NOTIFY", CTP_REQUEST_MSG);
            free(req);
            return -1;
        } else {
            ret = sock_accept(playback_info_hander->video_playback_sock_hdl, (struct sockaddr *)&remote_addr, &addrlen, NULL, NULL);

            if (ret == NULL) {
                printf("%s ::%d sock_accept \n", __func__, __LINE__);
                goto err1;
            }


            //malloc cli info res
            struct __playback_cli_info *cli = (struct __playback_cli_info *)calloc(1, sizeof(struct __playback_cli_info));

            if (cli == NULL) {
                printf("%s ::%d calloc \n", __func__, __LINE__);
                goto err1;
            }

            //get req
            strcpy(cli->pinfo.file_name, req->playback.file_name);
            cli->msec = req->playback.msec;
            cli->pinfo.sock = ret;

            printf("old req->playback.filename : %s sec:%d\n", req->playback.file_name, req->playback.msec);
            printf("new req->playback.filename : %s sec:%d\n", cli->pinfo.file_name, cli->msec);

            //free req

            free(req);
            //malloc data's bufffer
            cli->pinfo.data = (u8 *)calloc(IMAGE_SIZE, 1);

            if (cli->pinfo.data == NULL) {
                printf("%s ::%d malloc \n", __func__, __LINE__);
                free(cli);
                goto err1;
            }

            cli->pinfo.len = IMAGE_SIZE;

            memcpy(&cli->remote_addr, &remote_addr, sizeof(struct sockaddr_in));
            os_sem_create(&cli->sem, 0);
            os_mutex_pend(&playback_info_hander->mutex, 0);
            list_add_tail(&cli->entry, &playback_info_hander->cli_head);
            os_mutex_post(&playback_info_hander->mutex);

            printf("create cli thread\n");
            sprintf(name, "playback_cli_%d", cout++);
            ret1 = thread_fork(name, 21, 0x600, 0, &cli->pid, video_playback_cli_thread, (void *)cli);
            if (ret1 != OS_NO_ERR) {
                printf("thread fork err =%d\n", ret1);
                goto err1;
            }
            return 0;

        }


    }
err1:
    sock_unreg(playback_info_hander->video_playback_sock_hdl);
    return -1;
}
static void time_axis_play_thread(void *arg)
{
    video_playback_req_handler2(arg);

    printf("end video playback\n\n\n\n");
}

int video_playback_post_msg(struct net_req *_req)
{
    int ret = 0;
    static u32 count = 0;
    char buf[64];
    struct net_req *req = (struct net_req *)calloc(1, sizeof(struct net_req));

    if (req == NULL) {
        return -1;
    }

    memcpy(req, _req, sizeof(struct net_req));

    sprintf(buf, "time_axis_play_thread%d", count++);
    ret = thread_fork(buf, 25, 0x1000, 0, 0, time_axis_play_thread, (void *)req);
    if (ret != OS_NO_ERR) {
        return -1;
    }
    return 0;


}

#if 0
static void video_playback_thread(void *arg)
{
    int res;
    u32 msg[32];

    while (1) {
        puts("wait msg\n");
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));

        if (playback_info_hander->kill_flag) {
            break;
        }

        puts("get msg gogogo\n");
        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_EVENT:
                break;

            case Q_MSG:
                video_playback_req_handler((void *) msg[1]);
                break;

            default:
                break;
            }

            break;

        case OS_TIMER:
            break;

        case OS_TIMEOUT:
            break;
        }

    }

}
#endif
int playback_init(u16 port, int callback(void *priv, u8 *data, size_t len))
{
    puts("playback_init\n");
    int ret;
    struct sockaddr_in dest_addr;
    playback_info_hander->video_playback_sock_hdl = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);

    if (playback_info_hander->video_playback_sock_hdl == NULL) {
        printf("%s %d->Error in socket()\n", __func__, __LINE__);
        goto EXIT;
    }

    if (sock_set_reuseaddr(playback_info_hander->video_playback_sock_hdl)) {
        printf("%s %d->Error in sock_set_reuseaddr(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = htons(port);
    puts("playback_init\n");
    ret = sock_bind(playback_info_hander->video_playback_sock_hdl, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

    if (ret) {
        printf("%s %d->Error in bind(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }

    ret = sock_listen(playback_info_hander->video_playback_sock_hdl, 0xff);

    if (ret) {
        printf("%s %d->Error in listen()\n", __func__, __LINE__);
        goto EXIT;
    }

    puts("playback_init\n");

    if (callback == NULL) {
        playback_info_hander->cb = NULL;
    } else {
        playback_info_hander->cb = callback;
    }

    os_mutex_create(&playback_info_hander->mutex);
    INIT_LIST_HEAD(&playback_info_hander->cli_head);
    puts("create video_playback_thread\n");
    /* thread_fork(VIDEO_PLAYBACK_TASK_NAME, VIDEO_PLAYBACK_TASK_PRIO, VIDEO_PLAYBACK_TASK_STK, 40, &playback_info_hander->id, video_playback_thread, NULL); */


    return 0;
EXIT:
    sock_unreg(playback_info_hander->video_playback_sock_hdl);
    playback_info_hander->video_playback_sock_hdl = NULL;
    return -1;
}

static int playback_disconnect_all_cli(void)
{
    /* puts("---------------playback_disconnect_all_cli\n\n"); */
    struct __playback_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&playback_info_hander->mutex, 0);
    if (list_empty(&playback_info_hander->cli_head)) {
        puts("video_playback cli is emtry\n");
        os_mutex_post(&playback_info_hander->mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &playback_info_hander->cli_head) {
        cli = list_entry(pos, struct __playback_cli_info, entry);
        cli->kill_flag = 1;
        if (cli->stop) {
            os_sem_post(&cli->sem);
        }

        sock_set_quit(cli->pinfo.sock);
    }

    os_mutex_post(&playback_info_hander->mutex);
    return -1;
}

void playback_uninit(void)
{
    playback_disconnect_all_cli();
    sock_unreg(playback_info_hander->video_playback_sock_hdl);
    playback_info_hander->video_playback_sock_hdl = NULL;
    /* playback_info_hander->kill_flag = 1; */
    /* thread_kill(&playback_info_hander->id, KILL_WAIT); */
    /* os_taskq_post_msg(VIDEO_PLAYBACK_TASK_NAME, 1, 0); */
    /* playback_info_hander->kill_flag = 0; */
}


int playback_disconnect_cli(struct sockaddr_in *dst_addr)
{
    /* puts("---------------playback_disconnect_cli\n\n"); */
    struct __playback_cli_info *cli = NULL;
    os_mutex_pend(&playback_info_hander->mutex, 0);
    struct list_head *pos = NULL, *node = NULL;
    if (list_empty(&playback_info_hander->cli_head)) {
        puts("video_playback cli is emtry\n");
        os_mutex_post(&playback_info_hander->mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &playback_info_hander->cli_head) {
        cli = list_entry(pos, struct __playback_cli_info, entry);

        /* printf("remote_addr:%s:%d\n", inet_ntoa(cli->remote_addr.sin_addr.s_addr), ntohs(cli->remote_addr.sin_port)); */
        if (dst_addr != NULL
            && cli->remote_addr.sin_addr.s_addr == dst_addr->sin_addr.s_addr) {
            cli->kill_flag = 1;
            if (cli->stop) {
                os_sem_post(&cli->sem);
            }

            sock_set_quit(cli->pinfo.sock);
        } else {
            printf("not find dst_addr:%s:%d\n", inet_ntoa(dst_addr->sin_addr.s_addr), ntohs(dst_addr->sin_port));
        }

        os_mutex_post(&playback_info_hander->mutex);
        return 0;
        //break;
    }

    os_mutex_post(&playback_info_hander->mutex);
    return -1;
}

int playback_cli_pause(struct sockaddr_in *dst_addr)
{
    puts("-----------playback_cli_pause\n\n");
    struct __playback_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;

    os_mutex_pend(&playback_info_hander->mutex, 0);
    list_for_each_safe(pos, node, &playback_info_hander->cli_head) {
        cli = list_entry(pos, struct __playback_cli_info, entry);

        if (cli->remote_addr.sin_addr.s_addr == dst_addr->sin_addr.s_addr) {
            cli->stop = 1;
            os_mutex_post(&playback_info_hander->mutex);
            return 0;
            //		break;
        }
    }
    os_mutex_post(&playback_info_hander->mutex);
    return -1;
}

int playback_cli_continue(struct sockaddr_in *dst_addr)
{
    puts("---------------playback_cli_continue\n\n");
    struct __playback_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&playback_info_hander->mutex, 0);
    list_for_each_safe(pos, node, &playback_info_hander->cli_head) {
        cli = list_entry(pos, struct __playback_cli_info, entry);

        if (cli->remote_addr.sin_addr.s_addr == dst_addr->sin_addr.s_addr) {
            cli->stop = 0;

            os_sem_post(&cli->sem);
            os_mutex_post(&playback_info_hander->mutex);
            return 0;
            //break;
        }
    }
    os_mutex_post(&playback_info_hander->mutex);
    return -1;

}
//快放
int playback_cli_fast_play(struct sockaddr_in *dst_addr, u32 speed)
{
    puts("---------------playback_cli_fast_play\n\n");
    struct __playback_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&playback_info_hander->mutex, 0);
    list_for_each_safe(pos, node, &playback_info_hander->cli_head) {
        cli = list_entry(pos, struct __playback_cli_info, entry);

        if (cli->remote_addr.sin_addr.s_addr == dst_addr->sin_addr.s_addr) {
            cli->fast_ctrl = speed;
            os_mutex_post(&playback_info_hander->mutex);
            return 0;
            //break;
        }
    }
    os_mutex_post(&playback_info_hander->mutex);
    return -1;

}
