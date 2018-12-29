#ifndef __RF_API_H__
#define __RF_API_H__
#include "typedef.h"
#include "ioctl.h"
#include "printf.h"
#include "cpu.h"
#include "asm/spi.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"
#include "generic/gpio.h"
#include "generic/typedef.h"
#include "asm/port_waked_up.h"


/**************************************/
struct rf_platform_data {
    u8 *name;
    u16 width;
    u16 height;
    u16 audio_sample_rate;
    int fps;
    int fmt;
    int mem_size;
};

#define RF_PLATFORM_DATA_BEGIN(data) \
	static const struct rf_platform_data data = {


#define RF_PLATFORM_DATA_END() \
};
/**********************************/
struct rf_phy_device {
    char *name;
    const struct device_operations *ops;
};

#define REGISTER_RF_DEVICE(dev) \
	static struct rf_phy_device dev sec(.rf_phy_device)


extern struct rf_phy_device rf_phy_device_begin[];
extern struct rf_phy_device rf_phy_device_end[];

extern const struct device_operations rf_phy_dev_ops;
#define RFIOC_SET_CAP_SIZE             _IOW('U', 1, sizeof(unsigned int))
#define RFIOC_STREAM_ON                _IOW('U', 2, sizeof(unsigned int))
#define RFIOC_STREAM_OFF               _IOW('U', 3, sizeof(unsigned int))
#define RFIOC_REQBUFS                  _IOW('U', 4, sizeof(unsigned int))
#define RFIOC_DQBUF                    _IOW('U', 5, sizeof(unsigned int))
#define RFIOC_QBUF                     _IOW('U', 6, sizeof(unsigned int))
#define RFIOC_RESET                    _IOW('U', 7, sizeof(unsigned int))
#define RFIOC_SET_CH                   _IOW('U', 8, sizeof(unsigned int))

/*********************************命令包结构******************************************
命令起始标识	地址标识	命令号	命令参数长度 Len	命令参数args	CRC
	2bit        6bit     	1byte   	1byte       	Len byte    	2byte
说明：
命令起始标识：      2bit 固定为 0b10；
地址标识：          6 bit 可设置范围[0-63]。用于一对多情况。暂不用，都填0
命令号 :            1byte，[0-255]  用于区分不同命令 参见命令定义；
命令参数长度  :     1byte可设置范围[0-255]，标识参数长度
命令参数：          xbyte 用于传递相关命令的信息
CRC校验：           2byte
************************************************************************************/
/**************************************数据包结构************************************
数据起始标识	地址标识	数据包序号	数据包长度 Len		data			CRC
	2bit		6bit		1byte			2byte			Len byte		2byte

说明：
数据起始标识：       2bit 固定为 0b01；
地址标识：           6 bit 可设置范围[0-63]，用于一对多情况。暂不用，都填0
数据包序号 :          1byte，[0-255], 序号循环递增；
数据包长度：          2byte   标识数据包的长度；
CRC校验：            2byte
********************************************************************************/
#define CMD_START  0x01
#define CMD_END    0x02
#define CMD_CHECK  0x03
#define CMD_RESEND 0x04
#define CMD_ACK    0x05

#define CMD_ID    0x80   //命令包起始标识 2bit
#define DATA_ID   0x40   //数据包起始标识 2bit

struct rf_reqbufs {
    void *buf;
    int size;
};
#pragma pack(1)
struct _data_pack_head {
    u8 code;
    u8 p_seq;
    u16 p_len;
    // u16 crc;
};
#pragma pack()


#pragma pack(1)
struct _cmd_pack_head {
    u8 code;
    u8 cmd_num;
    u8 p_len;
    // u16 crc;
};

#pragma pack()


#pragma pack(1)
struct rf_frm_tag {
    u32 frm_len;
    u16 frm_seq;//seq自加
    u8 type;//视频类型
    /* u16  p_len; */
    /* u16  p_seq; */
    u16 crc;
};
#pragma pack()


struct rf_fh {
    u8 *name;
    u8 eof;
    u8 drop_frame;
    u8 streamoff;
    /* u8 uvc_out; */
    struct list_head entry;
    int open;
    int streamon;
    int free_size;
    u8 *buf;
    int b_offset;
    // struct dma_list dma_list[16];
    struct device device;
    struct videobuf_queue video_q;
    void *private_data;
    OS_SEM sem;
};
struct rf_videobuf {
    u32 len;
    u32 magic;
    u8 data[0];
};
#endif
