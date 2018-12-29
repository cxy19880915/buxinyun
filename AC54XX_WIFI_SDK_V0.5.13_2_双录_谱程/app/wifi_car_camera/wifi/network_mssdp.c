#include "mssdp/mssdp.h"
#include "fs/fs.h"
#include "app_config.h"
#include "stdlib.h"

#include "system/event.h"
#if defined CONFIG_NET_SERVER || defined CONFIG_NET_CLIENT
static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    if (type == MSSDP_SEARCH_MSG) {
        printf("ssdp client[0x%x] search, %s\n", dest_ipaddr, buf);
    } else if (type == MSSDP_NOTIFY_MSG) {
        printf("sssss\n");
//TODO
#if 1
        struct sys_event e;
        e.arg = "dhcp_srv";
        e.type = SYS_DEVICE_EVENT;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        sys_event_notify(&e);
#endif


        mssdp_uninit();

    }
}
#else

#define JSON_DOC "aaa"

static char json_buffer[1024];

const char *__attribute__((weak))get_dev_desc_doc()
{
    int ret;
    FILE *fd = fopen("mnt/spiflash/res/dev_desc.txt", "r");

    if (fd == NULL) {
        printf("%s~~~~~~~~%d   open fail\n", __func__, __LINE__);
        return NULL;
    }

    memset(json_buffer, 0, 1024);
    ret = fread(fd, json_buffer, 1024);
    if (ret <= 0) {
        printf("%s~~~~~~~~%d   read fail\n", __func__, __LINE__);
        return NULL;
    }

    fclose(fd);

    return json_buffer;
}

static void network_ssdp_cb(u32 dest_ipaddr, enum mssdp_recv_msg_type type, char *buf, void *priv)
{
    static int time_hdl;
    static int seq, total_drop_pkt;

    if (type == MSSDP_SEARCH_MSG) {
        printf("client[0x%x] search\n", dest_ipaddr);
    }
#ifdef CONFIG_PACKET_LOSS_RATE_ENABLE
    else if (type == MSSDP_NOTIFY_MSG) {
//        printf("MSSDP_NOTIFY_MSG->  %s \r\n",buf);

        static int last_time, last_seq;
        int drop_pkt, cur_time, time_diff;
        char *p_seq = strstr(buf, "UX_DATA:") + strlen("UX_DATA:");
        if (p_seq) {
            seq = atoi(p_seq);
            extern u32 timer_get_ms(void);
            time_diff = timer_get_ms() - last_time;
            drop_pkt = seq - last_seq - 1;
            if (drop_pkt) {
                total_drop_pkt += drop_pkt;
                printf("UX_DROP last_seq =%d, seq =%d, drop_pkt = %d, total_drop_pkt = %d, time_diff =%d \r\n", last_seq, seq, drop_pkt, total_drop_pkt, time_diff);
            }
            last_seq = seq;
            last_time = timer_get_ms();
        }
    }
    if (seq && time_lapse(&time_hdl, 1000)) {
        static int last_seq, once_drop_pkt;
//        printf("once_drop_pkt = %d , total_drop_pkt = %d \r\n", once_drop_pkt, total_drop_pkt);
        char s_buf[64];
        sprintf(s_buf, "MSSDP_NOTIFY UX_REPORT:ONCE_DROP:%d,TOTAL_DROP:%d", once_drop_pkt, total_drop_pkt);
        mssdp_send_msg(NULL, s_buf, 1 + strlen(s_buf));
        if (once_drop_pkt) {
            once_drop_pkt = 0;
        } else {
            once_drop_pkt = 1;
        }
        last_seq = seq;
    }
#endif
}
#endif

void network_mssdp_init(void)
{
    puts("mssdp run \n");
    mssdp_init("MSSDP_SEARCH ", "MSSDP_NOTIFY ", 3889, network_ssdp_cb, NULL);

#if defined  CONFIG_NET_SERVER
    mssdp_set_search_msg("xyz", 30); //主动3秒
#endif
#if defined  CONFIG_NET_CLIENT
    mssdp_set_notify_msg("xy", 5); //被动30秒
#endif

#if (!defined CONFIG_NET_CLIENT) && (!defined CONFIG_NET_SERVER)

#ifdef  CONFIG_PACKET_LOSS_RATE_ENABLE
    mssdp_set_notify_msg("UX_SEND_LEN:100,UX_SEND_ITV:50", 30);
#else
    mssdp_set_notify_msg((const char *)get_dev_desc_doc(), 60);
#endif

#endif
}

void network_mssdp_uninit(void)
{
    mssdp_uninit();
}



