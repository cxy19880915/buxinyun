#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H

#include "fs/fs.h"


#define AUDIO_SERVER_EVENT_DEC_END   0x20

#define AUDIO_SERVER_EVENT_ENC_ERR   0x30

#define AUDIO_REQ_DEC       0
#define AUDIO_REQ_ENC       1

#define AUDIO_DEC_OPEN      0
#define AUDIO_DEC_START     1
#define AUDIO_DEC_PAUSE     2
#define AUDIO_DEC_STOP      3


#define AUDIO_ENC_OPEN      4
#define AUDIO_ENC_START     5
#define AUDIO_ENC_PAUSE     6
#define AUDIO_ENC_STOP      7
#define AUDIO_ENC_CLOSE     8

struct audio_finfo {
    u8 channel;
    int sample_rate;
    u32 total_time;
};


struct audio_decoder_fsys_ops {
    void *(*fopen)(const char *path, const char *mode);
    int (*fread)(void *file, void *buf, u32 len);
    int (*fwrite)(void *file, void *buf, u32 len);
    int (*fseek)(void *file, u32 offset, int seek_mode);
    int (*fcat)(void *file_name, void *cat_fname, u8 cat_mode);
    int (*ftell)(void *file);
    int (*flen)(void *file);
    int (*fclose)(void *file);
};


struct audio_dec_output_ops {
    void *(*get_buf)(void *priv, u32 *len);
    void (*put_buf)(void *priv, void *buf, u32 len);
    struct audio_decoder_fsys_ops  *vfs_ops;
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
    u32 total_time;
    char *dec_type;
    struct audio_decoder_fsys_ops *vfs_ops;
};


struct audio_server {
    void *private_data;
    u32 data[0];
};

struct audio_enc_req {
    u8 cmd;
    u8 channel;
    u8 volume;
    u8 priority;
    u16 input_buf_len;
    u32 sample_rate;
    void *input_buf;
    char *wformat;//编码格式
    char *name;
};

union audio_req {
    struct audio_dec_req dec;
    struct audio_enc_req enc;
};


#define REGISTER_AUDIO_DECODER(ops) \
        const struct audio_decoder_ops ops sec(.audio_decoder)


extern const struct audio_decoder_ops audio_decoder_begin[];
extern const struct audio_decoder_ops audio_decoder_end[];

#define list_for_each_audio_decoder(p) \
    for (p = audio_decoder_begin; p < audio_decoder_end; p++)









#endif
