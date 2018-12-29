#include "system/includes.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"

static void *dev = NULL;
static u8 bindex = 0xff;
static u16 offset = 0;

extern u32 read_net_data_cbuf(void *buf, u32 len);

void rt_talk_voice_play(void)
{
    if (0xFF == bindex) {
        //printf("\n%s %d\n",__func__,__LINE__);
        dev_ioctl(dev, AUDIOC_STREAM_ON, (u32)&bindex);
    }
}


void rt_talk_voice_stop(void)
{
    if (0 == bindex) {
        //printf("\n%s %d\n",__func__,__LINE__);
        dev_ioctl(dev, AUDIOC_STREAM_OFF, bindex);
        bindex = 0xFF;
    }
}


static void rt_talk_dac_irq_handler(void *priv, void *data, int len)
{
    int rlen = len;
    u16 data_len;
    const u8 *table;
    u8 buf[1024];
    //printf("\n%s %d\n",__func__,__LINE__);
    if (bindex == 0xFF) {
        return;
    }

    data_len = read_net_data_cbuf(buf, rlen);

    if (data_len == 0) {
        memset((u8 *)data, 0, len);
        printf("\n rt_talk recv no data \n");
    } else {
        memcpy(data, buf, data_len);
        if (data_len < rlen) {
            memset((u8 *)data + data_len, 0, len - data_len);
        }
    }
}

int rt_talk_voice_init(void)
{
    int err;
    u32 arg[2];
    struct audio_format f;

    dev =  dev_open("audio", (void *)AUDIO_TYPE_DEC);
    if (!dev) {
        return 0;
    }

    f.volume = -1;
    f.channel = 1;
    f.sample_rate = 8000;
    f.priority = 9;

    err = dev_ioctl(dev, AUDIOC_SET_FMT, (u32)&f);
    if (err) {
        puts("format_err\n");
        return 0;
    }
    arg[0] = 0;
    arg[1] = (u32)rt_talk_dac_irq_handler;
    dev_ioctl(dev, IOCTL_REGISTER_IRQ_HANDLER, (u32)arg);

    return 0;
}

int rt_talk_voice_uninit(void)
{
    dev_close(dev);
    return 0;
}
