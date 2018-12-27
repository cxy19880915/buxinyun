#include "asm/cpu.h"
#include "asm/adc.h"
#include "system/init.h"
#include "system/timer.h"

#define FIRST_CHANNEL() \
	adc_scan_begin

#define NEXT_CHANNEL(ch) \
	++ch >= adc_scan_end? adc_scan_begin:ch

static struct adc_scan *channel = FIRST_CHANNEL();

static void adc_scan_process(void *p)
{
    while (!ADC_PND());
    channel->value = GPADC_RES;

    channel = NEXT_CHANNEL(channel);
    /* printf("ch: %d val:0x%x\n",channel->channel,channel->value); */
    ADCSEL(channel->channel);
    KITSTART();
}


static int adc_scan_init()
{
    ADCSEL(channel->channel);
    ADC_EN(1);
    ADCEN(1);
    ADC_BAUD(0x7);
    ADC_WTIME(0x1);
    KITSTART();

    //LDO_CON |= BIT(11);

    sys_hi_timer_add(0, adc_scan_process, 4);

    return 0;
}
platform_initcall(adc_scan_init);




