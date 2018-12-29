
#include "server/wifi_connect.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os/os_compat.h"
#include "wifi_ext.h"
#include "system/init.h"
#include "lwip.h"
#include "server/ctp_server.h"
#include "server/net_server.h"
#include "device/device.h"
#include "system/app_core.h"
#include "server/server_core.h"
#include "action.h"
#include "system/timer.h"
#include "http/http_server.h"
#include "asm/debug.h"
#include "app_config.h"
#include "ftpserver/stupid-ftpd.h"
#include "device/wifi_dev.h"
#include "server/network_mssdp.h"
#include "streaming_media_server/fenice_config.h"
#include "system/timer.h"
#include "server/video_rt_tcp.h"
#include "server/net2video.h"
#include "p2p_dev.h"
#include "lcd_driver.h"

#include "database.h"
extern unsigned int time_lapse(unsigned int *handle, unsigned int time_out);
struct fenice_config conf;
extern int user_cmd_cb(int cmd, char *buffer, int len, void *priv);
static struct server *ctp = NULL;
static u8 mac_addr[6];
static u8 pm_suspend_flag;
static char ssid[64];
static void *wifi_dev = NULL;
static void *cli_hdl = NULL;
static u32 pm_timehdl;

//CTP CDP 端口号
static struct ctp_server_info server_info = {
    .ctp_vaild = true,
    .ctp_port = CTP_CTRL_PORT,
    .cdp_vaild = true,
    .cdp_port = CDP_CTRL_PORT,
    .k_alive_type = CTP_ALIVE,
};

/*#define AP_AUTO_SUSPEND_TIMEOUT	30000*/
#ifdef AP_AUTO_SUSPEND_TIMEOUT
extern int hostapd_wlan_sta_authorized_num(void);

static void check_ap_if_need_suspend(void)
{
    if (wifi_module_is_init() && !pm_suspend_flag && get_cur_wifi_mode() == AP_MODE) {
        if (hostapd_wlan_sta_authorized_num() == 0) {
            if (time_lapse(&pm_timehdl, AP_AUTO_SUSPEND_TIMEOUT)) {
                dev_ioctl(wifi_dev, DEV_WIFI_PM_SUSPEND, 0);
            }
        } else {
            pm_timehdl = 0;
        }
    }
}
#endif



static unsigned int best_ch, least_cnt = -1;

static void get_best_ch_fn(unsigned int ch, unsigned int cnt)
{
    if (ch >= 12) {
        return;
    }
    if (cnt < least_cnt) { //cnt 越小 该信道干扰越小, 并且 禁止13信道
        least_cnt = cnt;
        best_ch = ch;
    }
}

#if defined CONFIG_NET_CLIENT

extern void video_disconnect_all_cli();
static int wifi_ipc_state_cb(void *priv, int on)
{
    if (on) {
    } else {
        video_disconnect_all_cli();
    }
    return 0;
}

#endif

#if defined CONFIG_NET_SERVER
extern void ipc_connect();
extern void ipc_disconnect();
static int wifi_ipc_state_cb(void *priv, int on)
{
    if (on) {
        ipc_connect();
    } else {
        ipc_disconnect();
    }
    return 0;
}


#endif

static void wifi_smp_connect_task(void *p)
{
    struct cfg_info info;
    dev_ioctl(wifi_dev, DEV_GET_WIFI_SMP_RESULT, (u32)&info);

    info.mode = STA_MODE;
    dev_ioctl(wifi_dev, DEV_STA_MODE, (u32)&info);
}

static int network_user_callback(void *network_ctx, enum NETWORK_EVENT state, void *priv)
{
    struct cfg_info info;

    switch (state) {

    case WIFI_EVENT_MODULE_INIT:

#if 0
        gpio_set_hd(IO_PORTG_00, 0);
        gpio_set_hd(IO_PORTG_01, 0);
        gpio_set_hd(IO_PORTG_02, 0);
        gpio_set_hd(IO_PORTG_03, 0);
        gpio_set_hd(IO_PORTG_04, 0);
        gpio_set_hd(IO_PORTG_05, 0);

#endif


//wifi module port seting
        info.port_status = 0;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_CS, (u32)&info);

        info.port_status = 1;
        dev_ioctl(wifi_dev, DEV_SET_WIFI_WKUP, (u32)&info);

        msleep(100);

        info.mode = AP_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SET_DEFAULT_MODE, (u32)&info);
        break;

    case WIFI_EVENT_MODULE_START:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START\n");

//void linked_info_dump(int padapter, u8 benable);
//void *netdev_priv(struct net_device *dev);
//void  *net_dev_find(void);
//linked_info_dump(*(int*)netdev_priv(net_dev_find()), 1);

#if 0 //是否使能自动扫描获取最佳信道
        int rtw_scan(void);
        rtw_scan();//扫描1秒
        rtw_scan();//扫描1秒
        rtw_scan();//扫描1秒
        int rtw_get_best_ch(void (*get_best_ch_fn)(int, int));
        rtw_get_best_ch(get_best_ch_fn);
        printf("hostapd_set_wifi_channel = %d\r\n", best_ch);
        int hostapd_set_wifi_channel(int channel);
        hostapd_set_wifi_channel(best_ch);
#endif


//设置VM保存AP_MODE
#if defined (WIFI_CAM_SUFFIX)

        sprintf(ssid, WIFI_CAM_PREFIX WIFI_CAM_SUFFIX);

#else
        dev_ioctl(wifi_dev, DEV_GET_MAC, (u32)&mac_addr);
        sprintf(ssid, WIFI_CAM_PREFIX"%02x%02x%02x%02x%02x%02x"
                , mac_addr[0]
                , mac_addr[1]
                , mac_addr[2]
                , mac_addr[3]
                , mac_addr[4]
                , mac_addr[5]);

#endif


        info.ssid = ssid;
        info.pwd = "12345678";
        info.mode = AP_MODE;
//        info.p2p_role = P2P_GC_MODE;
        info.force_default_mode = 0;
        dev_ioctl(wifi_dev, DEV_SAVE_DEFAULT_MODE, (u32)&info);

        break;

    case WIFI_EVENT_MODULE_STOP:
        puts("|network_user_callback->WIFI_EVENT_MODULE_STOP\n");
        break;

    case WIFI_EVENT_AP_START:
        puts("|network_user_callback->WIFI_EVENT_AP_START\n");
        break;

    case WIFI_EVENT_AP_STOP:
        puts("|network_user_callback->WIFI_EVENT_AP_STOP\n");

#if 0//8801要if 1，8189不能开，开了切换不了模式
        info.port_status = 0;
        printf("---------DEV_SET_WIFI_POWER-OFF-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        printf("---------DEV_SET_WIFI_POWER-ON-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_START:
        puts("|network_user_callback->WIFI_EVENT_STA_START\n");
        break;
    case WIFI_EVENT_MODULE_START_ERR:
        puts("|network_user_callback->WIFI_EVENT_MODULE_START_ERR\n");
        break;

    case WIFI_EVENT_STA_STOP:
        puts("|network_user_callback->WIFI_EVENT_STA_STOP\n");
#if 0
        info.port_status = 0;
        printf("---------DEV_SET_WIFI_POWER-OFF-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
        msleep(10);
        info.port_status = 1;
        printf("---------DEV_SET_WIFI_POWER-ON-----------\r\n");
        dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER, (u32)&info);
#endif
        break;

    case WIFI_EVENT_STA_DISCONNECT:
        puts("|network_user_callback->WIFI_STA_DISCONNECT\n");
#if defined CONFIG_NET_CLIENT
        wifi_ipc_state_cb(NULL, 0);
#endif
        break;

    case WIFI_EVENT_STA_SCAN_COMPLETED:

        /* if(wpa_supplicant_get_state() != STA_WPA_CONNECT_COMPLETED) */
        /* { */
        puts("|network_user_callback->WIFI_STA_SCAN_COMPLETED\n");
#if 0
        dev_ioctl(wifi_dev, DEV_GET_STA_SSID_INFO, (u32)&info);
        printf("sta_ssid_num->%d\n", info.sta_ssid_num);
#endif
        /* } */
        break;

    case WIFI_EVENT_STA_CONNECT_SUCC:
        dev_ioctl(wifi_dev, DEV_GET_WIFI_CHANNEL, (u32)&info);
        printf("|network_user_callback->WIFI_STA_CONNECT_SUCC,CH=%d\r\n", info.sta_channel);
        break;

    case WIFI_EVENT_MP_TEST_START:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_START\n");
        break;
    case WIFI_EVENT_MP_TEST_STOP:
        puts("|network_user_callback->WIFI_EVENT_MP_TEST_STOP\n");
        break;

    case WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID\n");
        break;
    case WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL:
        puts("|network_user_callback->WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL\n");
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_STA_NETWPRK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_P2P_START:
        puts("|network_user_callback->WIFI_EVENT_P2P_START\n");
        break;
    case WIFI_EVENT_P2P_STOP:
        puts("|network_user_callback->WIFI_EVENT_P2P_STOP\n");
        break;
    case WIFI_EVENT_P2P_GC_DISCONNECTED:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_DISCONNECTED\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC\n");
        break;
    case WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT\n");
        break;

    case WIFI_EVENT_SMP_CFG_TIMEOUT:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_TIMEOUT\n");
        break;
    case WIFI_EVENT_SMP_CFG_COMPLETED:
        puts("|network_user_callback->WIFI_EVENT_SMP_CFG_COMPLETED\n");
        thread_fork("wifi_smp_connect_task", 10, 0x1000, 0, 0, wifi_smp_connect_task, NULL);
        break;

    case WIFI_EVENT_PM_SUSPEND:
        puts("|network_user_callback->WIFI_EVENT_PM_SUSPEND\n");
        pm_suspend_flag = 1;
        break;
    case WIFI_EVENT_PM_RESUME:
        puts("|network_user_callback->WIFI_EVENT_PM_RESUME\n");
        pm_suspend_flag = 0;
        pm_timehdl = 0;
        break;

    case WIFI_EVENT_AP_ON_ASSOC:
        ;
        struct eth_addr *hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_ASSOC hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
        void stream_media_server_dhwaddr_close(struct eth_addr * dhwaddr);
        stream_media_server_dhwaddr_close(hwaddr);

		lcd_backlight_ctrl(false);
		sys_key_event_disable();
        break;
    case WIFI_EVENT_AP_ON_DISCONNECTED:
        hwaddr = (struct eth_addr *)network_ctx;
        printf("WIFI_EVENT_AP_ON_DISCONNECTED hwaddr = %02x:%02x:%02x:%02x:%02x:%02x \r\n\r\n",
               hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);

//        extern void dhcps_release_ipaddr(void *hwaddr);
//        dhcps_release_ipaddr(hwaddr->addr);//释放IP地址池,不同手机使用同一个IP地址

        ctp_keep_alive_find_dhwaddr_disconnect((struct eth_addr *)hwaddr->addr);

        //强制断开所有RTSP链接,实际上应该只断开响应客户端
        void stream_media_server_dhwaddr_close(struct eth_addr * dhwaddr);
        stream_media_server_dhwaddr_close(hwaddr);
#if defined CONFIG_NET_SERVER
        wifi_ipc_state_cb(NULL, 0);
        network_mssdp_init();
#endif
		lcd_backlight_ctrl(true);
		sys_key_event_enable();
        break;
    default:
        break;
    }

    return 0;
}


static void wifi_set_lan_setting_info(void)
{
    struct lan_setting lan_setting_info = {

        .WIRELESS_IP_ADDR0  = 192,
        .WIRELESS_IP_ADDR1  = 168,
        .WIRELESS_IP_ADDR2  = 1,
        .WIRELESS_IP_ADDR3  = 1,

        .WIRELESS_NETMASK0  = 255,
        .WIRELESS_NETMASK1  = 255,
        .WIRELESS_NETMASK2  = 255,
        .WIRELESS_NETMASK3  = 0,

        .WIRELESS_GATEWAY0  = 192,
        .WIRELESS_GATEWAY1  = 168,
        .WIRELESS_GATEWAY2  = 1,
        .WIRELESS_GATEWAY3  = 1,

        .SERVER_IPADDR1  = 192,
        .SERVER_IPADDR2  = 168,
        .SERVER_IPADDR3  = 1,
        .SERVER_IPADDR4  = 1,

        .CLIENT_IPADDR1  = 192,
        .CLIENT_IPADDR2  = 168,
        .CLIENT_IPADDR3  = 1,
        .CLIENT_IPADDR4  = 2,

        .SUB_NET_MASK1   = 255,
        .SUB_NET_MASK2   = 255,
        .SUB_NET_MASK3   = 255,
        .SUB_NET_MASK4   = 0,
    };

    struct cfg_info info;
    info.__lan_setting_info = &lan_setting_info;
    dev_ioctl(wifi_dev, DEV_SET_LAN_SETTING, (u32)&info);
}



void net_app_init(void)
{
    ctp = server_open("ctp_server", (void *)&server_info);

    if (!ctp) {
        printf("ctp server fail\n");
    }

    puts("http server init\n");
    http_get_server_init(HTTP_PORT); //8080

    preview_init(VIDEO_PREVIEW_PORT, NULL); //2226

    playback_init(VIDEO_PLAYBACK_PORT, NULL);
#if defined CONFIG_NET_CLIENT
    video_rt_tcp_server_init2(29999, user_cmd_cb);
#else
    video_rt_tcp_server_init(2229);
#endif

    network_mssdp_init();

    puts("ftpd server init\n");

    extern void ftpd_vfs_interface_cfg(void);
    ftpd_vfs_interface_cfg();
    stupid_ftpd_init("MAXUSERS=2\nUSER=FTPX	12345678	 0:/	  2   A\n", NULL);


    /*
     *代码段功能:修改RTSP的URL
     *默认配置  :URL为rtsp://192.168.1.1/rt传H264实时流,不传音频
     *
     */

#if 0
    extern void rtsp_modify_url(const char *user_custom_name, const char *user_custom_content);
    char *user_custom_name = "rt";
    char *user_custom_content =
        "stream\r\n  \
file_ext_name 264\r\n \
media_source live\r\n  \
priority 1\r\n  \
payload_type 96\r\n  \
clock_rate 90000\r\n  \
encoding_name H264\r\n  \
coding_type frame\r\n  \
byte_per_pckt 1458\r\n  \
stream_end";
    rtsp_modify_url(user_custom_name, user_custom_content);
#endif


    struct fenice_config conf;
    extern int fenice_video_rec_setup(void);
    extern int fenice_video_rec_exit(void);
    extern int fenice_video_rec_info(struct fenice_source_info * info);
    strncpy(conf.protocol, "UDP", 3);
    conf.exit = fenice_video_rec_exit;
    conf.setup = fenice_video_rec_setup;
    conf.info = fenice_video_rec_info;
    conf.port = RTSP_PORT;  /* 当为0时,用默认端口554 */

    extern int stream_media_server_init(struct fenice_config * conf);
    stream_media_server_init(&conf);


}
void net_app_uninit(void)
{
    puts("ctp server init\n");
    server_close(ctp);

    puts("http server init\n");
    http_get_server_uninit(); //8080

    preview_uninit(); //2226

    playback_uninit();

    puts("ftpd server init\n");
    stupid_ftpd_uninit();
}
void wifi_on(void)
{
    dev_ioctl(wifi_dev, DEV_NETWORK_START, 0);
    net_app_init();
}


void wifi_off(void)
{
    net_app_uninit();
    dev_ioctl(wifi_dev, DEV_NETWORK_STOP, 0);
}



void wifi_app_task(void *priv)
{
    int err = 0;
    unsigned int timehdl = 0;
    struct cfg_info info = {0};

    //if (!db_select("wfo")) {
    //    puts("wifi onoff : OFF \r\n");
    //    return;
    //}


//wifi app start
    wifi_dev = dev_open("wifi", NULL);


    info.cb = network_user_callback;
    info.net_priv = NULL;
    dev_ioctl(wifi_dev, DEV_SET_CB_FUNC, (u32)&info);

#if 0
    printf("\n >>>> DEV_SET_WIFI_POWER_SAVE<<<<   \n");
    dev_ioctl(wifi_dev, DEV_SET_WIFI_POWER_SAVE, 0);//打开就启用低功耗模式, 只有STA模式才有用
#endif

#if 1
    printf("\n >>>> DEV_SET_WIFI_TX_PWR_BY_RATE<<<   \n");

    info.tx_pwr_lmt_enable = 0;//  解除WIFI发送功率限制
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_LMT_ENABLE, (u32)&info);
    info.tx_pwr_by_rate = 1;// 设置WIFI根据不同datarate打不同power
    dev_ioctl(wifi_dev, DEV_SET_WIFI_TX_PWR_BY_RATE, (u32)&info);
#endif


    wifi_set_lan_setting_info();

    //wifi_on();
	
//网络测试工具，使用iperf
    //extern void iperf_test(void);
    //iperf_test();

    while (1) {

#ifdef AP_AUTO_SUSPEND_TIMEOUT
        check_ap_if_need_suspend();
#endif

        /* CLR_WDT(); */
        if (time_lapse(&timehdl, 3 * 1000)) {
            /* malloc_stats(); */
//                malloc_debug_show();
            if (wifi_module_is_init()) {

//                stats_display();
                printf("WIFI U= %d KB/s, D= %d KB/s\r\n", wifi_get_upload_rate() / 1024, wifi_get_download_rate() / 1024);
#if 0
                if (get_cur_wifi_info()->mode == STA_MODE) {
                    get_rx_signal();
                }
#endif
            }
        }

        msleep(2000);

    }

}


int wireless_net_init(void)//主要是create wifi 线程的
{
    puts("wifi early init \n\n\n\n\n\n");
    thread_fork("wifi_app_task", 10, 0x1000, 0, 0, wifi_app_task, NULL);
    return 0;
}

#if defined CONFIG_WIFI_ENABLE
late_initcall(wireless_net_init);
#endif

unsigned short DUMP_PORT()
{
    return _DUMP_PORT;
}

unsigned short FORWARD_PORT()
{
    return _FORWARD_PORT;
}

unsigned short BEHIND_PORT()
{
    return _BEHIND_PORT;
}

const char *get_rec_path_1()
{
    return CONFIG_REC_PATH_1;
}
const char *get_rec_path_2()
{
    return CONFIG_REC_PATH_2;
}
const char *get_root_path()
{
    return CONFIG_ROOT_PATH;
}

static struct cfg_info wifi_info;
char *get_wifi_ssid(void)
{
    wifi_info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&wifi_info);
    return wifi_info.ssid;
}


char *get_wifi_pwd(void)
{
    wifi_info.mode = NONE_MODE;
    dev_ioctl(wifi_dev, DEV_GET_CUR_WIFI_INFO, (u32)&wifi_info);
    return wifi_info.pwd;
}


#if defined CONFIG_NET_SERVER
static void dhcp_charge_device_event_handler(struct sys_event *event)
{
    if (!strncmp(event->arg, "dhcp_srv", 8)) {
        if (event->u.dev.event == DEVICE_EVENT_CHANGE) {
            wifi_ipc_state_cb(NULL, 1);

        }
    }
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, dhcp_charge_device_event_handler, 0);

#endif

#if 0
void sdio_recv_pkg_irq(void)
{
    static u32 thdll, count222;
    int ret22;
    ret22 = time_lapse(&thdll, 1000);
    if (ret22) {
        /* printf("sdio_recv_cnt = %d,  %d \r\n", ret22, count222); */
        count222 = 0;
    }
    ++count222;
}
#endif
