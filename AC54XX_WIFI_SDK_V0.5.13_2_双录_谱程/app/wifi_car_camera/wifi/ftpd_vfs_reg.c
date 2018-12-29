#include "app_config.h"
#include "ftpserver/stupid-ftpd.h"
#include "upgrade/upgrade_detect.h"


#define UPGRADE_FILE_SIZE 4*1024*1024
#define INCREASE_MEMORY_SIZE 1*1024*1024

extern int storage_device_ready();

typedef struct {
    int size;
    char buf[];
} VIR_FILE;

static void upgrade_flash_req(char *buffer, u32 len)
{
    int err;
    struct upgrade_req req = {0};

    req.type = UPGRADE_FROM_BUFF;
    req.buf = buffer;
    req.buf_size = len;
    err = upgrade_file_detect(&req);
}

static void *stupid_vfs_open(char *path,  char *mode)
{
    printf("\n path is %s \n", path);
    //强制有卡升级
    if (storage_device_ready()) {
        return NULL;
    }
    VIR_FILE *file = malloc(sizeof(VIR_FILE) + UPGRADE_FILE_SIZE);
    if (file == NULL) {
        printf("\n [error]%s %d\n", __func__, __LINE__);
        return NULL;
    }
    file->size = 0;
    return (void *)file;//必须返回非空指针
}

static int stupid_vfs_write(void  *file, void  *buf, u32 len)
{
    if (storage_device_ready()) {
        return -1;
    }
    //写文件到缓冲区，累加长度
    VIR_FILE *vir_file = (VIR_FILE *)file;

    memcpy(vir_file->buf + vir_file->size, buf, len);
    vir_file->size += len;

    return len;
}

static int stupid_vfs_close(void *file)
{

    if (storage_device_ready()) {
        return -1;
    }
    //这里准备升级完成,文件大小大于等于写长度，请求升级
    VIR_FILE *vir_file = (VIR_FILE *)file;
    upgrade_flash_req(vir_file->buf, vir_file->size);
    free(vir_file);

    return 0;
}

//注册一个ftpd的vfs接口

void ftpd_vfs_interface_cfg(void)
{
    struct ftpd_vfs_cfg info;
    info.fopen = stupid_vfs_open;
    info.fwrite = stupid_vfs_write;
    info.fclose = stupid_vfs_close;
//注册接口到ftp中
    stupid_vfs_reg(&info);

}

