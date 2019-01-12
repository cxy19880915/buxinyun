#ifndef CPU_SYS_INFO_H
#define CPU_SYS_INFO_H

#include "typedef.h"


struct boot_sysinfo {
    u32 cmd_zone_addr;
    u32 sdfile_head_addr[2];

    u32 uart: 5;
    u32 res: 7;
    u32 spi_clk_div: 8;
    u32 spi_cs_deselect: 4;
    u32 spi_input_delay: 4;
    u32 spi_data_width: 2;
    u32 spi_is_continue_read: 1;
    u32 spi_is_output: 1;

    u32 sys_clk;
    u32 osc_freq;
    u32 osc_type: 2;
    u32 osc_hc_en: 1;
    u32 osc_1pin_en: 1;

    u32 sdram_clk;
    u32 sdram_size;
    u32 sdram_refresh_time;
    u32 sdram_refresh_cycles;

    u32 sdram_mode: 1;
    u32 sdram_cl: 3;
    u32 sdram_colum: 4;
    u32 sdram_trfc: 7;
    u32 sdram_trp: 7;
    u32 sdram_trcd: 7;
    u32 sdram_wlcnt: 3;

    u32 sdram_trrd : 4;
    u32 sdram_wtr : 4;
    u32 sdram_rtw : 4;
    u32 sdram_wr  : 4;
    u32 sdram_wl  : 4;
    u32 sdram_rl  : 4;
    u32 sdram_rc  : 4;
    u32 sdram_rlcnt: 4;

    u32 sdram_d_dly: 1;
    u32 sdram_q_dly: 1;
    u32 sdram_ref_clk: 30;

    u32 eva_clk;
    u32 cfg_crc;
};




#endif
