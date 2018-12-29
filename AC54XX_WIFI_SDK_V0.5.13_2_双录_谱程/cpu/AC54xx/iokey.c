#include "key_driver.h"
#include "iokey.h"
#include "gpio.h"
#include "system/event.h"



static const struct iokey_platform_data *__this = NULL;


static u16 io_get_key_value(struct key_driver *_key)
{
    int i;

    for (i = 0; i < __this->num; i++) {
        if (gpio_read(__this->port[i].port) == __this->port[i].press_value) {
            return __this->port[i].key_value;
        }
    }

    return NO_KEY;
}

static int iokey_init(struct key_driver *_key, void *arg)
{
    int i;

    __this = (struct iokey_platform_data *)arg;
    if (__this == NULL) {
        return -EINVAL;
    }

    for (i = 0; i < __this->num; i++) {
        int v = __this->port[i].press_value;
        gpio_direction_input(__this->port[i].port);
        gpio_set_pull_up(__this->port[i].port, !v);
        gpio_set_pull_down(__this->port[i].port, !v);//内部下拉误差20%,关闭内部下拉 由外部电阻开下拉
        /* gpio_set_pull_down(__this->port[i].port, v);//内部下拉误差20%,关闭内部下拉 由外部电阻开下拉 */
    }

    return 0;
}

static const struct key_driver_ops io_key_ops = {
    .init = iokey_init,
    .get_value = io_get_key_value,
};

REGISTER_KEY_DRIVER(iokey) = {
    .name = "iokey",
    .prev_value = 0,
    .last_key   = NO_KEY,
    .filter_cnt = 2,
    .base_cnt   = 2,
    .long_cnt   = 75,
    .hold_cnt   = (75 + 15),
    .scan_time  = 10,
    .ops        = &io_key_ops,
};
