#include "asm/cpu.h"
#include "asm/adc.h"
#include "asm/gpio.h"
#include "system/init.h"
#include "system/timer.h"
#include "gpio.h"




REGISTER_ADC_SCAN(ntc_scan)
.channel = AD_CH03_PB12,
 .value = 0,
};

static u16 ntc_get_value(struct key_driver *key)
{
    return ntc_scan.value;
}


u16 ntc_flag_high = 1;
u16 ntc_flag_low = 1;
u16 ntc_flag_normal = 1;

u16 ntc_count_high = 0;
u16 ntc_count_low = 0;
u16 ntc_count_normal = 0;

u16 high_threhold = 320;//55C.
//u16 high_threhold = 456;//50C.
u16 low_threhold  = 780;//-5c.
u16 count_threhold  = 200;


static void ntc_scan_process(void *p)
{
    u16 ntc_value = 0;
    ntc_value = ntc_get_value(NULL);

    if (ntc_value <= high_threhold) {
        ntc_count_high++;

        ntc_count_normal = 0;
        ntc_flag_normal = 1;

        ntc_count_low = 0;
        ntc_flag_low = 1;
        //puts("\n  high  \n");

        if (ntc_count_high > count_threhold && (ntc_flag_high == 1)) {
            //puts("\n @@@@@@@@@@@@@@ 	high temperature \n");
            gpio_direction_output(IO_PORTB_11, 1);//charge_ctrl
            gpio_direction_output(IO_PORTE_02, 0);

            ntc_count_high = 0;
            ntc_flag_high = 0;
        }
    }

    else if ((ntc_value >= low_threhold) && (ntc_value <= 1022)) {
        ntc_count_low++;

        ntc_count_normal = 0;
        ntc_flag_normal = 1;

        ntc_count_high = 0;
        ntc_flag_high = 1;
        //puts("\n  low  \n");

        if (ntc_count_low > count_threhold && (ntc_flag_low == 1)) {
            //puts("\n @@@@@@@@@@@@@@ 	low temperature \n");
            gpio_direction_output(IO_PORTB_11, 1);//charge_ctrl
            gpio_direction_output(IO_PORTE_02, 0);

            ntc_count_low = 0;
            ntc_flag_low = 0;

        }

    } else if ((ntc_value > high_threhold) && (ntc_value < low_threhold)) {
        ntc_count_normal++;

        ntc_count_high = 0;
        ntc_flag_high = 1;

        ntc_count_low = 0;
        ntc_flag_low = 1;

        //puts("\n  normal  \n");

        if (ntc_count_normal > count_threhold && (ntc_flag_normal == 1)) {
            //puts("\n @@@@@@@@@@@@@@    normal temperature \n");

            //gpio_direction_output(IO_PORTH_01, 0);//charge_ctrl
            gpio_direction_input(IO_PORTB_11);
            gpio_direction_output(IO_PORTE_02, 1);

            ntc_count_normal = 0;
            ntc_flag_normal = 0;
        }
    }

    else {

        ntc_count_high = 0;
        ntc_flag_high = 1;

        ntc_count_low = 0;
        ntc_flag_low = 1;

        ntc_count_normal = 0;
        ntc_flag_normal = 1;

        //puts("\n ntc error \n");

    }

#if 0
    printf("ntc_value = %d\n\r", ntc_value);
    printf("count_high = %d flag_high = %d \n\r", ntc_count_high, ntc_flag_high);
    printf("count_low = %d flag_low = %d \n\r", ntc_count_low, ntc_flag_low);
    printf("count_normal = %d flag_normal = %d \n\r", ntc_count_normal, ntc_flag_normal);
#endif


}



static int ntc_scan_init()
{

    //gpio_direction_output(IO_PORTH_01, 1);//charge_ctrl
    sys_hi_timer_add(0, ntc_scan_process, 100);
    gpio_direction_output(IO_PORTE_02, 1);
    return 0;
}
platform_initcall(ntc_scan_init);






