#ifndef _VM_H_
#define _VM_H_

#include "ioctl.h"
#ifdef CONFIG_CHRDEV
#include "device/device.h"
#endif

typedef enum vm_run_mode {
    //1bit mode
    WINBOND_READ_DATA_MODE = (1 << 0),
    WINBOND_FAST_READ_MODE = (1 << 1),
    //2bit mode
    WINBOND_FAST_READ_DUAL_IO_NORMAL_READ_MODE  = (1 << 2),
    WINBOND_FAST_READ_DUAL_IO_CONTINUOUS_READ_MODE = (1 << 3),
    WINBOND_FAST_READ_DUAL_OUTPUT_MODE     = (1 << 4),
    //4bit mode
    WINBOND_FAST_READ_QUAD_IO_NORMAL_READ_MODE   = (1 << 5),
    WINBOND_FAST_READ_QUAD_IO_CONTINUOUS_READ_MODE = (1 << 6),
    WINBOND_FAST_READ_QUAD_OUTPUT_MODE     = (1 << 7),

} vm_run_mode;


typedef enum _vm_err {
    VM_ERR_NONE = 0,
    VM_WRITE_OVERFLOW = -1,
    VM_READ_NO_INDEX = -2,
    VM_PARM_ERR = -4,
} vm_err;

typedef u8 vm_hdl;

/*格式化VM_区域*/
extern void vm_eraser(void);

/*
每次上电初始化VM系统,
need_defrag_when_write_full: 当vm区域写满时是否需要整理碎片
*/
extern void vm_run_mode_and_keyt_init(vm_run_mode mode);
extern vm_err vm_init(void *dev_hdl, u32 vm_addr, bool need_defrag_when_write_full);

/******
**增加vm 索引务必要递增
**变长度读写,如果结构体成员顺序变换/成员增删等原因,有一定风险,请自行评估.
*/

//若vm数据区没有空间存储,则返回VM_WRITE_OVERFLOW, 其他情况返回VM_ERR_NONE
extern vm_err vm_write(vm_hdl hdl, const void *data_buf, u8 data_len);

//若vm内部存储数据长度大于 参数data_len, 则读出data_len长度的数据并 返回 data_len;
//若vm内部存储数据长度小于 参数data_len, 则读出 vm内部存储数据长度 的数据并 返回 vm内部存储数据长度;
//若vm内部存储数据长度与参数data_len长度相符, 则返回VM_ERR_NONE
//若vm内部从没有存储过 hdl代表的 索引, 则返回 VM_READ_NO_INDEX
extern vm_err vm_read(vm_hdl hdl, void *data_buf, u8 data_len);

/*手动整理碎片*/
extern vm_err vm_defrag(void);

/*侦查 vm 区域 使用情况, 返回使用的百分比*/
extern int vm_status(void);

extern const struct DEV_IO *dev_reg_vm(void *parm);
#endif  //_VM_H_




