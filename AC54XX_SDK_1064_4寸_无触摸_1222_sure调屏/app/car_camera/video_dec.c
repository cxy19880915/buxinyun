#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "video_dec.h"

#include "action.h"
#include "style.h"
#include "app_config.h"

const char *dec_path[DEC_PATH_NUM][2] = {
    { CONFIG_DEC_PATH_1, "-tMOVJPGAVI -sn" },
    { CONFIG_DEC_PATH_2, "-tMOVJPGAVI -sn" },
    /*{ CONFIG_DEC_PATH_3, "-tJPG -sn" },*/
};
extern int video_dec_change_status(struct video_dec_hdl *, struct intent *it);
extern int video_dec_set_config(struct video_dec_hdl *, struct intent *it);
extern int video_dec_get_config(struct intent *it);
extern int dec_set_config(struct video_dec_hdl *, struct intent *it);

struct video_dec_hdl dec_handler;

#define __this 	(&dec_handler)


/*
 * 发送一些状态给ui
 */
void video_dec_post_msg(const char *msg, ...)
{
#ifdef CONFIG_UI_ENABLE
    union uireq req;
    va_list argptr;

    va_start(argptr, msg);

    if (__this->ui) {
        req.msg.receiver = ID_WINDOW_VIDEO_REP;
        req.msg.msg = msg;
        req.msg.exdata = argptr;
        server_request(__this->ui, UI_REQ_MSG, &req);
    }

    va_end(argptr);
#endif

}

/*
 * 发送文件的信息给ui显示
 */
void video_dec_post_file_info_to_ui(int fname_len, int format)
{
#ifdef CONFIG_UI_ENABLE
    int attr;

    /*
     *发送文件名信息，如果文件名长度超过12byte，则为utf16编码格式
     */
    if (fname_len) {
        if (format == 0) {
            ASCII_ToUpper(__this->fname, fname_len);
            video_dec_post_msg("fname:ascii=%p", __this->fname);
        } else {
            video_dec_post_msg("fname:utf16=%p", __this->fname);
        }
    }

    /*
     * 获取文件属性，看是否为写保护文件
     */
    fget_attr(__this->req.dec.file, &attr);
    video_dec_post_msg("fattr:ro=%1", !!(attr & F_ATTR_RO));

    /*
     * 发送分辨率和影片时长信息
     */
    video_dec_post_msg("res:w=%2 h=%2,filmLen:s=%4", __this->req.dec.info.width,
                       __this->req.dec.info.height, __this->req.dec.info.total_time);


    /*
     * 发送当前解码状态信息
     */
    if (__this->file_type == FILE_TYPE_MOVE) {
        if (__this->status == VIDEO_DEC_PAUSE) {
            video_dec_post_msg("pause");
        } else if (__this->status == VIDEO_DEC_PLAYING) {
            video_dec_post_msg("play");
        }
    }

#endif
}


/*
 * 打开文件并启动解码
 */
int video_dec_file(int mode)
{
    int err;
    struct vfscan *fs = __this->fs[__this->curr_dir];

    if (!__this->video_dec) {
        return -EFAULT;
    }

    if (!fs) {
        video_dec_post_msg("noFile");
        return -ENOENT;
    }

    if (__this->req.dec.file) {
        server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
        fclose(__this->req.dec.file);
    }

    /*
     * 选择并打开一个文件, 如果fselect返回成功，则此文件不用时需要调用fclose关闭
     */
    __this->req.dec.file = fselect(fs, mode, 0);
    if (!__this->req.dec.file) {
        /*
         *目录内循环播放
         */
        if (mode == FSEL_NEXT_FILE) {
            __this->req.dec.file = fselect(fs, FSEL_FIRST_FILE, 0);
        } else if (mode == FSEL_PREV_FILE) {
            __this->req.dec.file = fselect(fs, FSEL_LAST_FILE, 0);
        }

        if (!__this->req.dec.file) {
            video_dec_post_msg("noFile");
            fscan_release(__this->fs[__this->curr_dir]);
            __this->fs[__this->curr_dir] = NULL;
            return -ENOENT;
        }
    }

    /*
     * 启动解码
     */
    err = server_request(__this->video_dec, VIDEO_REQ_DEC_START, &__this->req);
    if (err) {
        video_dec_post_msg("fileErr");
        /*
         * 遇到坏损文件无法播放时, 等待1s后播放下一个文件,
         * 如果此处时间设置过短且坏损文件过多时可能造成无法响应按键消息
         */
        if (mode == FSEL_FIRST_FILE) {
            mode = FSEL_NEXT_FILE;
        } else if (mode == FSEL_LAST_FILE) {
            mode = FSEL_PREV_FILE;
        }
        if (__this->status != VIDEO_DEC_STOP) {
            __this->timeout = sys_timeout_add((void *)mode, (void(*)(void *))video_dec_file, 1000);
        }
    } else {
        if (__this->req.dec.preview) {
            __this->status = VIDEO_DEC_PAUSE;
        } else {
            __this->status = VIDEO_DEC_PLAYING;
        }

        int format = 0;
        int len = fget_name(__this->req.dec.file, __this->fname, MAX_FILE_NAME_LEN);
        if (len) {
            if (!ASCII_StrCmpNoCase((char *)(__this->fname + len - 3), "JPG", 3)) {
                __this->file_type = FILE_TYPE_JPG;
            } else {
                __this->file_type = FILE_TYPE_MOVE;
            }
        }

        video_dec_post_file_info_to_ui(len, format);
    }

    return err;
}

/*
 * 文件夹扫描
 */
void video_dec_scan_dir()
{
    int curr_dir;

    if (__this->curr_dir >= DEC_PATH_NUM) {
        __this->curr_dir = 0;
    } else if (__this->curr_dir < 0) {
        __this->curr_dir = DEC_PATH_NUM - 1;
    }

    curr_dir = __this->curr_dir;

    if (!__this->video_dec) {
        return;
    }

    while (1) {
        if (!__this->fs[__this->curr_dir]) {
            __this->fs[__this->curr_dir] = fscan(dec_path[__this->curr_dir][0],
                                                 dec_path[__this->curr_dir][1]);
        }
        if (__this->fs[__this->curr_dir]) {
            break;
        }
        if (++__this->curr_dir == DEC_PATH_NUM) {
            __this->curr_dir = 0;
        }
        if (__this->curr_dir == curr_dir) {
            break;
        }
    }

    video_dec_file(FSEL_LAST_FILE);
}

/*
 * video_decoder server的事件回调函数
 */
static void dec_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case VIDEO_DEC_EVENT_CURR_TIME:
        /*
         *发送当前播放时间给UI
         */
        video_dec_post_msg("plyTime:s=%4", argv[1]);
        break;
    case VIDEO_DEC_EVENT_END:
#ifdef CONFIG_TOUCH_UI_ENABLE
        /*
         *解码结束，播放当前文件
         */
        video_dec_post_msg("end");
        if (__this->req.dec.file) {
            server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
            fseek(__this->req.dec.file, 0, SEEK_SET);
        }
        dec_open_file();
#else
        /*
         *解码结束，播放前一个文件
         */
        video_dec_file(FSEL_PREV_FILE);
#endif
        break;
    case VIDEO_DEC_EVENT_ERR:
        /*
         *解码出错，如果存储设备没有被拔出则播放前一个文件
         */
        if (fdir_exist(CONFIG_STORAGE_PATH)) {
            video_dec_file(FSEL_PREV_FILE);
        }
        break;
    }
}


/*
 *sd卡挂载成功后会调用此函数
 */
static int video_dec_start(void *p)
{
    __this->wait = 0;

    /*
     * 打开video_dec_server, 注册事件回调函数
     */
    if (!__this->video_dec) {
        __this->video_dec = server_open("video_dec_server", "video2");
        if (!__this->video_dec) {
            video_dec_post_msg("hwErr");
            return -EFAULT;
        }
        server_register_event_handler(__this->video_dec, NULL, dec_server_event_handler);
    }

    __this->req.dec.fb 		= "fb1";
    /*
     * 下面4个参数为解码出来的视频在屏幕上的起始坐标和长宽, 长宽配0即为显示屏大小
     */
#ifdef CONFIG_TOUCH_UI_ENABLE
    __this->req.dec.left 	= 224;
    __this->req.dec.top 	= 0;
    __this->req.dec.width 	= 768;// 0;
    __this->req.dec.height 	= 480;// 0;
#else
    __this->req.dec.left 	= 0;
    __this->req.dec.top 	= 0;
    __this->req.dec.width 	= 800;
    __this->req.dec.height 	= 480;
    __this->req.dec.border_right = 0;
    __this->req.dec.border_bottom = 1; // 0;
#endif

    /*
     * jpeg图片优先解缩略图标志
     */
    __this->req.dec.thm_first = 0;

    /*
      * 是否预览，设0即为自动播放
      */
    __this->req.dec.preview = 1;
    /*
     * 音频数据存放的buf地址和长度，buf设NULL则video_dec_server会根据长度自己申请
     */
    __this->req.dec.audio_p.buf = NULL;
    __this->req.dec.audio_p.buf_len = 10 * 1024;

    __this->curr_dir = 0;
    video_dec_scan_dir();

    return 0;
}

/*
 * 播放和暂停控制
 */
int video_dec_play_pause()
{
    int err;

    if (!__this->video_dec) {
        return -EFAULT;
    }

    if (!__this->req.dec.file) {
        return -ENOENT;
    }

    err = server_request(__this->video_dec, VIDEO_REQ_DEC_PLAY_PAUSE, &__this->req);
    if (err) {
        return -EFAULT;
    }

    if (__this->status != VIDEO_DEC_PLAYING) {
        __this->status = VIDEO_DEC_PLAYING;
        video_dec_post_msg("play");
    } else {
        __this->status = VIDEO_DEC_PAUSE;
        video_dec_post_msg("pause");
    }

    if (__this->timeout) {
        sys_timeout_del(__this->timeout);
        __this->timeout = 0;
    }

    return 0;
}

/*
 * 快进快退
 */
static int video_dec_ff_fr(int ff_fr_req)
{
    int err;
    union video_dec_req req;

    if (!__this->video_dec) {
        return -EFAULT;
    }

    if (!__this->req.dec.file) {
        return -ENOENT;
    }

    __this->ff_fr_times <<= 1;
    if (__this->ff_fr_times > 8) {
        __this->ff_fr_times = 2;
    }

    /*
     * on = true:开始快进/快退, on = false:停止快进/快退
     * times: 快进/快退的倍数, once = false时有效
     * once:  一次快进/快退，和msec_step配合，用于直接快进/快退到某个时间点
     */
    req.f.on = true;
    req.f.once = false;
    req.f.msec_step = 0;
    req.f.times = __this->ff_fr_times;
    err = server_request(__this->video_dec, ff_fr_req, &req);
    if (err) {
        return -EFAULT;
    }

    if (ff_fr_req == VIDEO_REQ_DEC_FF) {
        video_dec_post_msg("ff:t=%1", __this->ff_fr_times);
    } else {
        video_dec_post_msg("fr:t=%1", __this->ff_fr_times);
    }

    return 0;
}

/*
 * 停止解码
 */
static int video_dec_stop(int release)
{
    int i;

    __this->status = VIDEO_DEC_STOP;

    if (!__this->video_dec) {
        return 0;
    }

    if (__this->req.dec.file) {
        server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
        fclose(__this->req.dec.file);
        __this->req.dec.file = NULL;
    }

    /*
     * 释放文件扫描的资源
     */
    if (release) {
        for (i = 0; i < DEC_PATH_NUM; i++) {
            if (__this->fs[i]) {
                fscan_release(__this->fs[i]);
                __this->fs[i] = NULL;
            }
        }
    }

    if (__this->timeout) {
        sys_timeout_del(__this->timeout);
        __this->timeout = 0;
    }

    return 0;
}

#ifdef CONFIG_TOUCH_UI_ENABLE
int dec_open_file()
{
    int err;
    int attr;

    if (!__this->video_dec) {
        __this->video_dec = server_open("video_dec_server", "video2");
        if (!__this->video_dec) {
            video_dec_post_msg("hwErr");
            return -EFAULT;
        }
        server_register_event_handler(__this->video_dec, NULL, dec_server_event_handler);
    }

    if (!__this->req.dec.file) {
        server_close(__this->video_dec);
        __this->video_dec = NULL;
        video_dec_post_msg("noFile");
        return -ENOENT;
    }

    __this->req.dec.fb 		= "fb1";
    /*
     * 下面4个参数为解码出来的视频在屏幕上的起始坐标和长宽, 长宽配0即为显示屏大小
     */
    __this->req.dec.left 	= 224;
    __this->req.dec.top 	= 0;
    __this->req.dec.width 	= 768;// 0;
    __this->req.dec.height 	= 480;// 0;

    /*
     * 是否预览，设0即为自动播放
     */
    __this->req.dec.preview = 1;
    /*
     * 音频数据存放的buf地址和长度，buf设NULL则video_dec_server会根据长度自己申请
     */
    __this->req.dec.audio_p.buf = NULL;
    __this->req.dec.audio_p.buf_len = 10 * 1024;


    /*
     * 启动解码
     */
    err = server_request(__this->video_dec, VIDEO_REQ_DEC_START, &__this->req);
    if (err) {
        video_dec_post_msg("fileErr");
    } else {
        if (__this->req.dec.preview) {
            __this->status = VIDEO_DEC_PAUSE;
        } else {
            __this->status = VIDEO_DEC_PLAYING;
        }
        int len = fget_name(__this->req.dec.file, __this->fname, MAX_FILE_NAME_LEN);
        if (len) {
            if (!ASCII_StrCmpNoCase((char *)(__this->fname + len - 3), "JPG", 3)) {
                __this->file_type = FILE_TYPE_JPG;
            } else {
                __this->file_type = FILE_TYPE_MOVE;
            }

            /*
             *发送文件名信息，如果文件名长度超过12byte，则为utf16编码格式
             */
            /*if (format == 0) {*/
            ASCII_ToUpper(__this->fname, len);
            video_dec_post_msg("fname:ascii=%p", __this->fname);
            /*} else {
                video_dec_post_msg("fname:utf16=%p", __this->fname);
            }*/
        }
        /*
         * 发送分辨率和影片时长信息
         */
        video_dec_post_msg("res:w=%2 h=%2,filmLen:s=%4", __this->req.dec.info.width,
                           __this->req.dec.info.height, __this->req.dec.info.total_time);

        /*
         * 发送当前解码状态信息
         */
        if (__this->file_type == FILE_TYPE_MOVE) {
            if (__this->status == VIDEO_DEC_PAUSE) {
                video_dec_post_msg("pause");
            } else if (__this->status == VIDEO_DEC_PLAYING) {
                video_dec_post_msg("play");
            }
        }
    }
    return err;
}

#endif //CONFIG_TOUCH_UI_ENABLE

/*
 * 解码任务的状态机函数，由start_app负责调用
 */
static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int ret = 0;

    switch (state) {
    case APP_STA_CREATE:
        server_load(video_dec_server);
        memset(__this, 0, sizeof(struct video_dec_hdl));

#ifdef CONFIG_UI_ENABLE
        {
            /*
             *打开UI服务，请求显示界面
             */
            union uireq req;
            __this->ui = server_open("ui_server", 0);
            if (__this->ui) {
                req.show.id = ID_WINDOW_VIDEO_REP;
                server_request_async(__this->ui, UI_REQ_SHOW, &req);
            }
        }
#endif
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_VIDEO_DEC_MAIN:
            /*
             *等待存储设备(sd卡)挂载完成，然后自动调用video_dec_start
             */
#ifndef CONFIG_TOUCH_UI_ENABLE
            __this->wait = wait_completion(storage_device_ready, video_dec_start, NULL);
#endif
            break;
        case ACTION_VIDEO_DEC_CHANGE_STATUS:
            ret = video_dec_change_status(__this, it);
            break;
        case ACTION_VIDEO_DEC_SET_CONFIG:
            if (!__this->ui) {
                return -EINVAL;
            }
#ifdef CONFIG_TOUCH_UI_ENABLE
            ret = dec_set_config(__this, it);
#else
            ret = video_dec_set_config(__this, it);
#endif
            break;
#ifdef CONFIG_TOUCH_UI_ENABLE
        case ACTION_VIDEO_DEC_OPEN_FILE:
            if (!__this->req.dec.file) {
                __this->req.dec.file = (FILE *)it->data;
            }
            __this->dir_type = it->exdata;
            dec_open_file();
            break;
        case ACTION_VIDEO_DEC_CONTROL:
            video_dec_play_pause();
            break;
        case ACTION_VIDEO_DEC_CUR_PAGE:
            if (__this->req.dec.file) {
                server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
                server_close(__this->video_dec);
                __this->video_dec = NULL;
                fclose(__this->req.dec.file);
                __this->req.dec.file = NULL;
                __this->status = VIDEO_DEC_STOP;
            }
            break;
#endif
        }
        break;
    case APP_STA_PAUSE:
        if (!strcmp(it->data, "frm")) {
            video_dec_stop(1);
        }
        break;
    case APP_STA_RESUME:
        if (__this->status == VIDEO_DEC_STOP) {
            video_dec_scan_dir();
        }
        break;
    case APP_STA_STOP:
        video_dec_stop(1);
        if (__this->wait) {
            wait_completion_del(__this->wait);
        }
			//show_main_ui();
        break;

    case APP_STA_DESTROY:
        if (__this->video_dec) {
            server_close(__this->video_dec);
            __this->video_dec = NULL;
        }

#ifdef CONFIG_UI_ENABLE
        {
            union uireq req;
            if (__this->ui) {
                req.hide.id = ID_WINDOW_VIDEO_REP;
                server_request(__this->ui, UI_REQ_HIDE, &req);
                server_close(__this->ui);
                __this->ui = NULL;
            }

        }
#endif
        break;
    }


    return ret;
}

/*
 * 按键消息处理函数
 */
static int video_dec_key_event_handler(struct key_event *key)
{
    int err;
    struct intent it;

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            video_dec_play_pause();
            break;
        case KEY_UP:
            if (__this->status == VIDEO_DEC_PAUSE ||
                __this->status == VIDEO_DEC_STOP) {
                video_dec_file(FSEL_PREV_FILE);
            } else {
                if (__this->status != VIDEO_DEC_FR) {
                    __this->ff_fr_times = 1;
                }
                err = video_dec_ff_fr(VIDEO_REQ_DEC_FR);
                if (err == 0) {
                    __this->status = VIDEO_DEC_FR;
                }
            }
            break;
        case KEY_DOWN:
            if (__this->status == VIDEO_DEC_PAUSE ||
                __this->status == VIDEO_DEC_STOP) {
                video_dec_file(FSEL_NEXT_FILE);
            } else {
                if (__this->status != VIDEO_DEC_FF) {
                    __this->ff_fr_times = 1;
                }
                err = video_dec_ff_fr(VIDEO_REQ_DEC_FF);
                if (err == 0) {
                    __this->status = VIDEO_DEC_FF;
                }
            }
            break;
        case KEY_MODE:
            if (__this->status  == VIDEO_DEC_PLAYING
                || __this->status  == VIDEO_DEC_FF
                || __this->status  == VIDEO_DEC_FR) {
                if (__this->status  == VIDEO_DEC_FF
                    || __this->status  == VIDEO_DEC_FR) {
                    video_dec_play_pause();
                    os_time_dly(100);
                }
                video_dec_stop(0);
                video_dec_file(FSEL_NEXT_FILE);
                return true;
            }

            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_LONG:
        switch (key->value) {
        case KEY_UP:
            video_dec_stop(0);
            __this->curr_dir++;
            video_dec_scan_dir();
            break;
        case KEY_DOWN:
            video_dec_stop(0);
            __this->curr_dir--;
            video_dec_scan_dir();
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

/*
 * 设备消息处理函数
 */
static int video_dec_device_event_handler(struct sys_event *event)
{
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        if (!fdir_exist(CONFIG_STORAGE_PATH)) {
#ifdef CONFIG_TOUCH_UI_ENABLE
            if (__this->status != VIDEO_DEC_STOP) {
                //进入了解码才关闭dec
                if(__this->video_dec)  //?????ж?
                {
                    server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
                    server_close(__this->video_dec);
                    __this->video_dec = NULL;
                }
                if(__this->req.dec.file)  //?????ж?
                {
                    fclose(__this->req.dec.file);
                    __this->req.dec.file = NULL;
                }
                
                __this->status = VIDEO_DEC_STOP;
                video_dec_post_msg("noFile");
            }
#else
            video_dec_stop(1);
            video_dec_post_msg("noFile");
            if (!__this->wait) {
                __this->wait = wait_completion(storage_device_ready, video_dec_start, NULL);
            }
#endif
        }
        break;
    }

    return false;
}


static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return video_dec_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return video_dec_device_event_handler(event);
    default:
        return false;
    }
}
static const struct application_operation video_dec_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册video_dec app
 *
 */
REGISTER_APPLICATION(app_video_dec) = {
    .name 	= "video_dec",
    .action	= ACTION_VIDEO_DEC_MAIN,
    .ops 	= &video_dec_ops,
    .state  = APP_STA_DESTROY,
};











