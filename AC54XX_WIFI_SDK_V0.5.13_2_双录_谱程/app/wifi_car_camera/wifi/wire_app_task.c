
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os/os_compat.h"
#include "system/init.h"
#include "lwip.h"
#include "device/device.h"
#include "system/app_core.h"
#include "server/server_core.h"
#include "system/timer.h"
#include "sock_api/sock_api.h"
#include "app_config.h"
#include "eth/eth_phy.h"
#include "wifi_dev.h"
#include "database.h"
#include "action.h"
#include "server/rt_stream_pkg.h"

static void *dev = NULL ;

static int eth_state_cb(void *priv, int on)
{
    if (on) {
        puts("eth on \r\n");
    } else {
        puts("eth off \r\n");
    }
    return 0;
}

static void net_task(void *arg)
{
    char mac_addr[] = {0x12, 0x23, 0x22, 0x44, 0x55, 0x67};
    dev = dev_open("eth0", NULL);

    dev_ioctl(dev, DEV_SET_MAC, (int)mac_addr);

    dev_ioctl(dev, DEV_NETWORK_START, DHCP_IP);

    struct cfg_info info;
    info.cb = eth_state_cb;
    dev_ioctl(dev, DEV_SET_CB_FUNC, (u32)&info);


    //添加网络应用程序在这里

    //extern void iperf_test(void);
    //iperf_test();

    while (1) {
        msleep(10 * 1000);

        extern u32 eth_get_upload_rate(void);
        extern u32 eth_get_download_rate(void);
        printf("ETH U= %d KB/s, D= %d KB/s\r\n", eth_get_upload_rate() / 1024, eth_get_download_rate() / 1024);

    }

}

static int wire_net_init(void)//主要是有线网卡线程的
{
    puts("wire early init \n\n\n\n\n\n");

    thread_fork("net_task", 20, 0x1000, 0, 0, net_task, NULL);
    return 0;
}
#if defined CONFIG_ETH_PHY_ENABLE
late_initcall(wire_net_init);
#endif

