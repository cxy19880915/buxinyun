#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H

#include "fs/fs.h"


#define AUDIO_SERVER_EVENT_DEC_END   0x20


#define AUDIO_REQ_DEC       0
#define AUDIO_REQ_ENC       1

#define AUDIO_DEC_OPEN      0
#define AUDIO_DEC_START     1
#define AUDIO_DEC_PAUSE     2
#define AUDIO_DEC_STOP      3




struct audio_finfo {
    u8 channel;
    int sample_rate;
};

struct audio_dec_output_ops {
    void *(*get_buf)(void *priv, u32 *len);
    void (*put_buf)(void *priv, void *buf, u32 len);
};

struct audio_decoder_ops {
    const char *name;
    void *(*open)(FILE *file, void *priv, const struct audio_dec_output_ops *ops);
    int (*get_audio_info)(void *, struct audio_finfo *info);
    int (*start)(void *);
};



struct audio_dec_req {
    u8 cmd;
    u8 channel;
    u8 volume;
    u8 priority;
    u16 output_buf_len;
    u32 sample_rate;
    void *output_buf;
    FILE *file;
};

union audio_req {
    struct audio_dec_req dec;
};


#define REGISTER_AUDIO_DECODER(ops) \
        const struct audio_decoder_ops ops sec(.audio_decoder)


extern const struct audio_decoder_ops audio_decoder_begin[];
extern const struct audio_decoder_ops audio_decoder_end[];

#define list_for_each_audio_decoder(p) \
    for (p = audio_decoder_begin; p < audio_decoder_end; p++)
















#endif
