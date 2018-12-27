
#ifndef ASM_CPU_H
#define ASM_CPU_H


#include "AC54xx.h"

#ifndef __ASSEMBLY__

typedef unsigned char   		u8, bool, BOOL, bit1;
typedef char            		s8;
typedef unsigned short  		u16;
typedef signed short    		s16;
typedef unsigned int    		u32, tu8, tu16, tbool, tu32;
typedef signed int      		s32;
typedef unsigned long long 		u64;
typedef u32						FOURCC;
typedef long long s64 ;
typedef unsigned long long u64 ;
#endif


#define ___trig        __asm__ volatile ("trigger")


#define BIG_ENDIAN 			0x3021
#define LITTLE_ENDIAN 		0x4576
#define CPU_ENDIAN 			LITTLE_ENDIAN


#define CPU_CORE_NUM     2

#define NO_CACHE_ADDR(addr) \
	(u32)(((u32)(addr) >=0x4000000)?((u32)(addr)+0x4000000):(u32)(addr))

#define  CPU_TASK_CLR(a)
#define  CPU_TASK_SW(a) 		\
    do { \
        extern void clr_wdt(); \
        C0_ILAT_SET|=BIT(7-a); \
        clr_wdt(); \
    } while (0)

#define  CPU_INT_NESTING 	2


#ifndef __ASSEMBLY__

#if CPU_CORE_NUM > 1
static inline int current_cpu_id()
{
    unsigned id;
    asm volatile("%0 = cnum" : "=r"(id) ::);
    return id ;
}
#else
#define current_cpu_id() 	0
#endif

static inline int cpu_in_irq()
{
    int flag;
    __asm__ volatile("%0 = icfg" : "=r"(flag));
    return flag & 0xff;
}

#endif



#define SDRAM                     0
#define MEM_MALLOC                0

#if SDRAM > 0
#define PAGE_SHIFT 14
#define SDRAM_BEGIN_ADDR        (0x800000)
#define MAX_SDRAM_SIZE          (4*1024*1024)
#define MMU_MAX_SIZE            (4*1024*1024)  // mmu 管理的最大地址空间
#define MMU_ADDR_BEGIN          (0x400000)
#define MMU_ADDR_END            (0x800000)
#define STACK_TASK_SIZE         (1024*64)
#define STACK_START             MMU_ADDR_BEGIN
#define STACK_END               (MMU_ADDR_BEGIN+STACK_TASK_SIZE)
#else
#define PAGE_SHIFT 9
//#define SDRAM_BEGIN_ADDR        (0x0)
#define MAX_SDRAM_SIZE          (128*1024-PAGE_SIZE)
#define MMU_MAX_SIZE            (128*1024-PAGE_SIZE)  // mmu 管理的最大地址空间
#define MMU_ADDR_BEGIN          (0x80000)
#define MMU_ADDR_END            (MMU_ADDR_BEGIN+MMU_MAX_SIZE)
#define STACK_TASK_SIZE         (1024*1)
#define STACK_START             MMU_ADDR_BEGIN
#define STACK_TASK_MAX_SIZE     (1024*4)
#endif


#define PAGE_SIZE   			(1UL << PAGE_SHIFT)
#define PAGE_MASK   			(~(PAGE_SIZE-1))
#define PAGE_ALIGN(addr)    	(((addr)+PAGE_SIZE-1)&PAGE_MASK)

#define MMU_TAG_ADDR0   		0x49800

#if CPU_CORE_NUM > 1
#define MMU_TAG_ADDR1   		0x6a000
#endif

#if (MAX_SDRAM_SIZE/PAGE_SIZE  > 256 )
#define ufat_t u16
#else
#define ufat_t u8
#endif





#ifndef __ASSEMBLY__ //assembly

#define __asm_csync() \
    do { \
		asm volatile("csync;"); \
		asm volatile("csync;"); \
		asm volatile("csync;"); \
		asm volatile("csync;"); \
		asm volatile("csync;"); \
    } while (0)

#include "asm/irq.h"
#include "printf.h"
#include "system/generic/log.h"


#define arch_atomic_read(v)  \
	({ \
        __asm_csync(); \
		(*(volatile int *)&(v)->counter); \
	 })

extern volatile int cpu_lock_cnt[];
extern volatile int irq_lock_cnt[];


static inline void local_irq_disable()
{
    __builtin_pi32v2_cli();
    irq_lock_cnt[current_cpu_id()]++;
}


static inline void local_irq_enable()
{
    if (--irq_lock_cnt[current_cpu_id()] == 0) {
        __builtin_pi32v2_sti();
    }
}


#define arch_spin_trylock(lock) \
	do { \
		if(cpu_lock_cnt[current_cpu_id()]++ == 0) \
			asm volatile("lockset;"); \
        __asm_csync(); \
		while ((lock)->rwlock); \
		(lock)->rwlock = 1; \
		if (--cpu_lock_cnt[current_cpu_id()] == 0) \
			asm volatile("lockclr;"); \
	}while(0)

#define arch_spin_lock(lock) \
	do { \
        int ret = false; \
		if(cpu_lock_cnt[current_cpu_id()]++ == 0) \
			asm volatile("lockset;"); \
        __asm_csync(); \
		if (!(lock)->rwlock) { \
            ret = true; \
		    (lock)->rwlock = 1; \
        } \
		if (--cpu_lock_cnt[current_cpu_id()] == 0) \
			asm volatile("lockclr;"); \
        if (ret) \
            break; \
	}while(1)

#define arch_spin_unlock(lock) \
	do { \
		(lock)->rwlock = 0; \
	}while(0)




#define	CPU_SR_ALLOC() 	\
//	int flags

#define CPU_CRITICAL_ENTER()  \
	do { \
		local_irq_disable(); \
		if(cpu_lock_cnt[current_cpu_id()]++ == 0) \
			asm volatile("lockset;"); \
        __asm_csync(); \
	}while(0)


#define CPU_CRITICAL_EXIT() \
	do { \
		if (--cpu_lock_cnt[current_cpu_id()] == 0) \
			asm volatile("lockclr;"); \
		local_irq_enable(); \
	}while(0)

static inline void cpu_reset(void)
{
    extern void log_flush();

    log_flush();
    PWR_CON |= (1 << 4);
    while (1);
}

#define ASSERT(a,...)   \
		do { \
			if(!(a)){ \
                local_irq_disable(); \
				puts("\n\n\nASSERT-FAILD: "#a); \
				printf(" "__VA_ARGS__); \
                puts("\n\n\n"); \
                cpu_reset(); \
			} \
		}while(0);

#endif


#define MULU(Rm,Rn) __builtin_pi32_umul64(Rm,Rn);
#define MUL(Rm,Rn)  __builtin_pi32_smul64(Rm,Rn);
#define MLA(Rm,Rn)  __builtin_pi32_smla64(Rm,Rn);
#define MLA0(Rm,Rn) MUL(Rm,Rn);
#define MLS(Rm,Rn)  __builtin_pi32_smls64(Rm,Rn);
#define MLS0(Rm,Rn) MUL(-Rm,Rn);
#define MRSIS(Rm,Rn) (Rm = __builtin_pi32_sreadmacc(Rn));//mac read right shift imm&sat
#define MRSRS(Rm,Rn) (Rm = __builtin_pi32_sreadmacc(Rn)); //mac read right shift reg&sat
#define MACCLR()  __asm__ volatile ("clrmacc");
#define MACSET(h,l) __asm__ volatile ("mov maccl, %0; mov macch, %1"::"r" (l), "r" (h));
#define MACRL(l) __asm__ volatile ("mov %0, maccl":"=r" (l));
#define MACRH(l) __asm__ volatile ("mov %0, macch":"=r" (l));
#define MULSIS(Ro,Rm,Rn,Rp) MUL(Rm, Rn); MRSIS(Ro, Rp);
#define MULSRS(Ro,Rm,Rn,Rp) MUL(Rm, Rn); MRSRS(Ro, Rp);











#endif


