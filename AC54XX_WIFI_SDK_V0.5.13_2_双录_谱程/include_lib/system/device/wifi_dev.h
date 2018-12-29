#ifndef  __WIFI_DEV_H__
#define  __WIFI_DEV_H__


#include "asm/sdio_host_init.h"
#include "server/wifi_connect.h"
//#include "lwip.h"//不能加这里会导致编译错误

struct wifi_platform_data {
    enum WIFI_MODULE module;
    u32 sdio_parm;
    int wakeup_port;
    int cs_port;
    int power_port;
};
/* static int wifi_ioctl(struct device *device, int cmd, int parm)*/
/*
 *device :设备句柄
 *cmd    :例如(DEV_AP_MODE)命令
 *parm   :使用cfg_info结构,设置类的命令填充cfg_info参数如下表,返回类的命令，传入cfg_info结构
 *return : 0 成功 -1 失败
 * */


/*  cfg_info结构设置参数及返回参数
|类型                            |设置参数         |返回参数         |
|DEV_AP_MODE                     |mode,ssid,pwd    |mode,ssid,pwd    |
|DEV_STA_MODE                    |mode,ssid,pwd    |mode,ssid,pwd    |
|DEV_SMP_MODE                    |未实现           |未实现           |
|DEV_P2P_MODE                    |mode,p2p_role    |未实现           |
|DEV_P2P_MODE_CHANGE             |p2p_role         |未实现           |
|DEV_NET_SCANF                   |NULL             |NULL             |
|DEV_SET_AP_CONFIG               |ap_config_table  |ap_config_table  |
|DEV_GET_MAC                     |parm传入char指针 |parm传入char指针 |
|DEV_SET_DEFAULT_MODE            |mode,ssid,pwd,force |null          |
|DEV_SAVE_DEFAULT_MODE           |mode,ssid,pwd,force |null          |
|DEV_CHANGE_SAVING_MODE          |mode,ssid,pwd    |mode,ssid,pwd    |
|DEV_SET_CB_FUNC                 |cb,net_priv      |cb,net_priv      |
|DEV_NETWORK_START               |NULL             |NULL             |
|DEV_NETWORK_STOP                |NULL             |NULL             |
|DEV_SET_WIFI_CS                 |port_status      |port_status      |
|DEV_SET_WIFI_WKUP               |port_status      |port_status      |
|DEV_SET_WIFI_POWER              |port_status      |port_status      |
|DEV_STA_STATUS                  |                 |sta_status       |
|DEV_GET_DEFAULT_WIFI_INFO       |                 |mode,ssid,pwd    |
|DEV_GET_CUR_WIFI_INFO           |mode             |mode,ssid,pwd    |
|DEV_GET_CUR_STA_SSID_INFO       |                 |sta_ssid_info,sta_ssid_num |
|DEV_SET_LAN_SETTING             |  lan_setting    |       0         |
|DEV_SET_WIFI_POWER_SAVE         |NULL             |NULL             |
|DEV_SET_WIFI_TX_PWR_LMT_ENABLE  |tx_pwr_lmt_enable|NULL             |
|DEV_SET_WIFI_TX_PWR_BY_RATE  	 |tx_pwr_by_rate   |NULL             |
|DEV_GET_WIFI_CHANNEL            |                 |channel          |
|DEV_GET_WIFI_SMP_RESULT         |                 |ssid,pwd         |
|DEV_SET_SMP_CONFIG_TIMEOUT_SEC  |                 |timeout_sec      |
*/




enum {
    DEV_AP_MODE = 0x0,
    DEV_STA_MODE,   //STA模式,用于连接AP
    DEV_SMP_MODE,   //一键配置
    DEV_P2P_MODE,   //wifi direct p2p mode
    DEV_P2P_MODE_CHANGE,	//change to GC or GO mode, 要先同步调DEV_P2P_MODE
    DEV_NET_SCANF,  //STA模式请求扫描ssid
    DEV_SET_AP_CONFIG,
    DEV_GET_MAC,//获取MAC地址
    DEV_SET_MAC,//设置MAC地址
    DEV_SET_DEFAULT_MODE,//设置默认模式
    DEV_SAVE_DEFAULT_MODE,//保存默认配置，到VM区
    DEV_CHANGE_SAVING_MODE,////改变VM区默认配置
    DEV_SET_CB_FUNC,       //设置回调
    DEV_NETWORK_START,
    DEV_NETWORK_STOP,
    DEV_SET_WIFI_CS,      //设置引脚
    DEV_SET_WIFI_WKUP,    //设置引脚
    DEV_SET_WIFI_POWER,   //设置引脚
    DEV_STA_STATUS,        //获取STA状态
    DEV_GET_DEFAULT_WIFI_INFO,  //获取默认的WIFI_INFO(出厂设置的WIFI_INFO)
    DEV_GET_CUR_WIFI_INFO,//获取当前WIFI_INFO
    DEV_GET_STA_SSID_INFO,//获取扫描到ssid,备注：当STA连接成功时候，调用DEV_GET_STA_SSID_INFO时，先调用DEV_NET_SCANF
    DEV_SET_LAN_SETTING,//设置无线IP，掩码，网关，DHCP服务器信息
    DEV_SET_WIFI_POWER_SAVE,   //设置WIFI低功耗模式,有可能降低距离
    DEV_SET_WIFI_TX_PWR_LMT_ENABLE,   //设置WIFI发送功率限制   0:Disable, 1:Enable, 2: Depend on efuse
    DEV_SET_WIFI_TX_PWR_BY_RATE,   //设置WIFI根据不同datarate打不同power     0:Disable, 1:Enable, 2: Depend on efuse
    DEV_GET_WIFI_CHANNEL,   //获取当前WIFI连接的路由器的信道,通常在 WIFI_EVENT_STA_CONNECT_SUCC 事件获取得到
    DEV_WIFI_PM_SUSPEND,   //WIFI进入低功耗模式,停止sdio接收和发送数据,等待AP接入唤醒或者调用PM_RESUME唤醒
    DEV_WIFI_PM_RESUME,   //退出WIFI低功耗模式,恢复到正常模式
    DEV_GET_WIFI_SMP_RESULT, // 获取一键配置ssid和pwd
    DEV_SET_SMP_CONFIG_TIMEOUT_SEC, //设置一键配置超时时间
};
enum {
    STATIC_IP = 0x0,
    DHCP_IP = 0x1,
};

struct cfg_info {
    u32 timeout;
    enum WIFI_MODE mode;
    char *ssid;
    char *pwd;
    char *ap_config_table;
    int (*cb)(void *, enum NETWORK_EVENT, void *);
    void *net_priv;
    char force_default_mode;
    char port_status;
    char tx_pwr_lmt_enable;
    char tx_pwr_by_rate;
    enum wifi_sta_connect_state sta_status;
    struct netdev_ssid_info *sta_ssid_info;
    u32 sta_ssid_num;
    void *__lan_setting_info;
    u32 sta_channel;
    u8 p2p_role;
};


#define WIFI_PLATFORM_DATA_BEGIN(data) \
static const struct wifi_platform_data data = { \

#define WIFI_PLATFORM_DATA_END() \
};

extern const struct device_operations wifi_dev_ops;
#endif  /*WIFI_DEV_H*/
