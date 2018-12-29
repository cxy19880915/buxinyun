



#include "video_music.h"

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

#include "lyrics/lyrics.h"

#ifdef CONFIG_MUSIC_MOD

extern int video_music_set_config(struct intent *it);

static int select_file_and_paly(int mode);

struct music_play_hdl music_handler;

#define __this 	(&music_handler)
#define sizeof_this     (sizeof(struct music_play_hdl))

static FILE *file;
static struct server *music_audio = NULL;

/*
 * 发送一些状态给ui
 */
void music_ui_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_MUSIC;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif

}
void music_ui_post_msg_async(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_MUSIC;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request_async(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif

}

static void show_main_ui(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        return;
    }

    puts("show_main_ui\n");
    req.show.id = ID_WINDOW_MUSIC;
    server_request_async(__this->ui, UI_REQ_SHOW, &req);
#endif
}
static void hide_main_ui(void)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;

    if (!__this->ui) {
        puts("__this->ui == NULL!!!!\n");
        return;
    }

    puts("hide_main_ui\n");

    req.hide.id = ID_WINDOW_MUSIC;
    server_request(__this->ui, UI_REQ_HIDE, &req);
#endif
}

static void audio_server_event_handler(void *priv, int argc, int *argv)
{

    int ret;
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_DEC_END:
        puts("music: AUDIO_SERVER_EVENT_DEC_END.\n");

        /*
         * 播放结束，告诉UI打开下一个文件
         */
        music_ui_post_msg_async("end");
        break;
    case AUDIO_SERVER_EVENT_PLAY_TIME:
        music_ui_post_msg_async("ptime:t=%4", argv[1]);
        break;
    }
}

/*
 * 播放音乐，name为路径
 */
static int play_music(const char *name)
{
    union audio_req r;
    static u8 fname[MAX_FILE_NAME_LEN];
    int len, err;

    if (name && !file) {
        printf("play_music: %s\n", name);
        file = fopen(name, "r");
        if (!file) {
            puts("play_music open file fail!\n");
            return -ENOENT;
        }
    } else if (!name && !file) {
        puts("play_music open err!\n");
        ASSERT(0);
        return -EPERM;
    }

    if (!music_audio) {
        music_audio = server_open("audio_server", "dec");
        if (!music_audio) {
            fclose(file);
            puts("play_music open audio_server fail!\n");
            return -EPERM;
        }
        server_register_event_handler(music_audio, NULL, audio_server_event_handler);
    }

    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = -1;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 4096;
    r.dec.file            = file;
    r.dec.channel         = 0;
    r.dec.sample_rate     = 0;
    r.dec.priority = 1;
    err = server_request(music_audio, AUDIO_REQ_DEC, &r);
    if (err) {
        goto __err_p;
    }

    r.dec.cmd = AUDIO_DEC_START;
    err = server_request(music_audio, AUDIO_REQ_DEC, &r);

    if (err) {
        goto __err_p;
    }

    /* r.dec.cmd = AUDIO_DEC_GET_INFO; */
    /* server_request(music_audio, AUDIO_REQ_DEC, &r); */

    /*
     * 获取文件名
     */
    /* memset(fname, 0, MAX_FILE_NAME_LEN); */
    /* len = fget_name(file, fname, MAX_FILE_NAME_LEN); */
    /* printf("select music: %s\n", fname); */
    /*  */
    /*  */
    /* music_ui_post_msg("info:fname=%p ch=%1 sr=%4 br=%4 tt=%4", */
    /*                   fname, r.dec.info_ret.channel, r.dec.info_ret.sample_rate, */
    /*                   r.dec.info_ret.bit_rate, r.dec.info_ret.total_time); */
    puts("play_music start ok.\n");
    return 0;
__err_p:
    if (music_audio) {
        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(music_audio, AUDIO_REQ_DEC, &r);
        server_close(music_audio);
        music_audio = NULL;
    }
    fclose(file);
    file = NULL;

    return -EFAULT;
}


/*
 * 暂停播放音乐
 */
static int pause_music(void)
{
    union audio_req r;
    r.dec.cmd = AUDIO_DEC_PAUSE;
    server_request(music_audio, AUDIO_REQ_DEC, &r);
    puts("pause_music.\n");
    return 0;
}

/*
 * 继续播放音乐
 */
static int continue_music(void)
{
    union audio_req r;
    r.dec.cmd = AUDIO_DEC_START;
    server_request(music_audio, AUDIO_REQ_DEC, &r);
    puts("continue_music.\n");
    return 0;
}

/*
 * 停止播放音乐,release_scan为是否
 */
static int stop_music(void)
{
    union audio_req r;

    if (music_audio) {
        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(music_audio, AUDIO_REQ_DEC, &r);
        server_close(music_audio);
        music_audio = NULL;
    }
    if (file) {
        fclose(file);
        file = NULL;
    }
    if (__this->fs) {
        fscan_release(__this->fs);
        __this->fs = NULL;
    }
    puts("stop_music.\n");
    return 0;
}

/*
 * 选择文件并播放, mode为选择方式，如FSEL_NEXT_FILE
 */
static int select_file_and_paly(int mode)
{
    union audio_req r;
    static u8 fname[MAX_FILE_NAME_LEN];
    int len, err;

    /*
     * 扫描音乐文件
     */
    if (!__this->fs) {
        __this->fs = fscan(CONFIG_MUSIC_PATH, "-tMP3WMA -sn");
        if (!__this->fs) {
            puts("no music file1!\n");
            return -ENOENT;
        }
    }

    /*
     * 先关闭前一个文件的播放
     */
    if (file) {
        if (music_audio) {
            r.dec.cmd = AUDIO_DEC_STOP;
            server_request(music_audio, AUDIO_REQ_DEC, &r);
            server_close(music_audio);
            music_audio = NULL;
        }
        fclose(file);
        file = NULL;
    }

    /*
     * 选择文件
     */
    file = fselect(__this->fs, mode, 0);
    if (!file) {
        if (mode == FSEL_NEXT_FILE) {
            puts("Next file end.goto first file.\n");
            file = fselect(__this->fs, FSEL_FIRST_FILE, 0);
        } else if (mode == FSEL_PREV_FILE) {
            puts("Prev file head.goto last file.\n");
            file = fselect(__this->fs, FSEL_LAST_FILE, 0);
        }
        if (!file) {
            puts("no music file2!\n");
            if (__this->fs) {
                fscan_release(__this->fs);
                __this->fs = NULL;
            }
            return -ENOENT;
        }
    }

    /*
     * 获取文件名
     */
    /* memset(fname, 0, MAX_FILE_NAME_LEN); */
    /* len = fget_name(file, fname, MAX_FILE_NAME_LEN); */
    /* printf("select music: %s\n", fname); */

    err = play_music(NULL);

    /* r.dec.cmd = AUDIO_DEC_GET_INFO; */
    /* server_request(music_audio, AUDIO_REQ_DEC, &r); */
    /* music_ui_post_msg("info:fname=%p ch=%1 sr=%4 br=%4 tt=%4", */
    /*                   fname, r.dec.info_ret.channel, r.dec.info_ret.sample_rate, */
    /*                   r.dec.info_ret.bit_rate, r.dec.info_ret.total_time); */

    return err;
}

static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int len;
    int ret;
    static u8 fname[MAX_FILE_NAME_LEN];

    switch (state) {
    case APP_STA_CREATE:
        puts("Enter music play mode.\n");
        memset(__this, 0, sizeof_this);
#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EINVAL;
        }
#endif
        show_main_ui();
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_MUSIC_PLAY_MAIN:
            puts("ACTION_MUSIC_PLAY_MAIN.\n");
            if (!strcmp(it->data, "play")) {
                if (it->exdata) {
                    stop_music();
                    file = (FILE *)it->exdata;
                    ret = play_music(NULL);
                }
                if (ret == 0) {
                    __this->status = MUSIC_STA_PLAY;
                } else {
                    __this->status = MUSIC_STA_IDLE;
                }
            } else if (!strcmp(it->data, "getInfo")) {
                if (it->exdata) {
                    union audio_req *r;
                    r = (union audio_req *)it->exdata;
                    r->dec.cmd = AUDIO_DEC_GET_INFO;
                    server_request(music_audio, AUDIO_REQ_DEC, r);
                    /*
                     * 获取文件名
                     */
                    memset(fname, 0, MAX_FILE_NAME_LEN);
                    len = fget_name(file, fname, MAX_FILE_NAME_LEN);
                    printf("len: %d, select music: %s\n", len, fname);
                    put_buf(fname, len);
                    r->dec.info_ret.fname = fname;
                    r->dec.info_ret.name_code = 0;
                    /*
                     * 大于8.3格式的11字符为长文件名
                     */
                    if (len >= 12) {
                        r->dec.info_ret.name_code = 1;
                    }
                }
            }
            break;
        case ACTION_MUSIC_PLAY_SET_CONFIG:
            puts("ACTION_MUSIC_PLAY_SET_CONFIG.\n");
            /* video_music_set_config(it); */
            break;
        }
        break;
    case APP_STA_PAUSE:
        puts("music:APP_STA_PAUSE.\n");
        break;
    case APP_STA_RESUME:
        puts("music:APP_STA_RESUME.\n");
        break;
    case APP_STA_STOP:
        puts("music:APP_STA_STOP.\n");
        if (__this->status == MUSIC_STA_PLAY
            || __this->status == MUSIC_STA_PAUSE) {
            stop_music();
        }
        __this->status = MUSIC_STA_STOP;
        break;
    case APP_STA_DESTROY:
        puts("music:APP_STA_DESTROY.\n");
        hide_main_ui();
#ifdef CONFIG_UI_ENABLE
        puts("--------music close ui\n");
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
static int video_music_key_event_handler(struct key_event *key)
{
    int ret;
    union audio_req r;


    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            puts("music key ok\n");
            if (__this->status == MUSIC_STA_PLAY) {
                pause_music();
                __this->status = MUSIC_STA_PAUSE;
                music_ui_post_msg_async("pp:s=%1", 0);
            } else if (__this->status == MUSIC_STA_PAUSE) {
                continue_music();
                __this->status = MUSIC_STA_PLAY;
                music_ui_post_msg_async("pp:s=%1", 1);
            }
            break;
        case KEY_MENU:
            puts("music key menu\n");
            return true;
            break;
        case KEY_MODE:
            puts("music key mode\n");
            break;
        case KEY_UP:
            puts("music key up\n");
            break;
        case KEY_DOWN:
            puts("music key down\n");
            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_HOLD:
        /* switch (key->value) { */
        /*     case KEY_OK: */
        /*         break; */
        /*     case KEY_UP: */
        /*         puts("music key up hold\n"); */
        /*         if (__this->status == MUSIC_STA_PLAY) { */
        /*             r.dec.cmd = AUDIO_DEC_FR; */
        /*             r.dec.ff_fr_step = 4; */
        /*             server_request(music_audio, AUDIO_REQ_DEC, &r); */
        /*         } */
        /*         break; */
        /*     case KEY_DOWN: */
        /*         puts("music key down hold\n"); */
        /*         if (__this->status == MUSIC_STA_PLAY) { */
        /*             r.dec.cmd = AUDIO_DEC_FF; */
        /*             r.dec.ff_fr_step = 4; */
        /*             server_request(music_audio, AUDIO_REQ_DEC, &r); */
        /*         } */
        /*         break; */
        /*     default: */
        /*         break; */
        /* } */

        break;
    default:
        break;
    }

    return false;
}

/*
 *设备响应函数
 */
static int video_music_device_event_handler(struct sys_event *event)
{
    int err;
    int ret;
    struct intent it;

    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_IN:
            puts("\ncard in\n");
            break;
        case DEVICE_EVENT_OUT:
            puts("\ncard out\n");
            if (__this->status == MUSIC_STA_PLAY
                || __this->status == MUSIC_STA_PAUSE) {
                stop_music();
            }
            __this->status = MUSIC_STA_STOP;
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
        return video_music_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return video_music_device_event_handler(event);
        break;
    default:
        return false;
    }
}



static const struct application_operation video_music_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

REGISTER_APPLICATION(app_video_music) = {
    .name 	= "video_music",
    .action	= ACTION_MUSIC_PLAY_MAIN,
    .ops 	= &video_music_ops,
    .state  = APP_STA_DESTROY,
};







#endif

