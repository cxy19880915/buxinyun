/**
 * @file debug.c
 * @brief DebugModule
 * @author chenrixin@zh-jieli.net
 * @version 0.1.00
 * @date 2016-10-27
 */
#include "asm/cpu.h"
#include "asm/debug.h"
#include "os/os_cpu.h"
#define DEBUG_WR_SFR_EN do{DEBUG_WR_EN = 0xe7;}while(0);
static u32 debug_index;
//#define printf(fmt,arg...)


void debug_enter_critical()
{
//    if(DEBUG_WR_EN & BIT(0)){
//        return;
//    }
//    DEBUG_WR_EN = 0xe7;//set debug reg write enable
}

void debug_exit_critical()
{
//    if(DEBUG_WR_EN & BIT(0)){
//        DEBUG_WR_EN = 0xe7;//set debug reg write disable
//        return;
//    }
}


/* --------------------------------------------------------------------------*/
/**
 * @brief get_debug_index
 * 		»ñÈ¡¿ÉÒÔÊ¹ÓÃµÄdebugŽ°¿Ú
 *
 * @return -1 ±íÊŸÃ»ÓÐ¿ÉÒÔÊ¹ÓÃµÄŽ°¿Ú
 * 			0-7 ¿ÉÒÔÊ¹ÓÃµÄŽ°¿ÚºÅ
 */
/* --------------------------------------------------------------------------*/
static inline u32 get_debug_index()
{
    for (u32 i = 0; i < 8 ; i++) {
        if (!(debug_index & BIT(i))) {
            return i;
        }
    }

    return -1;

}
/* --------------------------------------------------------------------------*/
/**
 * @brief pc0_rang_limit cpu0 pcÖžÕëÔËÐÐ·¶Î§ÏÞÖÆ
 *
 * @param low_addr µÍµØÖ·
 * @param high_addr žßµØÖ·
 */
/* --------------------------------------------------------------------------*/
void pc0_rang_limit(void *low_addr, void *high_addr)
{
    printf("pc0 :%x---%x\n", (u32)low_addr, (u32)high_addr);

    DEBUG_MSG_CLR = 0xffffffff;
    C0_PC_LIMH = (u32)high_addr;
    C0_PC_LIML = (u32)low_addr;
}
/* --------------------------------------------------------------------------*/
/**
 * @brief pc0_rang_limit cpu0 pcÖžÕëÔËÐÐ·¶Î§ÏÞÖÆ
 *
 * @param low_addr µÍµØÖ·
 * @param high_addr žßµØÖ·
 */
/* --------------------------------------------------------------------------*/
void pc1_rang_limit(void *low_addr, void *high_addr)
{
    printf("pc1 :%x---%x\n", (u32)low_addr, (u32)high_addr);

    //DEBUG_MSG_CLR = 0xffffffff;
    C1_PC_LIMH = (u32)high_addr;
    C1_PC_LIML = (u32)low_addr;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief pc_rang_limit all cpu pcÖžÕëÔËÐÐ·¶Î§ÏÞÖÆ
 *
 * @param low_addr µÍµØÖ·
 * @param high_addr žßµØÖ·
 */
/* --------------------------------------------------------------------------*/
void pc_rang_limit(void *low_addr, void *high_addr)
{

    debug_enter_critical();

    if (OS_CPU_ID == 0) {
        pc0_rang_limit(low_addr, high_addr);
    } else {
        pc1_rang_limit(low_addr, high_addr);
    }

    debug_exit_critical();


}
/**
 * @brief debugŽ°¿Ú
 */
struct _wr_limit {
    volatile u32 wr_limit_h; ///<Ž°¿ÚžßµØÖ·
    volatile u32 wr_limit_l; ///<Ž°¿ÚµÍµØÖ·
};

#define wr_limit    ((struct _wr_limit *)&WR_LIM0H)
//#define wr_limit1    ((struct _wr_limit *)&C1_WR_LIM0H)
/* --------------------------------------------------------------------------*/
/**
 * @brief dev_write_range_limit
 * ²»¿ÉÒÔÊ¹ÓÃÒ»žöŽ°¿ÚÑ¡ÔñÒ»Æ¬ÄÚŽæÔÊÐíÍâÉèAÐŽ£¬Í¬Ê±Ê¹ÓÃÁíÒ»žöŽ°¿ÚœûÖ¹ÍâÉèBÐŽ¡£
 * @param limit_index debugŽ°¿ÚË÷Òý -1 ×Ô¶¯Ñ¡Ôñ¿ÉÒÔÊ¹ÓÃµÄŽ°¿Ú
 * @param low_addr  Ž°¿ÚµÄµÍµØÖ·
 * @param high_addr Ž°¿ÚµÄžßµØÖ·
 * @param is_allow_write
 *       1--Öž¶šÍâÉèÐŽŽ°¿ÚÄÚ²¿²»Ž¥·¢Òì³£
 *       0--ÈÎÒâÍâÉèÐŽŽ°¿ÚÄÚ²¿Ž¥·¢Òì³£
 *      µ±²ÎÊý¡Ÿis_allow_write¡¿Îª0µÄÊ±ºò£¬²ÎÊý¡Ÿdev¡¿ÎÞÐ§
 * @param dev Öž¶šÍâÉèµÄ±àºÅ
 * @return 0³É¹Š
 * 		  ÆäËû·µ»Ødebug_index
 * 	devnum ¶šÒåÔÚdebug.h
 * @note ²»¿ÉÒÔÊ¹ÓÃÒ»žöŽ°¿ÚÑ¡ÔñÒ»Æ¬ÄÚŽæÔÊÐíÍâÉèAÐŽ£¬Í¬Ê±Ê¹ÓÃÁíÒ»žöŽ°¿ÚœûÖ¹ÍâÉèBÐŽ¡£
 * low_addr high_addr Ö®²îÐèÒª>=4×ÖœÚ
 */
/* --------------------------------------------------------------------------*/
u32 dev_write_range_limit(u32 limit_index, void *low_addr, void *high_addr, u32 is_allow_write, u32 dev)
{
    if (limit_index == -1) {
        limit_index = get_debug_index();
    } else if (debug_index & BIT(limit_index)) {
        return debug_index;
    }

    debug_enter_critical();

    debug_index |= BIT(limit_index);
//    printf("dev %x write %s low_addr:%x--high_addr:%x\n",
//           dev,
//           is_allow_write ? ("allow") : ("limit"),
//           (u32)low_addr,
//           (u32)high_addr);

    DEBUG_MSG_CLR = 0xffffffff;
    wr_limit[limit_index].wr_limit_h = (u32)high_addr;
    wr_limit[limit_index].wr_limit_l = (u32)low_addr;

    if (is_allow_write) {
        if (limit_index > 3) {
            PRP_ALLOW_NUM1 |= dev << ((limit_index - 4) * 8);
        } else {
            PRP_ALLOW_NUM0 |= dev << (limit_index * 8);
        }

        DBG_CON0 |= ((1 << limit_index) << 24) | ((1 << limit_index) << 8);
    } else {
        if (limit_index > 3) {
            PRP_ALLOW_NUM1 &= ~(0xff << ((limit_index - 4) * 8));
        } else {
            PRP_ALLOW_NUM0 &= ~(0xff << (limit_index * 8));
        }

        DBG_CON0 &= ~((1 << limit_index) << 8);
        DBG_CON0 |= (1 << limit_index) << 24;
    }

    debug_exit_critical();

#if 0
    printf("WR_LIM%dL:%x--WR_LIM%dH:%x--DBG_CON0:%x--PRP_ALLOW_NUM0:%x--%x\n",
           limit_index,
           wr_limit[limit_index].wr_limit_l,
           limit_index,
           wr_limit[limit_index].wr_limit_h,
           DBG_CON0,
           PRP_ALLOW_NUM0, PRP_ALLOW_NUM1);
#endif
    return 0;
}
/* --------------------------------------------------------------------------*/
/**
 * @brief cpu0_write_range_limit ²¶×œCPU0ÐŽ³¬³ö·¶Î§
 *
 * @param limit_index debugŽ°¿ÚË÷Òý
 * @param low_addr ·¶Î§ÄÚµÄµÍµØÖ·
 * @param high_addr ·¶Î§ÍâµÄžßµØÖ·
 * @return 0³É¹Š
 * 		  ÆäËû·µ»Ødebug_index
 */
/* --------------------------------------------------------------------------*/
u32 cpu0_write_range_limit(u32 limit_index, void *low_addr, void *high_addr)
{
    if (limit_index == -1) {
        limit_index = get_debug_index();
    } else if (debug_index & BIT(limit_index)) {
        return debug_index;
    }

    debug_index |= BIT(limit_index);
//    printf("cpu0 ex limit low_addr:%x--high_addr:%x\n", (u32)low_addr, (u32)high_addr);


    DEBUG_MSG_CLR = 0xffffffff;

    wr_limit[limit_index].wr_limit_h = (u32)high_addr;
    wr_limit[limit_index].wr_limit_l = (u32)low_addr;
    DBG_CON0 |= (1 << (16 + limit_index));
#if 0
    printf("WR_LIM%dL:%x--WR_LIM%dH:%x---DBG_CON0:%x\n",
           limit_index,
           wr_limit[limit_index].wr_limit_l,
           limit_index,
           wr_limit[limit_index].wr_limit_h,
           DBG_CON0);
#endif
    return 0;
}
/* --------------------------------------------------------------------------*/
/**
 * @brief cpu1_write_range_limit ²¶×œCPU1ÐŽ³¬³ö·¶Î§
 *
 * @param limit_index debugŽ°¿ÚË÷Òý
 * @param low_addr ·¶Î§ÄÚµÄµÍµØÖ·
 * @param high_addr ·¶Î§ÍâµÄžßµØÖ·
 * @return 0³É¹Š
 * 		  ÆäËû·µ»Ødebug_index
 */
/* --------------------------------------------------------------------------*/
u32 cpu1_write_range_limit(u32 limit_index, void *low_addr, void *high_addr)
{
    if (limit_index == -1) {
        limit_index = get_debug_index();
    } else if (debug_index & BIT(limit_index)) {
        return debug_index;
    }
    debug_index |= BIT(limit_index);
//    printf("cpu1 ex limit low_addr:%x--high_addr:%x\n", (u32)low_addr, (u32)high_addr);


    DEBUG_MSG_CLR = 0xffffffff;

    wr_limit[limit_index].wr_limit_h = (u32)high_addr;
    wr_limit[limit_index].wr_limit_l = (u32)low_addr;
    DBG_CON0 |= (1 << limit_index);
#if 0
    printf("WR_LIM%dL:%x--WR_LIM%dH:%x---DBG_CON0:%x\n",
           limit_index,
           wr_limit1[limit_index].wr_limit_l,
           limit_index,
           wr_limit1[limit_index].wr_limit_h,
           DBG_CON0);
#endif
    return 0;
}
/* --------------------------------------------------------------------------*/
/**
 * @brief cpu_write_range_limt auto select index for cpu0&cpu1
 *
 * @param low_addr
 * @param high_addr
 *
 * @return 0 ³É¹Š
 * 		  -1 Ê§°Ü
 */
/* --------------------------------------------------------------------------*/
u32 cpu_write_range_limit(void *low_addr, u32 win_size)
{
    u32 i = get_debug_index();
    void *high_addr = low_addr + win_size;

    debug_enter_critical();

    if (i != -1) {
        cpu0_write_range_limit(i, low_addr, high_addr);
        i = get_debug_index();

        if (i != -1) {
            cpu1_write_range_limit(i, low_addr, high_addr);
        } else {
            debug_exit_critical();
            return -1;
        }
    } else {
        debug_exit_critical();
        return -1;
    }

    debug_exit_critical();

    return 0;
}

u32 sdr_write_range_limit(void *low_addr, u32 win_size, u32 is_allow_write, u32 dev)
{
    void *high_addr = (void *)((u32)low_addr + win_size);

    u32 i = get_debug_index();
    if (i == -1) {
        return -1;
    }


    debug_enter_critical();

    debug_index |= BIT(i);
    printf("sdr %x write %s low_addr:%x--high_addr:%x\n",
           dev,
           is_allow_write ? ("allow") : ("limit"),
           (u32)low_addr,
           (u32)high_addr);

    if (is_allow_write) {
        if (i > 3) {
            PRP_ALLOW_NUM1 |= dev << ((i - 4) * 8);
        } else {
            PRP_ALLOW_NUM0 |= dev << (i * 8);
        }

        DBG_CON0 |= ((1 << i) << 24) | ((1 << i) << 8);
    } else {
        if (i > 3) {
            PRP_ALLOW_NUM1 &= ~(0xff << ((i - 4) * 8));
        } else {
            PRP_ALLOW_NUM0 &= ~(0xff << (i * 8));
        }
        DBG_CON0 &= ~((1 << i) << 8);
    }


    DEBUG_MSG_CLR = 0xffffffff;

    wr_limit[i].wr_limit_h = (u32)high_addr;
    wr_limit[i].wr_limit_l = (u32)low_addr;
//    SDRDBG |= (1 << (8+i))|BIT(30);
    SDRDBG = (1 << (8 + i)) | BIT(30);
    SDRDBG = (1 << (8 + i)) | BIT(30);
    SDRDBG = (1 << (8 + i)) | BIT(30);
    SDRDBG = (1 << (8 + i)) | BIT(30);

    debug_exit_critical();

    return 0;
}


void debug_clear()
{

    debug_enter_critical();

    SDRDBG = 0;
    DBG_CON0 = 0;
    PRP_ALLOW_NUM0 = 0;
    PRP_ALLOW_NUM1 = 0;

    DEBUG_MSG_CLR = 0xffffffff;

    debug_index = 0;

    debug_exit_critical();

}
/* --------------------------------------------------------------------------*/
/**
 * @brief debug_init
 * enable peripheral_bus_inv_en bus_inv_expt_en
 * enable if_inv_en of_inv_en ex_inv_en
 * @none Ö»ÄÜµ÷ÓÃÒ»ŽÎ
 */
/* --------------------------------------------------------------------------*/
void debug_init()
{
//	DBG_CON1 |= 3;	///< enable peripheral_bus_inv_en bus_inv_expt_en

    debug_enter_critical();

    if (OS_CPU_ID == 0) {
        C0_CON |= (0xf << 9);	///< enable if_inv_en of_inv_en ex_inv_en
        debug_index = 0;
        DEBUG_MSG_CLR = 0xffffffff;
        debug_clear();
    } else {
        C1_CON |= (0xf << 9);	///< enable if_inv_en of_inv_en ex_inv_en
    }

    debug_exit_critical();

    printf("%s()debug_index=%d\n", __func__, debug_index);

//    request_irq(4, 0, exception_irq_handler, 0);
//    request_irq(5, 0, exception_irq_handler, 0);
//    request_irq(6, 0, exception_irq_handler, 0);

    //pc_rang_limit(&text_rodata_begin, &text_rodata_end);
}


