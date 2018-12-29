#include "fs.h"
#include "server/server_core.h"
#include "system/includes.h"
#include "os/os_compat.h"
#include "server/net_server.h"
#include "server/ctp_server.h"
#include "http/http_server.h"
#include "storage_device.h"
#include "server/vunpkg_server.h"
#include "fs/fs.h"
#include "lwip/sockets.h"
#include "server/simple_mov_unpkg.h"
//#include "rt_stream_pkg.h"
/* #include "action.h" */
#include "app_config.h"

extern void *memmem(const void *__haystack, size_t __haystacklen,
                    const void *__needle, size_t __needlelen);

extern int http_virfile_reg(const char *path, const char *contents, unsigned long len);



#define VIDEO_FILE_LIST_JSON_HEAD  "{\"file_list\":["
#define VIDEO_JSON_MEM   "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\",\"c\":\"%d\"},"
#define VIDEO_JSON_APP   "{\"op\":\"NOTIFY\",\"param\":{\"status\":\"%d\",\"desc\":\"{\\\"y\\\":%d,\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"d\\\":\\\"%d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"p\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\"}\"}}"
#define PICTURE_JSON_MEM   "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\",\"c\":\"%d\"},"
#define PICTURE_JSON_APP    "{\"op\":\"NOTIFY\",\"param\":{\"desc\":\"{\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\"}\"}}"
#define MAX_NUM  600
#define INFO_LEN 256
#define FILE_MAX_NUM    99999



const char __fs_arg[3][15] = {  "-tMOV -st",
                                "-tJPG  -st",
                                "-tMOVJPG -st"
                             };


struct file_info {
    struct list_head entry;
    u32 id;
    u32 len;
    u8(*fd)[INFO_LEN];
};



struct media_file_info {
    FILE *fd;
    u32  vaild;
    int _attr;
    struct vfs_attr attr;
    struct __mov_unpkg_info info;
    u8 channel; // 前后视区分
    u8 type;// 1视频  2 图片  4，，，
    u8 namebuf[64];
    u16 height; //use in jpg
    u16 width; // use in jpg

};



static struct list_head forward_file_list_head;
static struct list_head behind_file_list_head;
static int FMAX_ID = 0;
static int BMAX_ID = 0;
static u32 initing = 0;
static u8 forward_file_mem[MAX_NUM + 2][INFO_LEN];
static u8 behind_file_mem[MAX_NUM + 2][INFO_LEN];
static u8 f_path[64];
static u8 b_path[64];
static OS_MUTEX file_list_mutex = OS_MUTEX_INIT();
static u32 mutex_init = 0;

int send_json(struct media_file_info *__info, u32 status);



static u8 *__find_emtry_block_f(struct file_info *info)
{
    int i = 1;
    static int f_jpg_id = 1;
    u8 *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    for (i = 1; i < MAX_NUM; i++) {
        if (forward_file_mem[i][0] != '{') {
            printf("forward_file_mem[%d]\n", i);
            info->id = i;
            FMAX_ID = MAX(FMAX_ID, i);
            os_mutex_post(&file_list_mutex);
            return forward_file_mem[i];
        }
    }
    //如果没有找到空
    while (f_jpg_id < MAX_NUM) {
        if (memmem(forward_file_mem[f_jpg_id], INFO_LEN, "JPG", 3)) {
            os_mutex_post(&file_list_mutex);
            printf("delete jpg to set ,forward_file_mem[%d]\n", f_jpg_id);
            str = forward_file_mem[f_jpg_id];

        } else {
            printf("this is MOV%d\n", f_jpg_id);

        }


        f_jpg_id++;

        if (str != NULL) {
            return str;
        }

    }
    f_jpg_id = 1;



    os_mutex_post(&file_list_mutex);
    printf("file mem is full,please delete some file\n");

    return NULL;
}
static u8 *__find_emtry_block_b(struct file_info *info)
{
    int i = 1;
    static int b_jpg_id = 1;
    u8 *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    for (i = 1; i < MAX_NUM; i++) {
        if (behind_file_mem[i][0] != '{') {
            printf("behind_file_mem[%d]\n", i);
            info->id = i;
            BMAX_ID = MAX(BMAX_ID, i);
            os_mutex_post(&file_list_mutex);
            return behind_file_mem[i];
        }
    }
    while (b_jpg_id < MAX_NUM) {
        if (memmem(behind_file_mem[b_jpg_id], INFO_LEN, "JPG", 3)) {
            os_mutex_post(&file_list_mutex);
            printf("delete jpg to set ,behind_file_mem[%d]\n", b_jpg_id);
            str =  behind_file_mem[b_jpg_id];
        }
        b_jpg_id++;

        if (str != NULL) {
            return str;
        }

    }
    b_jpg_id = 1;

    os_mutex_post(&file_list_mutex);
    printf("file mem is full,please delete some file\n");

    return NULL;
}

static size_t forward_write_block(u8 *buffer, size_t len)
{

    struct file_info *info = malloc(sizeof(struct file_info));
    if (info == NULL) {
        return -1;
    }


    u8 *p = __find_emtry_block_f(info);
    if (p == NULL) {
        return -1;
    }


    memcpy(p, buffer, len);

    info->len = len;
    info->fd = (u8(*)[128])p;

    /* printf("info->fd=%s   info->len=%d\n", info->fd, info->len); */
    os_mutex_pend(&file_list_mutex, 0);
    list_add_tail(&info->entry, &forward_file_list_head);
    os_mutex_post(&file_list_mutex);
    return len;

}

static int forward_remove_block(const char *fname)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;

    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {

        os_mutex_post(&file_list_mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &forward_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            memset(__info->fd, ' ', INFO_LEN);

            list_del(&__info->entry);
            free(__info);
            os_mutex_post(&file_list_mutex);
            return 0;
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}
static int forward_change_block(const char *fname, char attr)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    char *str = NULL;

    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }
    list_for_each_safe(pos, node, &forward_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            str = memmem(__info->fd, __info->len, "\"y\":", 4);
            printf("attr=%c \n", attr);
            if (str) {
                *(str + 4) = attr;
                os_mutex_post(&file_list_mutex);
                return 0;
            }
        }

    }
    os_mutex_post(&file_list_mutex);
    return -1;
}

static int forward_remove_block_all()
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&forward_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &forward_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        list_del(&__info->entry);
        memset(__info->fd, ' ', INFO_LEN);
        free(__info);
    }
    os_mutex_post(&file_list_mutex);
    return 0;

}



static size_t behind_write_block(u8 *buffer, size_t len)
{

    /* printf("p addr ->0x%x\n",p); */

    struct file_info *info = malloc(sizeof(struct file_info));
    if (info == NULL) {
        return -1;
    }

    u8 *p = __find_emtry_block_b(info);
    if (p == NULL) {
        return -1;
    }

    memcpy(p, buffer, len);

    info->len = len;
    info->fd = (u8(*)[128])p;

    /* printf("info->fd=%s   info->len=%d\n", info->fd, info->len); */
    os_mutex_pend(&file_list_mutex, 0);
    list_add_tail(&info->entry, &behind_file_list_head);

    os_mutex_post(&file_list_mutex);
    return len;

}

static int behind_remove_block(const char *fname)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            memset(__info->fd, ' ', __info->len);
            list_del(&__info->entry);
            free(__info);
            os_mutex_post(&file_list_mutex);
            return 0;
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}

static int behind_change_block(const char *fname, char attr)
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;

    char *str = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);

        if (memmem(__info->fd, __info->len, fname, strlen(fname))) {
            str = memmem(__info->fd, __info->len, "\"y\":", 4);
            if (str) {
                *(str + 4) = attr;
                os_mutex_post(&file_list_mutex);
                return 0;
            }
        }
    }
    os_mutex_post(&file_list_mutex);
    return -1;
}




static int behind_remove_block_all()
{
    struct  file_info *__info = NULL;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&file_list_mutex, 0);
    if (list_empty(&behind_file_list_head)) {
        os_mutex_post(&file_list_mutex);
        return -1;
    }

    list_for_each_safe(pos, node, &behind_file_list_head) {
        __info = list_entry(pos, struct file_info, entry);
        list_del(&__info->entry);
        memset(__info->fd, ' ', INFO_LEN);
        free(__info);
    }
    os_mutex_post(&file_list_mutex);
    return 0;


}

void FILE_REMOVE_ALL()
{

    if (FILE_INITIND_CHECK()) {
        forward_remove_block_all();

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block_all();
#endif
    }
}


void __FILE_LIST_INIT(u8 is_forward, u32 file_num)
{

    FILE *fd = NULL;
    int ret = 0;
    u32 count = 0;
    u32 flag = 0;
    char res[32];
    char path[64];
    struct vfscan *fs = NULL;
    struct media_file_info media_info;
    memset(res, 0, 32);

    if (is_forward) {
        strcpy(res, get_rec_path_1());

        INIT_LIST_HEAD(&forward_file_list_head);
        memset(forward_file_mem, ' ', MAX_NUM * INFO_LEN);
        flag = 0;
    } else {
        strcpy(res, get_rec_path_2());
        INIT_LIST_HEAD(&behind_file_list_head);
        memset(behind_file_mem, ' ', MAX_NUM * INFO_LEN);
        flag = 1;
    }


    fs = fscan(res, "-tMOVJPG -st");
    if (fs == NULL) {
        return;
    }
    while (1) {
        if (fd == NULL) {
            fd = fselect(fs, FSEL_FIRST_FILE, 0);
        } else {
            fd = fselect(fs, FSEL_NEXT_FILE, 0);
        }
        if (fd == NULL || count >= file_num) {
            goto close1;
        }
        media_info.fd = fd;
        media_info.channel = flag;

        send_json(&media_info, 0x2);
        fclose(fd);
        if (ret <= 0) {
            continue;
        }


        count++;

    }
close1:

    fscan_release(fs);

}


void FILE_DELETE(const char *fname)
{
    printf("vf_list detele:%s\n", fname);

    if (FILE_INITIND_CHECK()) {
        return;
    }
    if (fname == NULL) {
        forward_remove_block_all();

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block_all();
#endif
    }

    if (strstr(fname, get_rec_path_1())) {
        forward_remove_block(fname);
    } else if (strstr(fname, get_rec_path_2())) {

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_remove_block(fname);
#endif
    }
}

void FILE_CHANGE_ATTR(const char *fname, char attr)
{
    if (FILE_INITIND_CHECK()) {
        return;
    }

    if (fname == NULL) {
        printf("file name is NULL\n");
        return ;
    }

    if (strstr(fname, get_rec_path_1())) {
        forward_change_block(fname, attr);
    } else if (strstr(fname, get_rec_path_2())) {
#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        behind_change_block(fname, attr);
#endif
    }

}


void FILE_GEN(void)
{

    FILE *fd = NULL;
    FILE *fd2 = NULL;
    u8 *str_ptr = NULL;
    u8 *str_ptr2 = NULL;
    char path[64];
    int j = 0;
    sprintf(path, "%s%s", get_rec_path_1(), "vf_list.txt");
    fd = fopen(path, "w+");

    //写json_head
    os_mutex_pend(&file_list_mutex, 0);
    str_ptr = forward_file_mem[0];
    memcpy(str_ptr, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
    str_ptr = forward_file_mem[FMAX_ID];
    if (FMAX_ID) {
        str_ptr = str_ptr + INFO_LEN;
        while (*(str_ptr--) != ',') {
            j++;
        }
        printf("file list ptr offset:%d\n", j);
        *(++str_ptr) = ' ';
        str_ptr2 = str_ptr;
    }
    str_ptr = forward_file_mem[FMAX_ID + 1];
    memcpy(str_ptr, "]}", 2);
    os_mutex_post(&file_list_mutex);

    fwrite(fd, forward_file_mem, (FMAX_ID + 2)*INFO_LEN);
    fclose(fd);
    *(str_ptr2) = ',';


#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
    sprintf(path, "%s%s", get_rec_path_2(), "vf_list.txt");
    fd2 = fopen(path, "w+");
    os_mutex_pend(&file_list_mutex, 0);
    str_ptr = behind_file_mem[0];
    memcpy(str_ptr, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
    str_ptr = behind_file_mem[BMAX_ID];
    if (BMAX_ID) {
        str_ptr = str_ptr + INFO_LEN;
        while (*(str_ptr--) != ',');
        *(++str_ptr) = ' ';
        str_ptr2 = str_ptr;
    }
    str_ptr = behind_file_mem[BMAX_ID + 1];
    memcpy(str_ptr, "]}", 2);
    os_mutex_post(&file_list_mutex);
    fwrite(fd2, behind_file_mem, (BMAX_ID + 2) *INFO_LEN);
    fclose(fd2);
    *(str_ptr2) = ',';
#endif
    printf("FMAX_ID= %d  BMAX_ID=%d\n", FMAX_ID, BMAX_ID);

}

void FILE_LIST_ADD(u32 status, const char *path)
{
    puts("FILE_LIST_ADD\n");
    u32 len;
    struct media_file_info media_info;
    FILE *fd = NULL;
    u8 flag = 0;
    printf("fs->%s\n", path);

    len = strlen(path);
    if (len < 3) {
        return;
    }
    if (!storage_device_ready()) {
        return;
    }
    printf("file open %s\n", path);
    fd = fopen(path, "r");
    if (fd == NULL) {
        return;
    }

    if (strstr(path, get_rec_path_1())) {
        flag = 0;
    } else {
        flag = 1;
    }
    media_info.fd = fd;
    media_info.channel = flag;
    send_json(&media_info, status);

    fclose(fd);
}


unsigned short __attribute__((weak))DUMP_PORT()
{
    return 0;
}

unsigned short __attribute__((weak))FORWARD_PORT()
{
    return 0;
}

unsigned short __attribute__((weak))BEHIND_PORT()
{
    return 0;
}

const char *__attribute__((weak))get_rec_path_1()
{
    return NULL;
}
const char *__attribute__((weak))get_rec_path_2()
{
    return NULL;
}
const char *__attribute__((weak))get_root_path()
{
    return NULL;
}





int vf_list(u8 type, u8 isforward, char *dir)
{
    struct vfscan *fs = NULL;
    FILE *vf_fd_forward = NULL;
    FILE *vf_fd_behind = NULL;
    FILE *fd = NULL;
    u32 open_count = 0;
    union vunpkg_req req;
    u8 namebuf[16];
    char *tmp_buf = NULL;
    const char *fs_arg = NULL;
    int err;
    int ret;
    int vaild;
    int file_sz = 0;
    char path[64];
    if (!storage_device_ready()) {
        return -1;
    }

    tmp_buf = (char *)malloc(512);

    if (tmp_buf == NULL) {
        printf("%s  %d malloc fail\n", __func__, __LINE__);
    }

#if 1

    if (isforward) {

        sprintf(path, "%s%s", get_rec_path_1(), "vf_list_a.txt");
        vf_fd_forward = fopen(path, "w+");

        if (vf_fd_forward == NULL) {
            printf("%s  %d fopen fail\n", __func__, __LINE__);

            return -1;
        }

        file_sz += fwrite(vf_fd_forward, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
        strcpy(dir, path);
    } else {

        sprintf(path, "%s%s", get_rec_path_2(), "vf_list_a.txt");
        vf_fd_behind = fopen(path, "w+");

        if (vf_fd_behind == NULL) {
            printf("%s  %d fopen fail\n", __func__, __LINE__);

            return -1;
        }

        file_sz += fwrite(vf_fd_behind, VIDEO_FILE_LIST_JSON_HEAD, strlen(VIDEO_FILE_LIST_JSON_HEAD));
        strcpy(dir, path);
    }

#endif
    //copy double dir
    switch (type) {
    case VID_JPG:
        fs_arg = __fs_arg[2];
        break;
    case VIDEO:
        fs_arg = __fs_arg[0];
        break;
    case JPG:
        fs_arg = __fs_arg[1];
        break;
    default:
        printf("%s:%d type err \n\n", __func__, __LINE__);
        break;

    }

    if (isforward) {
        fs = fscan(get_rec_path_1(), fs_arg);

        while (1) {
            if (fd == NULL) {
                fd = fselect(fs, FSEL_FIRST_FILE, 0);
            } else {
                fd = fselect(fs, FSEL_NEXT_FILE, 0);
            }

            if (fd == NULL) {
                printf("open_count->%d fs_arg->%s\n", open_count, fs_arg);
                if (open_count > 0) {
                    fseek(vf_fd_forward, file_sz - 1, SEEK_SET);
                }


                fwrite(vf_fd_forward, "]}", 2);
                fclose(vf_fd_forward);
                fscan_release(fs);
                malloc_stats();
                free(tmp_buf);
                return type;
                /* break; */
            }

            fget_name(fd, namebuf, 128);
            ret = strlen((char *)namebuf) - 3;
            struct vfs_attr attr;
            int _attr;
            fget_attrs(fd, &attr);
            fget_attr(fd, &_attr);

            if (!memcmp((namebuf + ret), "mov", 3)) {

#if 1
                struct __mov_unpkg_info info;

                if (is_vaild_mov_file(fd)) {
                    if (read_stts(fd, &info) != 0) {
                        vaild = 0;
                        fclose(fd);
                        continue;

                    }
                    if (0 != read_time_scale_dur(fd, &info)) {
                        vaild = 0;
                        fclose(fd);
                        continue;
                    }
                    if (0 != read_height_and_length(fd, &info)) {
                        vaild = 0;
                        fclose(fd);
                        continue;
                    }
                    vaild = 1;

                    if (_attr & F_ATTR_RO) {
                        vaild = 2;
                    }
                } else {

                    vaild = 0;
                    fclose(fd);
                    continue;
                }

                ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\"},"
                              , vaild
                              , get_rec_path_1()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , info.durition / info.scale
                              , info.height >> 16
                              , info.length >> 16
                              , info.scale / info.sample_duration
                              , flen(fd)
                             );
#endif
            } else {
#if 1
                //  puts("picture\n");
                u16 height, width;
                fseek(fd, 0x174, SEEK_SET);
                fread(fd, &height, 2);
                fread(fd, &width, 2);
                height = lwip_htons(height);
                width  = lwip_htons(width);
                ret = sprintf(tmp_buf, "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\"},"
                              , get_rec_path_1()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , height
                              , width
                              , flen(fd)
                             );
#endif
            }

            open_count++;
#if 1
            file_sz += fwrite(vf_fd_forward, tmp_buf, ret);
#endif
            fclose(fd);
        }

    } else {

//////////////////////////////////////////////////////////////
//////////////////////////2号文件夹////////////////////////////////////
#if 1
        open_count = 0;
        fs = fscan(get_rec_path_2(), fs_arg);

        while (1) {
            if (fd == NULL) {
                fd = fselect(fs, FSEL_FIRST_FILE, 0);
            } else {
                fd = fselect(fs, FSEL_NEXT_FILE, 0);
            }


            if (fd == NULL) {
                if (open_count > 0) {
                    fseek(vf_fd_behind, file_sz - 1, SEEK_SET);
                } else {
                    return NONE;
                }

                fwrite(vf_fd_behind, "]}", 2);
                fclose(vf_fd_behind);
                fscan_release(fs);
                return type;
            }

            fget_name(fd, namebuf, 128);
            int _attr;
            struct vfs_attr attr;
            fget_attrs(fd, &attr);

            fget_attr(fd, &_attr);
            if (!memcmp((namebuf + ret), "mov", 3)) {
                struct __mov_unpkg_info info;

                if (is_vaild_mov_file(fd)) {
                    read_stts(fd, &info);
                    read_time_scale_dur(fd, &info);
                    read_height_and_length(fd, &info);
                    vaild = 1;

                    if (_attr & F_ATTR_RO) {
                        vaild = 2;
                    }

                } else {

                    vaild = 0;
                    fclose(fd);
                    continue;
                }

                ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\"},"
                              , vaild
                              , get_rec_path_2()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , info.durition / info.scale
                              , info.height >> 16
                              , info.length >> 16
                              , info.scale / info.sample_duration
                              , flen(fd)
                             );
            } else {
                //       puts("picture\n");
                u16 height, width;
                fseek(fd, 0x174, SEEK_SET);
                fread(fd, &height, 2);
                fread(fd, &width, 2);
                height = lwip_htons(height);
                width  = lwip_htons(width);
                ret = sprintf(tmp_buf, "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\"},"
                              , get_rec_path_2()
                              , namebuf
                              , attr.crt_time.year
                              , attr.crt_time.month
                              , attr.crt_time.day
                              , attr.crt_time.hour
                              , attr.crt_time.min
                              , attr.crt_time.sec
                              , height
                              , width
                              , flen(fd)
                             );
            }

            open_count++;
            file_sz += fwrite(vf_fd_behind, tmp_buf, ret);
            fclose(fd);
        }

#endif
    }

    return NONE;
}








void __FILE_LIST_INIT(u8 is_forward, u32 file_num);

extern int send_ctp_string(int cmd_type, char *buf, char *_req, void *priv);
#if 0
static int get_and_write_file_attribute(u32 status, FILE *file_list_fd, FILE *fd, u8 flag)
{
    char namebuf[20];
    char tmp_buf[128] = {[0 ... 127] = 0};
    char tmp_buf2[128] = {[0 ... 127] = 0};
    int ret;
    int _attr;
    struct vfs_attr attr;
    struct __mov_unpkg_info info;
    u32 vaild = 0;
    fget_name(fd, namebuf, 20);
    ret = strlen(namebuf) - 3;
    fget_attrs(fd, &attr);
    fget_attr(fd, &_attr);
    char *buf = malloc(256);
    if (buf == NULL) {
        return -1;
    }
    /* printf("fname -> %s\n", namebuf); */
    if (!memcmp((namebuf + ret), "MOV", 3)) {
        /* time1 = timer_get_ms(); */
        if (is_vaild_mov_file(fd)) {
            if (read_stts(fd, &info) != 0) {
                vaild = 0;
                fclose(fd);
                continue;

            }
            if (0 != read_time_scale_dur(fd, &info)) {
                vaild = 0;
                fclose(fd);
                continue;
            }
            if (0 != read_height_and_length(fd, &info)) {
                vaild = 0;
                fclose(fd);
                continue;
            }


            vaild = 1;

            if (_attr & F_ATTR_RO) {
                vaild = 2;
            }

            /* time2 = timer_get_ms(); */
            /* printf("use in time:%dms\n",time2 - time1); */
        } else {

            vaild = 0;
            printf("Invalid file\n");
            return -1;
        }

        ret = sprintf(tmp_buf, "{\"y\":%d,\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"d\":\"%d\",\"h\":%d,\"w\":%d,\"p\":%d,\"s\":\"%d\",\"c\":\"%d\"},"
                      , vaild
                      , flag ? get_rec_path_2() : get_rec_path_1()
                      , namebuf
                      , attr.crt_time.year
                      , attr.crt_time.month
                      , attr.crt_time.day
                      , attr.crt_time.hour
                      , attr.crt_time.min
                      , attr.crt_time.sec
                      , info.durition / info.scale
                      , info.height >> 16
                      , info.length >> 16
                      , info.scale / info.sample_duration
                      , flen(fd)
                      , flag
                     );

        if (!flag) {
            forward_write_block(tmp_buf, ret);
        } else {
            behind_write_block(tmp_buf, ret);
        }

        if (status <= 1) {
            ret = sprintf(buf, "{\"op\":\"NOTIFY\",\"param\":{\"status\":\"%d\",\"desc\":\"{\\\"y\\\":%d,\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"d\\\":\\\"%d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"p\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\"}\"}}"
                          , status
                          , vaild
                          , flag ? get_rec_path_2() : get_rec_path_1()
                          , namebuf
                          , attr.crt_time.year
                          , attr.crt_time.month
                          , attr.crt_time.day
                          , attr.crt_time.hour
                          , attr.crt_time.min
                          , attr.crt_time.sec
                          , info.durition / info.scale
                          , info.height >> 16
                          , info.length >> 16
                          , info.scale / info.sample_duration
                          , flen(fd)
                          , flag
                         );
            send_ctp_string(CTP_NOTIFY_COMMAND, buf, "VIDEO_FINISH", NULL);
        }
    } else {
        //  puts("picture\n");
        u16 height, width;
        fseek(fd, 0x174, SEEK_SET);
        fread(fd, &height, 2);
        fread(fd, &width, 2);
        height = lwip_htons(height);
        width  = lwip_htons(width);
        ret = sprintf(tmp_buf, "{\"f\":\"%s%s\",\"t\":\"%04d%02d%02d%02d%02d%02d\",\"h\":%d,\"w\":%d,\"s\":\"%d\",\"c\":\"%d\"},"
                      , flag ? get_rec_path_2() : get_rec_path_1()
                      , namebuf
                      , attr.crt_time.year
                      , attr.crt_time.month
                      , attr.crt_time.day
                      , attr.crt_time.hour
                      , attr.crt_time.min
                      , attr.crt_time.sec
                      , height
                      , width
                      , flen(fd)
                      , flag
                     );

        /* ret = fwrite(file_list_fd, tmp_buf, ret); */
        if (!flag) {
            forward_write_block(tmp_buf, ret);
        } else {
            behind_write_block(tmp_buf, ret);
        }

        if (status <= 1) {
            sprintf(buf, "{\"op\":\"NOTIFY\",\"param\":{\"desc\":\"{\\\"f\\\":\\\"%s%s\\\",\\\"t\\\":\\\"%04d%02d%02d%02d%02d%02d\\\",\\\"h\\\":%d,\\\"w\\\":%d,\\\"s\\\":\\\"%d\\\",\\\"c\\\":\\\"%d\\\"}\"}}"
                    , flag ? get_rec_path_2() : get_rec_path_1()
                    , namebuf
                    , attr.crt_time.year
                    , attr.crt_time.month
                    , attr.crt_time.day
                    , attr.crt_time.hour
                    , attr.crt_time.min
                    , attr.crt_time.sec
                    , height
                    , width
                    , flen(fd)
                    , flag
                   );

            send_ctp_string(CTP_NOTIFY_COMMAND, buf, "PHOTO_CTRL", NULL);
        }
    }
    return ret;

}
#endif





int get_media_file_info(struct media_file_info *__info)
{
    int ret = 0;
    if (__info == NULL || __info->fd == NULL) {
        printf("file is not open\n");
        return -1;
    }
    fget_name(__info->fd, __info->namebuf, sizeof(__info->namebuf));
    ret = strlen((const char *)__info->namebuf) - 3;
    fget_attrs(__info->fd, &__info->attr);
    __info->attr.crt_time.sec += 1; //BUG BUG BUG
    fget_attr(__info->fd, &__info->_attr);

    if (!memcmp((__info->namebuf + ret), "MOV", 3)) {
        if (is_vaild_mov_file(__info->fd)) {
            if (0 != read_stts(__info->fd, &__info->info)) {
                __info->vaild = 0x0;
                printf("Invalid file\n");
                return -1;

            }
            if (0 != read_time_scale_dur(__info->fd, &__info->info)) {
                __info->vaild = 0x0;
                printf("Invalid file\n");
                return -1;
            }
            if (0 != read_height_and_length(__info->fd, &__info->info)) {
                __info->vaild = 0x0;
                printf("Invalid file\n");
                return -1;
            }

            if (__info->_attr & F_ATTR_RO) {
                __info->vaild = 0x2;
            } else {
                __info->vaild = 0x1;
            }
            __info->type = 0x1;

        } else {
            __info->vaild = 0x0;
            printf("Invalid file\n");
            return -1;
        }

    } else if (!memcmp((__info->namebuf + ret), "JPG", 3)) {
        __info->type = 0x2;
        fseek(__info->fd, 0x174, SEEK_SET);
        fread(__info->fd, &__info->height, 2);
        fread(__info->fd, &__info->width, 2);
        __info->height = lwip_htons(__info->height);
        __info->width  = lwip_htons(__info->width);
    } else {
        printf("FILE NAME:%s err!!!!!!\n", __info->namebuf);
        return -1;

    }
    return 0;

}

int send_json(struct media_file_info *__info, u32 status)
{
    int ret = 0;

    char *buffer = malloc(256);
    if (buffer == NULL) {
        printf("malloc fail \n");
        return -1;

    }

    if (get_media_file_info(__info)) {
        printf("get_media_file_info fail\n");
        return -1;
    }

    memset(buffer, 0, 256);
    if (__info->type & 0x1) {
        ret = sprintf(buffer, VIDEO_JSON_MEM
                      , __info->vaild
                      , __info->channel ? get_rec_path_2() : get_rec_path_1()
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , __info->info.durition / __info->info.scale
                      , __info->info.height >> 16
                      , __info->info.length >> 16
                      , __info->info.scale / __info->info.sample_duration
                      , flen(__info->fd)
                      , __info->channel
                     );

        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1) {
            memset(buffer, 0, 256);
            ret = sprintf(buffer, VIDEO_JSON_APP
                          , status
                          , __info->vaild
                          , __info->channel ? get_rec_path_2() : get_rec_path_1()
                          , __info->namebuf
                          , __info->attr.crt_time.year
                          , __info->attr.crt_time.month
                          , __info->attr.crt_time.day
                          , __info->attr.crt_time.hour
                          , __info->attr.crt_time.min
                          , __info->attr.crt_time.sec
                          , __info->info.durition / __info->info.scale
                          , __info->info.height >> 16
                          , __info->info.length >> 16
                          , __info->info.scale / __info->info.sample_duration
                          , flen(__info->fd)
                          , __info->channel
                         );
            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "VIDEO_FINISH", NULL);
        }
    } else if (__info->type & 0x2) {
        ret = sprintf(buffer, PICTURE_JSON_MEM
                      , __info->channel ? get_rec_path_2() : get_rec_path_1()
                      , __info->namebuf
                      , __info->attr.crt_time.year
                      , __info->attr.crt_time.month
                      , __info->attr.crt_time.day
                      , __info->attr.crt_time.hour
                      , __info->attr.crt_time.min
                      , __info->attr.crt_time.sec
                      , __info->height
                      , __info->width
                      , flen(__info->fd)
                      , __info->channel
                     );

        if (!__info->channel) {
            forward_write_block((u8 *)buffer, ret);
        } else {
            behind_write_block((u8 *)buffer, ret);
        }

        if (status <= 1) {
            memset(buffer, 0, 256);
            sprintf(buffer, PICTURE_JSON_APP
                    , __info->channel ? get_rec_path_2() : get_rec_path_1()
                    , __info->namebuf
                    , __info->attr.crt_time.year
                    , __info->attr.crt_time.month
                    , __info->attr.crt_time.day
                    , __info->attr.crt_time.hour
                    , __info->attr.crt_time.min
                    , __info->attr.crt_time.sec
                    , __info->height
                    , __info->width
                    , flen(__info->fd)
                    , __info->channel
                   );

            send_ctp_string(CTP_NOTIFY_COMMAND, buffer, "PHOTO_CTRL", NULL);
        }


    }
    return 0;
}






void FILE_LIST_INIT(u32 flag)
{
    char buf[128];
    u32 err = 0;
    if (initing) {
        printf("File list in doing, Please waiting\n");
        return;
    }
    initing = 1;
    if (!mutex_init) {

        err = os_mutex_create(&file_list_mutex);
        if (err != OS_NO_ERR) {
            printf("os mutex create fail \n");
            return;
        }
        mutex_init = 1;

    }
    __FILE_LIST_INIT(1, FILE_MAX_NUM);


#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
    __FILE_LIST_INIT(0, FILE_MAX_NUM);

#endif

    if (flag) {
        FILE_GEN();
        sprintf(buf, "type:1,path:%s%s", get_rec_path_1(), "vf_list.txt");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "FORWARD_MEDIA_FILES_LIST", "NOTIFY", buf);

#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
        sprintf(buf, "type:1,path:%s%s", get_rec_path_2(), "vf_list.txt");
        CTP_CMD_COMBINED(NULL, CTP_NO_ERR, "BEHIND_MEDIA_FILES_LIST", "NOTIFY", buf);
#endif
    }


    initing = 0;

}



void FILE_TEST()
{
    puts("FILE_TEST\n\n\n\n");
    __FILE_LIST_INIT(1, FILE_MAX_NUM);
    __FILE_LIST_INIT(0, FILE_MAX_NUM);
    puts("FILE_TEST\n\n\n\n");
}


void file_list_thread(void *arg)
{
    u32 *flag = (u32 *)arg;
    FILE_LIST_INIT(*flag);
}





void FILE_LIST_INIT_SMALL(u32 file_num)
{
    __FILE_LIST_INIT(2, file_num);
#if defined CONFIG_VIDEO3_ENABLE || defined CONFIG_VIDEO1_ENABLE
    __FILE_LIST_INIT(3, file_num);
#endif
}




int FILE_INITIND_CHECK()
{
    return initing;
}


void FILE_LIST_IN_MEM(u32 flag)
{
    static u32 count = 0;
    char buf[32];
    u32 space = 0;

    sprintf(buf, "file_list_thread%d", count++);
    thread_fork(buf, 10, 0x1000, 0, 0, file_list_thread, (void *)&flag);

    /* msleep(100);//等待SD卡稳定 */
}

