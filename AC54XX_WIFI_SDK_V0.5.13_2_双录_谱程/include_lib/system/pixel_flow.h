#ifndef _PIXEL_FLOW_H_
#define _PIXEL_FLOW_H_

#include "typedef.h"

extern u8 pixel_compute_flow(u8 *image2, u32 image_width, s32 *pixel_flow_x, s32 *pixel_flow_y);
extern u8 pixel_compute_flow_float(u8 *image2, u32 image_width, float *pixel_flow_x, float *pixel_flow_y);

#endif

