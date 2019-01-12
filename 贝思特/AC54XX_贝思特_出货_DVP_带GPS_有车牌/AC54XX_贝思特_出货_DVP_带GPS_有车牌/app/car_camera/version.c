#include "system/includes.h"
#include "generic/log.h"


static int app_version_check()
{
    lib_version_check();

#ifdef VIDEO_REC_SERVER_VERSION
    VERSION_CHECK(video_server, VIDEO_REC_SERVER_VERSION);
#endif

#ifdef VIDEO_DEC_SERVER_VERSION
    VERSION_CHECK(video_dec_server, VIDEO_DEC_SERVER_VERSION);
#endif

#ifdef UI_SERVER_VERSION
    VERSION_CHECK(ui_server, UI_SERVER_VERSION);
#endif

#ifdef AUDIO_SERVER_VERSION
    VERSION_CHECK(audio_server, AUDIO_SERVER_VERSION);
#endif

#ifdef VIDEO_ENGINE_SERVER_VERSION
    VERSION_CHECK(video_engine_server, VIDEO_ENGINE_SERVER_VERSION);
#endif

#ifdef USB_SERVER_VERSION
    VERSION_CHECK(usb_server, USB_SERVER_VERSION);
#endif

#ifdef DEV_ISP_VERSION
    VERSION_CHECK(isp, DEV_ISP_VERSION);
#endif
    return 0;
}
early_initcall(app_version_check);
