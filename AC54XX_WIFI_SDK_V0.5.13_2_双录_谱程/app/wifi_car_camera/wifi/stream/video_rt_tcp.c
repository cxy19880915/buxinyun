#include "common/common.h"
#include "server/rt_stream_pkg.h"
#include "lwip.h"
#include "server/stream_core.h"
#include "sock_api/sock_api.h"






static u8 inited = 0;//防止重复发送
static u32 aux_frame_cnt = 0;

extern int atoi(const char *__nptr);
struct cli_info {
    struct list_head entry;
    void *fd;
    struct sockaddr_in addr;
};


struct video_rt_tcp_server_info {
    struct list_head cli_head;
    struct sockaddr_in local_addr;
    void *fd;
    OS_SEM sem;
    u32 flag;
};

static struct video_rt_tcp_server_info server_info;

static struct cli_info *get_tcp_net_info();



static int  path_analyze(struct rt_stream_info *info, const *path)
{

    char *tmp = NULL;
    char *tmp2 = NULL;
    char ip[15] = {0};
    u16 port = 0;
    tmp = strstr(path, "tcp://");
    if (!tmp) {
        return -1;
    }

    tmp += strlen("tcp://");

    tmp2 = strchr(tmp, ':');
    printf("tmp=%s  len=%d\n", tmp, tmp2 - tmp);
    strncpy(ip, tmp, tmp2 - tmp);
    port = atoi(tmp2 + 1);

    printf("remote ip:%s  port:%d\n", ip, port);


    info->addr.sin_family = AF_INET;
    info->addr.sin_addr.s_addr = inet_addr(ip);
    info->addr.sin_port = htons(port);
    return 0;

}


static void *video_rt_tcp_init(const char *path, const char *mode)
{
    int ret = 0;
    struct rt_stream_info *r_info = calloc(1, sizeof(struct rt_stream_info));

    if (r_info == NULL) {
        printf("%s malloc fail\n", __FILE__);
        return NULL;
    }

    printf("video_rt_tcp_init r_info=0x%x\n\n\n", r_info);
    path_analyze(r_info, path);


    struct cli_info *info = get_tcp_net_info();
    if (info == NULL) {
        printf("%s get_tcp_net_info\n", __FILE__);
        free(r_info);
        return NULL;
    }

    memcpy((void *)&r_info->addr, (void *)&info->addr, sizeof(struct sockaddr));
    r_info->fd = info->fd;

    return (void *)r_info;
}

static u32 video_rt_tcp_send(void *hdr, u8 *data, u32 len, u8 type)
{
    /*os_time_dly(1+(RAND64L%6));//用于测试帧率控制*/
    /*os_time_dly(5);//用于测试帧率控制*/

    struct rt_stream_info *r_info = (struct rt_stream_info *)hdr;
    int ret;
    char *buffer = NULL;
    static u32 i = 0;
    char mem[64];
    u32 tmp2;
    u32 tmp;
    u32 start_code = 0x01000000;

    struct frm_head frame_head = {0};

    memset(&frame_head, 0, sizeof(struct frm_head));

    frame_head.type = type ;
    frame_head.seq = (i++);

    if (aux_frame_cnt < 60) { //前面1.5秒加速播放

        frame_head.timestamp += 0;
        aux_frame_cnt++;
    } else {

        frame_head.timestamp += 3300;
    }

//   frame_head.timestamp = 90000 / 30;
    frame_head.frm_sz = len;
    ret = sock_send(r_info->fd, (char *)&frame_head, sizeof(struct frm_head), 0);
    if (ret <= 0) {
        return -1;
    }


    if (type == H264_TYPE_VIDEO) {

        //防止数据源被改
        memcpy(mem, data, 64);
        buffer = mem;

        if (*((char *)(buffer + 4)) == 0x67) {
            //处理PPS帧和SPS帧 I帧
            memcpy(&tmp, buffer, 4);
            tmp = htonl(tmp);
            memcpy(buffer, &start_code, 4);
            memcpy(&tmp2, buffer + tmp + 4, 4);
            tmp2 = htonl(tmp2);
            memcpy(buffer + tmp + 4, &start_code, 4);
            memcpy(buffer + tmp + tmp2 + 8, &start_code, 4);
            //printf("tmp %d  tmp2 %d\n",tmp,tmp2);

        } else {
            //	   处理P帧
            memcpy(buffer, &start_code, 4);
        }

        ret = sock_send(r_info->fd, mem, 64, 0);
        if (ret <= 0) {
            return -1;
        }

        ret = sock_send(r_info->fd, (char *) data + 64, len - 64, 0);
        if (ret <= 0) {
            return -1;
        }



    } else {

        ret = sock_send(r_info->fd, (char *)data, len, 0);
        if (ret <= 0) {
            return -1;
        }

    }


    return len;

}

static void video_rt_tcp_uninit(void *hdr)
{
    puts("video_rt_tcp_uninit\n\n\n\n\n\n");
    aux_frame_cnt = 0;
    struct rt_stream_info *r_info = (struct rt_stream_info *)hdr;
    if (r_info != NULL) {
        free(r_info);

        r_info = NULL;
    }
}

REGISTER_NET_VIDEO_STREAM_SUDDEV(tcp_video_stream_sub) = {
    .name = "tcp",
    .open =  video_rt_tcp_init,
    .write = video_rt_tcp_send,
    .close = video_rt_tcp_uninit,
};





static struct cli_info *get_tcp_net_info()
{
    struct list_head *pos = NULL;
    struct cli_info *cli = NULL;
    struct cli_info *old_cli = NULL;
    int count = 0;
    os_sem_pend(&server_info.sem, 50);

    list_for_each(pos, &server_info.cli_head) {
        cli = list_entry(pos, struct cli_info, entry);
        printf("ip:%s   port:%d\n\n", inet_ntoa(cli->addr.sin_addr.s_addr), htons(cli->addr.sin_port));
        count++;
    }
    while (count > 1) {

        printf("count = %d\n\n", count);
        old_cli = list_first_entry(&server_info.cli_head, struct cli_info, entry);
        list_del(&old_cli->entry);
        sock_set_quit(old_cli->fd);
        sock_unreg(old_cli->fd);
        old_cli->fd = NULL;
        free(old_cli);
        count--;
    }

    return cli;


}




static void __do_sock_accpet(void *arg)
{

    socklen_t len = sizeof(server_info.local_addr);
    while (1) {

        struct cli_info *__cli = calloc(1, sizeof(sizeof(struct cli_info)));
        if (__cli == NULL) {
            printf("malloc fail\n");
            while (1);
        }


        __cli->fd  = sock_accept(server_info.fd, (struct sockaddr *)&__cli->addr, &len, NULL, NULL);
        if (__cli->fd == NULL) {
            printf("some error in here\n\n");
            continue;
        }
        if (server_info.flag) {
            break;
        }

        list_add_tail(&__cli->entry, &server_info.cli_head);


        os_sem_post(&server_info.sem);

    }

}


int video_rt_tcp_server_init(int port)
{
    int ret = 0;
    puts("video_rt_tcp_server_init\n");

    memset(&server_info, 0x0, sizeof(server_info));
    os_sem_create(&server_info.sem, 0);

    server_info.local_addr.sin_family = AF_INET;
    server_info.local_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    server_info.local_addr.sin_port = htons(port);


    server_info.fd = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);

    if (server_info.fd == NULL) {
        return -1;
    }

    u32 opt = 1;
    if (sock_setsockopt(server_info.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        printf("%s sock_bind fail\n", __FILE__);
        return -1;
    }


    if (sock_bind(server_info.fd, (struct sockaddr *)&server_info.local_addr, sizeof(struct sockaddr))) {
        printf("%s sock_bind fail\n", __FILE__);
        return -1;
    }
    sock_listen(server_info.fd, 0x5);

    INIT_LIST_HEAD(&server_info.cli_head);
    ret = thread_fork("__do_sock_accpet", 25, 0x100, 0, 0, __do_sock_accpet, NULL);
    if (ret != OS_NO_ERR) {
        printf("%s thread fork fail\n", __FILE__);
        return -1;

    }

    return 0;

}
void video_rt_tcp_server_uninit()
{
    printf("video_rt_tcp_server_uninit\n");
    server_info.flag = 1;
    sock_set_quit(server_info.fd);
    sock_unreg(server_info.fd);
    server_info.flag = 0;

}

