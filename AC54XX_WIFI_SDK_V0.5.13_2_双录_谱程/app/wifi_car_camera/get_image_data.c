#include "system/includes.h"
#include "os/os_compat.h"
#include "get_image_data.h"

static get_yuv_cfg  __info;

void get_yuv_task(void *priv)
{
    void *video_dev_fd = NULL, *fb2 = NULL;
    struct fb_var_screeninfo info = {0};
    struct fb_map_user map1;

    os_time_dly(100);
    //数据来源video，输出到fb2
    struct video_format f = {0};
    const char *dev_name = YUV_DATA_SOURCE;
    f.type  = VIDEO_BUF_TYPE_VIDEO_OVERLAY;
    f.fmt.win.left = 0;
    f.fmt.win.top    = 0;
    f.fmt.win.width  = YUV_DATA_WIDTH;
    f.fmt.win.height = YUV_DATA_HEIGHT;
    f.fmt.win.border_left   = 0;
    f.fmt.win.border_top    = 0;
    f.fmt.win.border_right  = 0;
    f.fmt.win.border_bottom = 0;
    f.fmt.win.scale = SCALE_NONE;//不插值
    f.private_data   = "fb2";
    video_dev_fd = dev_open(dev_name, &f);
    if (video_dev_fd == NULL) {
        printf("fb2 test open device %s faild\n", dev_name);
        return;
    }

    //打开显示通道
    fb2 = dev_open("fb2", (void *)FB_COLOR_FORMAT_YUV420);
    if (!fb2) {
        printf(" ||| %s ,,, %d \r\n", __FUNCTION__, __LINE__);
        dev_close(video_dev_fd);
        return;
    }

    dev_ioctl(video_dev_fd, VIDIOC_OVERLAY, 1);

    while (1) {

        if (__info.exit_state) {
            __info.exit_state = 0;
            break;
        }

        dev_ioctl(fb2, FBIOGET_FBUFFER_INUSED, (int)&map1);
        if ((u32)map1.baddr) {

            //需要使用YUV数据的任务的回调
            //printf("\nmap1.baddr is 0x%x\n",map1.baddr);
            __info.cb(map1.baddr);
            //获取到才释放
            dev_ioctl(fb2, FBIOPUT_FBUFFER_INUSED, (int)&map1);
        } else {
            //获取不到重试
            /* printf("\r\n ###### = 0x%x \r\n", *(u32 *)map1.baddr); */
            continue;
        }

    }

    dev_close(fb2);
    dev_ioctl(video_dev_fd, VIDIOC_OVERLAY, 0);
    dev_close(video_dev_fd);

}

static u8 get_yuv_init_flag;
void get_yuv_init(void (*cb)(u8 *data))
{
    if (get_yuv_init_flag == 1) {
        return;
    }
    get_yuv_init_flag = 1;
    __info.cb = cb;
    __info.exit_state = 0;
    thread_fork("GET_YUV_TASK", 15, 0x2000, 0, &__info.pid, get_yuv_task, NULL);
}
void get_yuv_uninit(void)
{
    if (get_yuv_init_flag == 0) {
        return;
    }
    get_yuv_init_flag = 0;
    __info.exit_state = 1;
    thread_kill(&__info.pid, KILL_WAIT);
}
