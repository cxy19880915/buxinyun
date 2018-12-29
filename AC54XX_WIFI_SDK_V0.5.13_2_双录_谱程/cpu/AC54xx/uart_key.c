#include "key_driver.h"
#include "system/event.h"
#include "asm/uart.h"

extern int getbyte(char *c);

static int uart_key_init(struct key_driver *_key)
{
    return 0;
}

static u8 uart_get_key_value(struct key_driver *_key)
{
    char c;
    u8 key_value;

    if (getbyte(&c) == 0) {
        return NO_KEY;
    }

    switch (c) {
    case 'm':
        key_value = KEY_MODE;
        break;
    case 'u':
        key_value = KEY_UP;
        break;
    case 'd':
        key_value = KEY_DOWN;
        break;
    case 'o':
        key_value = KEY_OK;
        break;
    case 'e':
        key_value = KEY_MENU;
        break;
    default:
        key_value = NO_KEY;
        break;
    }

    return key_value;
}

struct key_driver_ops uart_key_ops = {
    .init = uart_key_init,
    .get_value = uart_get_key_value,
};

REGISTER_KEY_DRIVER(uart_key) = {
    .prev_value = 0,
    .last_key = NO_KEY,
    .filter_cnt = 0,
    .base_cnt   = 0,
    .long_cnt   = 75,
    .hold_cnt   = (75 + 15),
    .scan_time  = 40,
    .ops        = &uart_key_ops,
};


