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



/*! \addtogroup FS
 *  @ingroup SYSTEM
 *  @brief File system api
 *  @attention The file operation function return -ENODEV mean no this file
 *  @{
 */

#ifndef EOF
#define EOF -1
#endif

/**
 * \name Seek orig for fseek function
 * \{
 */
#define SEEK_SET	0	/*!< Seek from beginning of file.  */
#define SEEK_CUR	1	/*!< Seek from current position.  */
#define SEEK_END	2	/*!< Seek from end of file.  */
/* \} name */

#ifndef FSELECT_MODE

/**
 * \name Select mode for fselect function
 * \{
 */
#define FSELECT_MODE

#define    FSEL_FIRST_FILE      0	/*!< 选择第一个文件 */
#define    FSEL_LAST_FILE       1	/*!< 选择最后一个文件 */
#define    FSEL_NEXT_FILE       2	/*!< 选择下一个文件 */
#define    FSEL_PREV_FILE       3	/*!< 选择上一个文件 */
#define    FSEL_CURR_FILE       4	/*!< not used */
#define    FSEL_BY_NUMBER       5	/*!< 根据文件序号选择 */
#define    FSEL_BY_SCLUST       6 	/*!< 根据分配单元选择文件 */
/* \} name */

#endif

/**
 * \name File attributes bits for fset_attr and fget_attr funciton
 * \{
 */
#define F_ATTR_RO       0x01	/*!< 只读 */
#define F_ATTR_ARC      0x02	/*!< 文件 */
#define F_ATTR_DIR      0x04	/*!< 目录 */
#define F_ATTR_VOL      0x08	/*!< 卷标 */
/* \} name */

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


/**
 * @brief 分区信息结构体
 */
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



/**
 * @brief 挂载点结构体
 */
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

/**
 * @brief 文件属性结构体
 */
struct vfs_attr {
    u8 attr;		/*!< 文件属性标志位 */
    u32 fsize;		/*!< 文件大小 */
    u32 sclust;		/*!< 最小分配单元 */
    struct sys_time crt_time;	/*!< 文件创建时间 */
    struct sys_time wrt_time;	/*!< 文件最后修改时间 */
};


/**
 * @brief 文件流结构体
 */
typedef struct {
    struct imount *mt;
    struct vfs_devinfo *dev;
    struct vfs_partition *part;
    void *private_data;
} FILE;


/**
 * @brief 文件扫描结构体
 */
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


/**
 * @brief 文件操作句柄结构体
 */
struct vfs_operations {
    const char *fs_type;
    int (*mount)(struct imount *);
    int (*unmount)(struct imount *);
    int (*format)(struct vfs_devinfo *, struct vfs_partition *);
    int (*fset_vol)(struct vfs_partition *, const char *name);
    int (*fget_free_space)(struct vfs_devinfo *, struct vfs_partition *, u32 *space);
    int (*fopen)(FILE *, const char *path, const char *mode);
    int (*fread)(FILE *, void *buf, u32 len);
    int (*fwrite)(FILE *, const void *buf, u32 len);
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



/**
 * @brief 挂载设备虚拟文件系统
 *
 * @param dev_name 设备名称
 * @param path 挂载点
 * @param fs_type 文件系统类型(支持"fat" "devfs" "ramfs" "sdfile")
 * @param dev_arg 设备参数指针
 *
 * @return 指向挂载点结构体的指针
 * @return NULL 挂载失败
 */
struct imount *mount(const char *dev_name, const char *path, const char *fs_type, void *dev_arg);

/**
 * @brief 卸载设备虚拟文件系统
 *
 * @param path 卸载路径
 *
 * @return 0: 卸载成功
 * @return other: 卸载失败
 */
int unmount(const char *path);

/**
 * @brief 格式化驱动器
 *
 * @param path 需要格式化的路径
 * @param fs_type 文件系统类型
 * @param clust_size 分配单元大小
 *
 * @return 0: 格式化成功
 * @return other: 格式化失败
 */
int f_format(const char *path, const char *fs_type, u32 clust_size);

/**
 * @brief 打开文件, 获取指向文件流的文件指针
 *
 * @param path 文件路径
 * @param mode 打开模式
 *
 * @return 指向文件流的文件指针
 * @return NULL: 打开失败
 * @note  打开模式只支持"r" "r+" "w" "w+"
 */
FILE *fopen(const char *path, const char *mode);

/**
 * @brief 从文件中读取数据
 *
 * @param file 指向文件流的文件指针
 * @param[out] buf 保存读取到的数据
 * @param len 想要读取的数据的字节长度
 *
 * @return 成功读取到的数据的字节长度
 */
int fread(FILE *file, void *buf, u32 len);

/**
 * @brief 写入数据到文件中
 *
 * @param file 指向文件流的文件指针
 * @param[in] buf 需要写入的数据
 * @param len 需要写入的数据的字节长度
 *
 * @return 成功写入的数据的字节长度
 */
int fwrite(FILE *file, const void *buf, u32 len);

/**
 * @brief 设置文件指针的位置
 *
 * @param file 指向文件流的文件指针
 * @param offset 偏移量
 * @param orig 偏移的基准位置
 *
 * @return 1: 成功
 * @return 0: 失败
 */
int fseek(FILE *file, int offset, int orig);

/**
 * @brief 获取文件的大小
 *
 * @param file 指向文件流的文件指针
 *
 * @return 文件大小(负值表示获取失败)
 */
int flen(FILE *file);

/**
 * @brief 获取文件指针的位置
 *
 * @param file 指向文件流的文件指针
 *
 * @return 文件指针的位置
 */
int fpos(FILE *file);

/**
 * @brief 复制文件
 *
 * @param format
 * @param ...
 *
 * @return
 * @note TODO
 */
int fcopy(const char *format, ...);

/**
 * @brief 获取文件名
 *
 * @param file 指向文件流的文件指针
 * @param name 保存文件名的buffer
 * @param len buffer的长度
 *
 * @return 文件名的长度(大于0)
 */
int fget_name(FILE *file, u8 *name, int len);

/**
 * @brief 重命名文件
 *
 * @param file 指向文件流的文件指针
 * @param path 文件的新路径
 *
 * @return 1: 成功
 * @return other: 失败
 */
int frename(FILE *file, const char *path);

/**
 * @brief 关闭文件流
 *
 * @param file 指向文件流的文件指针
 *
 * @return 0: 关闭成功
 */
int fclose(FILE *file);

/**
 * @brief 删除文件
 *
 * @param file 指向文件流的文件指针
 *
 * @return 0: 删除成功
 * @return other: 删除失败
 */
int fdelete(FILE *file);

/**
 * @brief 根据文件路径删除文件
 *
 * @param fname 文件路径
 *
 * @return 0: 删除成功
 * @return other: 删除失败
 */
int fdelete_by_name(const char *fname);

/**
 * @brief 获取剩余空间
 *
 * @param path 设备路径
 * @param space 保存剩余空间的大小
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_free_space(const char *path, u32 *space);

/**
 * @brief 文件扫描
 *
 * @param path 扫描路径
 * @param arg 扫描参数设置
 *
 * @return 虚拟文件扫描结构体句柄
 * @return NULL: 扫描失败
 * @note arg:
 * @note -t  文件类型
 * @note -r  包含子目录
 * @note -d  扫描文件夹
 * @note -a  文件属性 r: 读， /: 非
 * @note -s  排序方式， t:按时间排序， n:按文件号排序
 */
struct vfscan *fscan(const char *path, const char *arg);

/**
 * @brief 释放文件扫描结构体句柄
 *
 * @param fs 文件扫描结构体句柄
 */
void fscan_release(struct vfscan *fs);

/**
 * @brief 选择文件
 *
 * @param fs 文件扫描结构体句柄
 * @param selt_mode 选择模式
 * @param arg 选择参数
 *
 * @return 指向文件流的文件指针
 * @return NULL: 选择操作失败
 */
FILE *fselect(struct vfscan *fs, int selt_mode, int arg);

/**
 * @brief 退出目录
 *
 * @param dir 目录路径
 *
 * @return 1: 退出成功
 * @return 0: 没有此目录
 */
int fdir_exist(const char *dir);

/**
 * @brief
 *
 * @param file 指向文件流的文件指针
 * @param arg
 * @param name
 * @param len
 * @param iter
 *
 * @return
 * @note TODO
 */
int fdir(FILE *file, const char *arg, char *name, int len, struct fiter *iter);

/**
 * @brief 获取文件的文件属性
 *
 * @param file 指向文件流的文件指针
 * @param attr 保存文件的文件属性信息
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_attr(FILE *file, int *attr);

/**
 * @brief 设置文件的文件属性
 *
 * @param file 指向文件流的文件指针
 * @param attr 文件属性码
 *
 * @return 0: 设置成功
 * @return other: 设置失败
 */
int fset_attr(FILE *file, int attr);

/**
 * @brief 获取文件的详细信息
 *
 * @param file 指向文件流的文件指针
 * @param attr 保存文件的详细信息
 *
 * @return 0: 获取成功
 * @return other: 获取失败
 */
int fget_attrs(FILE *file, struct vfs_attr *attr);

/**
 * @brief 获取路径对应的分区
 *
 * @param path 分区路径
 *
 * @return 指向分区结构体的指针
 * @return NULL: 获取失败
 */
struct vfs_partition *fget_partition(const char *path);

/**
 * @brief 设置路径对应的卷标的名称
 *
 * @param path 卷标路径
 * @param name 卷标名称
 *
 * @return 0: 设置成功
 * @return other: 设置失败
 */
int fset_vol(const char *path, const char *name);

/**
 * @brief 移动文件
 *
 * @param file 指向文件流的文件指针
 * @param path_dst 目标路径
 * @param newFile 保存文件在新路径的文件流指针
 * @param clr_attr 是否清除原有文件的文件属性
 *
 * @return 0: 移动成功(自动关闭旧文件流)
 * @return other: 移动失败
 */
int fmove(FILE *file, const char *path_dst, FILE **newFile, int clr_attr);

/**
 * @brief 检查文件
 *
 * @param file 指向文件流的文件指针
 *
 * @return 文件的错误码
 */
int fcheck(FILE *file);

#endif






