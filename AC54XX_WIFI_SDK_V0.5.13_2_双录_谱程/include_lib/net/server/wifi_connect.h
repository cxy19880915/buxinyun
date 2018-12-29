#ifndef _WIFI_CONNECT_H_
#define _WIFI_CONNECT_H_

#include "generic/typedef.h"

#include "os/os_api.h"

#include "HSM/hsm.h"
#include "wifi_module_event.h"

enum NETWORK_EVENT {
    WIFI_EVENT_MODULE_INIT,
    WIFI_EVENT_MODULE_START,
    WIFI_EVENT_MODULE_STOP,
    WIFI_EVENT_MODULE_START_ERR,

    WIFI_EVENT_AP_START,
    WIFI_EVENT_AP_STOP,
    WIFI_EVENT_STA_START,
    WIFI_EVENT_STA_STOP,
    WIFI_EVENT_STA_SCAN_COMPLETED,

    WIFI_EVENT_STA_CONNECT_SUCC,
    WIFI_EVENT_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID,
    WIFI_EVENT_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL,
    WIFI_EVENT_STA_DISCONNECT,

    WIFI_EVENT_SMP_CFG_TIMEOUT,
    WIFI_EVENT_SMP_CFG_COMPLETED,

    WIFI_EVENT_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_EVENT_STA_NETWORK_STACK_DHCP_TIMEOUT,

    WIFI_EVENT_AP_ON_DISCONNECTED,
    WIFI_EVENT_AP_ON_ASSOC,

    WIFI_EVENT_MP_TEST_START,
    WIFI_EVENT_MP_TEST_STOP,

    WIFI_EVENT_P2P_START,
    WIFI_EVENT_P2P_STOP,
    WIFI_EVENT_P2P_GC_DISCONNECTED,
    WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_SUCC,
    WIFI_EVENT_P2P_GC_NETWORK_STACK_DHCP_TIMEOUT,

    WIFI_EVENT_PM_SUSPEND,
    WIFI_EVENT_PM_RESUME,
};

struct wifi_network_api_msg {
    int err;
    OS_SEM sem;
    u32 msg;
    union {
        struct WIFI_INIT_INFO vm_info;
    } ctx;
};

struct WIFI_CONNECT_T {
    HSM parent;
    // Child members
    void *parm;
};

enum WIFI_HSM_EVENTS {
    HSM_WIFI_OPEN = (HSME_START),
    HSM_WIFI_CLOSE,
    HSM_WIFI_SMP_CFG_SUCC,
    HSM_WIFI_SMP_CFG_TIMEOUT,
    HSM_WIFI_START_SMP_CFG,
    HSM_WIFI_STA_CONNECT_SUCC,
    HSM_WIFI_STA_CONNECT_FAIL_NOT_FOUND_SSID,
    HSM_WIFI_STA_CONNECT_FAIL_ASSOCIAT_FAIL,
    HSM_WIFI_STA_DISCONNECTED,
    HSM_WIFI_STA_DHCP_BOUND_SUCC,
    HSM_WIFI_STA_DHCP_BOUND_TIMEOUT,

    HSM_WIFI_STA_REQ_SCAN,
    HSM_WIFI_STA_REQ_SCAN_COMPLETED,
    HSM_WIFI_STA_REQ_CONNECT,

    HSM_WIFI_ENTER_AP,

    //WIFI_NET_API_MSG
    MSG_SAVE_WIFI_VM_INFO,

    HSM_WIFI_ENTER_MP_TEST,

    HSM_WIFI_ENTER_P2P,
    HSM_WIFI_P2P_GC_DISCONNECTED,
    HSM_WIFI_P2P_GC_DHCP_BOUND_SUCC,
    HSM_WIFI_P2P_GC_DHCP_BOUND_TIMEOUT,
};

int network_init(enum WIFI_MODULE module);
int network_stop(void);
int network_start(void);

struct WIFI_INIT_INFO *get_default_wifi_vm_info(void);
struct WIFI_INIT_INFO *get_cur_wifi_info(void);
enum WIFI_MODE get_cur_wifi_mode(void);

int wifi_network_enter_hostapd(char *ap_ssid, char *ap_pwd);
int wifi_network_enter_mp_test(void);
int wifi_network_enter_p2p(char role);

void wpa_supplicant_set_connect_timeout(int sec);
void wpa_supplicant_set_connect_block(int block);
int wifi_network_sta_connect(char *sta_ssid, char *sta_pwd);

struct netdev_ssid_info *wifi_network_get_scan_ssid(u32 *ssid_cnt);
int wifi_network_req_scan(void);

int get_cur_connect_sta(char sta_ssid[33], char sta_pwd[64]);
int wifi_network_save_wifi_vm_info(enum WIFI_MODE mode, char *ssid, char *pwd);
int wifi_network_set_default_mode(enum WIFI_MODE mode, char *ssid, char *pwd, char force);
int wifi_network_save_default_mode(enum WIFI_MODE mode, char *ssid, char *pwd, enum P2P_ROLE p2p_role, char force);

void network_set_cb(int (*cb)(void *, enum NETWORK_EVENT, void *), void *priv);

int wifi_network_start_smp_cfg(void);
void set_smp_cfg_timeout(int sec);
int get_monitor_result(u8 **p_ssid, u8 **p_passphrase);

enum wifi_module_state {
    WIFI_MODULE_STOP,
    WIFI_MODULE_START,
    WIFI_MODULE_ERROR,
};
enum wifi_module_state get_wifi_module_state(void);

enum wifi_sta_connect_state {
    WIFI_STA_DISCONNECT,
    WIFI_STA_CONNECT_SUCC,
    WIFI_STA_CONNECT_TIMEOUT_NOT_FOUND_SSID,
    WIFI_STA_CONNECT_TIMEOUT_ASSOCIAT_FAIL,
    WIFI_STA_NETWORK_STACK_DHCP_SUCC,
    WIFI_STA_NETWORK_STACK_DHCP_TIMEOUT,
};
enum wifi_sta_connect_state get_wifi_sta_connect_state(void);


#endif  //_WIFI_CONNECT_H_

