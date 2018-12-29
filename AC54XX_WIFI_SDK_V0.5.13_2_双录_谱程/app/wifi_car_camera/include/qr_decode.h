#ifndef _QR_DECODE_H_
#define _QR_DECODE_H_

#include "zbar/zbar.h"
#include "get_image_data.h"

enum QR_DECODE_EVENT {

    QR_DECODE_SUC,
    QR_DECODE_TIMEOUT,
};



void qr_decode_init(void);
void qr_decode_uninit(void);

#endif //_QR_DECODE_H_
