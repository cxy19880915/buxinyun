/* #include "video_audio.h" */
#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "video_dec.h"
#include "include/menu_parm_api.h"
#include "server/audio_server.h"

#include "action.h"
#include "style.h"
#include "app_config.h"

/* #include "lyrics/lyrics.h" */

#define Video_Audio_EN
#ifdef  Video_Audio_EN


enum audio_state {
    AUDIO_STATE_IDLE,
    AUDIO_STATE_ENC_OPEN,
    AUDIO_STATE_ENC_START,
    AUDIO_STATE_ENC_STOP,
    AUDIO_STATE_ENC_CLOSE,


    AUDIO_STATE_DEC_OPEN,
    AUDIO_STATE_DEC_START,
    AUDIO_STATE_DEC_PAUSE,
    AUDIO_STATE_DEC_CLOSE,
};



struct audio_play_hdl {
    u8 state;
    struct server *ui;
    struct server *audio;
    FILE *file;
};


struct audio_play_hdl audio_handler;

#define __this 	(&audio_handler)
#define sizeof_this     (sizeof(struct audio_play_hdl))


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wav文件播放，函数调用例子
static struct server *audio = NULL;
static struct server *audio_rec  = NULL;
static FILE *file;
static void audio_server_event_handler(void *priv, int argc, int *argv)
{
    union audio_req r;
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_DEC_END:
        if (audio) {
            r.dec.cmd = AUDIO_DEC_STOP;
            server_request(audio, AUDIO_REQ_DEC, &r);
            server_close(audio);
            fclose(file);
            audio = NULL;
            /* kvoice_disable = 0; */
        }
        break;
    case AUDIO_SERVER_EVENT_ENC_ERR:
        if (audio_rec) {
            void rec_wav_stop();
            printf("audio rec err\n");
            rec_wav_stop();
            /* kvoice_disable = 0; */
        }
        break;

    }
}

void play_wav_file(const char *file_name)
{
    union audio_req r;
    if (audio) {
        return;
    }

    file = fopen(file_name, "r");
    if (!file) {
        return;
    }

    audio = server_open("audio_server", "dec");
    if (!audio) {
        fclose(file);
        return;
    }

    /* kvoice_disable = 1; */

    memset(&r, 0, sizeof(r.dec));
    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = 20;//-1;//20;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 1024 * 4;
    r.dec.file            = file;
    r.dec.sample_rate     = 8000;
    r.dec.priority        = 0;
    r.dec.channel        = 1;
    server_request(audio, AUDIO_REQ_DEC, &r);

    printf("total_time= %d \n", r.dec.total_time); //获取文件播放长度，可以用于ui显示
    printf("sample_rate= %d \n", r.dec.sample_rate); //获取文件采样率，可以用于ui显示

    r.dec.cmd = AUDIO_DEC_START;
    server_request(audio, AUDIO_REQ_DEC, &r);
    server_register_event_handler(audio, NULL, audio_server_event_handler);//播放结束回调函数
}


/////////////////////////////////////////////////////////////////////////


//生成wav播放文件
void rec_wav_start(char *file_name)
{
    union audio_req r;
    struct server *rec  = NULL;
    audio_rec = server_open("audio_server", "enc");
    if (!audio_rec) {
        return;
    }

    memset(&r, 0, sizeof(r.enc));
    r.enc.cmd             = AUDIO_ENC_OPEN;
    r.enc.volume          = -1;
    r.enc.channel         = 1;
    r.enc.sample_rate     = 8 * 1000;
    r.enc.wformat         = "pcm";//编码格式为pcm
    server_request(audio_rec, AUDIO_REQ_ENC, &r);

    if (file_name == NULL) {
        r.enc.name = CONFIG_ROOT_PATH"aud****.wav";
    } else {
        r.enc.name = file_name;
    }

    printf("%s\n", "r.enc.name");
    r.enc.cmd = AUDIO_ENC_START;
    server_request(audio_rec, AUDIO_REQ_ENC, &r);
    server_register_event_handler(audio_rec, NULL, audio_server_event_handler);//回调函数
    /* return rec; */
}



void rec_wav_stop()
{
    if (!audio_rec) {
        return;
    }
    union audio_req r;
    r.enc.cmd = AUDIO_ENC_STOP;
    server_request(audio_rec, AUDIO_REQ_ENC, &r);
    r.enc.cmd = AUDIO_ENC_CLOSE;
    server_request(audio_rec, AUDIO_REQ_ENC, &r);
    server_close(audio_rec);
    audio_rec = NULL;

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * 发送一些状态给ui
 */
void audio_ui_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_AUDIO_EN
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_ADUIO;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif

}
void audio_ui_post_msg_async(const char *msg, ...)
{
#ifdef CONFIG_UI_AUDIO_EN
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_ADUIO;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request_async(__this->ui, UI_REQ_MSG, &req);
    }
    va_end(argptr);
#endif

}


static void show_main_ui(void)
{
#ifdef CONFIG_UI_AUDIO_EN
    union uireq req;

    if (!__this->ui) {
        return;
    }

    puts("show_main_ui\n");
    req.show.id = ID_WINDOW_AUDIO;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
}
static void hide_main_ui(void)
{
#ifdef CONFIG_UI_AUDIO_EN
    union uireq req;

    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_main_ui\n");

    req.hide.id = ID_WINDOW_ADUIO;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}



static void audio_dec_server_event_handler(void *priv, int argc, int *argv)
{
    union audio_req r;
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_DEC_END:
        if (__this->audio) {
            r.dec.cmd = AUDIO_DEC_STOP;
            server_request(__this->audio, AUDIO_REQ_DEC, &r);
            server_close(__this->audio);
            __this-> audio = NULL;
            /* kvoice_disable = 0; */
        }
        if (__this->file) {
            fclose(__this->file);
        }
        break;
    }
}



static int aud_play_close(void);
/*
 * 播放音乐，name为路径
 */
static int aud_play_open(const char *file_name)
{
    int err;
    union audio_req r;
    FILE *file_aud;
    /* if(__this->state != AUDIO_STATE_IDLE) */
    /*     return -1;  */
    /*  */
    if (__this->audio) {
        return -1;
    }

    if (!file_name) {
        return -1;
    }

    printf("aud play open \n");

    file_aud = fopen(file_name, "r");
    if (!file_aud) {
        return -1;
    }
    __this->audio = server_open("audio_server", "dec");
    if (!__this->audio) {
        fclose(file_aud);
        return -1;
    }
    memset(&r, 0, sizeof(r.dec));
    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = 20;//-1;//20;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 1024 * 4;
    r.dec.file            = file_aud;
    /* r.dec.sample_rate     = 0;//必须写零，由文件指定,否则不会解码文件信息 */
    r.dec.priority        = 0;
    __this->file          = file_aud;
    server_request(__this->audio, AUDIO_REQ_DEC, &r);

    printf("total_time= %d \n", r.dec.total_time); //获取文件播放长度，可以用于ui显示
    printf("sample_rate= %d \n", r.dec.sample_rate); //获取文件采样率，可以用于ui显示
    server_register_event_handler(__this->audio, NULL, audio_dec_server_event_handler);
    __this->state = AUDIO_STATE_DEC_OPEN;
    if (r.dec.total_time == 0) {
        aud_play_close();
#ifdef CONFIG_UI_AUDIO_EN

#endif
        printf("file broke \n");
        return -1;
    }
#ifdef CONFIG_UI_AUDIO_EN

#endif
    return 0;
}


/*
 * 暂停播放音乐
 */
static int aud_play_pause(void)
{
    union audio_req r;
    if (!__this->audio) {
        return -1;
    }
    if (__this->state != AUDIO_STATE_DEC_START) {
        return -1;
    }
    r.dec.cmd = AUDIO_DEC_PAUSE;
    server_request(__this->audio, AUDIO_REQ_DEC, &r);
    __this->state = AUDIO_STATE_DEC_PAUSE;
    puts("pause_audio.\n");
    return 0;
}

/*
 * 继续播放音乐
 */
static int aud_play_start(void)
{
    union audio_req r;
    if (!__this->audio) {
        return -1;
    }
    if (__this->state != AUDIO_STATE_DEC_OPEN && __this->state != AUDIO_STATE_DEC_PAUSE) {
        return -1;
    }
    r.dec.cmd = AUDIO_DEC_START;
    server_request(__this->audio, AUDIO_REQ_DEC, &r);
    __this->state = AUDIO_STATE_DEC_START;
    puts("start_paly.\n");
    return 0;
}



/*
 * 关闭播放音乐
 */
static int aud_play_close(void)
{
    union audio_req r;
    if (!__this->audio) {
        return -1;
    }
    if (__this->state == AUDIO_STATE_DEC_START) {
        aud_play_pause();
    }
    if (__this->state == AUDIO_STATE_DEC_OPEN || __this->state == AUDIO_DEC_PAUSE) {
        r.dec.cmd = AUDIO_DEC_STOP;
        if (__this->audio) {
            server_request(__this->audio, AUDIO_REQ_DEC, &r);
            server_close(__this->audio);
            __this->audio = NULL;

        }
        if (__this->file) {
            fclose(__this->file);
            __this-> file = NULL;
        }
        __this->state = AUDIO_STATE_IDLE;
    }
    puts("stop_audio.\n");
    return 0;
}







static int aud_rec_open()
{
    union audio_req r;
    if (__this->audio) {
        return -1;
    }
    /* if(__this->state != AUDIO_STATE_IDLE) */
    /*    return -1;  */
    memset(&r, 0, sizeof(r.enc));
    __this->audio = server_open("audio_server", "enc");
    if (!__this->audio) {
        printf("audio open err \n");
        return -1;
    }
    r.enc.cmd             = AUDIO_ENC_OPEN;
    r.enc.volume          = 20;
    r.enc.channel         = 1;
    r.enc.sample_rate     = 8 * 1000;
    r.enc.wformat = "pcm";
    server_request(__this->audio, AUDIO_REQ_ENC, &r);
    __this->state = AUDIO_STATE_ENC_OPEN;

    return 0;
}


static int aud_rec_start(char *name)
{
    if (!__this->audio) {
        return -1;
    }

    if (__this->state != AUDIO_STATE_ENC_OPEN && __this->state != AUDIO_STATE_ENC_STOP) {
        return -1;
    }

    printf("aud start\n");
    union audio_req r;
    memset(&r, 0, sizeof(r.enc));
    if (name == NULL) {
        r.enc.name = CONFIG_ROOT_PATH"aud****.wav";
    } else {
        r.enc.name = name ;
    }
    r.enc.cmd = AUDIO_ENC_START;
    server_request(__this->audio, AUDIO_REQ_ENC, &r);
    __this->state = AUDIO_STATE_ENC_START;
    return 0;
}

static int aud_rec_stop()
{
    if (!__this->audio) {
        return -1;
    }
    printf(" aud_rec_stop \n");
    if (__this->state != AUDIO_STATE_ENC_START) {
        return -1;
    }
    union audio_req r;
    memset(&r, 0, sizeof(r.enc));
    r.enc.cmd = AUDIO_ENC_STOP;
    server_request(__this->audio, AUDIO_REQ_ENC, &r);
    __this->state = AUDIO_STATE_ENC_STOP;
    return 0;
}

static int aud_rec_close()
{
    if (!__this->audio) {
        return -1;
    }
    if (__this->state  == AUDIO_STATE_ENC_START) {
        aud_rec_stop();
    }
    if (__this->state == AUDIO_STATE_ENC_STOP || __this->state == AUDIO_STATE_ENC_OPEN) {
        union audio_req r;
        memset(&r, 0, sizeof(r.enc));
        r.enc.cmd = AUDIO_ENC_CLOSE;
        server_request(__this->audio, AUDIO_REQ_ENC, &r);
        server_close(__this->audio);
        __this->audio = NULL;
        __this->state = AUDIO_STATE_IDLE;
    }
    return 0;
}







static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int len;
    int ret;
    int err = 0;
    static u8 fname[MAX_FILE_NAME_LEN];
    union audio_req r;
    switch (state) {
    case APP_STA_CREATE:
        sys_key_event_takeover(false, false);//由APP处理消息

        puts("Enter audio mode.\n");
        memset(__this, 0, sizeof_this);
#ifdef CONFIG_UI_AUDIO_EN
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EINVAL;
        }
        show_main_ui();
#endif
        __this->state = AUDIO_STATE_IDLE;
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case  ACTION_AUDIO_MAIN:
            if (!strcmp(it->data, "rec_open")) {
                err = aud_rec_open();
                if (err) {
                    break;
                }
            }
            if (!strcmp(it->data, "rec_start")) {
                err = aud_rec_open();
                if (err) {
                    break;
                }
                err = aud_rec_start((char *)it->exdata);
                if (err) {
                    break ;
                }

            }
            if (!strcmp(it->data, "rec_stop")) {
                err = aud_rec_stop();
                if (err) {
                    break;
                }
            }
            if (!strcmp(it->data, "play_open")) {
                err = aud_play_open((char *)it->exdata);
                if (err) {
                    break;
                }
            }
            if (!strcmp(it->data, "play_pause")) {
                err = aud_play_pause();
                if (err) {
                    break;
                }
            }
            if (!strcmp(it->data, "play_start")) {
                err = aud_play_start();
                if (err) {
                    break;
                }
            }

            break ;
        case ACTION_AUDIO_SET_CONFIG:
            puts("ACTION_audio_PLAY_SET_CONFIG.\n");
            /* video_audio_set_config(it); */
            break;

        }
        break ;
    case APP_STA_PAUSE:
        puts("audio:APP_STA_PAUSE.\n");
        break;
    case APP_STA_RESUME:
        puts("audio:APP_STA_RESUME.\n");
        break;
    case APP_STA_STOP:
        puts("audio:APP_STA_STOP.\n");
        aud_rec_stop();
        aud_play_pause();
        break;
    case APP_STA_DESTROY:
        puts("audio:APP_STA_DESTROY.\n");
        aud_rec_close();
        aud_play_close();
        __this->state = AUDIO_STATE_IDLE;

#ifdef CONFIG_UI_AUDIO_EN
        hide_main_ui();
        puts("--------audio close ui\n");
        if (__this->ui) {
            server_close(__this->ui);
            __this->ui = NULL;
        }
#endif
        break;
    }

    return 0;
}


/*
 *按键响应函数
 */
static int video_audio_key_event_handler(struct key_event *key)
{
    int ret;
    union audio_req r;
    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            puts("audio key ok\n");

#if 1
            if (__this->state == AUDIO_STATE_ENC_OPEN || __this->state == AUDIO_STATE_ENC_STOP) {
                /* __this->status = AUDIO_STA_PAUSE; */
                aud_rec_start(NULL);
                /* audio_ui_post_msg_async("pp:s=%1", 0); */
            } else if (__this->state == AUDIO_STATE_ENC_START) {
                aud_rec_stop();
                /* audio_ui_post_msg_async("pp:s=%1", 1); */
            } else if (__this->state == AUDIO_STATE_DEC_OPEN || __this->state == AUDIO_STATE_DEC_PAUSE) {
                aud_play_start();
            } else if (__this->state == AUDIO_STATE_DEC_START) {
                aud_play_pause();
            }

#endif
            break;
        case KEY_MENU:
            puts("audio key menu\n");
            return true;
            break;
        case KEY_MODE:
            puts("audio key mode\n");
            break;
        case KEY_UP:
            puts("audio key up\n");
            break;
        case KEY_DOWN:
            puts("audio key down\n");
            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_HOLD:

        break;
    default:
        break;
    }

    return false;
}

/*
 *设备响应函数
 */
static int video_audio_device_event_handler(struct sys_event *event)
{
    int err;
    int ret;
    struct intent it;
    union audio_req r;
    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("\ncard in\n");
            break;
        case DEVICE_EVENT_OUT:
            puts("\ncard out\n");
            aud_rec_close();
            aud_play_close();
            break;
        }
    } else if (!ASCII_StrCmp(event->arg, "charger", 7)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("\ncharger in\n");
            break;
        case DEVICE_EVENT_OUT:
            puts("\ncharger out\n");
            break;
        }
    }

    return false;
}
static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return video_audio_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return video_audio_device_event_handler(event);
        break;
    default:
        return false;
    }
}



static const struct application_operation video_audio_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

REGISTER_APPLICATION(app_video_audio) = {
    .name 	= "video_audio",
    .action	= ACTION_AUDIO_MAIN,
    .ops 	= &video_audio_ops,
    .state  = APP_STA_DESTROY,
};



#endif

