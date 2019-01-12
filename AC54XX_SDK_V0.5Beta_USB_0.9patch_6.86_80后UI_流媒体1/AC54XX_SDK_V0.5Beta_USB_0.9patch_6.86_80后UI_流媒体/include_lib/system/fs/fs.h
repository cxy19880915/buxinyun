#ifndef __FS_H__
#define __FS_H__



#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/ioctl.h"
#include "generic/atomic.h"
#include "system/task.h"
#include "system/malloc.h"
#include "system/time.h"
#include "stdarg.h"


#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

#ifndef FSELECT_MODE

#define FSELECT_MODE

#define    FSEL_FIRST_FILE      0
#define    FSEL_LAST_FILE       1
#define    FSEL_NEXT_FILE       2
#define    FSEL_PREV_FILE       3
#define    FSEL_CURR_FILE       4
#define    FSEL_BY_NUMBER       5
#define    FSEL_BY_SCLUST       6

#endif


#define F_ATTR_RO       0x01
#define F_ATTR_ARC      0x02
#define F_ATTR_DIR      0x04
#define F_ATTR_VOL      0x08

enum {
    FS_IOCTL_GET_FILE_NUM,
    FS_IOCTL_FILE_CHECK,
};


struct vfs_devinfo;
struct vfscan;
struct vfs_operations;



struct vfs_devinfo {
    void *fd;
    u32 sector_size;
    void *private_data;
};

#define VFS_PART_DIR_MAX 16


struct vfs_partition {
    struct vfs_partition *next;
    u32 offset;
    u32 clust_size;
    u32 total_size;
    u8 fs_attr;
    char dir[VFS_PART_DIR_MAX];
    void *private_data;
};

struct fiter {
    u32 index;
};




struct imount {
    int fd;
    const char *path;
    struct vfs_operations *ops;
    struct vfs_devinfo dev;
    struct vfs_partition part;
    struct list_head entry;
    atomic_t ref;
    OS_MUTEX mutex;
    u8 avaliable;
    u8 part_num;
};

struct vfs_attr {
    u8 attr;
    u32 fsize;
    u32 sclust;
    struct sys_time crt_time;
    struct sys_time wrt_time;
};

typedef struct {
    struct imount *mt;
    struct vfs_devinfo *dev;
    struct vfs_partition *part;
    void *private_data;
} FILE;


struct vfscan {
    u8 scan_file;
    u8 subpath;
    u8 scan_dir;
    u8 attr;
    char sort;
    char ftype[16];
    u16 file_number;
    void *priv;
    struct vfs_devinfo *dev;
    struct vfs_partition *part;
};


struct vfs_operations {
    const char *fs_type;
    int (*mount)(struct imount *);
    int (*unmount)(struct imount *);
    int (*format)(struct vfs_devinfo *, struct vfs_partition *);
    int (*fset_vol)(struct vfs_partition *, const char *name);
    int (*fget_free_space)(struct vfs_devinfo *, struct vfs_partition *, u32 *space);
    int (*fopen)(FILE *, const char *path, const char *mode);
    int (*fread)(FILE *, void *buf, u32 len);
    int (*fwrite)(FILE *, void *buf, u32 len);
    int (*fseek)(FILE *, int offset, int);
    int (*flen)(FILE *);
    int (*fpos)(FILE *);
    int (*fcopy)(FILE *, FILE *);
    int (*fget_name)(FILE *, u8 *name, int len);
    int (*frename)(FILE *, const char *path);
    int (*fclose)(FILE *);
    int (*fdelete)(FILE *);
    int (*fscan)(struct vfscan *, const char *path);
    void (*fscan_release)(struct vfscan *);
    int (*fsel)(struct vfscan *, int sel_mode, FILE *, int);
    int (*fget_attr)(FILE *, int *attr);
    int (*fset_attr)(FILE *, int attr);
    int (*fget_attrs)(FILE *, struct vfs_attr *);
    int (*fmove)(FILE *file, const char *path_dst, FILE *, int clr_attr);
    int (*ioctl)(void *, int cmd, int arg);
};

#define REGISTER_VFS_OPERATIONS(ops) \
	static const struct vfs_operations ops sec(.vfs_operations)


static inline struct vfs_partition *vfs_partition_next(struct vfs_partition *p)
{
    struct vfs_partition *n = (struct vfs_partition *)zalloc(sizeof(*n));

    if (n) {
        p->next = n;
    }
    return n;
}


static inline void vfs_partition_free(struct vfs_partition *p)
{
    struct vfs_partition *n = p->next;

    while (n) {
        p = n->next;
        free(n);
        n = p;
    }
}



struct imount *mount(const char *dev_name, const char *path, const char *fs_type, void *dev_arg);

int unmount(const char *path);

int f_format(const char *path, const char *fs_type, u32 clust_size);

FILE *fopen(const char *path, const char *mode);

int fread(FILE *file, void *buf, u32 len);

int fwrite(FILE *file, void *buf, u32 len);

int fseek(FILE *file, int offset, int orig);

int flen(FILE *file);

int fpos(FILE *file);

int fcopy(const char *format, ...);

int fget_name(FILE *file, u8 *name, int len);

int frename(FILE *file, const char *path);

int fclose(FILE *file);

int fdelete(FILE *file);
int fdelete_by_name(const char *fname);

int fget_free_space(const char *path, u32 *space);

/* arg:
 * -t  文件类型
 * -r  包含子目录
 * -d  扫描文件夹
 * -a  文件属性 r: 读， /: 非
 * -s  排序方式， t:按时间排序， n:按文件号排序
 */
struct vfscan *fscan(const char *path, const char *arg);

void fscan_release(struct vfscan *fs);

FILE *fselect(struct vfscan *fs, int selt_mode, int arg);

int fdir_exist(const char *dir);

int fdir(FILE *file, const char *arg, char *name, int len, struct fiter *iter);

int fget_attr(FILE *file, int *attr);

int fset_attr(FILE *file, int attr);

int fget_attrs(FILE *file, struct vfs_attr *attr);

struct vfs_partition *fget_partition(const char *path);

int fset_vol(const char *path, const char *name);

int fmove(FILE *file, const char *path_dst, FILE **newFile, int clr_attr);

int fcheck(FILE *file);
#endif






