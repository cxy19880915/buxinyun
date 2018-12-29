#ifndef __DAC_H__
#define __DAC_H__

#include "generic/typedef.h"

#define DAC_44_1KHZ            0
#define DAC_48KHZ              1
#define DAC_32KHZ              2
#define DAC_22_05KHZ           3
#define DAC_24KHZ              4
#define DAC_16KHZ              5
#define DAC_11_025KHZ          6
#define DAC_12KHZ              7
#define DAC_8KHZ               8

#define DAC_ISEL_FULL_PWR			4
#define DAC_ISEL_HALF_PWR			2
#define DAC_ISEL_THREE_PWR			1


struct dac_platform_data {
    u8 ldo_id;
};

void dac_early_init(u8 ldo_sel, u8 pwr_sel, u32 dly_msecs);

int dv16_dac_open(u8 sample_rate, u8 ldo_sel, u8 pwr_sel);

int dv16_dac_on();

int dv16_dac_off();

int dv16_dac_close();

void dv16_dac_set_volume(u8 gain);

int dv16_dac_set_sample_rate(u32 sample_rate);

void dv16_dac_irq_handler();

void dv16_dac_set_data_handler(void *priv, void (*handler)(void *, s16 *data, int len));




#endif
