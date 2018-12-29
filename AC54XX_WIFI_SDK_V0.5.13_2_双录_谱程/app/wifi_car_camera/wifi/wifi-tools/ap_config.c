#define USE_RTL8189
//#define USE_RTL8188


/*
1-7信道填写 HT40+, 5-13信道填写 HT40-

ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40-]\n\


ht_capab=[HT40-][SHORT-GI-20][SHORT-GI-40][DSSS_CCK-40]\n\
*/

/*
channel=1#\n\
*/

/*
chanlist=11-13\n\
acs_num_scans=3\n\
acs_chan_bias=1:0 6:0 11:0\n\
*/

/*
supported_rates=10\n\

basic_rates=10 20 55 110\n\



5G:
channel=36\n\
hw_mode=a\n\

*/
#if (defined USE_RTL8189) ||(defined USE_RTL8188)

static char hostapd_config_file[] =
    "interface=wlan0\n\
ssid=####SSID_LENTH_MUST_LESS_THAN_32\n\
channel=6\n\
macaddr_acl=0\n\
auth_algs=3\n\
ignore_broadcast_ssid=0\n\
wpa=#\n\
wpa_passphrase=#########wpa_passphrase_lenth_must_more_than_7_and_less_than_63\n\
wpa_key_mgmt=WPA-PSK\n\
wpa_pairwise=CCMP\n\
rsn_pairwise=CCMP\n\
wpa_group_rekey=86400\n\
wpa_gmk_rekey=86400\n\
wpa_ptk_rekey=86400\n\
driver=rtl871xdrv\n\
preamble=1\n\
hw_mode=g\n"
#ifdef USE_RTL8189
    "ieee80211n=1\n"
#elif (defined USE_RTL8188) //8188 暂时N模式还有问题
    "ieee80211n=0\n"
#endif
    "ht_capab=[SHORT-GI-20]\n\
wme_enabled=1\n\
device_name=RTL818x\n\
manufacturer=Realtek\n\
logger_syslog=-1\n\
logger_syslog_level=2\n\
logger_stdout=-1\n\
logger_stdout_level=2\n\
ctrl_interface=/var/run/hostapd\n\
ctrl_interface_group=0\n\
beacon_int=100\n\
dtim_period=1\n\
max_num_sta=4\n\
rts_threshold=2347\n\
fragm_threshold=2346\n\
wmm_enabled=1\n\
wmm_ac_bk_cwmin=4\n\
wmm_ac_bk_cwmax=10\n\
wmm_ac_bk_aifs=7\n\
wmm_ac_bk_txop_limit=0\n\
wmm_ac_bk_acm=0\n\
wmm_ac_be_aifs=3\n\
wmm_ac_be_cwmin=4\n\
wmm_ac_be_cwmax=10\n\
wmm_ac_be_txop_limit=0\n\
wmm_ac_be_acm=0\n\
wmm_ac_vi_aifs=2\n\
wmm_ac_vi_cwmin=3\n\
wmm_ac_vi_cwmax=4\n\
wmm_ac_vi_txop_limit=94\n\
wmm_ac_vi_acm=0\n\
wmm_ac_vo_aifs=2\n\
wmm_ac_vo_cwmin=2\n\
wmm_ac_vo_cwmax=3\n\
wmm_ac_vo_txop_limit=47\n\
wmm_ac_vo_acm=0\n\
eapol_key_index_workaround=0\n\
eap_server=0\n\
own_ip_addr=192.168.1.1\n";

#elif defined USE_S9082

static char hostapd_config_file[] =
    "interface=wlan0\n\
ssid=####SSID_LENTH_MUST_LESS_THAN_32\n\
channel=13#\n\
macaddr_acl=0\n\
auth_algs=3\n\
ignore_broadcast_ssid=0\n\
wpa=#\n\
wpa_passphrase=#########wpa_passphrase_lenth_must_more_than_7_and_less_than_63\n\
wpa_key_mgmt=WPA-PSK\n\
wpa_pairwise=CCMP\n\
rsn_pairwise=CCMP\n\
wpa_group_rekey=86400\n\
wpa_gmk_rekey=86400\n\
wpa_ptk_rekey=86400\n\
driver=tilk908x_drv\n\
hw_mode=g\n\
ieee80211n=1\n\
device_name=SCI9082\n\
manufacturer=SCI\n\
logger_syslog=-1\n\
logger_syslog_level=2\n\
logger_stdout=-1\n\
logger_stdout_level=2\n\
ctrl_interface=/var/run/hostapd\n\
ctrl_interface_group=0\n\
beacon_int=100\n\
dtim_period=1\n\
max_num_sta=4\n\
rts_threshold=2347\n\
fragm_threshold=2346\n\
wmm_enabled=1\n\
wmm_ac_bk_cwmin=4\n\
wmm_ac_bk_cwmax=10\n\
wmm_ac_bk_aifs=7\n\
wmm_ac_bk_txop_limit=0\n\
wmm_ac_bk_acm=0\n\
wmm_ac_be_aifs=3\n\
wmm_ac_be_cwmin=4\n\
wmm_ac_be_cwmax=10\n\
wmm_ac_be_txop_limit=0\n\
wmm_ac_be_acm=0\n\
wmm_ac_vi_aifs=2\n\
wmm_ac_vi_cwmin=3\n\
wmm_ac_vi_cwmax=4\n\
wmm_ac_vi_txop_limit=94\n\
wmm_ac_vi_acm=0\n\
wmm_ac_vo_aifs=2\n\
wmm_ac_vo_cwmin=2\n\
wmm_ac_vo_cwmax=3\n\
wmm_ac_vo_txop_limit=47\n\
wmm_ac_vo_acm=0\n\
eapol_key_index_workaround=0\n\
eap_server=0\n\
own_ip_addr=192.168.1.1\n";
#endif


/*wps channel only support 1 5 9 */

static char wps_hostapd_config_file[] = \
                                        "interface=wlan0\n\
ctrl_interface=/var/run/hostapd\n\
ctrl_interface_group=0\n\
macaddr_acl=0\n\
rsn_pairwise=CCMP\n\
wpa_gmk_rekey=86400\n\
wpa_ptk_rekey=86400\n\
preamble=1\n\
ht_capab=[SHORT-GI-20]\n\
logger_syslog=-1\n\
logger_syslog_level=2\n\
logger_stdout=-1\n\
logger_stdout_level=2\n\
dtim_period=1\n\
rts_threshold=2347\n\
fragm_threshold=2346\n\
wmm_enabled=1\n\
wmm_ac_bk_cwmin=4\n\
wmm_ac_bk_cwmax=10\n\
wmm_ac_bk_aifs=7\n\
wmm_ac_bk_txop_limit=0\n\
wmm_ac_bk_acm=0\n\
wmm_ac_be_aifs=3\n\
wmm_ac_be_cwmin=4\n\
wmm_ac_be_cwmax=10\n\
wmm_ac_be_txop_limit=0\n\
wmm_ac_be_acm=0\n\
wmm_ac_vi_aifs=2\n\
wmm_ac_vi_cwmin=3\n\
wmm_ac_vi_cwmax=4\n\
wmm_ac_vi_txop_limit=94\n\
wmm_ac_vi_acm=0\n\
wmm_ac_vo_aifs=2\n\
wmm_ac_vo_cwmin=2\n\
wmm_ac_vo_cwmax=3\n\
wmm_ac_vo_txop_limit=47\n\
wmm_ac_vo_acm=0\n\
eapol_key_index_workaround=0\n\
own_ip_addr=192.168.1.1\n\
ssid=wifi_camera_ac54_wlan_direct\n\
channel=06\n\
wpa=2\n\
wpa_passphrase=12345678\n\
auth_algs=3\n\
ignore_broadcast_ssid=0\n\
eap_server=1\n\
wps_state=2\n\
uuid=12345678-9abc-def0-1234-56789abcdef0\n\
device_name=wifi_camera_ac54_wlan_direct\n\
manufacturer=Realtek\n\
model_name=RTW_SOFTAP\n\
model_number=WLAN_ES\n\
serial_number=12345\n\
device_type=6-0050F204-1\n\
os_version=01020300\n\
config_methods=label display push_button keypad\n\
driver=rtl871xdrv\n\
beacon_int=100\n\
hw_mode=g\n\
ieee80211n=1\n\
wme_enabled=1\n\
wpa_key_mgmt=WPA-PSK\n\
wpa_pairwise=CCMP\n\
max_num_sta=4\n\
wpa_group_rekey=86400\n";




const char *get_hostapd_config_file(unsigned char enable_wps)
{
#if (defined USE_RTL8189) ||(defined USE_RTL8188) || (defined USE_S9082)
    if (enable_wps) {
        return wps_hostapd_config_file;
    } else {
        return hostapd_config_file;
    }
#endif
    return (char *)0;
}


