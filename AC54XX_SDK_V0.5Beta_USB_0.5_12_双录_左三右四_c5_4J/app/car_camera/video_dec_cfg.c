#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"

#include "video_dec.h"
#include "action.h"
#include "style.h"
#include "ui_ctr_def.h"
#include "app_config.h"
#include "storage_device.h"






/*
 * 删除当前文件
 */
static int video_dec_del_current_file(struct video_dec_hdl *__this)
{
    int attr;

    if (!__this->req.dec.file) {
        return 0;
    }

    fget_attr(__this->req.dec.file, &attr);
    if (attr & F_ATTR_RO) {
        return -EPERM;
    }

    server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
    fdelete(__this->req.dec.file);
    __this->req.dec.file = NULL;

    video_dec_file(FSEL_PREV_FILE);

    return 0;
}

/*
 * 删除所有文件
 */
static int video_dec_del_all_file(struct video_dec_hdl *__this)
{
    int err;
    int attr;
    FILE *file;
    struct vfscan *fs = __this->fs[__this->curr_dir];

    if (!__this->req.dec.file) {
        return 0;
    }

    server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
    fclose(__this->req.dec.file);
    __this->req.dec.file = NULL;
    __this->status = VIDEO_DEC_STOP;

    file = fselect(fs, FSEL_FIRST_FILE, 0);

    while (file) {
        fget_attr(file, &attr);
        if (attr & F_ATTR_RO) {
            err = fclose(file);
            if (err) {
                break;
            }
        } else {
            err = fdelete(file);
            if (err) {
                break;
            }
        }
        file = fselect(fs, FSEL_NEXT_FILE, 0);
    }

    fscan_release(fs);
    __this->fs[__this->curr_dir] = NULL;
    video_dec_scan_dir();

    return 0;
}


/*
 * 加锁当前文件
 */
static void video_dec_lock_current_file(struct video_dec_hdl *__this, bool lock)
{
    int attr;

    if (!__this->req.dec.file) {
        return;
    }

    fget_attr(__this->req.dec.file, &attr);

    if (lock) {
        if (attr & F_ATTR_RO) {
            return;
        }
        attr |= F_ATTR_RO;
    } else {
        if (!(attr & F_ATTR_RO)) {
            return;
        }
        attr &= ~F_ATTR_RO;
    }

    fset_attr(__this->req.dec.file, attr);

    video_dec_post_msg("fattr:ro=%1", lock);
}

/*
 * 加锁/解锁所有文件
 */
static void video_dec_lock_all_file(struct video_dec_hdl *__this, bool lock)
{
    int attr;
    FILE *file;
    struct vfscan *fs;

    fs = fscan(dec_path[__this->curr_dir][0], dec_path[__this->curr_dir][1]);

    if (!fs || fs->file_number == 0) {
        return;
    }
    file = fselect(fs, FSEL_FIRST_FILE, 0);

    while (file) {
        fget_attr(file, &attr);

        if (lock) {
            if (!(attr & F_ATTR_RO)) {
                attr |= F_ATTR_RO;
                fset_attr(file, attr);
            }
        } else {
            if (attr & F_ATTR_RO) {
                attr &= ~F_ATTR_RO;
                fset_attr(file, attr);
            }
        }

        fclose(file);

        file = fselect(fs, FSEL_NEXT_FILE, 0);
    }

    fscan_release(fs);
}


/*
 * 处理UI菜单操作请求
 */
int video_dec_set_config(struct video_dec_hdl *__this, struct intent *it)
{
    int err = 0;

    if (!strcmp(it->data, "del:cur")) {
        err = video_dec_del_current_file(__this);
    } else if (!strcmp(it->data, "del:all")) {
        err = video_dec_del_all_file(__this);
    } else if (!strcmp(it->data, "lock:cur")) {
        video_dec_lock_current_file(__this, true);
    } else if (!strcmp(it->data, "unlock:cur")) {
        video_dec_lock_current_file(__this, false);
    } else if (!strcmp(it->data, "lock:all")) {
        video_dec_lock_all_file(__this, true);
    } else if (!strcmp(it->data, "unlock:all")) {
        video_dec_lock_all_file(__this, false);
    } else if (!memcmp(it->data, "ppt:", 4)) {
        /*
         * 幻灯片播放, time = 0为停止播放
         */
        u32 time = (it->data[4] - '0') * 1000;
        if (time == 0) {
            sys_timer_del(__this->timer);
            video_dec_post_file_info_to_ui();
        } else {
            __this->timer = sys_timer_add((void *)FSEL_PREV_FILE,
                                          (void (*)(void *))video_dec_file, time);
        }
    }
    return err;
}


int video_dec_change_status(struct video_dec_hdl *__this, struct intent *it)
{
    if (!strcmp(it->data, "openMenu")) {
        /*
         * ui请求打开菜单, 如果没有文件则禁止打开
         */
        if (!__this->fs[__this->curr_dir]) {
            it->data = "dis";
        } else {
            if (__this->status == VIDEO_DEC_PLAYING) {
                video_dec_play_pause();
            }
            it->data = "en";
        }

    } else if (!strcmp(it->data, "exitMenu")) {

    }

    return 0;
}

#ifdef CONFIG_TOUCH_UI_ENABLE
/*
 * dec 文件操作
 * */
static void dec_lock_cur_file(struct video_dec_hdl *__this, struct intent *it, bool lock)
{
    int attr;

    if (!__this->req.dec.file) {
        return;
    }

    fget_attr(__this->req.dec.file, &attr);

    if (lock) {
        if (attr & F_ATTR_RO) {
            return;
        }
        attr |= F_ATTR_RO;
    } else {
        if (!(attr & F_ATTR_RO)) {
            return;
        }
        attr &= ~F_ATTR_RO;
    }

    fset_attr(__this->req.dec.file, attr);

    video_dec_post_msg("fattr:ro=%1", lock);


}

static void dec_lock_all_file(struct video_dec_hdl *__this, struct intent *it, bool lock)
{
    int attr;
    FILE *file;
    struct vfscan *fs;

    int data;
    u8 curr_dir, type;
    data = it->exdata;
    curr_dir = data / 10;
    type = data % 10;


    printf("\n unlock all ()%s \n", __FUNCTION__);
    if (curr_dir == 0) {
        switch (type) {
        case VIDEO_FILE:
            fs = fscan(CONFIG_DEC_PATH_1, "-tMOV -sn");
            break;
        case PHOTO_FILE:
            fs = fscan(CONFIG_DEC_PATH_1, "-tJPG -sn");
            break;
        case LOCK_FILE:
            fs = fscan(CONFIG_DEC_PATH_1, "-tMOVJPG -sn -ar");
            break;
        }
    } else {
        switch (type) {
        case VIDEO_FILE:
            fs = fscan(CONFIG_DEC_PATH_2, "-tMOV -sn");
            break;
        case PHOTO_FILE:
            fs = fscan(CONFIG_DEC_PATH_2, "-tJPG -sn");
            break;
        case LOCK_FILE:
            fs = fscan(CONFIG_DEC_PATH_2, "-tMOVJPG -sn -ar");
            break;
        }
    }

    if (!fs || fs->file_number == 0) {
        return;
    }
    file = fselect(fs, FSEL_FIRST_FILE, 0);

    while (file) {
        fget_attr(file, &attr);

        if (lock) {
            if (!(attr & F_ATTR_RO)) {
                attr |= F_ATTR_RO;
                fset_attr(file, attr);
            }
        } else {
            if (attr & F_ATTR_RO) {
                attr &= ~F_ATTR_RO;
                fset_attr(file, attr);
            }
        }
        fclose(file);
        file = fselect(fs, FSEL_NEXT_FILE, 0);
    }

    fscan_release(fs);

}

static int dec_del_cur_file(struct video_dec_hdl *__this, struct intent *it)
{
    int attr;
    server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
    if(__this->video_dec)
    {
        server_close(__this->video_dec);
        __this->video_dec = NULL;
    }
    if(__this->req.dec.file)
    {
        fdelete(__this->req.dec.file);
        __this->req.dec.file = NULL;
    }
    __this->status = VIDEO_DEC_STOP;  //Ôö¼Ó

    return 0;
}
/*
 * 删除所有文件
 */
static int dec_del_all_file(struct video_dec_hdl *__this, struct intent *it)
{
    int err;
    int attr;
    FILE *file;
    struct vfscan *fs ;

    const static char *cTYPE[] = {
        "-tMOV -sn -d",
        "-tJPG -sn -d",
        "-tMOVJPG -sn -ar -d",//只读文件
    };
    if (it->exdata < 2) {
        fs = fscan((it->data + strlen("delall:")), cTYPE[it->exdata]);
    } else {
        return -EINVAL;
    }

    if (__this->req.dec.file) {
        server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
        fclose(__this->req.dec.file);
        __this->req.dec.file = NULL;
        __this->status = VIDEO_DEC_STOP;
    }

    file = fselect(fs, FSEL_FIRST_FILE, 0);


    while (file) {
        fget_attr(file, &attr);
        if (attr & F_ATTR_RO) {
            err = fclose(file);
            if (err) {
                break;
            }
        } else if (attr & F_ATTR_DIR) {
            puts("is dir\n");
            err = fclose(file);
            if (err) {
                break;
            }
        } else {
            err = fdelete(file);
            if (err) {
                break;
            }
        }

        file = fselect(fs, FSEL_NEXT_FILE, 0);
    }

    fscan_release(fs);

    return 0;
}

/*
 *  * 删除文件夹
 *   */
static int dec_del_dir(struct video_dec_hdl *__this, struct intent *it)
{
    int err;
    int attr;
    FILE *file;
    struct vfscan *fs ;
    u8 dir_num;
    u8 file_num;
    char cur_path[128];
    const static char *cTYPE[] = {
        "-tMOV -sn",
        "-tJPG -sn",
        "-tMOVJPG -sn -ar",//只读文件
    };

    if (__this->req.dec.file) {
        server_request(__this->video_dec, VIDEO_REQ_DEC_STOP, &__this->req);
        fclose(__this->req.dec.file);
        __this->req.dec.file = NULL;
        __this->status = VIDEO_DEC_STOP;
    }

    strcpy(cur_path, it->data + strlen("deldir:"));

    fs = fscan(cur_path, "-t -sn -d"); //扫出文件夹
    dir_num = fs->file_number;
    fscan_release(fs);
    fs = fscan(cur_path, "-tMOVJPG -sn -ar -d"); //扫出只读文件
    file_num = fs->file_number;
    fscan_release(fs);

    if (dir_num == 0) {
        if (0 == file_num) {
            //没有只读文件直接删除文件夹
            err = fdelete_by_name(cur_path);
            if (!err) {
                goto __del_end;
            }
        }
    }

    //有只读文件逐个删除
    fs = fscan(cur_path, cTYPE[it->exdata]);

    file = fselect(fs, FSEL_FIRST_FILE, 0);

    while (file) {
        fget_attr(file, &attr);

        if (attr & F_ATTR_RO) {
            err = fclose(file);
            if (err) {
                break;
            }
        } else {
            err = fdelete(file);
            if (err) {
                break;
            }
        }

        file = fselect(fs, FSEL_NEXT_FILE, 0);
    }

__del_end:
    fscan_release(fs);

    return 0;
}

int dec_set_config(struct video_dec_hdl *__this, struct intent *it)
{
    int err = 0;
    if (!strcmp(it->data, "del:cur")) {
        err = dec_del_cur_file(__this, it);
    } else if (!strncmp(it->data, "delall:", 7)) {
        err = dec_del_all_file(__this, it);
    } else if (!strcmp(it->data, "lock:cur")) {
        dec_lock_cur_file(__this, it, true);
    } else if (!strcmp(it->data, "unlock:cur")) {
        dec_lock_cur_file(__this, it, false);
    } else if (!strcmp(it->data, "unlock:all")) {
        dec_lock_all_file(__this, it, false);
    } else if (!strcmp(it->data, "lock:all")) {

    } else if (!strncmp(it->data, "deldir:", 7)) {
        err = dec_del_dir(__this, it);
    }
    return err;
}
#endif

