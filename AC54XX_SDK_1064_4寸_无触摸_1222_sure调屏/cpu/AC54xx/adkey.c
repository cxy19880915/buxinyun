#include "asm/adkey.h"
#include "asm/adc.h"
#include "generic/gpio.h"
#include "device/key_driver.h"
#include "device/device.h"


static struct key_driver adkey_driver;
static struct adkey_platform_data *__this = NULL;



REGISTER_ADC_SCAN(adkey_scan)
.value = 0,
};


static int adkey_init(struct key_driver *key, void *arg)
{
    __this = (struct adkey_platform_data *)arg;
    if (!__this) {
        return -EINVAL;
    }

    adkey_scan.channel = __this->ad_channel;

    gpio_direction_input(__this->io);
    gpio_set_pull_up(__this->io, 0);
    gpio_set_pull_down(__this->io, 0);
    gpio_set_die(__this->io, 1);

    adkey_driver.prev_value = 0;
    adkey_driver.last_key = NO_KEY;
    adkey_driver.scan_time = 10;
    adkey_driver.base_cnt = __this->base_cnt;
    adkey_driver.long_cnt = __this->long_cnt;
    adkey_driver.hold_cnt = __this->hold_cnt;

    return 0;
}

static u16 adkey_get_value(struct key_driver *key)
{
    int i;
    struct adkey_value_table *table = &__this->table;

   //// /* printf("adkey: %x\n", adkey_scan.value); */
    if(adkey_scan.value > 0x300)
        return NO_KEY;

    for (i = 0; i < ADKEY_MAX_NUM; i++) {
        if (adkey_scan.value >= table->ad_value[i]) {
            return table->key_value[i];
        }
        if (table->ad_value[i] == 0) {
            break;
        }
    }

    return NO_KEY;
}

static const struct key_driver_ops adkey_driver_ops = {
    .init 		= adkey_init,
    .get_value 	= adkey_get_value,
};

REGISTER_KEY_DRIVER(adkey_driver) = {
    .name = "adkey",
    .ops = &adkey_driver_ops,
};

