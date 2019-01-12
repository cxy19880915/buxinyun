#include "asm/includes.h"
#include "asm/sysinfo.h"
#include "asm/ldo.h"
#include "system/task.h"

#include "app_config.h"
#include "power_check_onetime.h"


#ifdef CONFIG_DEBUG_ENABLE
extern void debug_uart_init();
#endif

static char *debug_msg[32] = {
    /*0---7*/
    "reserved",
    "reserved",
    "prp_ex_limit_err",
    "sdr_wr_err",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    /*8---15*/
    "c1_div_zero",
    "c0_div_zero",
    "c1_pc_limit_err_r",
    "c1_wr_limit_err_r",
    "c0_pc_limit_err_r",
    "c0_wr_limit_err_r",
    "c1_misaligned",
    "c0_misaligned",
    /*16---23*/
    "c1_if_bus_inv",
    "c1_rd_bus_inv",
    "c1_wr_bus_inv",
    "c0_if_bus_inv",
    "c0_rd_bus_inv",
    "c0_wr_bus_inv",
    "prp_bus_inv",
    "reserved",
    /*24---31*/
    "c1_mmu_wr_excpt",
    "c1_mmu_rd_excpt",
    "c0_mmu_wr_excpt",
    "c0_mmu_rd_excpt",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
};


___interrupt
void exception_irq_handler(void)
{
    u32 rets_addr, reti_addr;
    u32 i;
    u32 tmp_sp, tmp_usp, tmp_ssp;
    u32 *tmp_sp_ptr;


    __asm__ volatile("[--sp] = {r15-r0}");

    __asm__ volatile("%0 = rets ;" : "=r"(rets_addr));
    __asm__ volatile("%0 = reti ;" : "=r"(reti_addr));

    __asm__ volatile("%0 = sp ;" : "=r"(tmp_sp));
    __asm__ volatile("%0 = usp ;" : "=r"(tmp_usp));
    __asm__ volatile("%0 = ssp ;" : "=r"(tmp_ssp));


    FPGA_TRI = 0;

    puts("\nput sp\n");
    tmp_sp_ptr = (u32 *)tmp_sp;
    for (i = 0; i < 20; i++) {
        put_u32hex(*tmp_sp_ptr);
        tmp_sp_ptr++;
    }

    if (OS_CPU_ID == 0) {
        C1_CON |= BIT(2);
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
    } else {
        C0_CON |= BIT(2);
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
        __asm__ volatile("csync");
    }


    if (OS_CPU_ID == 0) {
        printf("\n\n\n!!!!! cpu 0 %s: rets_addr = 0x%x, reti_addr = 0x%x\n DEBUG_MSG = 0x%x, DEBUG_PRP_NUM = 0x%x DSPCON=%x\n",
               __func__, rets_addr, reti_addr, DEBUG_MSG, DEBUG_PRP_NUM, C0_CON);
    } else {
        printf("\n\n\n!!!!! cpu 1 %s: rets_addr = 0x%x, reti_addr = 0x%x\n DEBUG_MSG = 0x%x, DEBUG_PRP_NUM = 0x%x DSPCON=%x\n",
               __func__, rets_addr, reti_addr, DEBUG_MSG, DEBUG_PRP_NUM, C1_CON);
    }

    printf("\nsp : 0x%x, usp : 0x%x, ssp : 0x%x\n", tmp_sp, tmp_usp, tmp_ssp);

    printf("\nWR_LIM4H : 0x%x, WR_LIM4L : 0x%x, SDRDBG : 0x%x\n", WR_LIM4H, WR_LIM4L, SDRDBG);


    for (i = 0; i < 32; i++) {
        if (BIT(i)&DEBUG_MSG) {
            puts(debug_msg[i]);
            put_u32hex(i);
        }
    }

    log_flush();
    while (1);
}

static void cpu_xbus_init()
{
    ENC_SEL = 0;

    xbus_ch01_lvl = 1; //isp0 tnr wr
    xbus_ch02_lvl = 0; //isp0 stc wr
    xbus_ch03_lvl = 3; //imc ch0 wr encode
    xbus_ch04_lvl = 3; //imc ch1 wr encode
    xbus_ch05_lvl = 0; //imc ch2 wr
    xbus_ch06_lvl = 0; //imc ch3 wr
    xbus_ch07_lvl = 0; //imc ch4 wr
    xbus_ch08_lvl = 2; //imb obuf wr

    xbus_ch17_lvl = 1; //isp0 tnr rd
    xbus_ch18_lvl = 2; //imc replay
    xbus_ch19_lvl = 0; //imc ch0 osd
    xbus_ch20_lvl = 0; //imc ch1 osd
    xbus_ch21_lvl = 2; //imb ibuf rd
    xbus_ch22_lvl = 0; //lcd ibuf rd

    xbus_lv1_prd = 8;
    xbus_lv2_prd = 16;

    AVC_BUS_CON = 0;
    AVC_BUS_CON = 1 | (31 << 1) | (9 << 6);

    xbus_dist0_ext = 1;

    clk_set("avc", 360000000);
}

extern u32 text_rodata_begin, text_rodata_end;

void cpu1_main()
{
    local_irq_disable();

    interrupt_init();

    request_irq(1, 7, exception_irq_handler, 1);

//    debug_init();

    os_start();

    local_irq_enable();

    while (1) {
        __asm__ volatile("idle");
    }
}

static void wdt_init()
{
    /*
     * 超时: 0-15 对应 {1ms, 2ms, 4ms, 8ms, ...512ms, 1s, 2s, 4s, 8s, 16s, 32s}
     */
    CLK_CON0 |= BIT(9);
    CRC1_REG = 0x6EA5;
    WDT_CON = BIT(6) | BIT(4) | 0x0c;
    CRC1_REG = 0;
}

__attribute__((noinline))
void clr_wdt()
{
    WDT_CON |= BIT(6);
    //do st here

}

void close_wdt()
{
    CRC1_REG = 0x6EA5;
    WDT_CON &= ~BIT(4);
    CRC1_REG = 0;
}


/*
 *lev: 0--->2.2v
 *lev: 1--->2.3v
 *lev: 2--->2.4v
 *lev: 3--->2.5v
 *lev: 4--->2.6v
 *lev: 5--->2.7v
 *lev: 6--->2.8v
 *lev: 7--->2.9v
 */
void lvd_cfg(u8 lev)
{
    LVD_CON = 0;//先关闭,再配置

    LVD_CON |= (0x7 & (lev));//lev
    //LVD_CON &= ~BIT(7);//0:force reset at onece   1:force reset delay after 40us
    LVD_CON |= BIT(7);//0:force reset at onece   1:force reset delay after 40us
    delay(10);
    LVD_CON &= ~BIT(6);//force reset system
    delay(10);
    LVD_CON &= ~BIT(5);//ldo_in
    delay(10);
    LVD_CON |= BIT(4);//AEN
    delay(100);
    LVD_CON |= BIT(3);//AOE
    delay(10);
}


u32 sdfile_init(u32 cmd_zone_addr, u32 *head_addr, int num);

/*
 * 此函数在cpu0上电后首先被调用,负责初始化cpu内部模块
 *
 * 此函数返回后，操作系统才开始初始化并运行
 *
 */

void setup_arch()
{
    wdt_init();

    clk_early_init();

    interrupt_init();

#ifdef CONFIG_DEBUG_ENABLE
    debug_uart_init();
#endif
    log_early_init();

    puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("\n         setup_arch %s %s", __DATE__, __TIME__);
    puts("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    rtc_early_init();

    power_check_once();

    cpu_xbus_init();

//    debug_init();

    /*sdr_write_range_limit((void *)NO_CACHE_ADDR(&text_rodata_begin),
                          (u32)&text_rodata_end - (u32)&text_rodata_begin, false, 0);*/

    /*sdfile_init(boot.cmd_zone_addr, boot.sdfile_head_addr, 2);*/

    /*
     * 内部ldo设为1.1v
     */
#ifndef CONFIG_BOARD_DEVELOP_20170411
    dvdd_ctrl(DVDD_0904);
    /* ddrvdd_ctrl(DDRVDD_1596_2300, 0, 0); *///ddrvdd uboot用于控制ddr的供电使能，此处不可操作
    lvd_cfg(0);
#endif

    request_irq(1, 7, exception_irq_handler, 0);

}








