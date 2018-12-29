#include <string.h>
int uaputl_main(int argc, char *argv[]);

/*

ÆÁ±Î11N·½·¨:
Enable11n=0
#    HTCapInfo=0x111c
#    AMPDU=0x03
#     HT_MCS_MAP
*/
/*
   	 1M    2M    5.5M  11M   6M    9M    12M   18M   24M   36M   48M   54M
     0x82, 0x84, 0x8b, 0x96, 0x8C, 0x92, 0x98, 0xA4, 0xB0, 0xC8, 0xE0, 0xEc
 */


static const char uaputl_conf_file[] =	  "\
ap_config={\n\
    SSID=\"Marvell_88W8XXX_UAP\"    # SSID of Micro AP, use '\' as an escape character before '\"' character in SSID\n\
    BeaconPeriod=100                # Beacon period in TU\n\
    Channel=1                       # Radio Channel	\n\
#   Channel=0,1                     # auto channel select (MCS mode)\n\
#   Channel=6,2                     # primary channel 6, secondary channel above.\n\
#   Channel=6,4                     # primary channel 6, secondary channel below\n\
    ChanList=1,6,11                 # Scan channel list\n\
#   AP_MAC=00:34:22:77:46:41        # MAC address of AP\n\
    Band=0                          # 0 for 2.4GHz band\n\
                                    # 1 for 5GHz band\n\
    RxAntenna=0                     # 0 for Antenna A\n\
                                    # 1 for Antenna B\n\
    TxAntenna=0                     # 0 for Antenna A\n\
                                    # 1 for Antenna B\n\
    Rate=0x82,0x84,0x8b,0x96,0x0C,0x12,0x18,0x24,0x30,0x48,0x60,0x6c\n\
                                    # Set of data rate that a station\n\
                                    # in the BSS may use\n\
                                    # (in unit of 500 kilobit/s)\n\
    TxPowerLevel=24                 # Transmit power level in dBm, MAX IS 29\n\
    BroadcastSSID=1                 # Broadcast SSID feature\n\
                                    # 1: Enable  0:Disable\n\
    RTSThreshold=2347               # RTS threshold value\n\
    FragThreshold=2346              # Fragmentation threshold value\n\
    DTIMPeriod=1                    # DTIM period in beacon periods\n\
    MCBCdataRate=0                  # MCBC rate to use for packet transmission\n\
                                    # 0:auto\n\
                                    # >0 fix rate (in unit of 500 kilobit/s)\n\
    PktFwdCtl=1                     # Packet forwarding control\n\
                                    # 0: forward all packets to the host\n\
                                    # 1: firmware handles intr-BSS packets\n\
    StaAgeoutTimer=1800             # Inactive client station age out timer value\n\
                                    # in units of 100ms\n\
    PSStaAgeoutTimer=400            # Inactive client PS station age out timer value\n\
                                    # in units of 100ms\n\
    MaxStaNum=4                     # Max number of stations allowed to connect\n\
    Retrylimit=5                    # Retry limit to use for packet transmissions\n\
    AuthMode=0                      # 0:Open authentication\n\
                                    # 1:shared key authentication\n\
    Protocol=1                      # protocol to use\n\
                                    # 1: No security  	2: Static WEP\n\
                                    # 8: WPA	32: WPA2    40:WPA2 Mixed Mode\n\
    RSNReplayProtection=0           # RSN replay protection 0: disabled, 1: enabled\n\
    PairwiseUpdateTimeout=100       #Pairwise Handshake update timeout: 100ms\n\
    PairwiseHandshakeRetries=3      #Pairwise Handshake retries: 3\n\
    GroupwiseUpdateTimeout=100      #Groupwise Handshake update timeout: 100ms\n\
    GroupwiseHandshakeRetries=3           #Groupwise Handshake retries: 3\n\
\n\
# **** WEP security setting******\n\
#   KeyIndex=0                      # Index of WEP key to use (0 to 3)\n\
#   Sample Key configurations are\n\
#       Key_0=\"55555\"\n\
#       Key_1=1234567890\n\
#       Key_2=\"1234567890123\"\n\
#       Key_3=12345678901234567890123456\n\
\n\
# **** Pairwise Cipher setting******\n\
#    Configure both of the following for Protocol=40\n\
#    PwkCipherWPA=4                 # Pairwise cipher type\n\
                                    # 4: TKIP     8: AES CCMP\n\
                                    # 12: AES CCMP + TKIP\n\
#    PwkCipherWPA2=8                # Pairwise cipher type\n\
                                    # 4: TKIP     8: AES CCMP\n\
                                    # 12: AES CCMP + TKIP\n\
\n\
# **** Group Cipher setting******\n\
\n\
#    GwkCipher=4                    # group cipher type\n\
                                    # 4: TKIP     8: AES CCMP\n\
\n\
#    PSK=\"1234567890\"               # WPA/WPA2 passphrase\n\
     GroupRekeyTime= 86400          # Group key re-key interval, in second.\n\
                                    # 0 mean never re-key\n\
\n\
    Enable11n=1                     # 1 to enable, 0 to disable\n\
    HTCapInfo=0x111c                # HTCapInfo\n\
                                    # Bit 15-13: Reserved set to 0\n\
                                    # Bit 12: DSS/CCK mode in 40MHz enable/disable\n\
                                    # Bit 11-10: Reserved set to 0\n\
                                    # Bit 9-8: Reserved set to 0x01\n\
                                    # Bit 7: Reserved set to 0\n\
                                    # Bit 6: Short GI in 40 Mhz enable/disable\n\
                                    # Bit 5: Short GI in 20 Mhz enable/disable\n\
                                    # Bit 4: Green field enable/disble\n\
                                    # Bit 3-2: Reserved set to 1\n\
                                    # Bit 1: 20/40 Mhz enable disable.\n\
                                    # Bit 0: Reserved set to 0\n\
    AMPDU=0x03                      # AMPDU\n\
                                    # Bit 7-5: Reserved set to 0\n\
                                    # Bit 4-2: Minimum MPDU Start spacing\n\
                                    #          Set to 0 for no restriction\n\
                                    #          Set to 1 for 1/4 us\n\
                                    #          Set to 2 for 1/2 us\n\
                                    #          Set to 3 for 1 us\n\
                                    #          Set to 4 for 2 us\n\
                                    #          Set to 5 for 4 us\n\
                                    #          Set to 6 for 8 us\n\
                                    #          Set to 7 for 16 us\n\
                                    # Bit 1-0: Max A-MPDU length\n\
     HT_MCS_MAP=0x000000ff          # Bit 7-0: MCS_SET_0\n\
                                    # Bit 15-8:MCS_SET_1\n\
#    Enable2040Coex=1                #Enable 20/40 coex feature\n\
\n\
                                    #802.11D specific configuration\n\
    11d_enable=0                    # 0-disable 1-enable\n\
   country=CN                      # country information\n\
\n\
}\n\
\n\
\n\
\n\
ap_mac_filter={\n\
    FilterMode=0                    # Mode of filter table\n\
                                    # 0: filter table is disabled\n\
                                    # 1: allow MAC address in the filter table to associate\n\
                                    # 2: block MAC address in the filter table\n\
    Count=0                         # Number of entries in filter table,up to 16\n\
#Sample mac settings are\n\
#    mac_1=00:50:23:45:76:22         # mac address\n\
#    mac_2=00:34:22:77:46:34         # mac address\n\
}\n\
\n\
# Wmm param setting\n\
Wmm_parameters={\n\
    Qos_info=0x80\n\
    AC_BE\n\
    Aifsn=1\n\
    Ecw_max=1\n\
    Ecw_min=1\n\
    Tx_op=1\n\
    AC_BK\n\
    Aifsn=2\n\
    Ecw_max=2\n\
    Ecw_min=2\n\
    Tx_op=2\n\
    AC_VI\n\
    Aifsn=3\n\
    Ecw_max=3\n\
    Ecw_min=3\n\
    Tx_op=3\n\
    AC_VO\n\
    Aifsn=4\n\
    Ecw_max=4\n\
    Ecw_min=4\n\
    Tx_op=4\n\
}\n\n\n\n\n\n\n\n\n";

const char *GET_UAPUTL_CONF_FILE(void)
{
    return uaputl_conf_file;
}


void marvell_uap_uninit(void)
{
    char *argv[4] = {"uaputl", 0};

    argv[1] = "bss_stop";
    uaputl_main(2, argv);
}

int marvell_uap_init(const char *ssid, const char *passphrase)
{
    int ret;
    int argc;

    char *argv[8] = {"uaputl", 0};

    //if started, stop it first
    argc = 0;
    argv[++argc] = "bss_stop";
    uaputl_main(++argc, argv);

    /*^--------------------^*/
    argc = 0;
    argv[++argc] = "sys_config";
    argv[++argc] = "uaputl.conf";
    ret = uaputl_main(++argc, argv);
    if (ret) {
        return ret;
    }
    /*^--------------------^*/

    /*^--------------------^*/
    argc = 0;
    argv[++argc] = "sys_cfg_ssid";
    argv[++argc] = (char *)ssid;
    ret = uaputl_main(++argc, argv);
    if (ret) {
        return ret;
    }

    /*^--------------------^*/

    /*^--------------------^*/
//    argc = 0;
//    argv[++argc] = "sys_cfg_tx_power";
//    argv[++argc] = "29";
//    ret = uaputl_main(++argc,argv);
//    if(ret)
//        return ret;
//    /*^--------------------^*/
//
//    /*^--------------------^*/
//    argc = 0;
//    argv[++argc] = "sys_cfg_channel";
//    argv[++argc] = "13";
//    ret = uaputl_main(++argc,argv);
//    if(ret)
//        return ret;
    /*^--------------------^*/

    if (strcmp(passphrase, "")) {
        /*^--------------------^*/
        argc = 0;
        argv[++argc] = "sys_cfg_protocol";
        argv[++argc] = "32";
        ret = uaputl_main(++argc, argv);
        if (ret) {
            return ret;
        }
        /*^--------------------^*/

        /*^--------------------^*/
        argc = 0;
        argv[++argc] = "sys_cfg_wpa_passphrase";
        argv[++argc] = (char *)passphrase;
        ret = uaputl_main(++argc, argv);
        if (ret) {
            return ret;
        }
        /*^--------------------^*/

        /*^--------------------^*/
        argc = 0;
        argv[++argc] = "sys_cfg_cipher";
        argv[++argc] = "8";
        argv[++argc] = "8";
        ret = uaputl_main(++argc, argv);
        if (ret) {
            return ret;
        }
        /*^--------------------^*/
    }

    /*^--------------------^*/
    argc = 0;
    argv[++argc] = "bss_start";
    ret = uaputl_main(++argc, argv);
    if (ret) {
        return ret;
    }
    /*^--------------------^*/

    return 0;
}

