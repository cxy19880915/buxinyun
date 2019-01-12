#ifndef __LADC_H__
#define __LADC_H__

#include "generic/typedef.h"
// #include "generic/list.h"


#define LADC_BUF_FLAG()                (ADC_CON & BIT(8))//buf标志位，0：buf1可读写   1：buf0可读写
#define LADC_PND()          		   (ADC_CON & BIT(7))
#define LADC_CPND()         		   (ADC_CON |= BIT(6))
#define LADC_IE()           		   (ADC_CON & BIT(5))

/* ADC采样率设置 */
#define ADC_SAMPRATE_44_1KHZ           0
#define ADC_SAMPRATE_48KHZ             1
#define ADC_SAMPRATE_32KHZ             2
#define ADC_SAMPRATE_22_05KHZ          3
#define ADC_SAMPRATE_24KHZ             4
#define ADC_SAMPRATE_16KHZ             5
#define ADC_SAMPRATE_11_025KHZ         6
#define ADC_SAMPRATE_12KHZ             7
#define ADC_SAMPRATE_8KHZ              8


/* 通道选择 */
#define LADC_CH_MIC_L					BIT(0)//PH6
#define LADC_CH_MIC_R					BIT(1)//PH11
#define LADC_CH_LINE0_L					BIT(2)//PH7
#define LADC_CH_LINE0_R					BIT(3)//PH8
#define LADC_CH_LINE1_L					BIT(4)//PH9
#define LADC_CH_LINE1_R					BIT(5)//PH10

#define LADC_MIC_MASK					(BIT(0) | BIT(1))
#define LADC_LINE0_MASK					(BIT(2) | BIT(3))
#define LADC_LINE1_MASK					(BIT(4) | BIT(5))

struct adc_platform_data {
    u8 ladc_in_channel;
    u8 ldo_sel;
};


int dv16_adc_open(int sample_rate, int channel, u8 ldo_sel);


int dv16_adc_set_sample_rate(int sample_rate);

int dv16_adc_start();


int dv16_adc_stop();


int dv16_adc_close();

void dv16_adc_set_data_handler(void *priv, void (*handler)(void *, u8 *data, int len));

int dv16_adc_set_gain(u8 gain_val);

#endif










