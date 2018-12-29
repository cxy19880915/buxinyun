#include "sock_api/sock_api.h"
#include "os/os_compat.h"
#include "server/net_server.h"
/* #include "app_config.h" */
#include "server/server_core.h"
#include "server/ctp_server.h"
#include "server/rt_stream_pkg.h"
#include "server/simple_mov_unpkg.h"
#include "server/packet.h"
#include "generic/list.h"

#define VIDEO_PREVIEW_TASK_NAME "video_preview"
#define VIDEO_PREVIEW_TASK_STK  0x1000
#define VIDEO_PREVIEW_TASK_PRIO 22



#define IMAGE_SIZE 800*1024

struct __preview {
    struct list_head cli_head;
    u32 id;
    u8 state;
    void *video_preview_sock_hdl;
    int (*cb)(void *priv, u8 *data, size_t len);
    u8 kill_flag;

    OS_MUTEX mutex;

};


struct __preview preview_info;
#define preview_info_hander (&preview_info)

struct __preview_cli_info {
    struct list_head entry;
    char (*filename)[64];
    u32 num;
    int offset;
    u16 timeinv;
    u8 direct;
    struct sockaddr_in remote_addr;
    struct __packet_info pinfo;
    u8 kill_flag;
    int pid;
    u32 tmp_is_30fps;

};


static void video_preview_cli_thread(void *arg)
{
    int ret;
    FILE *fd;
    char name[32];
    char buf[256];
    int i = 0;
    int count = 0;
    u32 msec = 0;
    struct __preview_cli_info *cli = (struct __preview_cli_info *)arg;

    /* puts("\n\n\n\n\n\nstart  video_preview_cli_thread\n\n\n\n\n\n\n"); */
    cli->pinfo.type = PREVIEW_TYPE;
    while (1) {
        if (!strstr(cli->filename[i], "storage")) {
            printf("cli->filename[%d]=%s\n", i, cli->filename[i]);
            sprintf(buf, "status:%d", 1);
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "MULTI_COVER_FIGURE", "NOTIFY", buf);
            break;
        }

        if (cli->kill_flag) {
            goto err2;
        }

        /* printf("%s : filename[%d]:%s  pid:0x%x\n", __func__, i, cli->filename[i], &cli->pid); */

        fd = fopen(cli->filename[i], "r");

        if (fd == NULL) {
            printf("%s   fopen fail\n", __func__);
            /* sprintf(buf, "path_%d:%s", i, cli->filename[i]); */
            CTP_CMD_COMBINED(NULL, CTP_OPEN_FILE, "MULTI_COVER_FIGURE", "NOTIFY", CTP_OPEN_FILE_MSG);
            i++;
            continue;
            //goto err2;

        }
        if (!is_vaild_mov_file(fd)) {
            /* sprintf(buf, "path_%d:%s", i, cli->filename[i]); */
            printf("%s   is Invalid MOV\n", __func__);
            CTP_CMD_COMBINED(NULL, CTP_OPEN_FILE, "MULTI_COVER_FIGURE", "NOTIFY", CTP_OPEN_FILE_MSG);
            i++;
            fclose(fd);
            continue;
        }

        cli->pinfo.fd = fd;
        strcpy(cli->pinfo.file_name, cli->filename[i]);
        if (update_data(&cli->pinfo)) {
            printf("get media info fail\n");
            goto err2;
        }
        ret = send_media_packet(&cli->pinfo);

        if (ret <= 0) {
            /* sprintf(buf, "path_%d:%s", i, cli->filename[i]); */
            CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "MULTI_COVER_FIGURE", "NOTIFY", CTP_NET_ERR_MSG);
            goto err2;
        }

        ret = send_date_packet(&cli->pinfo, 0);
        if (ret <= 0) {
            /* sprintf(buf, "path:%s", cli->filename[i]); */
            CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "MULTI_COVER_FIGURE", "NOTIFY", CTP_NET_ERR_MSG);
            goto err2;
        }


        ret = send_video_packet(&cli->pinfo, 0);

        if (ret <= 0) {
            /* sprintf(buf, "path_%d:%s", i, cli->filename[i]); */
            CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "MULTI_COVER_FIGURE", "NOTIFY", CTP_NET_ERR_MSG);
            goto err2;
        }
        msec = i * cli->pinfo.info.sample_duration * 1000 / cli->pinfo.info.scale;

        /* printf("real offset sec at %d\n", msec); */
        if (msec) {
            incr_date_time(&cli->pinfo.time, msec / 1000);

        }

        fclose(fd);
        i++;
    }

err2:


    /* printf("cli ip addr->%s  port:%d\n\n", inet_ntoa(cli->remote_addr.sin_addr.s_addr), ntohs(cli->remote_addr.sin_port)); */
    if (cli->filename != NULL) {
        /* puts("free cli->filename free =---------------\n\n\n\n"); */
        free(cli->filename);
    }
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


    os_mutex_pend(&preview_info_hander->mutex, 0);
    list_del(&cli->entry);
    os_mutex_post(&preview_info_hander->mutex);
    /* puts("third\n"); */
    sock_unreg(cli->pinfo.sock);
    /* puts("four\n"); */
    free(cli->pinfo.data);
    free(cli);

    /* video_preview_and_thus_disconnect(cli); */
    /* puts("end  video_preview_cli_thread\n"); */
}

//return sample index  offset : ms
//后期需要换找I帧的方法
static int find_idr_frame(struct __packet_info *pinfo, u32 offset)
{
    int i = 0;
    char buf[5];
    size_t sample_offset;
    size_t sample_size;
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
        sample_offset = get_chunk_offset(pinfo->info.stco_tab, i);
        fseek(pinfo->fd, sample_offset, SEEK_SET);
        ret = fread(pinfo->fd, buf, 5);

        if (buf[4] != 0x67) {
            i++;
        } else {
            return i;
        }

    }

}


static int send_date_per_fps(struct __preview_cli_info *cli, u32 i)
{
    int ret = 0;

    int is_30fps = 0;


    u32 sec = (i - cli->tmp_is_30fps) / (cli->pinfo.info.scale / cli->pinfo.info.sample_duration);

    if (sec) {

        ret = send_date_packet(&cli->pinfo, sec);

        cli->tmp_is_30fps = i;
    }

    return ret;

}
//成功：返回起始帧或结束帧 失败；-1
static int switch_direct_and_file(struct __preview_cli_info *cli)
{
    int i = 0;

    if (cli->direct) {
        //behind
        if (cli->offset <= 0) {
            /* puts("playback end \n"); */
            send_end_packet(&cli->pinfo);
            cli->pinfo.fd = play_prev(&cli->pinfo);

            if (cli->pinfo.fd == NULL) {
                return -1;
            }

            update_data(&cli->pinfo);
            /* printf("--------------durition:%d\n", cli->pinfo.info.durition); */
            cli->offset = ((cli->pinfo.info.durition / cli->pinfo.info.scale) * 1000);
            i = cli->pinfo.info.video_sample_count - 1;
            //             send_media_packet(&cli->pinfo);
        }

    } else {
        //forward
        if (cli->offset >= ((cli->pinfo.info.durition / cli->pinfo.info.scale) * 1000)) {
            /* puts("playback end \n"); */
            send_end_packet(&cli->pinfo);
            cli->pinfo.fd = play_next(&cli->pinfo);

            if (cli->pinfo.fd == NULL) {
                return -1;
            }

            update_data(&cli->pinfo);
            i = 0;
            cli->offset = 0;
//               send_media_packet(&cli->pinfo);
        }

    }

    if (cli->direct) {
        cli->offset -= cli->timeinv;
    } else {
        cli->offset += cli->timeinv;
    }

    return i;

}

static void video_thus_cli_thread(void *arg)
{
    int ret;
    FILE *fd;
    char name[32];
    char buf[64];
    int i = 0;
    int count = 0;
    u32 msec = 0;
    struct __preview_cli_info *cli = (struct __preview_cli_info *)arg;

    /* puts("start  video_preview_cli_thread\n"); */
    cli->pinfo.type = PREVIEW_TYPE;
    /* printf("%s : filename:%s  num:%d  offset:%d timeinv:%d\n", __func__, cli->filename[i], cli->num, cli->offset, cli->timeinv); */
    fd = fopen(cli->filename[i], "r");

    if (fd == NULL) {
        /* sprintf(buf, "path:%s", cli->filename[0]); */
        CTP_CMD_COMBINED(NULL, CTP_OPEN_FILE, "THUMBNAILS", "NOTIFY", CTP_OPEN_FILE_MSG);
        goto err2;
    }

    if (!is_vaild_mov_file(fd)) {
        /* sprintf(buf, "path_%d:%s", i, cli->filename[i]); */
        CTP_CMD_COMBINED(NULL, CTP_OPEN_FILE, "THUMBNAILS", "NOTIFY", CTP_OPEN_FILE_MSG);
        fclose(fd);
        goto err2;
    }


    cli->pinfo.fd = fd;
    strcpy(cli->pinfo.file_name, cli->filename[0]);

    if (update_data(&cli->pinfo)) {
        printf("get media info fail\n");
        goto err2;
    }

    ret = send_media_packet(&cli->pinfo);

    if (ret <= 0) {
        /* sprintf(buf, "path:%s", cli->filename[i]); */
        CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "THUMBNAILS", "NOTIFY", CTP_NET_ERR_MSG);
        goto err2;
    }
    incr_date_time(&cli->pinfo.time, cli->offset / 1000);

    while (1) {
        if (cli->kill_flag) {
            goto err2;
        }

        i = find_idr_frame(&cli->pinfo, cli->offset);


#if 0 //暂时不用，当时间偏移量大于当前，切换到下视频
        i = switch_direct_and_file(cli);

        if (i < 0) {
            goto err2;
        }
#endif
        if ((count + 1) == cli->num) {
            /* printf("send end packet\n"); */
            send_end_packet(&cli->pinfo);
        }



        ret = send_date_packet(&cli->pinfo, cli->timeinv / 1000);
        if (ret <= 0) {
            CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "THUMBNAILS", "NOTIFY", CTP_NET_ERR_MSG);
            goto err2;
        }


        ret = send_video_packet(&cli->pinfo, i);

        if (ret <= 0) {
            /* sprintf(buf, "path:%s", cli->filename[i]); */
            CTP_CMD_COMBINED(NULL, CTP_NET_ERR, "THUMBNAILS", "NOTIFY", CTP_NET_ERR_MSG);
            goto err2;
        }

        count++;
        cli->offset += cli->timeinv;

        if (cli->num != 0 && count == cli->num) {
            /* puts("!!!!!!!count!!!!!!!!\n"); */

            sprintf(buf, "status:%d", 1);
            CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "THUMBNAILS", "NOTIFY", buf);

            break;
        }
    }

err2:
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


    os_mutex_pend(&preview_info_hander->mutex, 0);
    list_del(&cli->entry);
    os_mutex_post(&preview_info_hander->mutex);

    /* puts("third\n"); */
    sock_unreg(cli->pinfo.sock);
    /* puts("four\n"); */
    free(cli->pinfo.data);
    free(cli);

    /* video_preview_and_thus_disconnect(NULL); */
    /* puts("end  video_thus_cli_thread\n"); */
}


static int create_app_thread(struct net_req	 *req, void *sock, struct sockaddr_in *remote_addr)
{
    char name[32];
    static u32 count = 0;
    //malloc cli info res
    struct __preview_cli_info *cli = (struct __preview_cli_info *)calloc(1, sizeof(struct __preview_cli_info));

    if (cli == NULL) {
        goto __err1;
    }

    //get req
    // strcpy(cli->filename, req->pre.filename);
    cli->filename =  req->pre.filename;
    cli->pinfo.sock = sock;
    //free req

    //malloc data's bufffer
    cli->pinfo.data = (u8 *)malloc(IMAGE_SIZE);

    if (cli->pinfo.data == NULL) {
        while (1) {
            printf("Warn malloc fail,alloc angan\n");
            msleep(200);
            cli->pinfo.data = (u8 *)malloc(IMAGE_SIZE);
            if (cli->pinfo.data != NULL) {
                break;
            }
        }
        /* free(cli); */
        /* goto __err1; */
    }

    cli->pinfo.len = IMAGE_SIZE;


    /* printf("create cli thread\n"); */
    memcpy(&cli->remote_addr, remote_addr, sizeof(struct sockaddr_in));
    os_mutex_pend(&preview_info_hander->mutex, 0);
    list_add_tail(&cli->entry, &preview_info_hander->cli_head);
    os_mutex_post(&preview_info_hander->mutex);
    switch (req->pre.type) {
    case PREVIEW:

        sprintf(name, "preview_cli_%d", count++);

        thread_fork(name, 24, 0x600, 0, &cli->pid, video_preview_cli_thread, (void *)cli);

        break;

    case THUS:
        cli->num = req->pre.num;
        cli->offset = req->pre.offset;
        cli->timeinv = req->pre.timeinv;
        /* printf("req->pre.filename : %s   num :%d   offset:%d   timeinv:%d\n", cli->filename, cli->num, cli->offset, cli->timeinv); */
        sprintf(name, "thus_cli_%x", remote_addr->sin_addr.s_addr);
        thread_fork(name, 24, 0x600, 0, &cli->pid, video_thus_cli_thread, (void *)cli);

        break;

    default:
        break;
    }

    free(req);
    return 0;

__err1:
    free(req);
    return -1;

}


int video_preview_req_handler2(void *msg)
{
    struct sockaddr_in remote_addr;
    fd_set rdset;
    struct timeval tv = {5, 0};
    void *sock = NULL;
    int ret;
    char buf[64];
    socklen_t addrlen = sizeof(remote_addr);
    if (msg == NULL) {
        goto __err1;
    }

    /* struct sock_hdl *hdl = (struct sock_hdl *)preview_info_hander->video_preview_sock_hdl; */
    struct net_req *req = (struct net_req *)msg;
    strcpy(buf, "status:0");
    if (req->pre.type == PREVIEW) {
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "MULTI_COVER_FIGURE", "NOTIFY", buf);
    } else if (req->pre.type == THUS) {
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "THUMBNAILS", "NOTIFY", buf);
    }

    while (1) {
        FD_ZERO(&rdset);
        FD_SET(sock_get_socket(preview_info_hander->video_preview_sock_hdl), &rdset);

        ret = sock_select(preview_info_hander->video_preview_sock_hdl, &rdset, NULL, NULL, &tv);
        if (ret < 0) {
            goto __err1;
        } else if (ret == 0) {
            puts("accept time out\n");
            if (req->pre.type == PREVIEW) {
                CTP_CMD_COMBINED(NULL, CTP_REQUEST, "MULTI_COVER_FIGURE", "NOTIFY", CTP_REQUEST_MSG);
            } else if (req->pre.type == THUS) {
                CTP_CMD_COMBINED(NULL, CTP_REQUEST, "THUMBNAILS", "NOTIFY", CTP_REQUEST_MSG);
            }

            free(req);
            return -1;

        } else {
            sock = sock_accept(preview_info_hander->video_preview_sock_hdl, (struct sockaddr *)&remote_addr, &addrlen, NULL, NULL);

            if (sock == NULL) {
                goto __err1;
            }

            if (create_app_thread(req, sock, &remote_addr)) {
                goto __err1;
            }
            return 0;
        }

    }
__err1:
    sock_unreg(preview_info_hander->video_preview_sock_hdl);
    return -1;

}
static void pre_thread(void *arg)
{
    video_preview_req_handler2(arg);
}

int  video_preview_post_msg(struct net_req *_req)
{
    int ret = 0;
    static u32 count = 0;
    char buf[32];
    struct net_req *req = (struct net_req *)calloc(1, sizeof(struct net_req));

    if (req == NULL) {
        return -1;
    }
    memcpy(req, _req, sizeof(struct net_req));
    sprintf(buf, "pre_thread%d", count++);
    ret = thread_fork(buf, 25, 0x1000, 0, 0, pre_thread, (void *)req);
    if (ret != OS_NO_ERR) {
        free(req);
        return -1;
    }
    return 0;

}


#if 0
static void video_preview_thread(void *arg)
{
    int res;
    u32 msg[32];

    while (1) {
        puts("wait msg\n\n\n\n\n\n\n");
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        puts("get msg gogogo\n");

        if (preview_info_hander->kill_flag) {
            break;
        }

        switch (res) {
        case OS_TASKQ:
            switch (msg[0]) {
            case Q_EVENT:
                break;

            case Q_MSG:
                video_preview_req_handler((void *) msg[1]);
                puts("1111111111111111111111111\n\n\n\n\n\n\n\n");
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

int preview_init(u16 port, int callback(void *priv, u8 *data, size_t len))
{
    /* puts("preview_init\n"); */
    int ret;
    struct sockaddr_in dest_addr;
    preview_info_hander->video_preview_sock_hdl = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);

    if (preview_info_hander->video_preview_sock_hdl == NULL) {
        printf("%s %d->Error in socket()\n", __func__, __LINE__);
        goto EXIT;
    }

    if (sock_set_reuseaddr(preview_info_hander->video_preview_sock_hdl)) {
        printf("%s %d->Error in sock_set_reuseaddr(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = htons(port);
    ret = sock_bind(preview_info_hander->video_preview_sock_hdl, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

    if (ret) {
        printf("%s %d->Error in bind(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }

    ret = sock_listen(preview_info_hander->video_preview_sock_hdl, 0xff);

    if (ret) {
        printf("%s %d->Error in listen()\n", __func__, __LINE__);
        goto EXIT;
    }

    os_mutex_create(&preview_info_hander->mutex);
    INIT_LIST_HEAD(&preview_info_hander->cli_head);
    /* puts("create video_preview_thread\n"); */
    /* thread_fork(VIDEO_PREVIEW_TASK_NAME, VIDEO_PREVIEW_TASK_PRIO, VIDEO_PREVIEW_TASK_STK, 40, &preview_info_hander->id, video_preview_thread, NULL); */


    return 0;
EXIT:
    sock_unreg(preview_info_hander->video_preview_sock_hdl);
    preview_info_hander->video_preview_sock_hdl = NULL;
    return -1;
}
int video_preview_and_thus_all_disconnect(void)
{
    puts("---------------video_cli_disconnect\n\n");
    struct __preview_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&preview_info_hander->mutex, 0);
    if (list_empty(&preview_info_hander->cli_head)) {
        puts("video_preview cli is emtry\n");
        os_mutex_post(&preview_info_hander->mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &preview_info_hander->cli_head) {
        cli = list_entry(pos, struct __preview_cli_info, entry);
        cli->kill_flag = 1;
        sock_set_quit(cli->pinfo.sock);
    }
    os_mutex_post(&preview_info_hander->mutex);
    return 0;

}
void preview_uninit(void)
{
    /* preview_info_hander->kill_flag = 1; */

    video_preview_and_thus_all_disconnect();
    sock_unreg(preview_info_hander->video_preview_sock_hdl);
    /* thread_kill(&preview_info_hander->id, KILL_WAIT); */
    /* os_taskq_post_msg(VIDEO_PREVIEW_TASK_NAME, 1, 0); */
    /* preview_info_hander->kill_flag = 0; */
}

int video_cli_slide(struct sockaddr_in *dst_addr, u8 direct)
{
    /* puts("---------------video_cli_slide\n\n"); */
    struct __preview_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&preview_info_hander->mutex, 0);
    if (list_empty(&preview_info_hander->cli_head)) {
        puts("video_preview cli is emtry\n");
        os_mutex_post(&preview_info_hander->mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &preview_info_hander->cli_head) {
        cli = list_entry(pos, struct __preview_cli_info, entry);

        if (cli->remote_addr.sin_addr.s_addr == dst_addr->sin_addr.s_addr
            && cli->remote_addr.sin_port == dst_addr->sin_port) {
            cli->direct = direct;
            os_mutex_post(&preview_info_hander->mutex);
            return 0;
            //break;
        }
    }
    os_mutex_post(&preview_info_hander->mutex);
    return -1;

}
int video_preview_and_thus_disconnect(struct sockaddr_in *dst_addr)
{
    /* puts("---------------video_cli_disconnect\n\n"); */
    struct __preview_cli_info *cli = NULL;
    struct list_head *pos = NULL, *node = NULL;

    os_mutex_pend(&preview_info_hander->mutex, 0);
    if (list_empty(&preview_info_hander->cli_head)) {
        puts("video_preview cli is emtry\n");
        os_mutex_post(&preview_info_hander->mutex);
        return -1;
    }


    list_for_each_safe(pos, node, &preview_info_hander->cli_head) {
        cli = list_entry(pos, struct __preview_cli_info, entry);

        if (dst_addr != NULL
            && cli->remote_addr.sin_addr.s_addr == dst_addr->sin_addr.s_addr) {
            cli->kill_flag = 1;
            sock_set_quit(cli->pinfo.sock);
            os_mutex_post(&preview_info_hander->mutex);
            return 0;
        }
    }
    os_mutex_post(&preview_info_hander->mutex);
    printf("%s not find it\n", __func__);
    return -1;

}
