#ifndef __DRV_MANAGE_H__
#define __DRV_MANAGE_H__

//#ifdef _USE_DLMALLOC_
//#define malloc_dbg  dlmalloc
//#define mem_malloc  dlmalloc
//#define mem_free    dlfree
//#else
#define malloc_dbg  malloc
#define mem_malloc  malloc
#define mem_free    free
//#endif
#include "device_drive.h"
#include "typedef.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif
#define DRV_MEM_MAGIC   0x55aa55aa
#define DRV_MEM_FREE    0xaa55aa55
#define DEV_ACTIVE      0x55aa55bb
#define DEV_UNACTIVE      0xaa5555bb
typedef struct dev_list_t {
    struct list_head entry;
    u32 mem_maigc;   //Check whether the memory is free or modified =DRV_MEM_MAGIC is valid other value is invalid
    const dev_io_t *dev_io;
    const struct dev_list_t *father;// logic device's physics device
    u32 dev_status;
    char volume_name;
} DEV_LIST_T;
typedef const DEV_LIST_T *DEV_HANDLE;

#define DEVICE_REG(dev,parm) dev_register(dev_reg_##dev((parm)))

s32 dev_init(u32 prio, u32 stack_size);
typedef const dev_io_t *(*GET_PART_FUN)(DEV_HANDLE hdev);
void dev_set_get_part_hook(GET_PART_FUN get_part_fun);

s32 dev_register(const dev_io_t *dev_io);
DEV_HANDLE dev_open(const char *name, void *parm);
s32 dev_read(DEV_HANDLE hdev, u8 *buf, u32 addr, u32 len);
s32 dev_write(DEV_HANDLE hdev, u8 *buf, u32 addr, u32 len);
s32 dev_ioctl(DEV_HANDLE hdev, void *pram, u32 cmd);
s32 dev_power(DEV_HANDLE hdev, u32 mode);
s32 dev_close(DEV_HANDLE hdev);
u32 dev_reg_msg(void *pram);
u32 dev_remove_msg(void *pram);
DEV_HANDLE dev_get_next(u32 pos);
typedef struct __msg_devicechange_t {
    enum dev_sta sta;
    DEV_HANDLE hdev;
} msg_devicechange_t;

#define ls_dev()  //do{printf_dev_list(__FILE__,__LINE__);}while(0);
void printf_dev_list(const char *a, u32 b);


#if 0

#define drv_err(fmt,...) do{\
            printf("%s-%d"#fmt"\r\n",__FILE__,__LINE__,##__VA_ARGS__);\
        }while(0);


#define  drv_dbg(...) //drv_err(...)
//#define  drv_dbg    drv_err
#define drv_printf_buf  //printf_buf

#else

#define drv_printf_buf
#define  drv_dbg(...)
#define drv_err(...)
#endif

#ifdef __cplusplus
}
#endif


#endif
