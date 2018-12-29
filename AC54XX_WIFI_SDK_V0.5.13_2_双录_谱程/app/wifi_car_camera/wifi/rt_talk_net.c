#include "app_config.h"
#include "sock_api/sock_api.h"
#include "os/os_api.h"
#include "server/rt_stream_pkg.h"
#include "circular_buf.h"

#define CHECK_CODE 0x88
#define CHECK_CODE_NUM 32

#define DATA_BUF_SIZE  16*1024
#define RECV_TIME_OUT  3*1000
#define AUDIO_TIME_DELAY  100     //ms

/*
static void *rt_talk->sock;
static cbuffer_t rt_talk->cbuf_net_data;
char data_buf[DATA_BUF_SIZE];

static int thread_pid;
static u32 rt_talk->recv_time = 0;
*/

static u32 old_frame_seq = 0;
static u8 rt_talk_status = 0;

struct __rt_talk {

    void *sock;
    u8 data_buf[DATA_BUF_SIZE];
    int thread_pid;
    u32 recv_time;
    cbuffer_t cbuf_net_data;

};

struct __rt_talk  *rt_talk;

extern void rt_talk_voice_play(void);
extern void rt_talk_voice_stop(void);
extern int rt_talk_voice_init(void);
extern int rt_talk_voice_uninit(void);
extern unsigned int time_lapse(unsigned int *handle, unsigned int time_out);



static int bytecmp(unsigned char *p, unsigned char ch, unsigned int num)
{
    while (num-- > 0) {
        if (*p++ != ch) {
            return -1;
        }
    }
    return 0;
}

u8 get_rt_talk_status(void)
{
    return rt_talk_status;
}

//图片封包函数
static int get_pcm_packet(char *buf, int len, char *pcm_buf, int *pcm_buf_len)
{
    static u32 total_payload_len = 0;
    static u8 finish = 0;

    u32 position = 0;
    u32 frame_head_size = sizeof(struct frm_head);

    u8 frame_type;
    u32 cur_frame_seq;
    u32 frame_offset;
    u32 slice_data_len;
    u32 frame_size;

    if (len < frame_head_size) {
        printf("\n%s %d->data err\n", __func__, __LINE__);
        goto ERROR;
    }

    do {
        struct frm_head  *head_info = (struct frm_head *)(buf + position);
        frame_type = head_info->type & 0x7F;
        cur_frame_seq = head_info->seq;
        frame_offset = head_info->offset;
        slice_data_len = head_info->payload_size;
        frame_size = head_info->frm_sz;

        len -= (frame_head_size + slice_data_len);//如果帧头长+数据长度!=包长度
        if (len < 0) {
            printf("\n%s %d->data err\n", __func__, __LINE__); //认为是错误的数据包
            goto ERROR;
        }

        switch (frame_type) {

        case PCM_TYPE_AUDIO:

            if (cur_frame_seq < old_frame_seq) { //如果当前的seq小于旧的seq,说明是旧的数据包,跳过不处理

                printf("\n%s %d->recv old seq\n", __func__, __LINE__);
                goto continue_deal;

            } else if (cur_frame_seq > old_frame_seq) { //如果当前seq大于旧的seq,认为是新的数据包,接收包初始化

                if (total_payload_len && (finish == 0)) {
                    printf("\n%s %d->recv old seq\n", __func__, __LINE__);
                }

                old_frame_seq = cur_frame_seq;
                total_payload_len = 0;
                finish = 0;
                memset(pcm_buf, CHECK_CODE, 4096);
            }

            if (bytecmp((unsigned char *)pcm_buf + frame_offset, CHECK_CODE, CHECK_CODE_NUM) != 0) {
                printf("\n%s %d->repeat seq\n", __func__, __LINE__);
                goto continue_deal;

            }

            memcpy(pcm_buf + frame_offset, (buf + position) + frame_head_size, slice_data_len);//对应数据放置到对应的位置

            total_payload_len += slice_data_len;//累加总长度

            if (total_payload_len == frame_size) { //如果数据量相等,说明帧数据接收完成
                *pcm_buf_len = total_payload_len;
                finish = 1;
                //printf("\n%s %d->finish\n",__func__,__LINE__);
                return 0;
            }

continue_deal:

            position += (frame_head_size + slice_data_len);

            break;

        default:
            printf("\n%s %d---type >> %d\n", __func__, __LINE__, frame_type);
            break;

        }
    } while (len > 0);

ERROR:

    return -1;

}

u32 read_net_data_cbuf(void *buf, u32 len)
{
    u32 rlen;

    rlen = cbuf_read(&rt_talk->cbuf_net_data, buf, len);

    return rlen;
}

u32 write_net_data_cbuf(void *buf, u32 len)
{
    u32 wlen;

    wlen = cbuf_write(&rt_talk->cbuf_net_data, buf, len);

    return wlen;
}

static void rt_talk_data_recv(void *priv)
{
    int recv_len;
    u32 w_len;
    char recv_buf[4000];
    int pcm_buf_len;
    char pcm_buf[8000];
    void *dev;

    while (1) {

        recv_len = sock_recvfrom(rt_talk->sock, recv_buf, sizeof(recv_buf), 0, NULL, NULL);
        if (recv_len <= 0) {
            printf("\n%s %d->exit\n", __func__, __LINE__);
            rt_talk_voice_stop();
            break;
        }
        rt_talk->recv_time = 0;
        if (get_pcm_packet(recv_buf, recv_len, pcm_buf, &pcm_buf_len) == 0) {

            w_len = write_net_data_cbuf(pcm_buf, pcm_buf_len);
            if (w_len == 0) {
                printf("\n%s %d->cbuf_clear\n", __func__, __LINE__);
                cbuf_clear(&rt_talk->cbuf_net_data);
            } else {
                u32 data_size;
                data_size = cbuf_get_data_size(&rt_talk->cbuf_net_data);
                if (data_size >= 16 * AUDIO_TIME_DELAY) {
                    //printf("\n%s %d->play\n",__func__,__LINE__);
                    rt_talk_voice_play();
                } else if (data_size < 16 * 20) {

                    printf("\n%s %d->stop\n", __func__, __LINE__);
                    rt_talk_voice_stop();

                }
            }
        }
    }

}

static int rt_talk_recv_cb(enum sock_api_msg_type type, void *priv)
{

    if (time_lapse(&rt_talk->recv_time, RECV_TIME_OUT)) {

        puts("|RT_TALK_NO_RECV_DATA|\n");
        rt_talk_voice_stop();
        return -1;
    }

    return 0;
}

int rt_talk_net_init(void)
{
    struct sockaddr_in dest_addr;
    int err;
    if (rt_talk != NULL) {
        printf("\nAPP not allow open again\n");
        return 0;
    }

    rt_talk = calloc(sizeof(struct __rt_talk), 1);
    if (rt_talk == NULL) {
        printf("\nnot enough space\n");
        goto EXIT;
    }

    rt_talk_voice_init();
    rt_talk->sock = sock_reg(AF_INET, SOCK_DGRAM, 0, rt_talk_recv_cb, NULL);

    if (rt_talk->sock == NULL) {
        printf("\n%s %d ->Error in socket()\n", __func__, __LINE__);
        goto EXIT;
    }

    if (sock_set_reuseaddr(rt_talk->sock)) {
        printf("%s %d->Error in sock_set_reuseaddr(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_port = htons(2231);

    err = sock_bind(rt_talk->sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
    if (err) {
        printf("%s %d->Error in bind(),errno=%d\n", __func__, __LINE__, errno);
        goto EXIT;
    }
    cbuf_init(&rt_talk->cbuf_net_data, rt_talk->data_buf, sizeof(rt_talk->data_buf));
    old_frame_seq = 0;
    rt_talk_status = 1;
    thread_fork("RT_RECV_THREAD", 25, 0x1000, 0, &rt_talk->thread_pid, rt_talk_data_recv, NULL);

    return 0;

EXIT:

    if (rt_talk->sock) {
        sock_unreg(rt_talk->sock);
    }
    return -1;
}

int rt_talk_net_uninit(void)
{
    if (rt_talk != NULL) {
        sock_unreg(rt_talk->sock);
        thread_kill(&rt_talk->thread_pid, 0);
        rt_talk_status = 0;
        rt_talk_voice_uninit();
        free(rt_talk);
        rt_talk = NULL;
    }
    return 0;
}
