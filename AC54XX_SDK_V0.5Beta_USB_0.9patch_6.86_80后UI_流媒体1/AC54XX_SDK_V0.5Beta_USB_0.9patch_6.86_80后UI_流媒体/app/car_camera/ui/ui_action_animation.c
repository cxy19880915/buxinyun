#include "ui/includes.h"
#include "server/ui_server.h"
#include "fs.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"

#include "ani_style.h"
#include "app_config.h"


#ifdef CONFIG_UI_ENABLE

#define STYLE_NAME  ANI

REGISTER_UI_STYLE(STYLE_NAME);

static struct server *video = NULL;
static struct server *audio = NULL;
static FILE *file;

static void boot_video_play_end(void *_priv, int argc, int *argv)
{
    switch (argv[0]) {
    case VIDEO_DEC_EVENT_END:
    case VIDEO_DEC_EVENT_ERR:
        server_request(video, VIDEO_REQ_DEC_STOP, NULL);
        server_close(video);
        fclose(file);
        ui_server_show_completed();
        break;
    default:
        break;
    }
}

static int play_video(const char *name)
{
    union video_dec_req req;

    video = NULL;

    file = fopen(name, "r");
    if (!file) {
        return -ENOENT;
    }


    video = server_open("video_dec_server", "video2");
    if (!video) {
        return -EFAULT;
    }


    req.dec.fb      = "fb1";
    req.dec.left 	= 0;
    req.dec.top 	= 0;
    req.dec.width 	= 0;
    req.dec.height 	= 0;
    req.dec.preview = 0;
    req.dec.file    = file;
    req.dec.audio_p.buf = NULL;
    req.dec.audio_p.buf_len = 10 * 1024;

    server_register_event_handler(video, NULL, boot_video_play_end);

    return server_request(video, VIDEO_REQ_DEC_START, &req);
}

static void play_voice(const char *name)
{
    union audio_req r;

    audio = NULL;

    file = fopen(name, "r");
    if (!file) {
        return;
    }

    audio = server_open("audio_server", "dec");
    if (!audio) {
        fclose(file);
        return;
    }

    r.dec.cmd             = AUDIO_DEC_OPEN;
    r.dec.volume          = -1;
    r.dec.output_buf      = NULL;
    r.dec.output_buf_len  = 1024;
    r.dec.file            = file;
    r.dec.channel         = 1;
    r.dec.sample_rate     = 8000;
    r.dec.priority        = 0;
    server_request(audio, AUDIO_REQ_DEC, &r);

    r.dec.cmd = AUDIO_DEC_START;
    server_request(audio, AUDIO_REQ_DEC, &r);
}

static void stop_voice()
{
    union audio_req r;

    if (audio) {
        r.dec.cmd = AUDIO_DEC_STOP;
        server_request(audio, AUDIO_REQ_DEC, &r);
        server_close(audio);
        fclose(file);
    }
}




static int poweron_animation_handler(void *_ani, enum element_change_event e, void *arg)
{
    int err;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        err = play_video("mnt/spiflash/audlogo/boot.mov");
        if (err == 0) {
            ui_hide(ANI_ID_BASEFORM);
            return true;
        }
        play_voice("mnt/spiflash/audlogo/poweron.adp");
       
        break;
    case ON_CHANGE_ANIMATION_END:
        stop_voice();
        ui_server_show_completed();
        break;
    default:
        return video ? true : false;
    }
    return false;
}

REGISTER_UI_EVENT_HANDLER(ANI_ID_POWER_ON)
.onchange = poweron_animation_handler,
};

static int poweroff_animation_handler(void *_ani, enum element_change_event e, void *arg)
{
    int err;

    switch (e) {
    case ON_CHANGE_FIRST_SHOW:
        err = play_video("mnt/spiflash/audlogo/off.mov");
        if (err == 0) {
            ui_hide(ANI_ID_BASEFORM_3);
            return true;
        }
        play_voice("mnt/spiflash/audlogo/poweroff.adp");
        break;
    case ON_CHANGE_ANIMATION_END:
        stop_voice();
        ui_server_show_completed();
        break;
    default:
        return video ? true : false;
    }

    return false;
}

REGISTER_UI_EVENT_HANDLER(ANI_ID_POWER_OFF)
.onchange = poweroff_animation_handler,
};


#endif
