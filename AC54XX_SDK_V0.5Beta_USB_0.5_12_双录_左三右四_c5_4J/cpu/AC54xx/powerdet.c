#include "system/includes.h"
#include "asm/adc.h"
#include "asm/rtc.h"
#include "asm/powerdet.h"
#include "generic/gpio.h"
#include "device/device.h"
#include "device/key_driver.h"
#include "timer.h"
#include "app_config.h"

static u8 cur_power_level;
static struct powerdet_platform_data *_this = NULL;

REGISTER_ADC_SCAN(ldoin_scan)
.channel = AD_CH14_V50,
 .value = 0,
};

static int ldoin_init(struct key_driver *key, void *arg)
{
    void *fd = dev_open("rtc", NULL);
    if (fd) {
        dev_ioctl(fd, IOCTL_SET_VDD50_EN, 0);
        dev_close(fd);
    }

    return 0;
}

static u16 ldoin_get_value(struct key_driver *key)
{
    return ldoin_scan.value;
}

static const struct key_driver_ops ldoin_driver_ops = {
    .init 		= ldoin_init,
    .get_value 	= ldoin_get_value,
};

REGISTER_KEY_DRIVER(ldoin_driver) = {
    .name = "ad_power",
    .ops  = &ldoin_driver_ops,
};

REGISTER_ADC_SCAN(ldo_vbg_scan)
.channel = AD_CH15_LDO_VBG,
 .value = 0,
};

static int ldo_vbg_init(struct key_driver *key, void *arg)
{
    LDO_CON |= BIT(11);//VBG_EN
    LDO_CON |= BIT(20);//ADCOE

    return 0;
}

static u16 ldo_vbg_get_value(struct key_driver *key)
{
    return ldo_vbg_scan.value;
}

static const struct key_driver_ops ldo_vbg_driver_ops = {
    .init 		= ldo_vbg_init,
    .get_value 	= ldo_vbg_get_value,
};

REGISTER_KEY_DRIVER(ldo_vbg_driver) = {
    .name = "ad_ldo",
    .ops  = &ldo_vbg_driver_ops,
};

u16 get_power_value()
{
    u16 in_vol;
    u16 refer_vol;
    u16 vol;

    refer_vol = ldo_vbg_get_value(NULL);
    in_vol = ldoin_get_value(NULL);

#if ENABLE_SAMPLE_VAL
    vol = in_vol * 0x181 / refer_vol;
#else
    vol = (in_vol * 3 * LDO_REFERENCE_VOL + 0x181 * 2) / refer_vol;
#endif

    /* printf("%x : %x : %x\n", in_vol, refer_vol, vol); */
    /* printf("%d ", vol); */
    /* printf("0x%x\n", vol); */

    return vol;
}

u8 get_power_level(u16 vol)
{
    int level;

    //获取电压值对应的消息
    for (level = _this->nlevel - 1; level >= 0 ; level--) {
        if (vol >= _this->power_level[level]) {
            return level + 1;
        }
    }

    return 0;
}

#define N 12
u16 value_buf[N + 1];
static u16 ad_filter()
{
    unsigned char i, j;
    u32 sum = 0;
    static u8 cnt = 0;
    static u16 value = 0;
    u16 tempval;

    if (!value) {
        value = get_power_value();
    }
    value_buf[cnt++] = get_power_value();

    if (cnt > N) {
        cnt = 0;
    } else {
        return value;
    }

    for (j = 0; j < N - 1; j++) {
        for (i = 0; i < N - j; i++) {
            if (value_buf[i] > value_buf[i + 1]) {
                tempval = value_buf[i];
                value_buf[i] = value_buf[i + 1];
                value_buf[i + 1] = tempval;
            }
        }
    }

    for (i = 1; i < N - 1; i++) {
        sum += value_buf[i];
    }

    value = sum / (N - 2);

    return (value);
}

static void send_power_msg()
{
    struct sys_event e;

    e.arg = "powerdet";
    e.type = SYS_DEVICE_EVENT;
    e.u.dev.event = DEVICE_EVENT_CHANGE;
    e.u.dev.value = cur_power_level;
    sys_event_notify(&e);
}
static void powerdet_scan(void *_key)
{
    static u8 power_scan_cnt = 0;
    static u8 last_power_level = 5;
    static u8 power_level;
    u16 power_vol;
    static u16 low_cnt;

    power_vol = ad_filter();
    /* printf("0x%x\n", power_vol); */
    power_level = get_power_level(power_vol);
    /* printf("power_level=%d\n",power_level); */
    if (!usb_is_charging()) {
        if (power_level > last_power_level) { //下一次得到的电压值需比上一次的小
            power_level = last_power_level;
        }
    }

    //去抖
    if (power_level != last_power_level) {//level change
        last_power_level = power_level;
        power_scan_cnt = 0;
    } else {
        if (power_scan_cnt == PWR_SCAN_TIMES) {//去抖
            if (cur_power_level != last_power_level) {
                cur_power_level = last_power_level;
                low_cnt = 0;
                send_power_msg();
                /* printf("upower_level = %x\n", cur_power_level); */
                return;
            } else {
                if (last_power_level == 0) {
                    if (low_cnt++ > 10) {
                        cur_power_level = last_power_level;
                        low_cnt = 0;
                        send_power_msg();
                        printf("cpower_level = %x\n", cur_power_level);
                    }
                } else {
                    low_cnt = 0;
                }
            }
        } else if (power_scan_cnt > PWR_SCAN_TIMES) {
            power_scan_cnt = PWR_SCAN_TIMES - 1;
        } else {
            power_scan_cnt++;
        }
    }
}

static int powerdet_driver_init(const struct dev_node *node,  void *arg)
{
    _this = (struct powerdet_platform_data *)arg;

    cur_power_level = _this->nlevel;
    ldo_vbg_init(NULL, NULL);
    ldoin_init(NULL, NULL);
    sys_hi_timer_add(NULL, powerdet_scan, 100);

    return 0;
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~beautiful line~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static struct device dev_powerdet;

static int powerdet_open(const struct dev_node *node, struct device **device, void *arg)
{
    *device = &dev_powerdet;
    return 0;
}

static int powerdet_ioctl(struct device *device, u32 cmd, u32 arg)
{
    int err = 0;
    u32 *level = (u32 *)arg;
    switch (cmd) {
    case POWER_DET_GET_LEVEL:
        *level = cur_power_level;
        break;
    }
    return err;
}


const struct device_operations powerdet_dev_ops = {
    .init = powerdet_driver_init,
    .open = powerdet_open,
    .ioctl = powerdet_ioctl,
};
