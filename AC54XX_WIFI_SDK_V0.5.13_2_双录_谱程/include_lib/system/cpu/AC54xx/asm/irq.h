#ifndef CPU_IRQ_H
#define CPU_IRQ_H


#include "asm/hwi.h"

#define ___interrupt 	__attribute__((interrupt("")))




#define local_irq_save(flags) \
	do { \
		int tmp; \
		__asm__ volatile ("%0 = icfg" : "=r"(flags)); \
		tmp = flags & ~BIT(8); \
		__asm__ volatile ("icfg = %0" :: "r"(tmp)); \
	}while(0)


#define local_irq_restore(flags) \
	do { \
		__asm__ volatile ("icfg = %0" :: "r"(flags)); \
   	}while(0)



#endif




