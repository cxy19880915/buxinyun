#include "asm/adc.h"
#include "asm/rtc.h"
#include "generic/gpio.h"
#include "device/device.h"
#include "device/key_driver.h"
#include "timer.h"
#include "app_config.h"
#include "power_check_onetime.h"

#define LDO_REFERENCE_VOLL	124 //1.24基准电压

#define PWR_VALUE_3_50		0x156
#define PWR_VALUE_3_55		0x15b
#define PWR_VALUE_3_60		0x15f
#define PWR_VALUE_3_65		0x163
#define PWR_VALUE_3_70		0x168
#define PWR_VALUE_3_75		0x16e
#define PWR_VALUE_3_80		0x173
#define PWR_VALUE_3_85		0x177
#define PWR_VALUE_3_90		0x17c
#define PWR_VALUE_3_95		0x182


static u32 _power_level[] = {
    PWR_VALUE_3_50,
    PWR_VALUE_3_55,
    PWR_VALUE_3_60,
};

static u16 ad_value[] = {0, 0};
static u8 channel[] = {AD_CH15_LDO_VBG, AD_CH14_V50};
static void adc_scan_process(void *p)
{
    static u8 flag = 0;

    if (!flag) {
        flag = 1;
        ad_value[0] = GPADC_RES;
        ADCSEL(channel[1]);
    } else {
        flag = 0;
        ad_value[1] = GPADC_RES;
        ADCSEL(channel[0]);
    }

    KITSTART();
    //pmsg("========================================================\n");
    while(!ADC_PND());
}


static int adc_scan_init()
{
    ADCSEL(channel[0]);
    ADC_EN(1);
    ADCEN(1);
    ADC_BAUD(0x7);
    ADC_WTIME(0x1);
    KITSTART();
    return 0;
}

static int ldo_vbg_init(struct key_driver *key, void *arg)
{
    LDO_CON |= BIT(11);//VBG_EN
    LDO_CON |= BIT(20);//ADCOE
    return 0;
}
extern int rtc_vdd50_enable();
static int ldoin_init(struct key_driver *key, void *arg)
{
    rtc_vdd50_enable();
    return 0;
}

static u8 get_power_level(u16 vol)
{
    int level;

    //获取电压值对应的消息
    for (level = sizeof(_power_level) / sizeof(_power_level[0]) - 1; level >= 0 ; level--) {
        if (vol >= _power_level[level]) {
            return level + 1;
        }
    }
    return 0;
}

static u16 ldo_vbg_get_value(struct key_driver *key)
{
    return ad_value[0];
}

static u16 ldoin_get_value(struct key_driver *key)
{
    return ad_value[1];
}


static u16 get_power_value()
{
    u16 in_vol;
    u16 refer_vol;
    u16 vol;

    refer_vol = ldo_vbg_get_value(NULL);
    in_vol = ldoin_get_value(NULL);
#if ENABLE_SAMPLE_VAL
    vol = in_vol * 0x181 / refer_vol;
#else
    vol = (in_vol * 3 * LDO_REFERENCE_VOLL + 0x181 * 2) / refer_vol;
#endif
//    vol = in_vol * 0x181 / refer_vol;
    /* printf("%x : %x : %x\n", in_vol, refer_vol, vol); */
//    printf("%d ", vol);
//    printf("0x%x\n", vol);

    return vol;
}



#define N 3
u16 value_buf1[N + 1];
static u16 ad_filter()
{
    u32 ad_value;

    ad_value = get_power_value();

    return ad_value;
}
static u16 power_vol;
static unsigned char powerdet_scan()
{
    static u8 power_level;

__HERE:
    adc_scan_process(NULL);//get ad value

    power_vol = ad_filter();
    //delay(2000);
    power_level = get_power_level(power_vol);
    put_u8hex(power_level);
    static u32 cnt = 0;
    static u32 cnt2 = 0;
    if (power_level <= 1) {
        if (cnt2) {
            cnt2--;
        }
        if (cnt++ > 5) {
            cnt = 0;
            return true;
        } else {
            goto __HERE;
        }
    } else {
        /*      if (cnt) { */
        /* cnt--; */
        /* } */
        if (cnt2++ > 20) {
            cnt2 = 0;
            return false;
        } else {
            goto __HERE;
        }
    }

    return false;
}

static int charging_fliter(u8 flag)
{
    int i = 0;

    while (i++ < 10) {
        if (flag) {
            return 1;
        }
        delay(100);
    }
    return 0;
}

/*开机电量检测 */
static int powerdet_driver_init()
{

    if (!charging_fliter(usb_is_charging()))
    {
        //避免坏电池或者接近输出截止状态的电池拉电,导致插电无法开机

        puts("battery check first\n");
        adc_scan_init();
        ldo_vbg_init(NULL, NULL);
        ldoin_init(NULL, NULL);
        while (!charging_fliter(usb_is_charging()))
        {
            if (powerdet_scan())
            {
                power_off_wkup_set(get_usb_wkup_gpio(), 1);

                while (1)
                {
                    if (charging_fliter(usb_is_charging())) {
                        puts("chip reset\n");
                        cpu_reset();
                    } else {
                        //delay(1000);
                        putchar('o');	//电池低电，一直按住开机键,不重复检测开机电压
                    }
                }
            }
            else
            {
                puts("power enough\n");
                break;
            }
        }
    }
    return 0;
}
/*系统上电原因打印*/
static void system_reset_reson_check_once()
{
    volatile u32 reset_flag;
    reset_flag = (PWR_CON & 0xe0) >> 5;

    switch (reset_flag) {
    case 0:
        puts("\n =====power on reset=====\n");
        break;
    case 1:
        puts("\n =====VCM reset======\n");
        break;
    case 2:
        puts("\n =====PR2 “4”s “0”voltage reset=====\n");
        break;
    case 3:
        puts("\n =====LVD lower power reset=====\n");
        break;
    case 4:
        puts("\n =====WDT reset=====\n");
        break;
    case 5:
        puts("\n =====sofware reset=====\n");
        break;
    default:
        puts("\n =====other reason======\n");
        break;
    }
}

/*该函数只在开机时调用一次*/
void power_check_once()
{
    system_reset_reson_check_once();
    powerdet_driver_init();
}


