#ifndef  __WIFI_EXT_H__
#define  __WIFI_EXT_H__
//该头文件用来放从RTL8189库调用函数
//不调用todo/todo.h,todo.h只在rtl8189和hostap库中使用
#include "wifi_module_event.h"   //这两要改文件路径
#include "wpa_supplicant_event.h"

u32 netdev_get_ssid_info(struct netdev_ssid_info *info, u32 network_ssid_cnt);
void wifi_module_set_cb(int (*cb)(void *, enum WIFI_MODULE_EVENT, void *), void *priv);
void wpa_supplicant_set_cb(int (*cb)(struct wpa_supplicant *, enum wpa_supplicant_state, void *), void *priv);
int wifi_module_init(struct WIFI_INIT_INFO *info);
void wifi_module_remove(void);

int wifi_module_enter_ap_mode(char *ssid, char *pwd);
void wifi_module_exit_ap_mode(void);
int wifi_module_enter_sta_mode(char *ssid, char *pwd);
void wifi_module_exit_sta_mode(void);
int wifi_module_enter_smp_mode(void);
void wifi_module_exit_smp_mode(void);
int wifi_module_enter_p2p_mode(unsigned char role);
void wifi_module_exit_p2p_mode(void);
int wifi_module_enter_mp_test_mode(void);
void wifi_module_exit_mp_test_mode(void);

void wpa_supplicant_uninit(void);
void wpa_supplicant_req_scan_ssid(struct wpa_supplicant *wpa_s, int sec);

int wifi_module_is_init(void);
u32 wifi_get_upload_rate(void);

u32 wifi_get_download_rate(void);
u8 get_rx_signal(void);
void netdev_set_power_save(void);
void netdev_set_tx_pwr_lmt_enable(char sel);
void netdev_set_tx_pwr_by_rate(char sel);
u32 netdev_get_channel(void);
int wifi_pm_suspend(void);
int wifi_pm_resume(void);
#endif  /*WIFI_EXT_H*/
