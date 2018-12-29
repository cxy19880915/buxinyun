#include "pixel_flow.h"
#include "get_image_data.h"

static void flow_one_frame(char *inputFrame)
{

    s32 pixel_flow_x, pixel_flow_y, qual;
    qual = pixel_compute_flow(inputFrame, YUV_DATA_HEIGHT, &pixel_flow_x, &pixel_flow_y);

    printf("qual=%d,  x = %d, y = %d \r\n", qual, pixel_flow_x, pixel_flow_y);

    //此处串口发送光流信息
}
int flow_init(void)
{
    get_yuv_init(flow_one_frame);
    return 0;
}

int flow_uninit(void)
{
    get_yuv_uninit();
    return 0;
}



