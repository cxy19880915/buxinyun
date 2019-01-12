#ifndef ASM_PORT_IO_H
#define ASM_PORT_IO_H


#define IO_GROUP_NUM 		16

#define IO_PORTA_BASE 		(ls_io_base + 0x00*4)

#define IO_PORTA_00 				(IO_GROUP_NUM * 0 + 0)
#define IO_PORTA_01 				(IO_GROUP_NUM * 0 + 1)
#define IO_PORTA_02 				(IO_GROUP_NUM * 0 + 2)
#define IO_PORTA_03 				(IO_GROUP_NUM * 0 + 3)
#define IO_PORTA_04 				(IO_GROUP_NUM * 0 + 4)
#define IO_PORTA_05 				(IO_GROUP_NUM * 0 + 5)
#define IO_PORTA_06 				(IO_GROUP_NUM * 0 + 6)
#define IO_PORTA_07 				(IO_GROUP_NUM * 0 + 7)
#define IO_PORTA_08 				(IO_GROUP_NUM * 0 + 8)
#define IO_PORTA_09 				(IO_GROUP_NUM * 0 + 9)
#define IO_PORTA_10 				(IO_GROUP_NUM * 0 + 10)
#define IO_PORTA_11 				(IO_GROUP_NUM * 0 + 11)
#define IO_PORTA_12 				(IO_GROUP_NUM * 0 + 12)
#define IO_PORTA_13 				(IO_GROUP_NUM * 0 + 13)
#define IO_PORTA_14 				(IO_GROUP_NUM * 0 + 14)
#define IO_PORTA_15 				(IO_GROUP_NUM * 0 + 15)

#define IO_PORTB_00 				(IO_GROUP_NUM * 1 + 0)
#define IO_PORTB_01 				(IO_GROUP_NUM * 1 + 1)
#define IO_PORTB_02 				(IO_GROUP_NUM * 1 + 2)
#define IO_PORTB_03 				(IO_GROUP_NUM * 1 + 3)
#define IO_PORTB_04 				(IO_GROUP_NUM * 1 + 4)
#define IO_PORTB_05 				(IO_GROUP_NUM * 1 + 5)
#define IO_PORTB_06 				(IO_GROUP_NUM * 1 + 6)
#define IO_PORTB_07 				(IO_GROUP_NUM * 1 + 7)
#define IO_PORTB_08 				(IO_GROUP_NUM * 1 + 8)
#define IO_PORTB_09 				(IO_GROUP_NUM * 1 + 9)
#define IO_PORTB_10 				(IO_GROUP_NUM * 1 + 10)
#define IO_PORTB_11 				(IO_GROUP_NUM * 1 + 11)
#define IO_PORTB_12 				(IO_GROUP_NUM * 1 + 12)
#define IO_PORTB_13 				(IO_GROUP_NUM * 1 + 13)
#define IO_PORTB_14 				(IO_GROUP_NUM * 1 + 14)
#define IO_PORTB_15 				(IO_GROUP_NUM * 1 + 15)

#define IO_PORTC_00 				(IO_GROUP_NUM * 2 + 0)
#define IO_PORTC_01 				(IO_GROUP_NUM * 2 + 1)
#define IO_PORTC_02 				(IO_GROUP_NUM * 2 + 2)
#define IO_PORTC_03 				(IO_GROUP_NUM * 2 + 3)
#define IO_PORTC_04 				(IO_GROUP_NUM * 2 + 4)
#define IO_PORTC_05 				(IO_GROUP_NUM * 2 + 5)
#define IO_PORTC_06 				(IO_GROUP_NUM * 2 + 6)
#define IO_PORTC_07 				(IO_GROUP_NUM * 2 + 7)
#define IO_PORTC_08 				(IO_GROUP_NUM * 2 + 8)
#define IO_PORTC_09 				(IO_GROUP_NUM * 2 + 9)
#define IO_PORTC_10 				(IO_GROUP_NUM * 2 + 10)
#define IO_PORTC_11 				(IO_GROUP_NUM * 2 + 11)
#define IO_PORTC_12 				(IO_GROUP_NUM * 2 + 12)
#define IO_PORTC_13 				(IO_GROUP_NUM * 2 + 13)
#define IO_PORTC_14 				(IO_GROUP_NUM * 2 + 14)
#define IO_PORTC_15 				(IO_GROUP_NUM * 2 + 15)

#define IO_PORTD_00 				(IO_GROUP_NUM * 3 + 0)
#define IO_PORTD_01 				(IO_GROUP_NUM * 3 + 1)
#define IO_PORTD_02 				(IO_GROUP_NUM * 3 + 2)
#define IO_PORTD_03 				(IO_GROUP_NUM * 3 + 3)
#define IO_PORTD_04 				(IO_GROUP_NUM * 3 + 4)
#define IO_PORTD_05 				(IO_GROUP_NUM * 3 + 5)
#define IO_PORTD_06 				(IO_GROUP_NUM * 3 + 6)
#define IO_PORTD_07 				(IO_GROUP_NUM * 3 + 7)
#define IO_PORTD_08 				(IO_GROUP_NUM * 3 + 8)
#define IO_PORTD_09 				(IO_GROUP_NUM * 3 + 9)
#define IO_PORTD_10 				(IO_GROUP_NUM * 3 + 10)
#define IO_PORTD_11 				(IO_GROUP_NUM * 3 + 11)
#define IO_PORTD_12 				(IO_GROUP_NUM * 3 + 12)
#define IO_PORTD_13 				(IO_GROUP_NUM * 3 + 13)
#define IO_PORTD_14 				(IO_GROUP_NUM * 3 + 14)
#define IO_PORTD_15 				(IO_GROUP_NUM * 3 + 15)


#define IO_PORTE_00 				(IO_GROUP_NUM * 4 + 0)
#define IO_PORTE_01 				(IO_GROUP_NUM * 4 + 1)
#define IO_PORTE_02 				(IO_GROUP_NUM * 4 + 2)
#define IO_PORTE_03 				(IO_GROUP_NUM * 4 + 3)
#define IO_PORTE_04 				(IO_GROUP_NUM * 4 + 4)
#define IO_PORTE_05 				(IO_GROUP_NUM * 4 + 5)
#define IO_PORTE_06 				(IO_GROUP_NUM * 4 + 6)
#define IO_PORTE_07 				(IO_GROUP_NUM * 4 + 7)
#define IO_PORTE_08 				(IO_GROUP_NUM * 4 + 8)
#define IO_PORTE_09 				(IO_GROUP_NUM * 4 + 9)
#define IO_PORTE_10 				(IO_GROUP_NUM * 4 + 10)
#define IO_PORTE_11 				(IO_GROUP_NUM * 4 + 11)
#define IO_PORTE_12 				(IO_GROUP_NUM * 4 + 12)
#define IO_PORTE_13 				(IO_GROUP_NUM * 4 + 13)
#define IO_PORTE_14 				(IO_GROUP_NUM * 4 + 14)
#define IO_PORTE_15 				(IO_GROUP_NUM * 4 + 15)


#define IO_PORTF_00 				(IO_GROUP_NUM * 5 + 0)
#define IO_PORTF_01 				(IO_GROUP_NUM * 5 + 1)
#define IO_PORTF_02 				(IO_GROUP_NUM * 5 + 2)
#define IO_PORTF_03 				(IO_GROUP_NUM * 5 + 3)
#define IO_PORTF_04 				(IO_GROUP_NUM * 5 + 4)
#define IO_PORTF_05 				(IO_GROUP_NUM * 5 + 5)
#define IO_PORTF_06 				(IO_GROUP_NUM * 5 + 6)
#define IO_PORTF_07 				(IO_GROUP_NUM * 5 + 7)
#define IO_PORTF_08 				(IO_GROUP_NUM * 5 + 8)
#define IO_PORTF_09 				(IO_GROUP_NUM * 5 + 9)
#define IO_PORTF_10 				(IO_GROUP_NUM * 5 + 10)
#define IO_PORTF_11 				(IO_GROUP_NUM * 5 + 11)
#define IO_PORTF_12 				(IO_GROUP_NUM * 5 + 12)
#define IO_PORTF_13 				(IO_GROUP_NUM * 5 + 13)
#define IO_PORTF_14 				(IO_GROUP_NUM * 5 + 14)
#define IO_PORTF_15 				(IO_GROUP_NUM * 5 + 15)


#define IO_PORTG_00 				(IO_GROUP_NUM * 6 + 0)
#define IO_PORTG_01 				(IO_GROUP_NUM * 6 + 1)
#define IO_PORTG_02 				(IO_GROUP_NUM * 6 + 2)
#define IO_PORTG_03 				(IO_GROUP_NUM * 6 + 3)
#define IO_PORTG_04 				(IO_GROUP_NUM * 6 + 4)
#define IO_PORTG_05 				(IO_GROUP_NUM * 6 + 5)
#define IO_PORTG_06 				(IO_GROUP_NUM * 6 + 6)
#define IO_PORTG_07 				(IO_GROUP_NUM * 6 + 7)
#define IO_PORTG_08 				(IO_GROUP_NUM * 6 + 8)
#define IO_PORTG_09 				(IO_GROUP_NUM * 6 + 9)
#define IO_PORTG_10 				(IO_GROUP_NUM * 6 + 10)
#define IO_PORTG_11 				(IO_GROUP_NUM * 6 + 11)
#define IO_PORTG_12 				(IO_GROUP_NUM * 6 + 12)
#define IO_PORTG_13 				(IO_GROUP_NUM * 6 + 13)
#define IO_PORTG_14 				(IO_GROUP_NUM * 6 + 14)
#define IO_PORTG_15 				(IO_GROUP_NUM * 6 + 15)



#define IO_PORTH_00 				(IO_GROUP_NUM * 7 + 0)
#define IO_PORTH_01 				(IO_GROUP_NUM * 7 + 1)
#define IO_PORTH_02 				(IO_GROUP_NUM * 7 + 2)
#define IO_PORTH_03 				(IO_GROUP_NUM * 7 + 3)
#define IO_PORTH_04 				(IO_GROUP_NUM * 7 + 4)
#define IO_PORTH_05 				(IO_GROUP_NUM * 7 + 5)
#define IO_PORTH_06 				(IO_GROUP_NUM * 7 + 6)
#define IO_PORTH_07 				(IO_GROUP_NUM * 7 + 7)
#define IO_PORTH_08 				(IO_GROUP_NUM * 7 + 8)
#define IO_PORTH_09 				(IO_GROUP_NUM * 7 + 9)
#define IO_PORTH_10 				(IO_GROUP_NUM * 7 + 10)
#define IO_PORTH_11 				(IO_GROUP_NUM * 7 + 11)
#define IO_PORTH_12 				(IO_GROUP_NUM * 7 + 12)
#define IO_PORTH_13 				(IO_GROUP_NUM * 7 + 13)
#define IO_PORTH_14 				(IO_GROUP_NUM * 7 + 14)
#define IO_PORTH_15 				(IO_GROUP_NUM * 7 + 15)



#define IO_PORTI_00 				(IO_GROUP_NUM * 8 + 0)
#define IO_PORTI_01 				(IO_GROUP_NUM * 8 + 1)
#define IO_PORTI_02 				(IO_GROUP_NUM * 8 + 2)
#define IO_PORTI_03 				(IO_GROUP_NUM * 8 + 3)
#define IO_PORTI_04 				(IO_GROUP_NUM * 8 + 4)
#define IO_PORTI_05 				(IO_GROUP_NUM * 8 + 5)
#define IO_PORTI_06 				(IO_GROUP_NUM * 8 + 6)
#define IO_PORTI_07 				(IO_GROUP_NUM * 8 + 7)
#define IO_PORTI_08 				(IO_GROUP_NUM * 8 + 8)
#define IO_PORTI_09 				(IO_GROUP_NUM * 8 + 9)
#define IO_PORTI_10 				(IO_GROUP_NUM * 8 + 10)
#define IO_PORTI_11 				(IO_GROUP_NUM * 8 + 11)
#define IO_PORTI_12 				(IO_GROUP_NUM * 8 + 12)
#define IO_PORTI_13 				(IO_GROUP_NUM * 8 + 13)
#define IO_PORTI_14 				(IO_GROUP_NUM * 8 + 14)
#define IO_PORTI_15 				(IO_GROUP_NUM * 8 + 15)



#define IO_MAX_NUM 					(IO_PORTI_15+1)


#define IO_PORT_PR_00               (IO_MAX_NUM + 0)
#define IO_PORT_PR_01               (IO_MAX_NUM + 1)
#define IO_PORT_PR_02               (IO_MAX_NUM + 2)
#define IO_PORT_PR_03               (IO_MAX_NUM + 3)




struct gpio_reg {
    volatile unsigned long out;
    volatile unsigned long in;
    volatile unsigned long dir;
    volatile unsigned long die;
    volatile unsigned long pu;
    volatile unsigned long pd;
    volatile unsigned long hd;
};

struct gpio_platform_data {
    unsigned int gpio;
};

#define GPIO_PLATFORM_DATA_BEGIN(data) \
	static const struct gpio_platform_data data = { \


#define GPIO_PLATFORM_DATA_END() \
	};



#endif

