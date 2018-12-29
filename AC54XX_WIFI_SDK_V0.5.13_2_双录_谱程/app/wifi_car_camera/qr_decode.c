#include "system/includes.h"
#include "os/os_compat.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "qr_decode.h"

static zbar_image_scanner_t *scanner = NULL;
static u8 qr_init_state = 0;

static int set_wifi_mode(const char *ssid, const char *pwd, int mode, int save_state)
{
    void *wifi_dev = NULL;
    wifi_dev = dev_open("wifi", NULL);

    if (ssid == NULL) {
        printf("ssid is null\n");
        return 0;
    }
    extern void ctp_srv_disconnect_all_cli(void);
    //断开所有客户端
    ctp_srv_disconnect_all_cli();
    //填充STA_MODE所要连接的路由器热点名称和密码
    struct cfg_info info;
    info.ssid = ssid;
    info.pwd = pwd;
    info.mode = mode;
    if (save_state) {
        dev_ioctl(wifi_dev, DEV_CHANGE_SAVING_MODE, (u32)&info);
    }
    if (mode == STA_MODE) {
        //切换WIFI模式到STA模式,切换成功后设备自行连接上热点
        dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
    } else {
        //切换WIFI模式到STA模式,切换成功后设备自行连接上热点
        dev_ioctl(wifi_dev, DEV_AP_MODE, (u32)&info);
    }

    dev_close(wifi_dev);

    return 0;
}

void qr_decode_result(enum QR_DECODE_EVENT type, char *qr_type, char *data, void *priv)
{
    switch (type) {
    case QR_DECODE_SUC:
        printf("\n qr_type is %s \n", qr_type);
        json_object *new_obj = NULL;
        json_object *tmp;
        char *ssid, *pwd, *save, *auth;

        new_obj = json_tokener_parse(data);
        tmp =  json_object_object_get(new_obj, "AUTH");
        auth = json_object_get_string(tmp);
        printf("\nauth is %s\n", auth);
        if (!strcmp(auth, "JL_ONLY")) {
            tmp =  json_object_object_get(new_obj, "SSID");
            ssid = json_object_get_string(tmp);
            printf("\nssid is %s\n", ssid);
            tmp =  json_object_object_get(new_obj, "PWD");
            pwd = json_object_get_string(tmp);
            printf("\npwd is %s\n", pwd);
            tmp =  json_object_object_get(new_obj, "SAVE");
            save = json_object_get_string(tmp);
            printf("\nsave is %d\n", atoi(save));
            set_wifi_mode(ssid, pwd, STA_MODE, atoi(save));
        }
        json_object_put(new_obj);
        break;
    case QR_DECODE_TIMEOUT:
        break;
    }
}

void qr_decode_cb_func(u8 *image_data)
{
    if (qr_init_state == 0) {
        if (scanner != NULL) {
            printf("\n\n zbar_image_scanner_destroy \n\n");
            zbar_image_scanner_destroy(scanner);
            scanner = NULL;
        }
        return;
    }
    if (scanner == NULL) {
        return;
    }
    zbar_image_t *image = NULL;
    char *data = NULL;
    zbar_symbol_type_t typ;
    /* wrap image data */
    image = zbar_image_create();
    zbar_image_set_format(image, zbar_fourcc('Y', '8', '0', '0'));
    zbar_image_set_size(image, YUV_DATA_WIDTH, YUV_DATA_HEIGHT);
    zbar_image_set_data(image, image_data, YUV_DATA_WIDTH * YUV_DATA_HEIGHT, NULL);
    zbar_scan_image(scanner, image);
    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for (; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        typ = zbar_symbol_get_type(symbol);
        data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(typ), data);
        qr_decode_result(QR_DECODE_SUC, zbar_get_symbol_name(typ), data, NULL);

    }
    zbar_image_destroy(image);
}

void qr_decode_init(void)
{
    if (qr_init_state == 1) {
        return;
    }
    qr_init_state = 1;
    /* create a reader */
    scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

}

void qr_decode_uninit(void)
{
    qr_init_state = 0;
}

u8 get_qr_init_state(void)
{
    return qr_init_state;
}
