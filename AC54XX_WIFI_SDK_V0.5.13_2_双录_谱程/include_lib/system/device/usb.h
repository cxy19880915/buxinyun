
#ifndef __USB__API_H__
#define __USB__API_H__
#include "typedef.h"
#include "ioctl.h"
#include "device/device.h"
#ifdef __cplusplus
extern "C" {
#endif



struct uvc_parm {
    s16 brightness_min;
    s16 brightness_max;
    s16 brightness_def;
    s16 brightness_res;
    s16 brightness_cur;

    s16 contrast_min;
    s16 contrast_max;
    s16 contrast_def;
    s16 contrast_res;
    s16 contrast_cur;

    s16 hue_min;
    s16 hue_max;
    s16 hue_def;
    s16 hue_res;
    s16 hue_cur;

    s16 saturation_min;
    s16 saturation_max;
    s16 saturation_def;
    s16 saturation_res;
    s16 saturation_cur;

    s16 sharpness_min;
    s16 sharpness_max;
    s16 sharpness_def;
    s16 sharpness_res;
    s16 sharpness_cur;

    s16 gamma_min;
    s16 gamma_max;
    s16 gamma_def;
    s16 gamma_res;
    s16 gamma_cur;

    s16 white_balance_temp_min;
    s16 white_balance_temp_max;
    s16 white_balance_temp_def;
    s16 white_balance_temp_res;
    s16 white_balance_temp_cur;

    s16 power_line_freq_min;
    s16 power_line_freq_max;
    s16 power_line_freq_def;
    s16 power_line_freq_res;
    s16 power_line_freq_cur;
};
struct usb_host_parm {
    u32 hspeed;
    u32 ot;
};


//video和hid不可以同时使能
#ifndef     USB_MSD_EN
#define     USB_MSD_EN           1
#define     USB_MIC_EN          0
#define     USB_SPEAKER_EN       0
#define     USB_HID_EN           0
#define     USB_VIDEO_EN         1
#define 	USB_VIDEO_PRIVATE_EN 0
#endif

#define MASSSTORAGE_CLASS   (BIT(0))
#define SPEAKER_CLASS       (BIT(1))
#define MIC_CLASS           (BIT(2))
#define AUDIO_CLASS         ((BIT(1)|BIT(2)))
#define HID_CLASS           (BIT(3))
#define VIDEO_CLASS         (BIT(4))

typedef struct _usb_slv_str_desc {
    u32 vid_pid;
    const u8 *manufacture;
    const u8 *product_desc;
    const u8 *chipserial;
    const u8 *msd_desc;
    const u8 *spk_desc;
    const u8 *mic_desc;
    const u8 *video_desc;
} USB_SLV_STR_DESC;

struct usb_slv_arg {
    u16 ot;
    const USB_SLV_STR_DESC *pstr_desc;
    void *parm;

};
struct uvc_frame_info {
    u16 width;
    u16 height;
};
typedef struct camera_info {
    struct uvc_frame_info frame[5];
} CAMERA_INFO;


typedef enum _usb_audio_ctl_type {
    USB_MIC_VOLUME = 0x10,
    USB_MIC_LEFT_VOLUME,
    USB_MIC_RIGHT_VOLUME,
    USB_MIC_MUTE,

    USB_SPK_VOLUME = 0x20,
    USB_SPK_LEFT_VOLUME,
    USB_SPK_RIGHT_VOLUME,
    USB_SPK_MUTE,
} USB_AUDIO_CTL_TYPE;

enum uvc_event {
    UVC_BUTTON_PRESSED = 0,
    UVC_BUTTON_RELEASED,
    UVC_STREAM_ERROR,
    UVC_CONTROL_CHANGE,
};
typedef u16(* USB_MIC_GET_DATA)(void *ptr, u16 len);
typedef void(* USB_SPK_WRITE_DATA)(void *ptr, u16 len);
typedef u32(* USB_AUDIO_VOLUME_CTL)(USB_AUDIO_CTL_TYPE type, u32 volume); //

typedef void (*OPEN_CAMERA)(u32 frame_index);
typedef void (*CLOSE_CAMERA)();
typedef void *(*CONFIG_CMAERA)(u32 cmd, void *parm);
typedef u32(*USB_VIDEO_INDATA)(void *ptr, u32 len, u32 *is_frame_end);

struct jl_msg_ops {
    int (*read)(void *dev, void *buf, u32 len);
    int (*send)(void *dev, const void *buf, u32 len);
};

struct scsi_private_request {
    struct jl_msg_ops ops;
    void *dev;
    u8 *cmd;
    u32 cmd_len;
};

struct uvc_unit_ctrl {
    u8  request;
    u8  unit;
    u32 len;
    u8 *data;
};

struct usb_mass_storage {
    int (*dev_attribute)(int id);
    int (*dev_read)(int id, u8 *buf, u32 addr, int len);
    int (*dev_write)(int id, u8 *buf, u32 addr, int len);
    int (*dev_ask_block_num)(int id);
    int (*dev_ask_block_size)(int id);
    int (*dev_ask_lun_num)(void);
    int (*dev_state_check)(int id);
    int (*scsi_private)(struct scsi_private_request *req);
};

#define FMT_MAX_RESO_NUM    5

struct uvc_format {
    u8 format;
    u16 width[FMT_MAX_RESO_NUM * 2];
    u16 height[FMT_MAX_RESO_NUM * 2];
    int (*video_open)(int id, int fmt, int frame_id, int fps);
    int (*video_reqbuf)(int id, void *buf, u32 len, u32 *frame_end);
    int (*video_close)(int id);
    int (*processing_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*private_cmd_response)(u16 cmd, u16 data);
};

struct usb_uvc_camera {
    u8 bind;
    u8 format;
    u8 interface_num;
    u16 width[2][FMT_MAX_RESO_NUM * 2];
    u16 height[2][FMT_MAX_RESO_NUM * 2];
    int (*video_open)(int id, int fmt, int frame_id, int fps);
    int (*video_reqbuf)(int id, void *buf, u32 len, u32 *frame_end);
    int (*video_close)(int id);
    int (*processing_unit_response)(struct uvc_unit_ctrl *ctl_req);
    int (*private_cmd_response)(u16 cmd, u16 data);
};

typedef struct _usb_audio_arg {
    u32 uac_magic;
    USB_SPK_WRITE_DATA usb_spk_write_data;
    USB_MIC_GET_DATA usb_mic_get_data;
    USB_AUDIO_VOLUME_CTL usb_audio_volume_ctl;
} USB_AUDIO_ARG;

/*
 * uvc host
 */
struct uvc_reqbufs {
    void *buf;
    int size;
};

struct uvc_capability {
    int fmt;
    int fps;
    int reso_num;
    struct uvc_frame_info reso[8];
};

struct uvc_processing_unit {
    u8 request;
    u8 type;
    u16 value;
    u16 index;
    u8 buf[4];
    int len;
};

struct uvc_event_listener {
    void *priv;
    int (*handler)(void *priv, enum uvc_event event);
};
struct usb_device_id {
    u16 vendor;
    u16 product;
};

struct usb_string {
    u8 code_type;
    u16 language;
    int len;
    u8 *string;
};

#define UVC_CAMERA_FMT_YUY2     0x1
#define UVC_CAMERA_FMT_MJPG     0x2
#define UVC_CAMERA_FMT_H264     0x3

#define USBIOC_MASS_STORAGE_CONNECT     _IOW('U', 0, sizeof(struct usb_mass_storage))
#define USBIOC_UVC_CAMERA0_CONNECT      _IOW('U', 1, sizeof(struct uvc_format))
#define USBIOC_UVC_CAMERA1_CONNECT      _IOW('U', 2, sizeof(struct uvc_format))
#define USBIOC_SLAVE_MODE_START         _IOW('U', 3, sizeof(unsigned int))
#define USBIOC_SLAVE_DISCONNECT         _IOW('U', 4, sizeof(unsigned int))
#define UVCIOC_QUERYCAP                 _IOR('U', 5, sizeof(struct uvc_capability))
#define UVCIOC_SET_CAP_SIZE             _IOW('U', 5, sizeof(unsigned int))
#define UVCIOC_STREAM_ON                _IOW('U', 7, sizeof(unsigned int))
#define UVCIOC_STREAM_OFF               _IOW('U', 8, sizeof(unsigned int))
#define UVCIOC_REQBUFS                  _IOW('U', 9, sizeof(unsigned int))
#define UVCIOC_DQBUF                    _IOW('U', 10, sizeof(unsigned int))
#define UVCIOC_QBUF                     _IOW('U', 11, sizeof(unsigned int))
#define UVCIOC_RESET                    _IOW('U', 12, sizeof(unsigned int))
#define UVCIOC_REQ_PROCESSING_UNIT      _IOR('U', 13, sizeof(struct uvc_processing_unit))
#define UVCIOC_SET_PROCESSING_UNIT      _IOW('U', 14, sizeof(struct uvc_processing_unit))
#define UVCIOC_GET_DEVICE_ID            _IOR('U', 15, sizeof(struct usb_device_id))

#define USBIOC_SLAVE_RESET              _IOW('U', 16, sizeof(unsigned int))

/***********2018-06-21************/
#define USBIOC_GET_DEVICE_ID            _IOR('U', 17, sizeof(struct usb_device_id))
#define USBIOC_GET_MANUFACTURER         _IOR('U', 18, sizeof(struct usb_string))
#define USBIOC_GET_PRODUCT_NAME         _IOR('U', 19, sizeof(struct usb_string))
/*********************************/

/***********2018-07-30************/
#define UVCIOC_SET_EVENT_LISTENER       _IOW('U', 20, sizeof(struct uvc_event_listener))

#define     USB_SLAVE_RUN			1002
#define     USB_DISABLECONNECT		1003
#define     USB_ENABLECONNECT		1004
#define     USB_FORCE_RESET			1005

#define     IOCTL_GET_MAX_LUN		1006
#define     IOCTL_GET_CUR_LUN		1007
#define     IOCTL_SET_CUR_LUN		1008

struct usb_platform_data {
    u32 id;
    u32 ctl_irq_int;
    u32 online_check_cnt;
    u32 offline_check_cnt;
    u32 isr_priority;
    u32 host_ot;
    u32 host_speed;
    u32 slave_ot;
};
#define USB_PLATFORM_DATA_BEGIN(data) \
	static const struct usb_platform_data data = {\


#define USB_PLATFORM_DATA_END()  \
};


/*****************USB WIFI**********************/
#include "os/os_compat.h"
#define USB_WIFI_GET_NUM	_IOR('W',0,sizeof(int))
#define USB_WIFI_GET_FUNC	_IOR('W',1,sizeof(int))

typedef struct usb_wifi_fun {
    /*usb_wifi_dev lib function*/
    void *(*usb_open)(void *p);
    int (*usb_close)(void);
    int (*usb_online)(char *name);
    int (*usb_ioctl)(struct device *device, u32 cmd, u32 arg);

    /*wifi_todo function*/
    int (*usb_bulk_only_receive_int)(u8 ep, u8 *pbuf, u32 len);
    int (*usb_bulk_only_send)(u8 ep, u8 *pbuf, u32 len);
    int (*usb_ctl_msg)(u8 Request, u8 RequestType, u16 Value, u16 Index, void *Date, u16 Length);
} USB_WIFI_FUN;

typedef void(* USB_WIFI_RX_COMPLETE_CALLBACK)(u32);
extern void set_usb_wifi_rx_complete_callback(USB_WIFI_RX_COMPLETE_CALLBACK func, char usb_index);
extern char *usb_wifi_get_name(void);
extern int husb_wifi_isr(void);
extern USB_WIFI_FUN *husb_wifi_func_get(void);
extern int husb_wifi_init(struct device *device);
extern int husb_wifi_exit(struct device *device);
extern const struct device_operations usbwifi_dev_ops;

typedef struct usbwifi_platform_data {
    char usb_index;
    char *name;
    void *prive;
    struct usb_device *usb_dev;
    struct device dev;

} HOST_USB_WIFI;

/*************END OF USB WIFI***************/



struct uvc_platform_data {
    u16 width;
    u16 height;
    int fps;
    int fmt;
    int mem_size;
    int timeout;
    u8  put_msg;
};

#define UVC_PLATFORM_DATA_BEGIN(data) \
    static const struct uvc_platform_data data = {\

#define UVC_PLATFORM_DATA_END()  \
};

extern const struct device_operations usb_dev_ops;
extern const struct device_operations mass_storage_ops;
extern const struct device_operations uvc_host_ops;
extern const struct device_operations uvc_dev_ops;


#ifdef __cplusplus
}
#endif

#endif
