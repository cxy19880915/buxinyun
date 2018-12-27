
/*--------------------------------------------------------------------------*/
/**@file     dv16.h
   @brief    芯片资源头文件
   @details
   @author
   @date    2016-6-14
   @note    DV16
*/
/*----------------------------------------------------------------------------*/

#ifndef _DV16_
#define _DV16_

//Note:
//there are 256 words(1024 bytes) in the sfr space
//byte(8bit)       SFR offset-address is:   0x03, 0x07, 0x0b, 0x0f, 0x13 ......
//half-word(16bit) SFR offset-address is:   0x02, 0x06, 0x0a, 0x0e, 0x12 ......
//word(24/32bit)   SFR offset-address is:   0x00, 0x04, 0x08, 0x0c, 0x10 ......

#define hs_base     0x3f90000
#define ls_base     0x3f80000

//===============================================================================//
//
//      high speed sfr address define
//
//===============================================================================//
#define ls_io_base    (ls_base + 0x000*4)
#define ls_uart_base  (ls_base + 0x100*4)
#define ls_spi_base   (ls_base + 0x200*4)
#define ls_sd_base    (ls_base + 0x300*4)
#define ls_tmr_base   (ls_base + 0x400*4)
#define ls_fusb_base  (ls_base + 0x500*4)
#define ls_husb_base  (ls_base + 0x600*4)
#define ls_adda_base  (ls_base + 0x700*4)
#define ls_clk_base   (ls_base + 0x800*4)
#define ls_oth_base   (ls_base + 0x900*4)
#define ls_alnk_base  (ls_base + 0xa00*4)
#define ls_pwm_base   (ls_base + 0xb00*4)

#define hs_sdr_base   (hs_base + 0x000*4)
#define hs_eva_base   (hs_base + 0x100*4)
#define hs_cpu_base   (hs_base + 0x200*4)
#define hs_sfc_base   (hs_base + 0x300*4)
#define hs_jpg_base   (hs_base + 0x400*4)
#define hs_oth_base   (hs_base + 0x500*4)
#define hs_dbg_base   (hs_base + 0x600*4)
#define hs_jive_base  (hs_base + 0x800*4)
#define hs_jlmd_base  (hs_base + 0x900*4)

#define DBG_CON0                (*(volatile u32 *)(hs_dbg_base + 0x00*4))       // for debug only

#define DEBUG_WR_EN             (*(volatile u32 *)(hs_dbg_base + 0x02*4))       // for debug only
#define C0_PC_LIMH              (*(volatile u32 *)(hs_dbg_base + 0x03*4))       // for debug only
#define C0_PC_LIML              (*(volatile u32 *)(hs_dbg_base + 0x04*4))       // for debug only
#define C1_PC_LIMH              (*(volatile u32 *)(hs_dbg_base + 0x05*4))       // for debug only
#define C1_PC_LIML              (*(volatile u32 *)(hs_dbg_base + 0x06*4))       // for debug only
#define WR_LIM0H                (*(volatile u32 *)(hs_dbg_base + 0x07*4))       // for debug only
#define WR_LIM0L                (*(volatile u32 *)(hs_dbg_base + 0x08*4))       // for debug only
#define WR_LIM1H                (*(volatile u32 *)(hs_dbg_base + 0x09*4))       // for debug only
#define WR_LIM1L                (*(volatile u32 *)(hs_dbg_base + 0x0a*4))       // for debug only
#define WR_LIM2H                (*(volatile u32 *)(hs_dbg_base + 0x0b*4))       // for debug only
#define WR_LIM2L                (*(volatile u32 *)(hs_dbg_base + 0x0c*4))       // for debug only
#define WR_LIM3H                (*(volatile u32 *)(hs_dbg_base + 0x0d*4))       // for debug only
#define WR_LIM3L                (*(volatile u32 *)(hs_dbg_base + 0x0e*4))       // for debug only
#define WR_LIM4H                (*(volatile u32 *)(hs_dbg_base + 0x0f*4))       // for debug only
#define WR_LIM4L                (*(volatile u32 *)(hs_dbg_base + 0x10*4))       // for debug only
#define WR_LIM5H                (*(volatile u32 *)(hs_dbg_base + 0x11*4))       // for debug only
#define WR_LIM5L                (*(volatile u32 *)(hs_dbg_base + 0x12*4))       // for debug only
#define WR_LIM6H                (*(volatile u32 *)(hs_dbg_base + 0x13*4))       // for debug only
#define WR_LIM6L                (*(volatile u32 *)(hs_dbg_base + 0x14*4))       // for debug only
#define WR_LIM7H                (*(volatile u32 *)(hs_dbg_base + 0x15*4))       // for debug only
#define WR_LIM7L                (*(volatile u32 *)(hs_dbg_base + 0x16*4))       // for debug only

#define PRP_ALLOW_NUM0          (*(volatile u32 *)(hs_dbg_base + 0x18*4))       // for debug only
#define PRP_ALLOW_NUM1          (*(volatile u32 *)(hs_dbg_base + 0x19*4))       // for debug only
#define DEBUG_MSG               (*(volatile u32 *)(hs_dbg_base + 0x20*4))       // for debug only
#define DEBUG_MSG_CLR           (*(volatile u32 *)(hs_dbg_base + 0x21*4))       // for debug only
#define DEBUG_PRP_NUM           (*(volatile u32 *)(hs_dbg_base + 0x22*4))       // for debug only

#define FPGA_TRI                (*(volatile u32 *)(hs_dbg_base + 0xff*4))

#define IOMC0                   (*(volatile u32 *)(ls_io_base + 0x70*4))         //
#define IOMC1                   (*(volatile u32 *)(ls_io_base + 0x71*4))         //
#define IOMC2                   (*(volatile u32 *)(ls_io_base + 0x72*4))         //
#define IOMC3                   (*(volatile u32 *)(ls_io_base + 0x73*4))         //
#define WKUP_CON0               (*(volatile u32 *)(ls_io_base + 0x74*4))         //
#define WKUP_CON1               (*(volatile u32 *)(ls_io_base + 0x75*4))         //
#define WKUP_CON2               (*(volatile u32 *)(ls_io_base + 0x76*4))         //write only;
#define WKUP_CON3               (*(volatile u32 *)(ls_io_base + 0x77*4))         //


#define UT0_HRXCNT              (*(volatile u32 *)(ls_uart_base + 0x00*4))
#define UT0_OTCNT               (*(volatile u32 *)(ls_uart_base + 0x01*4))
#define UT0_TXADR               (*(volatile u32 *)(ls_uart_base + 0x02*4))       //26bit write only;
#define UT0_TXCNT               (*(volatile u32 *)(ls_uart_base + 0x03*4))
#define UT0_RXEADR              (*(volatile u32 *)(ls_uart_base + 0x04*4))       //26bit write only;
#define UT0_CON                 (*(volatile u32 *)(ls_uart_base + 0x05*4))
#define UT0_BUF                 (*(volatile u8  *)(ls_uart_base + 0x06*4))
#define UT0_BAUD                (*(volatile u32 *)(ls_uart_base + 0x07*4))       //16bit write only;
#define UT0_RXSADR              (*(volatile u32 *)(ls_uart_base + 0x08*4))       //26bit write only;
#define UT0_RXCNT               (*(volatile u32 *)(ls_uart_base + 0x09*4))

#define UT1_HRXCNT              (*(volatile u32 *)(ls_uart_base + 0x10*4))
#define UT1_OTCNT               (*(volatile u32 *)(ls_uart_base + 0x11*4))
#define UT1_TXADR               (*(volatile u32 *)(ls_uart_base + 0x12*4))       //26bit write only;
#define UT1_TXCNT               (*(volatile u32 *)(ls_uart_base + 0x13*4))
#define UT1_RXEADR              (*(volatile u32 *)(ls_uart_base + 0x14*4))       //26bit write only;
#define UT1_CON                 (*(volatile u32 *)(ls_uart_base + 0x15*4))
#define UT1_BUF                 (*(volatile u8  *)(ls_uart_base + 0x16*4))
#define UT1_BAUD                (*(volatile u32 *)(ls_uart_base + 0x17*4))       //16bit write only;
#define UT1_RXSADR              (*(volatile u32 *)(ls_uart_base + 0x18*4))       //26bit write only;
#define UT1_RXCNT               (*(volatile u32 *)(ls_uart_base + 0x19*4))

#define UT2_CON                 (*(volatile u32 *)(ls_uart_base + 0x20*4))
#define UT2_BUF                 (*(volatile u8  *)(ls_uart_base + 0x21*4))
#define UT2_BAUD                (*(volatile u32 *)(ls_uart_base + 0x22*4))       //write only;
#define UT3_HRXCNT              (*(volatile u32 *)(ls_uart_base + 0x30*4))
#define UT3_OTCNT               (*(volatile u32 *)(ls_uart_base + 0x31*4))
#define UT3_TXADR               (*(volatile u32 *)(ls_uart_base + 0x32*4))       //26bit write only;
#define UT3_TXCNT               (*(volatile u32 *)(ls_uart_base + 0x33*4))
#define UT3_RXEADR              (*(volatile u32 *)(ls_uart_base + 0x34*4))       //26bit write only;
#define UT3_CON                 (*(volatile u32 *)(ls_uart_base + 0x35*4))
#define UT3_BUF                 (*(volatile u8  *)(ls_uart_base + 0x36*4))
#define UT3_BAUD                (*(volatile u32 *)(ls_uart_base + 0x37*4))       //16bit write only;
#define UT3_RXSADR              (*(volatile u32 *)(ls_uart_base + 0x38*4))       //26bit write only;
#define UT3_RXCNT               (*(volatile u32 *)(ls_uart_base + 0x39*4))


//SPI
#define SPI0_CON                (*(volatile u32 *)(ls_spi_base + 0x00*4))
#define SPI0_BAUD               (*(volatile u8  *)(ls_spi_base + 0x01*4))
#define SPI0_BUF                (*(volatile u8  *)(ls_spi_base + 0x02*4))
#define SPI0_ADR                (*(volatile u32 *)(ls_spi_base + 0x03*4))       //26bit write only;
#define SPI0_CNT                (*(volatile u32 *)(ls_spi_base + 0x04*4))       //write only;

#define SPI1_CON                (*(volatile u32 *)(ls_spi_base + 0x10*4))
#define SPI1_BAUD               (*(volatile u8  *)(ls_spi_base + 0x11*4))
#define SPI1_BUF                (*(volatile u8  *)(ls_spi_base + 0x12*4))
#define SPI1_ADR                (*(volatile u32 *)(ls_spi_base + 0x13*4))       //26bit write only;
#define SPI1_CNT                (*(volatile u32 *)(ls_spi_base + 0x14*4))       //write only;

#define SPI2_CON                (*(volatile u32 *)(ls_spi_base + 0x20*4))
#define SPI2_BAUD               (*(volatile u8  *)(ls_spi_base + 0x21*4))
#define SPI2_BUF                (*(volatile u8  *)(ls_spi_base + 0x22*4))
#define SPI2_ADR                (*(volatile u32 *)(ls_spi_base + 0x23*4))       //26bit write only;
#define SPI2_CNT                (*(volatile u32 *)(ls_spi_base + 0x24*4))       //write only;

//SD
#define SD0_CON0                (*(volatile u32 *)(ls_sd_base + 0x00*4))
#define SD0_CON1                (*(volatile u32 *)(ls_sd_base + 0x01*4))
#define SD0_CON2                (*(volatile u32 *)(ls_sd_base + 0x02*4))
#define SD0_CPTR                (*(volatile u32 *)(ls_sd_base + 0x03*4))        //26bit write only;
#define SD0_DPTR                (*(volatile u32 *)(ls_sd_base + 0x04*4))        //26bit write only;
#define SD0_CTU_CON             (*(volatile u32 *)(ls_sd_base + 0x05*4))
#define SD0_CTU_CNT             (*(volatile u32 *)(ls_sd_base + 0x06*4))

#define SD1_CON0                (*(volatile u32 *)(ls_sd_base + 0x10*4))
#define SD1_CON1                (*(volatile u32 *)(ls_sd_base + 0x11*4))
#define SD1_CON2                (*(volatile u32 *)(ls_sd_base + 0x12*4))
#define SD1_CPTR                (*(volatile u32 *)(ls_sd_base + 0x13*4))        //nbit
#define SD1_DPTR                (*(volatile u32 *)(ls_sd_base + 0x14*4))        //nbit
#define SD1_CTU_CON             (*(volatile u32 *)(ls_sd_base + 0x15*4))
#define SD1_CTU_CNT             (*(volatile u32 *)(ls_sd_base + 0x16*4))

#define SD2_CON0                (*(volatile u32 *)(ls_sd_base + 0x20*4))
#define SD2_CON1                (*(volatile u32 *)(ls_sd_base + 0x21*4))
#define SD2_CON2                (*(volatile u32 *)(ls_sd_base + 0x22*4))
#define SD2_CPTR                (*(volatile u32 *)(ls_sd_base + 0x23*4))        //nbit
#define SD2_DPTR                (*(volatile u32 *)(ls_sd_base + 0x24*4))        //nbit
#define SD2_CTU_CON             (*(volatile u32 *)(ls_sd_base + 0x25*4))
#define SD2_CTU_CNT             (*(volatile u32 *)(ls_sd_base + 0x26*4))

//TIMER

#define T0_CON                  (*(volatile u32 *)(ls_tmr_base + 0x00*4))
#define T0_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x01*4))
#define T0_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x02*4))
#define T0_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x03*4))

#define T1_CON                  (*(volatile u32 *)(ls_tmr_base + 0x10*4))
#define T1_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x11*4))
#define T1_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x12*4))
#define T1_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x13*4))

#define T2_CON                  (*(volatile u32 *)(ls_tmr_base + 0x20*4))
#define T2_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x21*4))
#define T2_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x22*4))
#define T2_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x23*4))

#define T3_CON                  (*(volatile u32 *)(ls_tmr_base + 0x30*4))
#define T3_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x31*4))
#define T3_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x32*4))
#define T3_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x33*4))

#define T4_CON                  (*(volatile u32 *)(ls_tmr_base + 0x40*4))
#define T4_CNT                  (*(volatile u32 *)(ls_tmr_base + 0x41*4))
#define T4_PRD                  (*(volatile u32 *)(ls_tmr_base + 0x42*4))
#define T4_PWM                  (*(volatile u32 *)(ls_tmr_base + 0x43*4))

//PWM
#define PWMTMR0CON              (*(volatile u32 *)(ls_pwm_base + 0x00*4))
#define PWMTMR0CNT              (*(volatile u32 *)(ls_pwm_base + 0x01*4))
#define PWMTMR0PR               (*(volatile u32 *)(ls_pwm_base + 0x02*4))
#define PWMCMP0                 (*(volatile u32 *)(ls_pwm_base + 0x03*4))
#define PWMTMR1CON              (*(volatile u32 *)(ls_pwm_base + 0x04*4))
#define PWMTMR1CNT              (*(volatile u32 *)(ls_pwm_base + 0x05*4))
#define PWMTMR1PR               (*(volatile u32 *)(ls_pwm_base + 0x06*4))
#define PWMCMP1                 (*(volatile u32 *)(ls_pwm_base + 0x07*4))
#define PWMTMR2CON              (*(volatile u32 *)(ls_pwm_base + 0x08*4))
#define PWMTMR2CNT              (*(volatile u32 *)(ls_pwm_base + 0x09*4))
#define PWMTMR2PR               (*(volatile u32 *)(ls_pwm_base + 0x0a*4))
#define PWMCMP2                 (*(volatile u32 *)(ls_pwm_base + 0x0b*4))
#define PWMTMR3CON              (*(volatile u32 *)(ls_pwm_base + 0x0c*4))
#define PWMTMR3CNT              (*(volatile u32 *)(ls_pwm_base + 0x0d*4))
#define PWMTMR3PR               (*(volatile u32 *)(ls_pwm_base + 0x0e*4))
#define PWMCMP3                 (*(volatile u32 *)(ls_pwm_base + 0x0f*4))
#define PWMTMR4CON              (*(volatile u32 *)(ls_pwm_base + 0x10*4))
#define PWMTMR4CNT              (*(volatile u32 *)(ls_pwm_base + 0x11*4))
#define PWMTMR4PR               (*(volatile u32 *)(ls_pwm_base + 0x12*4))
#define PWMCMP4                 (*(volatile u32 *)(ls_pwm_base + 0x13*4))
#define PWMTMR5CON              (*(volatile u32 *)(ls_pwm_base + 0x14*4))
#define PWMTMR5CNT              (*(volatile u32 *)(ls_pwm_base + 0x15*4))
#define PWMTMR5PR               (*(volatile u32 *)(ls_pwm_base + 0x16*4))
#define PWMCMP5                 (*(volatile u32 *)(ls_pwm_base + 0x17*4))
#define PWMCON0                 (*(volatile u32 *)(ls_pwm_base + 0x18*4))
#define PWMCON1                 (*(volatile u32 *)(ls_pwm_base + 0x19*4))

//FUSB
#define FUSB_CON0               (*(volatile u32 *)(ls_fusb_base + 0x00*4))
#define FUSB_CON1               (*(volatile u32 *)(ls_fusb_base + 0x01*4))
#define FUSB_EP0_CNT            (*(volatile u32 *)(ls_fusb_base + 0x02*4))      //write only;
#define FUSB_EP1_CNT            (*(volatile u32 *)(ls_fusb_base + 0x03*4))      //write only;
#define FUSB_EP2_CNT            (*(volatile u32 *)(ls_fusb_base + 0x04*4))      //write only;
#define FUSB_EP3_CNT            (*(volatile u32 *)(ls_fusb_base + 0x05*4))      //write only;
#define FUSB_EP0_ADR            (*(volatile u32 *)(ls_fusb_base + 0x06*4))      //26bit write only;
#define FUSB_EP1_TADR           (*(volatile u32 *)(ls_fusb_base + 0x07*4))      //write only;
#define FUSB_EP1_RADR           (*(volatile u32 *)(ls_fusb_base + 0x08*4))      //write only;
#define FUSB_EP2_TADR           (*(volatile u32 *)(ls_fusb_base + 0x09*4))      //write only;
#define FUSB_EP2_RADR           (*(volatile u32 *)(ls_fusb_base + 0x0a*4))      //write only;
#define FUSB_EP3_TADR           (*(volatile u32 *)(ls_fusb_base + 0x0b*4))      //write only;
#define FUSB_EP3_RADR           (*(volatile u32 *)(ls_fusb_base + 0x0c*4))      //write only;
#define FUSB_IO_CON0            (*(volatile u32 *)(ls_fusb_base + 0x0d*4))      //

#define HUSB0_SIE_CON            (*(volatile u32 *)(ls_husb_base + 0x00*4))
#define HUSB0_EP0_CNT            (*(volatile u32 *)(ls_husb_base + 0x01*4))      //write only;
#define HUSB0_EP1_CNT            (*(volatile u32 *)(ls_husb_base + 0x02*4))      //write only;
#define HUSB0_EP2_CNT            (*(volatile u32 *)(ls_husb_base + 0x03*4))      //write only;
#define HUSB0_EP3_CNT            (*(volatile u32 *)(ls_husb_base + 0x04*4))      //write only;
#define HUSB0_EP4_CNT            (*(volatile u32 *)(ls_husb_base + 0x05*4))      //write only;
#define HUSB0_EP5_CNT            (*(volatile u32 *)(ls_husb_base + 0x06*4))      //write only;
#define HUSB0_EP6_CNT            (*(volatile u32 *)(ls_husb_base + 0x07*4))      //write only;
#define HUSB0_EP0_ADR            (*(volatile u32 *)(ls_husb_base + 0x08*4))      //26bit write only;
#define HUSB0_EP1_TADR           (*(volatile u32 *)(ls_husb_base + 0x09*4))      //26bit write only;
#define HUSB0_EP1_RADR           (*(volatile u32 *)(ls_husb_base + 0x0a*4))      //26bit write only;
#define HUSB0_EP2_TADR           (*(volatile u32 *)(ls_husb_base + 0x0b*4))      //26bit write only;
#define HUSB0_EP2_RADR           (*(volatile u32 *)(ls_husb_base + 0x0c*4))      //26bit write only;
#define HUSB0_EP3_TADR           (*(volatile u32 *)(ls_husb_base + 0x0d*4))      //26bit write only;
#define HUSB0_EP3_RADR           (*(volatile u32 *)(ls_husb_base + 0x0e*4))      //26bit write only;
#define HUSB0_EP4_TADR           (*(volatile u32 *)(ls_husb_base + 0x0f*4))      //26bit write only;
#define HUSB0_EP4_RADR           (*(volatile u32 *)(ls_husb_base + 0x10*4))      //26bit write only;
#define HUSB0_EP5_TADR           (*(volatile u32 *)(ls_husb_base + 0x11*4))      //26bit write only;
#define HUSB0_EP5_RADR           (*(volatile u32 *)(ls_husb_base + 0x12*4))      //26bit write only;
#define HUSB0_EP6_TADR           (*(volatile u32 *)(ls_husb_base + 0x13*4))      //26bit write only;
#define HUSB0_EP6_RADR           (*(volatile u32 *)(ls_husb_base + 0x14*4))      //26bit write only;

#define HUSB_COM_CON0            (*(volatile u32 *)(ls_husb_base + 0x15*4))
#define HUSB0_PHY_CON0           (*(volatile u32 *)(ls_husb_base + 0x16*4))
#define HUSB0_PHY_CON1           (*(volatile u32 *)(ls_husb_base + 0x17*4))
#define HUSB0_PHY_CON2           (*(volatile u32 *)(ls_husb_base + 0x18*4))
#define HUSB0_ISO_CON0           (*(volatile u32 *)(ls_husb_base + 0x19*4))
#define HUSB0_ISO_CON1           (*(volatile u32 *)(ls_husb_base + 0x1a*4))

#define HUSB1_SIE_CON            (*(volatile u32 *)(ls_husb_base + 0x20*4))
#define HUSB1_EP0_CNT            (*(volatile u32 *)(ls_husb_base + 0x21*4))      //write only;
#define HUSB1_EP1_CNT            (*(volatile u32 *)(ls_husb_base + 0x22*4))      //write only;
#define HUSB1_EP2_CNT            (*(volatile u32 *)(ls_husb_base + 0x23*4))      //write only;
#define HUSB1_EP3_CNT            (*(volatile u32 *)(ls_husb_base + 0x24*4))      //write only;
#define HUSB1_EP4_CNT            (*(volatile u32 *)(ls_husb_base + 0x25*4))      //write only;
#define HUSB1_EP5_CNT            (*(volatile u32 *)(ls_husb_base + 0x26*4))      //write only;
#define HUSB1_EP6_CNT            (*(volatile u32 *)(ls_husb_base + 0x27*4))      //write only;
#define HUSB1_EP0_ADR            (*(volatile u32 *)(ls_husb_base + 0x28*4))      //26bit write only;
#define HUSB1_EP1_TADR           (*(volatile u32 *)(ls_husb_base + 0x29*4))      //26bit write only;
#define HUSB1_EP1_RADR           (*(volatile u32 *)(ls_husb_base + 0x2a*4))      //26bit write only;
#define HUSB1_EP2_TADR           (*(volatile u32 *)(ls_husb_base + 0x2b*4))      //26bit write only;
#define HUSB1_EP2_RADR           (*(volatile u32 *)(ls_husb_base + 0x2c*4))      //26bit write only;
#define HUSB1_EP3_TADR           (*(volatile u32 *)(ls_husb_base + 0x2d*4))      //26bit write only;
#define HUSB1_EP3_RADR           (*(volatile u32 *)(ls_husb_base + 0x2e*4))      //26bit write only;
#define HUSB1_EP4_TADR           (*(volatile u32 *)(ls_husb_base + 0x2f*4))      //26bit write only;
#define HUSB1_EP4_RADR           (*(volatile u32 *)(ls_husb_base + 0x30*4))      //26bit write only;
#define HUSB1_EP5_TADR           (*(volatile u32 *)(ls_husb_base + 0x31*4))      //26bit write only;
#define HUSB1_EP5_RADR           (*(volatile u32 *)(ls_husb_base + 0x32*4))      //26bit write only;
#define HUSB1_EP6_TADR           (*(volatile u32 *)(ls_husb_base + 0x33*4))      //26bit write only;
#define HUSB1_EP6_RADR           (*(volatile u32 *)(ls_husb_base + 0x34*4))      //26bit write only;

#define HUSB_COM_CON1            (*(volatile u32 *)(ls_husb_base + 0x35*4))
#define HUSB1_PHY_CON0           (*(volatile u32 *)(ls_husb_base + 0x36*4))
#define HUSB1_PHY_CON1           (*(volatile u32 *)(ls_husb_base + 0x37*4))
#define HUSB1_PHY_CON2           (*(volatile u32 *)(ls_husb_base + 0x38*4))
#define HUSB1_ISO_CON0           (*(volatile u32 *)(ls_husb_base + 0x39*4))
#define HUSB1_ISO_CON1           (*(volatile u32 *)(ls_husb_base + 0x3a*4))

#define DAC_LEN                 (*(volatile u32 *)(ls_adda_base + 0x00*4))      //16bit write only;
#define DAC_CON                 (*(volatile u32 *)(ls_adda_base + 0x01*4))
#define DAC_ADR                 (*(volatile u32 *)(ls_adda_base + 0x02*4))      //26bit write only;
#define DAC_TRML                (*(volatile u8  *)(ls_adda_base + 0x03*4))      //8bit write only;
#define DAC_TRMR                (*(volatile u8  *)(ls_adda_base + 0x04*4))      //8bit write only;

#define ADC_CON                 (*(volatile u32 *)(ls_adda_base + 0x08*4))
#define ADC_ADR                 (*(volatile u32 *)(ls_adda_base + 0x0b*4))      //26bit write only;
#define ADC_LEN                 (*(volatile u32 *)(ls_adda_base + 0x0c*4))      //16bit write only;
#define ADC_RES12               (*(volatile u32 *)(ls_adda_base + 0x0d*4))      //12bit read only;

#define DAA_CON0                (*(volatile u32 *)(ls_adda_base + 0x10*4))
#define DAA_CON1                (*(volatile u32 *)(ls_adda_base + 0x11*4))
#define DAA_CON2                (*(volatile u32 *)(ls_adda_base + 0x12*4))
#define DAA_CON3                (*(volatile u32 *)(ls_adda_base + 0x13*4))      //15bit;
#define DAA_CON4                (*(volatile u32 *)(ls_adda_base + 0x14*4))      //15bit;
#define DAA_CON5                (*(volatile u32 *)(ls_adda_base + 0x15*4))      //15bit;

#define ADA_CON0                (*(volatile u32 *)(ls_adda_base + 0x20*4))      //15bit;
#define ADA_CON1                (*(volatile u32 *)(ls_adda_base + 0x21*4))      //15bit;
#define ADA_CON2                (*(volatile u32 *)(ls_adda_base + 0x22*4))      //15bit;

#define GPADC_CON               (*(volatile u32 *)(ls_adda_base + 0x30*4))
#define GPADC_RES               (*(volatile u32 *)(ls_adda_base + 0x31*4))      //10bit read only;

#define ALNK_CON0               (*(volatile u32 *)(ls_alnk_base + 0x00*4))
#define ALNK_CON1               (*(volatile u32 *)(ls_alnk_base + 0x01*4))
#define ALNK_CON2               (*(volatile u32 *)(ls_alnk_base + 0x02*4))
#define ALNK_ADR0               (*(volatile u32 *)(ls_alnk_base + 0x03*4))
#define ALNK_ADR1               (*(volatile u32 *)(ls_alnk_base + 0x04*4))
#define ALNK_ADR2               (*(volatile u32 *)(ls_alnk_base + 0x05*4))
#define ALNK_ADR3               (*(volatile u32 *)(ls_alnk_base + 0x06*4))
#define ALNK_LEN                (*(volatile u32 *)(ls_alnk_base + 0x07*4))


#define PWR_CON                 (*(volatile u8  *)(ls_clk_base + 0x00*4))
#define CLK_CON0                (*(volatile u32 *)(ls_clk_base + 0x01*4))
#define CLK_CON1                (*(volatile u32 *)(ls_clk_base + 0x02*4))
#define CLK_CON2                (*(volatile u32 *)(ls_clk_base + 0x03*4))
#define LCLK_GAT                (*(volatile u32 *)(ls_clk_base + 0x04*4))
#define HCLK_GAT                (*(volatile u32 *)(ls_clk_base + 0x05*4))
#define ACLK_GAT                (*(volatile u32 *)(ls_clk_base + 0x06*4))
#define PLL0_NF                 (*(volatile u32 *)(ls_clk_base + 0x07*4))
#define PLL0_NR                 (*(volatile u32 *)(ls_clk_base + 0x08*4))
#define PLL1_NF                 (*(volatile u32 *)(ls_clk_base + 0x09*4))
#define PLL1_NR                 (*(volatile u32 *)(ls_clk_base + 0x0a*4))
#define OSA_CON                 (*(volatile u32 *)(ls_clk_base + 0x0b*4))
#define PLL_CON0                (*(volatile u32 *)(ls_clk_base + 0x0c*4))
#define PLL_CON1                (*(volatile u32 *)(ls_clk_base + 0x0d*4))
#define PLL_CON2                (*(volatile u32 *)(ls_clk_base + 0x0e*4))
#define PLL3_NF                 (*(volatile u32 *)(ls_clk_base + 0x0f*4))
#define PLL3_NR                 (*(volatile u32 *)(ls_clk_base + 0x10*4))

#define HTC_CON                 (*(volatile u32 *)(ls_oth_base + 0x00*4))
#define LDO_CON                 (*(volatile u32 *)(ls_oth_base + 0x01*4))
#define LVD_CON                 (*(volatile u32 *)(ls_oth_base + 0x02*4))
#define IRTC_CON                (*(volatile u32 *)(ls_oth_base + 0x03*4))
#define IRTC_BUF                (*(volatile u8  *)(ls_oth_base + 0x04*4))
#define MODE_CON                (*(volatile u8  *)(ls_oth_base + 0x05*4))
#define CRC0_FIFO               (*(volatile u8  *)(ls_oth_base + 0x06*4))       //write only;
#define CRC0_REG                (*(volatile u32 *)(ls_oth_base + 0x07*4))
#define WDT_CON                 (*(volatile u8  *)(ls_oth_base + 0x08*4))
#define CHIP_ID                 (*(volatile u8  *)(ls_oth_base + 0x09*4))       //read only;
#define IRFLT_CON               (*(volatile u8  *)(ls_oth_base + 0x0a*4))
#define IIC0_CON                (*(volatile u32 *)(ls_oth_base + 0x0b*4))
#define IIC0_BUF                (*(volatile u8  *)(ls_oth_base + 0x0c*4))
#define IIC0_BAUD               (*(volatile u8  *)(ls_oth_base + 0x0d*4))
#define IIC0_DMA_ADR            (*(volatile u32 *)(ls_oth_base + 0x0e*4))       //write only
#define IIC0_DMA_CNT            (*(volatile u32 *)(ls_oth_base + 0x0f*4))       //write only
#define IIC0_DMA_NRATE          (*(volatile u32 *)(ls_oth_base + 0x10*4))       //write only
#define IIC1_CON                (*(volatile u32 *)(ls_oth_base + 0x11*4))
#define IIC1_BUF                (*(volatile u8  *)(ls_oth_base + 0x12*4))
#define IIC1_BAUD               (*(volatile u8  *)(ls_oth_base + 0x13*4))
#define IIC1_DMA_ADR            (*(volatile u32 *)(ls_oth_base + 0x14*4))       //write only
#define IIC1_DMA_CNT            (*(volatile u32 *)(ls_oth_base + 0x15*4))       //write only
#define IIC1_DMA_NRATE          (*(volatile u32 *)(ls_oth_base + 0x16*4))       //write only
#define PWM8_CON                (*(volatile u32 *)(ls_oth_base + 0x17*4))
#define PAP_CON                 (*(volatile u32 *)(ls_oth_base + 0x18*4))
#define PAP_BUF                 (*(volatile u32 *)(ls_oth_base + 0x19*4))
#define PAP_ADR                 (*(volatile u32 *)(ls_oth_base + 0x1a*4))       //26bit write only;
#define PAP_CNT                 (*(volatile u32 *)(ls_oth_base + 0x1b*4))       //write only;
#define PAP_DAT0                (*(volatile u32 *)(ls_oth_base + 0x1c*4))       //write only;
#define PAP_DAT1                (*(volatile u32 *)(ls_oth_base + 0x1d*4))       //write only;

#define EFUSE_MPW_CON           (*(volatile u32 *)(ls_oth_base + 0x1f*4))
#define EFUSE_CON               (*(volatile u32 *)(ls_oth_base + 0x20*4))
#define MPUCON                  (*(volatile u32 *)(ls_oth_base + 0x21*4))
#define MPUSTART                (*(volatile u32 *)(ls_oth_base + 0x22*4))
#define MPUEND                  (*(volatile u32 *)(ls_oth_base + 0x23*4))
#define MPUCATCH0               (*(volatile u32 *)(ls_oth_base + 0x24*4))
#define MPUCATCH1               (*(volatile u32 *)(ls_oth_base + 0x25*4))
#define PLCNTCON                (*(volatile u8  *)(ls_oth_base + 0x26*4))
#define PLCNTVL                 (*(volatile u32 *)(ls_oth_base + 0x27*4))       //read only;
#define CS_CON                  (*(volatile u32 *)(ls_oth_base + 0x28*4))
#define CS_REG                  (*(volatile u32 *)(ls_oth_base + 0x29*4))
#define CS_FIFO                 (*(volatile u32 *)(ls_oth_base + 0x2a*4))       //write only
#define CS_RADR                 (*(volatile u32 *)(ls_oth_base + 0x2b*4))       //write only
#define CS_RCNT                 (*(volatile u32 *)(ls_oth_base + 0x2c*4))       //write only
#define RAND64L                 (*(volatile u32 *)(ls_oth_base + 0x2d*4))       //read only
#define RAND64H                 (*(volatile u32 *)(ls_oth_base + 0x2e*4))       //read only
#define ETHCON                  (*(volatile u32 *)(ls_oth_base + 0x2f*4))       //read only

#define SHA1CON                 (*(volatile u32 *)(ls_oth_base + 0x30*4))
#define SHA1ADR                 (*(volatile u32 *)(ls_oth_base + 0x31*4))
#define SHA1ARES0               (*(volatile u32 *)(ls_oth_base + 0x32*4))
#define SHA1ARES1               (*(volatile u32 *)(ls_oth_base + 0x33*4))
#define SHA1ARES2               (*(volatile u32 *)(ls_oth_base + 0x34*4))
#define SHA1ARES3               (*(volatile u32 *)(ls_oth_base + 0x35*4))
#define SHA1ARES4               (*(volatile u32 *)(ls_oth_base + 0x36*4))

#define CRC1_FIFO               (*(volatile u8  *)(ls_oth_base + 0x40*4))       //write only;
#define CRC1_REG                (*(volatile u32 *)(ls_oth_base + 0x41*4))

#define RINGOSC_CON             (*(volatile u32 *)(ls_oth_base + 0x42*4))
#define MBIST_CON               (*(volatile u32 *)(ls_oth_base + 0x43*4))


//===============================================================================//
//
//      high speed sfr address define
//
//===============================================================================//
#define SDRCON0                 (*(volatile u32 *)(hs_sdr_base + 0x00*4))
//#define SDRSPTR                 (*(volatile u32 *)(hs_sdr_base + 0x01*4))
//#define SDRQPTR                 (*(volatile u32 *)(hs_sdr_base + 0x02*4))
#define SDRREFREG               (*(volatile u32 *)(hs_sdr_base + 0x03*4))       //write only
//#define SDRDMACNT               (*(volatile u32 *)(hs_sdr_base + 0x04*4))
#define SDRCON1                 (*(volatile u32 *)(hs_sdr_base + 0x05*4))       //write only
#define SDRREFSUM               (*(volatile u32 *)(hs_sdr_base + 0x06*4))       //13bit
#define SDRDBG                  (*(volatile u32 *)(hs_sdr_base + 0x07*4))       //32bit
#define SDRCON2                 (*(volatile u32 *)(hs_sdr_base + 0x08*4))       //write only 32bit
#define SDRCON3                 (*(volatile u32 *)(hs_sdr_base + 0x09*4))       //32bit
#define SDRCON4                 (*(volatile u32 *)(hs_sdr_base + 0x0a*4))       //32bit
#define SDRCON5                 (*(volatile u32 *)(hs_sdr_base + 0x0b*4))       //write only 32bit
#define SDRCON6                 (*(volatile u32 *)(hs_sdr_base + 0x0c*4))       //32bit
#define SDRCON7                 (*(volatile u32 *)(hs_sdr_base + 0x0d*4))       //32bit
#define SDRCON8                 (*(volatile u32 *)(hs_sdr_base + 0x0e*4))       //32bit
#define SDRCON9                 (*(volatile u32 *)(hs_sdr_base + 0x0f*4))       //32bit

#define UDLLCON0                (*(volatile u32 *)(hs_sdr_base + 0x10*4))       //32bit
#define UDLLCON1                (*(volatile u32 *)(hs_sdr_base + 0x11*4))       //32bit
#define LDLLCON0                (*(volatile u32 *)(hs_sdr_base + 0x12*4))       //32bit
#define LDLLCON1                (*(volatile u32 *)(hs_sdr_base + 0x13*4))       //32bit

#define C0_CON                  (*(volatile u32 *)(hs_cpu_base + 0x00*4))       //
#define C1_CON                  (*(volatile u32 *)(hs_cpu_base + 0x01*4))       //
#define CACHE_CON               (*(volatile u32 *)(hs_cpu_base + 0x02*4))       //
#define TTB_L1                  (*(volatile u32 *)(hs_cpu_base + 0x03*4))       //
#define PRP_CON                 (*(volatile u32 *)(hs_cpu_base + 0x04*4))       //
#define CACHE_WAY               (*(volatile u32 *)(hs_cpu_base + 0x05*4))       //

#define C0_IPND0                (*(volatile u32 *)(hs_cpu_base + 0x10*4))       // Read only
#define C0_IPND1                (*(volatile u32 *)(hs_cpu_base + 0x11*4))       // Read only
#define C0_IPND2                (*(volatile u32 *)(hs_cpu_base + 0x12*4))       // Read only
#define C0_IPND3                (*(volatile u32 *)(hs_cpu_base + 0x13*4))       // Read only
#define C0_IPND4                (*(volatile u32 *)(hs_cpu_base + 0x14*4))       // Read only
#define C0_IPND5                (*(volatile u32 *)(hs_cpu_base + 0x15*4))       // Read only
#define C0_IPND6                (*(volatile u32 *)(hs_cpu_base + 0x16*4))       // Read only
#define C0_IPND7                (*(volatile u32 *)(hs_cpu_base + 0x17*4))       // Read only
#define C0_ILAT_SET             (*(volatile u32 *)(hs_cpu_base + 0x18*4))       // Write only
#define C0_ILAT_CLR             (*(volatile u32 *)(hs_cpu_base + 0x19*4))       // Write only
#define C0_TTMR_CON             (*(volatile u32 *)(hs_cpu_base + 0x1a*4))
#define C0_TTMR_CNT             (*(volatile u32 *)(hs_cpu_base + 0x1b*4))
#define C0_TTMR_PRD             (*(volatile u32 *)(hs_cpu_base + 0x1c*4))

#define C0_ICFG0                (*(volatile u32 *)(hs_cpu_base + 0x20*4))
#define C0_ICFG1                (*(volatile u32 *)(hs_cpu_base + 0x21*4))
#define C0_ICFG2                (*(volatile u32 *)(hs_cpu_base + 0x22*4))
#define C0_ICFG3                (*(volatile u32 *)(hs_cpu_base + 0x23*4))
#define C0_ICFG4                (*(volatile u32 *)(hs_cpu_base + 0x24*4))
#define C0_ICFG5                (*(volatile u32 *)(hs_cpu_base + 0x25*4))
#define C0_ICFG6                (*(volatile u32 *)(hs_cpu_base + 0x26*4))
#define C0_ICFG7                (*(volatile u32 *)(hs_cpu_base + 0x27*4))
#define C0_ICFG8                (*(volatile u32 *)(hs_cpu_base + 0x28*4))
#define C0_ICFG9                (*(volatile u32 *)(hs_cpu_base + 0x29*4))
#define C0_ICFG10               (*(volatile u32 *)(hs_cpu_base + 0x2a*4))
#define C0_ICFG11               (*(volatile u32 *)(hs_cpu_base + 0x2b*4))
#define C0_ICFG12               (*(volatile u32 *)(hs_cpu_base + 0x2c*4))
#define C0_ICFG13               (*(volatile u32 *)(hs_cpu_base + 0x2d*4))
#define C0_ICFG14               (*(volatile u32 *)(hs_cpu_base + 0x2e*4))
#define C0_ICFG15               (*(volatile u32 *)(hs_cpu_base + 0x2f*4))
#define C0_ICFG16               (*(volatile u32 *)(hs_cpu_base + 0x30*4))
#define C0_ICFG17               (*(volatile u32 *)(hs_cpu_base + 0x31*4))
#define C0_ICFG18               (*(volatile u32 *)(hs_cpu_base + 0x32*4))
#define C0_ICFG19               (*(volatile u32 *)(hs_cpu_base + 0x33*4))
#define C0_ICFG20               (*(volatile u32 *)(hs_cpu_base + 0x34*4))
#define C0_ICFG21               (*(volatile u32 *)(hs_cpu_base + 0x35*4))
#define C0_ICFG22               (*(volatile u32 *)(hs_cpu_base + 0x36*4))
#define C0_ICFG23               (*(volatile u32 *)(hs_cpu_base + 0x37*4))
#define C0_ICFG24               (*(volatile u32 *)(hs_cpu_base + 0x38*4))
#define C0_ICFG25               (*(volatile u32 *)(hs_cpu_base + 0x39*4))
#define C0_ICFG26               (*(volatile u32 *)(hs_cpu_base + 0x3a*4))
#define C0_ICFG27               (*(volatile u32 *)(hs_cpu_base + 0x3b*4))
#define C0_ICFG28               (*(volatile u32 *)(hs_cpu_base + 0x3c*4))
#define C0_ICFG29               (*(volatile u32 *)(hs_cpu_base + 0x3d*4))
#define C0_ICFG30               (*(volatile u32 *)(hs_cpu_base + 0x3e*4))
#define C0_ICFG31               (*(volatile u32 *)(hs_cpu_base + 0x3f*4))

#define C1_IPND0                (*(volatile u32 *)(hs_cpu_base + 0x40*4))       // Read only
#define C1_IPND1                (*(volatile u32 *)(hs_cpu_base + 0x41*4))       // Read only
#define C1_IPND2                (*(volatile u32 *)(hs_cpu_base + 0x42*4))       // Read only
#define C1_IPND3                (*(volatile u32 *)(hs_cpu_base + 0x43*4))       // Read only
#define C1_IPND4                (*(volatile u32 *)(hs_cpu_base + 0x44*4))       // Read only
#define C1_IPND5                (*(volatile u32 *)(hs_cpu_base + 0x45*4))       // Read only
#define C1_IPND6                (*(volatile u32 *)(hs_cpu_base + 0x46*4))       // Read only
#define C1_IPND7                (*(volatile u32 *)(hs_cpu_base + 0x47*4))       // Read only
#define C1_ILAT_SET             (*(volatile u32 *)(hs_cpu_base + 0x48*4))       // Write only
#define C1_ILAT_CLR             (*(volatile u32 *)(hs_cpu_base + 0x49*4))       // Write only
#define C1_TTMR_CON             (*(volatile u32 *)(hs_cpu_base + 0x4a*4))
#define C1_TTMR_CNT             (*(volatile u32 *)(hs_cpu_base + 0x4b*4))
#define C1_TTMR_PRD             (*(volatile u32 *)(hs_cpu_base + 0x4c*4))

#define C1_ICFG0                (*(volatile u32 *)(hs_cpu_base + 0x50*4))
#define C1_ICFG1                (*(volatile u32 *)(hs_cpu_base + 0x51*4))
#define C1_ICFG2                (*(volatile u32 *)(hs_cpu_base + 0x52*4))
#define C1_ICFG3                (*(volatile u32 *)(hs_cpu_base + 0x53*4))
#define C1_ICFG4                (*(volatile u32 *)(hs_cpu_base + 0x54*4))
#define C1_ICFG5                (*(volatile u32 *)(hs_cpu_base + 0x55*4))
#define C1_ICFG6                (*(volatile u32 *)(hs_cpu_base + 0x56*4))
#define C1_ICFG7                (*(volatile u32 *)(hs_cpu_base + 0x57*4))
#define C1_ICFG8                (*(volatile u32 *)(hs_cpu_base + 0x58*4))
#define C1_ICFG9                (*(volatile u32 *)(hs_cpu_base + 0x59*4))
#define C1_ICFG10               (*(volatile u32 *)(hs_cpu_base + 0x5a*4))
#define C1_ICFG11               (*(volatile u32 *)(hs_cpu_base + 0x5b*4))
#define C1_ICFG12               (*(volatile u32 *)(hs_cpu_base + 0x5c*4))
#define C1_ICFG13               (*(volatile u32 *)(hs_cpu_base + 0x5d*4))
#define C1_ICFG14               (*(volatile u32 *)(hs_cpu_base + 0x5e*4))
#define C1_ICFG15               (*(volatile u32 *)(hs_cpu_base + 0x5f*4))
#define C1_ICFG16               (*(volatile u32 *)(hs_cpu_base + 0x60*4))
#define C1_ICFG17               (*(volatile u32 *)(hs_cpu_base + 0x61*4))
#define C1_ICFG18               (*(volatile u32 *)(hs_cpu_base + 0x62*4))
#define C1_ICFG19               (*(volatile u32 *)(hs_cpu_base + 0x63*4))
#define C1_ICFG20               (*(volatile u32 *)(hs_cpu_base + 0x64*4))
#define C1_ICFG21               (*(volatile u32 *)(hs_cpu_base + 0x65*4))
#define C1_ICFG22               (*(volatile u32 *)(hs_cpu_base + 0x66*4))
#define C1_ICFG23               (*(volatile u32 *)(hs_cpu_base + 0x67*4))
#define C1_ICFG24               (*(volatile u32 *)(hs_cpu_base + 0x68*4))
#define C1_ICFG25               (*(volatile u32 *)(hs_cpu_base + 0x69*4))
#define C1_ICFG26               (*(volatile u32 *)(hs_cpu_base + 0x6a*4))
#define C1_ICFG27               (*(volatile u32 *)(hs_cpu_base + 0x6b*4))
#define C1_ICFG28               (*(volatile u32 *)(hs_cpu_base + 0x6c*4))
#define C1_ICFG29               (*(volatile u32 *)(hs_cpu_base + 0x6d*4))
#define C1_ICFG30               (*(volatile u32 *)(hs_cpu_base + 0x6e*4))
#define C1_ICFG31               (*(volatile u32 *)(hs_cpu_base + 0x6f*4))

#define CPASS_CON               (*(volatile u32 *)(hs_cpu_base + 0x70*4))
#define CPASS_ADRH              (*(volatile u32 *)(hs_cpu_base + 0x71*4))
#define CPASS_ADRL              (*(volatile u32 *)(hs_cpu_base + 0x72*4))
#define CPASS_BUF_LAST          (*(volatile u32 *)(hs_cpu_base + 0x73*4))
#define CPASS_CPF_ADRH          (*(volatile u32 *)(hs_cpu_base + 0x74*4))
#define CPASS_CPF_ADRL          (*(volatile u32 *)(hs_cpu_base + 0x75*4))

//eva
#define EVA_CON                 (*(volatile u32 *)(hs_eva_base + 0x00*4))

//SFC
#define SFC_CON                 (*(volatile u32 *)(hs_sfc_base + 0x00*4))
#define SFC_BAUD                (*(volatile u32 *)(hs_sfc_base + 0x01*4))

#define SFC_BASE_ADR            (*(volatile u32 *)(hs_sfc_base + 0x03*4))

#define   jpg_base 0x3f94800

#define JPG0_CON0                (*(volatile u32 *)(jpg_base + 0x00*4))
#define JPG0_CON1                (*(volatile u32 *)(jpg_base + 0x01*4))
#define JPG0_CON2                (*(volatile u32 *)(jpg_base + 0x02*4))
#define JPG0_YDCVAL              (*(volatile u32 *)(jpg_base + 0x03*4))
#define JPG0_UDCVAL              (*(volatile u32 *)(jpg_base + 0x04*4))
#define JPG0_VDCVAL              (*(volatile u32 *)(jpg_base + 0x05*4))
#define JPG0_YPTR0               (*(volatile u32 *)(jpg_base + 0x06*4))
#define JPG0_UPTR0               (*(volatile u32 *)(jpg_base + 0x07*4))
#define JPG0_VPTR0               (*(volatile u32 *)(jpg_base + 0x08*4))
#define JPG0_YPTR1               (*(volatile u32 *)(jpg_base + 0x09*4))
#define JPG0_UPTR1               (*(volatile u32 *)(jpg_base + 0x0a*4))
#define JPG0_VPTR1               (*(volatile u32 *)(jpg_base + 0x0b*4))
#define JPG0_BADDR               (*(volatile u32 *)(jpg_base + 0x0c*4))
#define JPG0_BCNT                (*(volatile u32 *)(jpg_base + 0x0d*4))
#define JPG0_MCUCNT              (*(volatile u32 *)(jpg_base + 0x0e*4))
#define JPG0_PRECNT              (*(volatile u32 *)(jpg_base + 0x0f*4))
#define JPG0_YUVLINE             (*(volatile u32 *)(jpg_base + 0x10*4))
#define JPG0_CFGRAMADDR          (*(volatile u32 *)(jpg_base + 0x11*4))
#define JPG0_CFGRAMVAL           (*(volatile u32 *)(jpg_base + 0x12*4))
#define JPG0_PTR_NUM             (*(volatile u32 *)(jpg_base + 0x13*4))

//#define JPG1_CON0                (*(volatile u32 *)(hs_jpg_base + 0x40*4))
//#define JPG1_CON1                (*(volatile u32 *)(hs_jpg_base + 0x41*4))
//#define JPG1_CON2                (*(volatile u32 *)(hs_jpg_base + 0x42*4))
//#define JPG1_YDCVAL              (*(volatile u32 *)(hs_jpg_base + 0x43*4))
//#define JPG1_UDCVAL              (*(volatile u32 *)(hs_jpg_base + 0x44*4))
//#define JPG1_VDCVAL              (*(volatile u32 *)(hs_jpg_base + 0x45*4))
//#define JPG1_YPTR0               (*(volatile u32 *)(hs_jpg_base + 0x46*4))
//#define JPG1_UPTR0               (*(volatile u32 *)(hs_jpg_base + 0x47*4))
//#define JPG1_VPTR0               (*(volatile u32 *)(hs_jpg_base + 0x48*4))
//#define JPG1_YPTR1               (*(volatile u32 *)(hs_jpg_base + 0x49*4))
//#define JPG1_UPTR1               (*(volatile u32 *)(hs_jpg_base + 0x4a*4))
//#define JPG1_VPTR1               (*(volatile u32 *)(hs_jpg_base + 0x4b*4))
//#define JPG1_BADDR               (*(volatile u32 *)(hs_jpg_base + 0x4c*4))
//#define JPG1_BCNT                (*(volatile u32 *)(hs_jpg_base + 0x4d*4))
//#define JPG1_MCUCNT              (*(volatile u32 *)(hs_jpg_base + 0x4e*4))
//#define JPG1_PRECNT              (*(volatile u32 *)(hs_jpg_base + 0x4f*4))
//#define JPG1_YUVLINE             (*(volatile u32 *)(hs_jpg_base + 0x50*4))
//#define JPG1_CFGRAMADDR          (*(volatile u32 *)(hs_jpg_base + 0x51*4))
//#define JPG1_CFGRAMVAL           (*(volatile u32 *)(hs_jpg_base + 0x52*4))
////#define JPG1_CFGRAMVAL           (*(volatile u32 *)(hs_jpg_base + 0x2c00*4))
//#define JPG1_PTR_NUM             (*(volatile u32 *)(hs_jpg_base + 0x53*4))

//DMA BYTE COPY
#define   dma_copy_base 0x3f94f00
#define DMA_COPY_TASK_ADR       (*(volatile u32 *)(dma_copy_base + 0x00*4))   //write only
#define DMA_COPY_CON            (*(volatile u32 *)(dma_copy_base + 0x01*4))

//JIVE
#define JIVE_CON0                (*(volatile u32 *)(hs_jive_base + 0x00*4))
#define JIVE_POINTER             (*(volatile u32 *)(hs_jive_base + 0x01*4))
#define JIVE_SQINT_CNT           (*(volatile u32 *)(hs_jive_base + 0x02*4))

//JLMD
#define JLMD_CON0                (*(volatile u32 *)(hs_jlmd_base + 0x00*4))
#define JLMD_CON1                (*(volatile u32 *)(hs_jlmd_base + 0x01*4))
#define JLMD_CON2                (*(volatile u32 *)(hs_jlmd_base + 0x02*4))
#define JLMD_CON3                (*(volatile u32 *)(hs_jlmd_base + 0x03*4))
#define JLMD_CON4                (*(volatile u32 *)(hs_jlmd_base + 0x04*4))
#define JLMD_SRC                 (*(volatile u32 *)(hs_jlmd_base + 0x05*4))
#define JLMD_REF                 (*(volatile u32 *)(hs_jlmd_base + 0x06*4))
#define JLMD_BG                  (*(volatile u32 *)(hs_jlmd_base + 0x07*4))
#define JLMD_OBJ                 (*(volatile u32 *)(hs_jlmd_base + 0x08*4))

#define ENC_CON                 (*(volatile u32 *)(hs_oth_base + 0x00*4))
#define ENC_KEY                 (*(volatile u32 *)(hs_oth_base + 0x01*4))
#define ENC_ADR                 (*(volatile u32 *)(hs_oth_base + 0x02*4))
#define SFC_UNENC_ADRH          (*(volatile u32 *)(hs_oth_base + 0x03*4))
#define SFC_UNENC_ADRL          (*(volatile u32 *)(hs_oth_base + 0x04*4))


#define AES_CON                 (*(volatile u32 *)(hs_oth_base + 0x10*4))
#define AES_DAT0                (*(volatile u32 *)(hs_oth_base + 0x11*4))
#define AES_DAT1                (*(volatile u32 *)(hs_oth_base + 0x12*4))
#define AES_DAT2                (*(volatile u32 *)(hs_oth_base + 0x13*4))
#define AES_DAT3                (*(volatile u32 *)(hs_oth_base + 0x14*4))
#define AES_KEY                 (*(volatile u32 *)(hs_oth_base + 0x15*4))
#define AES_ENCRES0             (*(volatile uint32_t *)(hs_oth_base + 0x11*4))   //read only
#define AES_ENCRES1             (*(volatile uint32_t *)(hs_oth_base + 0x12*4))   //read only
#define AES_ENCRES2             (*(volatile uint32_t *)(hs_oth_base + 0x13*4))   //read only
#define AES_ENCRES3             (*(volatile uint32_t *)(hs_oth_base + 0x14*4))   //read only
#define AES_DECRES0             (*(volatile uint32_t *)(hs_oth_base + 0x11*4))   //read only
#define AES_DECRES1             (*(volatile uint32_t *)(hs_oth_base + 0x12*4))   //read only
#define AES_DECRES2             (*(volatile uint32_t *)(hs_oth_base + 0x13*4))   //read only
#define AES_DECRES3             (*(volatile uint32_t *)(hs_oth_base + 0x14*4))   //read only




#define GPDMA_RD_CON            (*(volatile u32 *)(hs_oth_base + 0x20*4))
#define GPDMA_RD_SPTR           (*(volatile u32 *)(hs_oth_base + 0x21*4))
#define GPDMA_RD_CNT            (*(volatile u32 *)(hs_oth_base + 0x22*4))

#define GPDMA_WR_CON            (*(volatile u32 *)(hs_oth_base + 0x30*4))
#define GPDMA_WR_SPTR           (*(volatile u32 *)(hs_oth_base + 0x31*4))
#define GPDMA_WR_CNT            (*(volatile u32 *)(hs_oth_base + 0x32*4))

//video ram
#define hs_vram_base  0x3f94e00

#define VIDEO_YBASE             (*(volatile u32 *)(hs_vram_base + 0x0*4))
#define VIDEO_UBASE             (*(volatile u32 *)(hs_vram_base + 0x1*4))
#define VIDEO_VBASE             (*(volatile u32 *)(hs_vram_base + 0x2*4))
#define VIDEO_CON               (*(volatile u32 *)(hs_vram_base + 0x3*4))
#define VIDEO_ADDR              (*(volatile u32 *)(hs_vram_base + 0x4*4))
#define VIDEO_WDAT              (*(volatile u32 *)(hs_vram_base + 0x5*4))
#define VIDEO_RDAT              (*(volatile u32 *)(hs_vram_base + 0x6*4))

//encode_select
#define ENC_SEL                 (*(volatile u32 *)(hs_oth_base + 0x50*4))

// .............AVC .......................
//#define avc_base 0x3f9d000
#define avc_base 0x3f94000

//
#define AVC_CON0                (*(volatile u32 *)(avc_base + 0x00*4))
#define AVC_V0_16ROW_CNT        (*(volatile u32 *)(avc_base + 0x01*4))
#define AVC_V1_PERMIT_CNT       (*(volatile u32 *)(avc_base + 0x02*4))
#define AVC_AFBC_UNCOMPRESS     (*(volatile u32 *)(avc_base + 0x03*4))
//

#define AVC0_BUF_NUM            (*(volatile u32 *)(avc_base + 0x04*4))
#define AVC0_CON1               (*(volatile u32 *)(avc_base + 0x05*4))
#define AVC0_DB_OFFSET          (*(volatile u32 *)(avc_base + 0x06*4))
#define AVC0_MV_RANGE           (*(volatile u32 *)(avc_base + 0x07*4))
#define AVC0_CON2               (*(volatile u32 *)(avc_base + 0x08*4))
#define AVC0_YENC_BASE_ADR0     (*(volatile u32 *)(avc_base + 0x09*4))
#define AVC0_UENC_BASE_ADR0     (*(volatile u32 *)(avc_base + 0x0a*4))
#define AVC0_VENC_BASE_ADR0     (*(volatile u32 *)(avc_base + 0x0b*4))
#define AVC0_YENC_BASE_ADR1     (*(volatile u32 *)(avc_base + 0x0c*4))
#define AVC0_UENC_BASE_ADR1     (*(volatile u32 *)(avc_base + 0x0d*4))
#define AVC0_VENC_BASE_ADR1     (*(volatile u32 *)(avc_base + 0x0e*4))
#define AVC0_LDSZA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x0f*4))
#define AVC0_LDSZB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x10*4))
#define AVC0_LDCPA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x11*4))
#define AVC0_LDCPB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x12*4))
#define AVC0_WBSZA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x13*4))
#define AVC0_WBSZB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x14*4))
#define AVC0_WBCPA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x15*4))
#define AVC0_WBCPB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x16*4))
#define AVC0_CPA_START_ADR      (*(volatile u32 *)(avc_base + 0x17*4))
#define AVC0_CPA_END_ADR        (*(volatile u32 *)(avc_base + 0x18*4))
#define AVC0_CPB_START_ADR      (*(volatile u32 *)(avc_base + 0x19*4))
#define AVC0_CPB_END_ADR        (*(volatile u32 *)(avc_base + 0x1a*4))
#define AVC0_BS_ADDR            (*(volatile u32 *)(avc_base + 0x1b*4))
#define AVC0_QP_CON0            (*(volatile u32 *)(avc_base + 0x1c*4))
#define AVC0_QP_CON1            (*(volatile u32 *)(avc_base + 0x1d*4))
#define AVC0_ROI0_XY            (*(volatile u32 *)(avc_base + 0x1e*4))
#define AVC0_ROI1_XY            (*(volatile u32 *)(avc_base + 0x1f*4))
#define AVC0_ROI2_XY            (*(volatile u32 *)(avc_base + 0x20*4))
#define AVC0_ROI3_XY            (*(volatile u32 *)(avc_base + 0x21*4))
#define AVC0_ROIX_QP            (*(volatile u32 *)(avc_base + 0x22*4))
#define AVC0_VAR_MUL16          (*(volatile u32 *)(avc_base + 0x23*4))
#define AVC0_DECIMATE           (*(volatile u32 *)(avc_base + 0x24*4))
#define AVC0_BCNT               (*(volatile u32 *)(avc_base + 0x25*4))
#define AVC0_HEADER_CON         (*(volatile u32 *)(avc_base + 0x26*4))
#define AVC0_TOTAL_MB_COST      (*(volatile u32 *)(avc_base + 0x27*4))
#define AVC0_INTER_WEIGHT       (*(volatile u32 *)(avc_base + 0x28*4))
#define AVC0_DEADZONE           (*(volatile u32 *)(avc_base + 0x29*4))
#define AVC0_HAS_BUF            (*(volatile u32 *)(avc_base + 0x2a*4))
//
#define AVC1_BUF_NUM            (*(volatile u32 *)(avc_base + 0x2b*4))
#define AVC1_CON1               (*(volatile u32 *)(avc_base + 0x2c*4))
#define AVC1_DB_OFFSET          (*(volatile u32 *)(avc_base + 0x2d*4))
#define AVC1_MV_RANGE           (*(volatile u32 *)(avc_base + 0x2e*4))
#define AVC1_CON2               (*(volatile u32 *)(avc_base + 0x2f*4))
#define AVC1_YENC_BASE_ADR0     (*(volatile u32 *)(avc_base + 0x30*4))
#define AVC1_UENC_BASE_ADR0     (*(volatile u32 *)(avc_base + 0x31*4))
#define AVC1_VENC_BASE_ADR0     (*(volatile u32 *)(avc_base + 0x32*4))
#define AVC1_YENC_BASE_ADR1     (*(volatile u32 *)(avc_base + 0x33*4))
#define AVC1_UENC_BASE_ADR1     (*(volatile u32 *)(avc_base + 0x34*4))
#define AVC1_VENC_BASE_ADR1     (*(volatile u32 *)(avc_base + 0x35*4))
#define AVC1_LDSZA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x36*4))
#define AVC1_LDSZB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x37*4))
#define AVC1_LDCPA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x38*4))
#define AVC1_LDCPB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x39*4))
#define AVC1_WBSZA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x3a*4))
#define AVC1_WBSZB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x3b*4))
#define AVC1_WBCPA_BASE_ADR     (*(volatile u32 *)(avc_base + 0x3c*4))
#define AVC1_WBCPB_BASE_ADR     (*(volatile u32 *)(avc_base + 0x3d*4))
#define AVC1_CPA_START_ADR      (*(volatile u32 *)(avc_base + 0x3e*4))
#define AVC1_CPA_END_ADR        (*(volatile u32 *)(avc_base + 0x3f*4))
#define AVC1_CPB_START_ADR      (*(volatile u32 *)(avc_base + 0x40*4))
#define AVC1_CPB_END_ADR        (*(volatile u32 *)(avc_base + 0x41*4))
#define AVC1_BS_ADDR            (*(volatile u32 *)(avc_base + 0x42*4))
#define AVC1_QP_CON0            (*(volatile u32 *)(avc_base + 0x43*4))
#define AVC1_QP_CON1            (*(volatile u32 *)(avc_base + 0x44*4))
#define AVC1_ROI0_XY            (*(volatile u32 *)(avc_base + 0x45*4))
#define AVC1_ROI1_XY            (*(volatile u32 *)(avc_base + 0x46*4))
#define AVC1_ROI2_XY            (*(volatile u32 *)(avc_base + 0x47*4))
#define AVC1_ROI3_XY            (*(volatile u32 *)(avc_base + 0x48*4))
#define AVC1_ROIX_QP            (*(volatile u32 *)(avc_base + 0x49*4))
#define AVC1_VAR_MUL16          (*(volatile u32 *)(avc_base + 0x4a*4))
#define AVC1_DECIMATE           (*(volatile u32 *)(avc_base + 0x4b*4))
#define AVC1_BCNT               (*(volatile u32 *)(avc_base + 0x4c*4))
#define AVC1_HEADER_CON         (*(volatile u32 *)(avc_base + 0x4d*4))
#define AVC1_TOTAL_MB_COST      (*(volatile u32 *)(avc_base + 0x4e*4))
#define AVC1_INTER_WEIGHT       (*(volatile u32 *)(avc_base + 0x4f*4))
#define AVC1_DEADZONE           (*(volatile u32 *)(avc_base + 0x50*4))
#define AVC1_HAS_BUF            (*(volatile u32 *)(avc_base + 0x51*4))
//
#define AVC_BUS_CON             (*(volatile u32 *)(avc_base + 0x52*4))
#define AVC_FENCFREF_COOR       (*(volatile u32 *)(avc_base + 0x53*4))
#define AVC_DECABAC_COOR        (*(volatile u32 *)(avc_base + 0x54*4))
#define AVC_BUF01_CNT           (*(volatile u32 *)(avc_base + 0x55*4))


//...........  Full Speed USB .....................
#define FADDR       0x00
#define POWER       0x01
#define INTRTX1     0x02
#define INTRTX2     0x03
#define INTRRX1     0x04
#define INTRRX2     0x05
#define INTRUSB     0x06
#define INTRTX1E    0x07
#define INTRTX2E    0x08
#define INTRRX1E    0x09
#define INTRRX2E    0x0a
#define INTRUSBE    0x0b
#define FRAME1      0x0c
#define FRAME2      0x0d
#define INDEX       0x0e
#define DEVCTL      0x0f
#define TXMAXP      0x10
#define CSR0        0x11
#define TXCSR1      0x11
#define TXCSR2      0x12
#define RXMAXP      0x13
#define RXCSR1      0x14
#define RXCSR2      0x15
#define COUNT0      0x16
#define RXCOUNT1    0x16
#define RXCOUNT2    0x17
#define TXTYPE      0x18
#define TXINTERVAL  0x19
#define RXTYPE      0x1a
#define RXINTERVAL  0x1b

//...........  High Speed USB .....................
#define husb0_base ls_base + 0x4000

#define H0_FADDR         (*(volatile u8  *)(husb0_base + 0x000))
#define H0_POWER         (*(volatile u8  *)(husb0_base + 0x001))
#define H0_INTRTX        (*(volatile u16 *)(husb0_base + 0x002))
#define H0_INTRRX        (*(volatile u16 *)(husb0_base + 0x004))
#define H0_INTRTXE       (*(volatile u16 *)(husb0_base + 0x006))
#define H0_INTRRXE       (*(volatile u16 *)(husb0_base + 0x008))
#define H0_INTRUSB       (*(volatile u8  *)(husb0_base + 0x00a))
#define H0_INTRUSBE      (*(volatile u8  *)(husb0_base + 0x00b))
#define H0_FRAME         (*(volatile u16 *)(husb0_base + 0x00c))
#define H0_INDEX         (*(volatile u8  *)(husb0_base + 0x00e))
#define H0_TESTMODE      (*(volatile u8  *)(husb0_base + 0x00f))

#define H0_FIFO0         (*(volatile u8  *)(husb0_base + 0x020))
#define H0_FIFO1         (*(volatile u8  *)(husb0_base + 0x024))
#define H0_FIFO2         (*(volatile u8  *)(husb0_base + 0x028))
#define H0_FIFO3         (*(volatile u8  *)(husb0_base + 0x02c))
#define H0_FIFO4         (*(volatile u8  *)(husb0_base + 0x030))
#define H0_DEVCTL        (*(volatile u8  *)(husb0_base + 0x060))

#define H0_CSR0          (*(volatile u16 *)(husb0_base + 0x102))
#define H0_COUNT0        (*(volatile u16 *)(husb0_base + 0x108))
#define H0_NAKLIMIT0     (*(volatile u8  *)(husb0_base + 0x10b))
#define H0_CFGDATA       (*(volatile u8  *)(husb0_base + 0x10f))

#define H0_EP1TXMAXP     (*(volatile u16 *)(husb0_base + 0x110))
#define H0_EP1TXCSR      (*(volatile u16 *)(husb0_base + 0x112))
#define H0_EP1RXMAXP     (*(volatile u16 *)(husb0_base + 0x114))
#define H0_EP1RXCSR      (*(volatile u16 *)(husb0_base + 0x116))
#define H0_EP1RXCOUNT    (*(volatile u16 *)(husb0_base + 0x118))
#define H0_EP1TXTYPE     (*(volatile u8  *)(husb0_base + 0x11a))
#define H0_EP1TXINTERVAL (*(volatile u8  *)(husb0_base + 0x11b))
#define H0_EP1RXTYPE     (*(volatile u8  *)(husb0_base + 0x11c))
#define H0_EP1RXINTERVAL (*(volatile u8  *)(husb0_base + 0x11d))
#define H0_EP1FIFOSIZE   (*(volatile u8  *)(husb0_base + 0x11f))

#define H0_EP2TXMAXP     (*(volatile u16 *)(husb0_base + 0x120))
#define H0_EP2TXCSR      (*(volatile u16 *)(husb0_base + 0x122))
#define H0_EP2RXMAXP     (*(volatile u16 *)(husb0_base + 0x124))
#define H0_EP2RXCSR      (*(volatile u16 *)(husb0_base + 0x126))
#define H0_EP2RXCOUNT    (*(volatile u16 *)(husb0_base + 0x128))
#define H0_EP2TXTYPE     (*(volatile u8  *)(husb0_base + 0x12a))
#define H0_EP2TXINTERVAL (*(volatile u8  *)(husb0_base + 0x12b))
#define H0_EP2RXTYPE     (*(volatile u8  *)(husb0_base + 0x12c))
#define H0_EP2RXINTERVAL (*(volatile u8  *)(husb0_base + 0x12d))
#define H0_EP2FIFOSIZE   (*(volatile u8  *)(husb0_base + 0x12f))

#define H0_EP3TXMAXP     (*(volatile u16 *)(husb0_base + 0x130))
#define H0_EP3TXCSR      (*(volatile u16 *)(husb0_base + 0x132))
#define H0_EP3RXMAXP     (*(volatile u16 *)(husb0_base + 0x134))
#define H0_EP3RXCSR      (*(volatile u16 *)(husb0_base + 0x136))
#define H0_EP3RXCOUNT    (*(volatile u16 *)(husb0_base + 0x138))
#define H0_EP3TXTYPE     (*(volatile u8  *)(husb0_base + 0x13a))
#define H0_EP3TXINTERVAL (*(volatile u8  *)(husb0_base + 0x13b))
#define H0_EP3RXTYPE     (*(volatile u8  *)(husb0_base + 0x13c))
#define H0_EP3RXINTERVAL (*(volatile u8  *)(husb0_base + 0x13d))
#define H0_EP3FIFOSIZE   (*(volatile u8  *)(husb0_base + 0x13f))

#define H0_EP4TXMAXP     (*(volatile u16 *)(husb0_base + 0x140))
#define H0_EP4TXCSR      (*(volatile u16 *)(husb0_base + 0x142))
#define H0_EP4RXMAXP     (*(volatile u16 *)(husb0_base + 0x144))
#define H0_EP4RXCSR      (*(volatile u16 *)(husb0_base + 0x146))
#define H0_EP4RXCOUNT    (*(volatile u16 *)(husb0_base + 0x148))
#define H0_EP4TXTYPE     (*(volatile u8  *)(husb0_base + 0x14a))
#define H0_EP4TXINTERVAL (*(volatile u8  *)(husb0_base + 0x14b))
#define H0_EP4RXTYPE     (*(volatile u8  *)(husb0_base + 0x14c))
#define H0_EP4RXINTERVAL (*(volatile u8  *)(husb0_base + 0x14d))
#define H0_EP4FIFOSIZE   (*(volatile u8  *)(husb0_base + 0x14f))

#define H0_TX_DPKTDIS    (*(volatile u16 *)(husb0_base + 0x342))
#define H0_C_T_UCH       (*(volatile u16 *)(husb0_base + 0x344))

#define husb1_base ls_base + 0x6000

#define H1_FADDR         (*(volatile u8  *)(husb1_base + 0x000))
#define H1_POWER         (*(volatile u8  *)(husb1_base + 0x001))
#define H1_INTRTX        (*(volatile u16 *)(husb1_base + 0x002))
#define H1_INTRRX        (*(volatile u16 *)(husb1_base + 0x004))
#define H1_INTRTXE       (*(volatile u16 *)(husb1_base + 0x006))
#define H1_INTRRXE       (*(volatile u16 *)(husb1_base + 0x008))
#define H1_INTRUSB       (*(volatile u8  *)(husb1_base + 0x00a))
#define H1_INTRUSBE      (*(volatile u8  *)(husb1_base + 0x00b))
#define H1_FRAME         (*(volatile u16 *)(husb1_base + 0x00c))
#define H1_INDEX         (*(volatile u8  *)(husb1_base + 0x00e))
#define H1_TESTMODE      (*(volatile u8  *)(husb1_base + 0x00f))

#define H1_FIFO0         (*(volatile u8  *)(husb1_base + 0x020))
#define H1_FIFO1         (*(volatile u8  *)(husb1_base + 0x024))
#define H1_FIFO2         (*(volatile u8  *)(husb1_base + 0x028))
#define H1_FIFO3         (*(volatile u8  *)(husb1_base + 0x02c))
#define H1_FIFO4         (*(volatile u8  *)(husb1_base + 0x030))
#define H1_DEVCTL        (*(volatile u8  *)(husb1_base + 0x060))

#define H1_CSR0          (*(volatile u16 *)(husb1_base + 0x102))
#define H1_COUNT0        (*(volatile u16 *)(husb1_base + 0x108))
#define H1_NAKLIMIT0     (*(volatile u8  *)(husb1_base + 0x10b))
#define H1_CFGDATA       (*(volatile u8  *)(husb1_base + 0x10f))

#define H1_EP1TXMAXP     (*(volatile u16 *)(husb1_base + 0x110))
#define H1_EP1TXCSR      (*(volatile u16 *)(husb1_base + 0x112))
#define H1_EP1RXMAXP     (*(volatile u16 *)(husb1_base + 0x114))
#define H1_EP1RXCSR      (*(volatile u16 *)(husb1_base + 0x116))
#define H1_EP1RXCOUNT    (*(volatile u16 *)(husb1_base + 0x118))
#define H1_EP1TXTYPE     (*(volatile u8  *)(husb1_base + 0x11a))
#define H1_EP1TXINTERVAL (*(volatile u8  *)(husb1_base + 0x11b))
#define H1_EP1RXTYPE     (*(volatile u8  *)(husb1_base + 0x11c))
#define H1_EP1RXINTERVAL (*(volatile u8  *)(husb1_base + 0x11d))
#define H1_EP1FIFOSIZE   (*(volatile u8  *)(husb1_base + 0x11f))

#define H1_EP2TXMAXP     (*(volatile u16 *)(husb1_base + 0x120))
#define H1_EP2TXCSR      (*(volatile u16 *)(husb1_base + 0x122))
#define H1_EP2RXMAXP     (*(volatile u16 *)(husb1_base + 0x124))
#define H1_EP2RXCSR      (*(volatile u16 *)(husb1_base + 0x126))
#define H1_EP2RXCOUNT    (*(volatile u16 *)(husb1_base + 0x128))
#define H1_EP2TXTYPE     (*(volatile u8  *)(husb1_base + 0x12a))
#define H1_EP2TXINTERVAL (*(volatile u8  *)(husb1_base + 0x12b))
#define H1_EP2RXTYPE     (*(volatile u8  *)(husb1_base + 0x12c))
#define H1_EP2RXINTERVAL (*(volatile u8  *)(husb1_base + 0x12d))
#define H1_EP2FIFOSIZE   (*(volatile u8  *)(husb1_base + 0x12f))

#define H1_EP3TXMAXP     (*(volatile u16 *)(husb1_base + 0x130))
#define H1_EP3TXCSR      (*(volatile u16 *)(husb1_base + 0x132))
#define H1_EP3RXMAXP     (*(volatile u16 *)(husb1_base + 0x134))
#define H1_EP3RXCSR      (*(volatile u16 *)(husb1_base + 0x136))
#define H1_EP3RXCOUNT    (*(volatile u16 *)(husb1_base + 0x138))
#define H1_EP3TXTYPE     (*(volatile u8  *)(husb1_base + 0x13a))
#define H1_EP3TXINTERVAL (*(volatile u8  *)(husb1_base + 0x13b))
#define H1_EP3RXTYPE     (*(volatile u8  *)(husb1_base + 0x13c))
#define H1_EP3RXINTERVAL (*(volatile u8  *)(husb1_base + 0x13d))
#define H1_EP3FIFOSIZE   (*(volatile u8  *)(husb1_base + 0x13f))

#define H1_EP4TXMAXP     (*(volatile u16 *)(husb1_base + 0x140))
#define H1_EP4TXCSR      (*(volatile u16 *)(husb1_base + 0x142))
#define H1_EP4RXMAXP     (*(volatile u16 *)(husb1_base + 0x144))
#define H1_EP4RXCSR      (*(volatile u16 *)(husb1_base + 0x146))
#define H1_EP4RXCOUNT    (*(volatile u16 *)(husb1_base + 0x148))
#define H1_EP4TXTYPE     (*(volatile u8  *)(husb1_base + 0x14a))
#define H1_EP4TXINTERVAL (*(volatile u8  *)(husb1_base + 0x14b))
#define H1_EP4RXTYPE     (*(volatile u8  *)(husb1_base + 0x14c))
#define H1_EP4RXINTERVAL (*(volatile u8  *)(husb1_base + 0x14d))
#define H1_EP4FIFOSIZE   (*(volatile u8  *)(husb1_base + 0x14f))

#define H1_TX_DPKTDIS    (*(volatile u16 *)(husb1_base + 0x342))
#define H1_C_T_UCH       (*(volatile u16 *)(husb1_base + 0x344))

//==============================================================//
//  xbus
//==============================================================//
#define eva_base    0x3f98000
#define xbus_base   eva_base + 0x0000
#define xbus_ch00_lvl           (*(volatile u32 *)(xbus_base + 0x00*4))
#define xbus_ch01_lvl           (*(volatile u32 *)(xbus_base + 0x01*4))
#define xbus_ch02_lvl           (*(volatile u32 *)(xbus_base + 0x02*4))
#define xbus_ch03_lvl           (*(volatile u32 *)(xbus_base + 0x03*4))
#define xbus_ch04_lvl           (*(volatile u32 *)(xbus_base + 0x04*4))
#define xbus_ch05_lvl           (*(volatile u32 *)(xbus_base + 0x05*4))
#define xbus_ch06_lvl           (*(volatile u32 *)(xbus_base + 0x06*4))
#define xbus_ch07_lvl           (*(volatile u32 *)(xbus_base + 0x07*4))
#define xbus_ch08_lvl           (*(volatile u32 *)(xbus_base + 0x08*4))
#define xbus_ch09_lvl           (*(volatile u32 *)(xbus_base + 0x09*4))
#define xbus_ch10_lvl           (*(volatile u32 *)(xbus_base + 0x0a*4))
#define xbus_ch11_lvl           (*(volatile u32 *)(xbus_base + 0x0b*4))
#define xbus_ch12_lvl           (*(volatile u32 *)(xbus_base + 0x0c*4))
#define xbus_ch13_lvl           (*(volatile u32 *)(xbus_base + 0x0d*4))
#define xbus_ch14_lvl           (*(volatile u32 *)(xbus_base + 0x0e*4))
#define xbus_ch15_lvl           (*(volatile u32 *)(xbus_base + 0x0f*4))

#define xbus_ch16_lvl           (*(volatile u32 *)(xbus_base + 0x10*4))
#define xbus_ch17_lvl           (*(volatile u32 *)(xbus_base + 0x11*4))
#define xbus_ch18_lvl           (*(volatile u32 *)(xbus_base + 0x12*4))
#define xbus_ch19_lvl           (*(volatile u32 *)(xbus_base + 0x13*4))
#define xbus_ch20_lvl           (*(volatile u32 *)(xbus_base + 0x14*4))
#define xbus_ch21_lvl           (*(volatile u32 *)(xbus_base + 0x15*4))
#define xbus_ch22_lvl           (*(volatile u32 *)(xbus_base + 0x16*4))
#define xbus_ch23_lvl           (*(volatile u32 *)(xbus_base + 0x17*4))
#define xbus_ch24_lvl           (*(volatile u32 *)(xbus_base + 0x18*4))
#define xbus_ch25_lvl           (*(volatile u32 *)(xbus_base + 0x19*4))
#define xbus_ch26_lvl           (*(volatile u32 *)(xbus_base + 0x1a*4))
#define xbus_ch27_lvl           (*(volatile u32 *)(xbus_base + 0x1b*4))
#define xbus_ch28_lvl           (*(volatile u32 *)(xbus_base + 0x1c*4))
#define xbus_ch29_lvl           (*(volatile u32 *)(xbus_base + 0x1d*4))
#define xbus_ch30_lvl           (*(volatile u32 *)(xbus_base + 0x1e*4))
#define xbus_ch31_lvl           (*(volatile u32 *)(xbus_base + 0x1f*4))

#define xbus_lv1_prd            (*(volatile u32 *)(xbus_base + 0x20*4))
#define xbus_lv2_prd            (*(volatile u32 *)(xbus_base + 0x21*4))
#define xbus_dist0_con          (*(volatile u32 *)(xbus_base + 0x22*4))
#define xbus_dist1_con          (*(volatile u32 *)(xbus_base + 0x23*4))
#define xbus_dist0_ext          (*(volatile u32 *)(xbus_base + 0x24*4))

//==============================================================//
//  isc
//==============================================================//
#define isc_base    eva_base + 0x0800
#define isc_pnd_con             (*(volatile u32 *)(isc_base + 0x00*4))
#define isc_dmx_con0            (*(volatile u32 *)(isc_base + 0x01*4))
#define isc_dmx_con1            (*(volatile u32 *)(isc_base + 0x02*4))

#define isc_sen0_con            (*(volatile u32 *)(isc_base + 0x08*4))
#define isc_sen0_vblk           (*(volatile u32 *)(isc_base + 0x09*4))
#define isc_sen0_vact           (*(volatile u32 *)(isc_base + 0x0a*4))
#define isc_sen0_hblk           (*(volatile u32 *)(isc_base + 0x0b*4))
#define isc_sen0_hact           (*(volatile u32 *)(isc_base + 0x0c*4))

#define isc_sen1_con            (*(volatile u32 *)(isc_base + 0x10*4))
#define isc_sen1_vblk           (*(volatile u32 *)(isc_base + 0x11*4))
#define isc_sen1_vact           (*(volatile u32 *)(isc_base + 0x12*4))
#define isc_sen1_hblk           (*(volatile u32 *)(isc_base + 0x13*4))
#define isc_sen1_hact           (*(volatile u32 *)(isc_base + 0x14*4))

#define isc_lcds_con            (*(volatile u32 *)(isc_base + 0x18*4))
#define isc_lcds_vblk           (*(volatile u32 *)(isc_base + 0x19*4))
#define isc_lcds_vact           (*(volatile u32 *)(isc_base + 0x1a*4))
#define isc_lcds_hblk           (*(volatile u32 *)(isc_base + 0x1b*4))
#define isc_lcds_hact           (*(volatile u32 *)(isc_base + 0x1c*4))

//==============================================================//
//  isp0
//==============================================================//
#define isp0_base   eva_base + 0x1000
#define isp0_pnd_con            (*(volatile u32 *)(isp0_base + 0x000*4))
#define isp0_scn_con            (*(volatile u32 *)(isp0_base + 0x001*4))

#define isp0_src_con            (*(volatile u32 *)(isp0_base + 0x004*4))
#define isp0_src_haw            (*(volatile u32 *)(isp0_base + 0x005*4))
#define isp0_src_vaw            (*(volatile u32 *)(isp0_base + 0x006*4))

#define isp0_blc_off_r          (*(volatile u32 *)(isp0_base + 0x008*4))
#define isp0_blc_off_gr         (*(volatile u32 *)(isp0_base + 0x009*4))
#define isp0_blc_off_gb         (*(volatile u32 *)(isp0_base + 0x00a*4))
#define isp0_blc_off_b          (*(volatile u32 *)(isp0_base + 0x00b*4))

#define isp0_dpc_th0            (*(volatile u32 *)(isp0_base + 0x00c*4))
#define isp0_dpc_th1            (*(volatile u32 *)(isp0_base + 0x00d*4))
#define isp0_dpc_th2            (*(volatile u32 *)(isp0_base + 0x00e*4))

#define isp0_lsc_cen_x          (*(volatile u32 *)(isp0_base + 0x010*4))
#define isp0_lsc_cen_y          (*(volatile u32 *)(isp0_base + 0x011*4))
#define isp0_lsc_dth_th         (*(volatile u32 *)(isp0_base + 0x012*4))
#define isp0_lsc_dth_prm0       (*(volatile u32 *)(isp0_base + 0x013*4))
#define isp0_lsc_dth_prm1       (*(volatile u32 *)(isp0_base + 0x014*4))
#define isp0_lsc_lut_r          (*(volatile u32 *)(isp0_base + 0x015*4))
#define isp0_lsc_lut_g          (*(volatile u32 *)(isp0_base + 0x016*4))
#define isp0_lsc_lut_b          (*(volatile u32 *)(isp0_base + 0x017*4))

#define isp0_awb_gain_r         (*(volatile u32 *)(isp0_base + 0x018*4))
#define isp0_awb_gain_g         (*(volatile u32 *)(isp0_base + 0x019*4))
#define isp0_awb_gain_b         (*(volatile u32 *)(isp0_base + 0x01a*4))

#define isp0_drc_lut            (*(volatile u32 *)(isp0_base + 0x01b*4))

#define isp0_tnr_con            (*(volatile u32 *)(isp0_base + 0x01c*4))
#define isp0_tnr_base           (*(volatile u32 *)(isp0_base + 0x01d*4))
#define isp0_tnr_size           (*(volatile u32 *)(isp0_base + 0x01e*4))
#define isp0_tnr_2d_str         (*(volatile u32 *)(isp0_base + 0x01f*4))
#define isp0_tnr_3d_th0         (*(volatile u32 *)(isp0_base + 0x020*4))
#define isp0_tnr_3d_th1         (*(volatile u32 *)(isp0_base + 0x021*4))
#define isp0_tnr_mt_th          (*(volatile u32 *)(isp0_base + 0x022*4))
#define isp0_tnr_wmax           (*(volatile u32 *)(isp0_base + 0x023*4))
#define isp0_tnr_wmin           (*(volatile u32 *)(isp0_base + 0x024*4))
#define isp0_tnr_wslope         (*(volatile u32 *)(isp0_base + 0x025*4))
#define isp0_tnr_break          (*(volatile u32 *)(isp0_base + 0x026*4))
#define isp0_tnr_scale0         (*(volatile u32 *)(isp0_base + 0x027*4))
#define isp0_tnr_scale1         (*(volatile u32 *)(isp0_base + 0x028*4))
#define isp0_tnr_scale2         (*(volatile u32 *)(isp0_base + 0x029*4))
#define isp0_tnr_scale3         (*(volatile u32 *)(isp0_base + 0x02a*4))
#define isp0_tnr_scale4         (*(volatile u32 *)(isp0_base + 0x02b*4))
#define isp0_tnr_scale5         (*(volatile u32 *)(isp0_base + 0x02c*4))
#define isp0_tnr_scale6         (*(volatile u32 *)(isp0_base + 0x02d*4))
#define isp0_tnr_scale7         (*(volatile u32 *)(isp0_base + 0x02e*4))

#define isp0_ccm_r_coe0         (*(volatile u32 *)(isp0_base + 0x030*4))
#define isp0_ccm_r_coe1         (*(volatile u32 *)(isp0_base + 0x031*4))
#define isp0_ccm_r_coe2         (*(volatile u32 *)(isp0_base + 0x032*4))
#define isp0_ccm_r_off          (*(volatile u32 *)(isp0_base + 0x033*4))
#define isp0_ccm_g_coe0         (*(volatile u32 *)(isp0_base + 0x034*4))
#define isp0_ccm_g_coe1         (*(volatile u32 *)(isp0_base + 0x035*4))
#define isp0_ccm_g_coe2         (*(volatile u32 *)(isp0_base + 0x036*4))
#define isp0_ccm_g_off          (*(volatile u32 *)(isp0_base + 0x037*4))
#define isp0_ccm_b_coe0         (*(volatile u32 *)(isp0_base + 0x038*4))
#define isp0_ccm_b_coe1         (*(volatile u32 *)(isp0_base + 0x039*4))
#define isp0_ccm_b_coe2         (*(volatile u32 *)(isp0_base + 0x03a*4))
#define isp0_ccm_b_off          (*(volatile u32 *)(isp0_base + 0x03b*4))

#define isp0_gma_r_lut          (*(volatile u32 *)(isp0_base + 0x03c*4))
#define isp0_gma_g_lut          (*(volatile u32 *)(isp0_base + 0x03d*4))
#define isp0_gma_b_lut          (*(volatile u32 *)(isp0_base + 0x03e*4))
#define isp0_csc_y_lut          (*(volatile u32 *)(isp0_base + 0x03f*4))

#define isp0_dnr_sim_th         (*(volatile u32 *)(isp0_base + 0x040*4))
#define isp0_dnr_rng_sgm        (*(volatile u32 *)(isp0_base + 0x041*4))
#define isp0_dnr_gaus_c00       (*(volatile u32 *)(isp0_base + 0x042*4))
#define isp0_dnr_gaus_c01       (*(volatile u32 *)(isp0_base + 0x043*4))
#define isp0_dnr_gaus_c02       (*(volatile u32 *)(isp0_base + 0x044*4))
#define isp0_dnr_gaus_c03       (*(volatile u32 *)(isp0_base + 0x045*4))
#define isp0_dnr_gaus_c11       (*(volatile u32 *)(isp0_base + 0x046*4))
#define isp0_dnr_gaus_c12       (*(volatile u32 *)(isp0_base + 0x047*4))
#define isp0_dnr_gaus_c13       (*(volatile u32 *)(isp0_base + 0x048*4))
#define isp0_dnr_gaus_c22       (*(volatile u32 *)(isp0_base + 0x049*4))
#define isp0_dnr_gaus_c23       (*(volatile u32 *)(isp0_base + 0x04a*4))
#define isp0_dnr_gaus_c33       (*(volatile u32 *)(isp0_base + 0x04b*4))
#define isp0_dnr_cmid_en        (*(volatile u32 *)(isp0_base + 0x04c*4))

#define isp0_shp_lone_th        (*(volatile u32 *)(isp0_base + 0x04d*4))
#define isp0_shp_ech_min        (*(volatile u32 *)(isp0_base + 0x04e*4))
#define isp0_shp_ech_max        (*(volatile u32 *)(isp0_base + 0x04f*4))

#define isp0_shp_hf_th0         (*(volatile u32 *)(isp0_base + 0x050*4))
#define isp0_shp_hf_th1         (*(volatile u32 *)(isp0_base + 0x051*4))
#define isp0_shp_hf_th2         (*(volatile u32 *)(isp0_base + 0x052*4))
#define isp0_shp_hf_amt         (*(volatile u32 *)(isp0_base + 0x053*4))
#define isp0_shp_hf_gain        (*(volatile u32 *)(isp0_base + 0x054*4))
#define isp0_shp_hf_c00         (*(volatile u32 *)(isp0_base + 0x055*4))
#define isp0_shp_hf_c01         (*(volatile u32 *)(isp0_base + 0x056*4))
#define isp0_shp_hf_c02         (*(volatile u32 *)(isp0_base + 0x057*4))
#define isp0_shp_hf_c10         (*(volatile u32 *)(isp0_base + 0x058*4))
#define isp0_shp_hf_c11         (*(volatile u32 *)(isp0_base + 0x059*4))
#define isp0_shp_hf_c12         (*(volatile u32 *)(isp0_base + 0x05a*4))
#define isp0_shp_hf_c20         (*(volatile u32 *)(isp0_base + 0x05b*4))
#define isp0_shp_hf_c21         (*(volatile u32 *)(isp0_base + 0x05c*4))
#define isp0_shp_hf_c22         (*(volatile u32 *)(isp0_base + 0x05d*4))
#define isp0_shp_hf_sft         (*(volatile u32 *)(isp0_base + 0x05e*4))

#define isp0_shp_mf_th0         (*(volatile u32 *)(isp0_base + 0x060*4))
#define isp0_shp_mf_th1         (*(volatile u32 *)(isp0_base + 0x061*4))
#define isp0_shp_mf_amt         (*(volatile u32 *)(isp0_base + 0x062*4))
#define isp0_shp_mf_gain        (*(volatile u32 *)(isp0_base + 0x063*4))
#define isp0_shp_mf_c00         (*(volatile u32 *)(isp0_base + 0x064*4))
#define isp0_shp_mf_c01         (*(volatile u32 *)(isp0_base + 0x065*4))
#define isp0_shp_mf_c02         (*(volatile u32 *)(isp0_base + 0x066*4))
#define isp0_shp_mf_c10         (*(volatile u32 *)(isp0_base + 0x067*4))
#define isp0_shp_mf_c11         (*(volatile u32 *)(isp0_base + 0x068*4))
#define isp0_shp_mf_c12         (*(volatile u32 *)(isp0_base + 0x069*4))
#define isp0_shp_mf_c20         (*(volatile u32 *)(isp0_base + 0x06a*4))
#define isp0_shp_mf_c21         (*(volatile u32 *)(isp0_base + 0x06b*4))
#define isp0_shp_mf_c22         (*(volatile u32 *)(isp0_base + 0x06c*4))
#define isp0_shp_mf_sft         (*(volatile u32 *)(isp0_base + 0x06d*4))

#define isp0_shp_cr_smt_th      (*(volatile u32 *)(isp0_base + 0x070*4))
#define isp0_shp_cr_c00         (*(volatile u32 *)(isp0_base + 0x071*4))
#define isp0_shp_cr_c01         (*(volatile u32 *)(isp0_base + 0x072*4))
#define isp0_shp_cr_c02         (*(volatile u32 *)(isp0_base + 0x073*4))
#define isp0_shp_cr_c10         (*(volatile u32 *)(isp0_base + 0x074*4))
#define isp0_shp_cr_c11         (*(volatile u32 *)(isp0_base + 0x075*4))
#define isp0_shp_cr_c12         (*(volatile u32 *)(isp0_base + 0x076*4))
#define isp0_shp_cr_c20         (*(volatile u32 *)(isp0_base + 0x077*4))
#define isp0_shp_cr_c21         (*(volatile u32 *)(isp0_base + 0x078*4))
#define isp0_shp_cr_c22         (*(volatile u32 *)(isp0_base + 0x079*4))
#define isp0_shp_cr_sft         (*(volatile u32 *)(isp0_base + 0x07a*4))

#define isp0_cbs_y_gain         (*(volatile u32 *)(isp0_base + 0x080*4))
#define isp0_cbs_u_gain         (*(volatile u32 *)(isp0_base + 0x081*4))
#define isp0_cbs_v_gain         (*(volatile u32 *)(isp0_base + 0x082*4))
#define isp0_cbs_y_offs0        (*(volatile u32 *)(isp0_base + 0x083*4))
#define isp0_cbs_y_offs1        (*(volatile u32 *)(isp0_base + 0x084*4))
#define isp0_cbs_u_offs         (*(volatile u32 *)(isp0_base + 0x085*4))
#define isp0_cbs_v_offs         (*(volatile u32 *)(isp0_base + 0x086*4))

#define isp0_out_hst            (*(volatile u32 *)(isp0_base + 0x088*4))
#define isp0_out_hed            (*(volatile u32 *)(isp0_base + 0x089*4))
#define isp0_out_vst            (*(volatile u32 *)(isp0_base + 0x08a*4))
#define isp0_out_ved            (*(volatile u32 *)(isp0_base + 0x08b*4))

#define isp0_stc_ae_base0       (*(volatile u32 *)(isp0_base + 0x090*4))
#define isp0_stc_ae_base1       (*(volatile u32 *)(isp0_base + 0x091*4))
#define isp0_stc_ae_base2       (*(volatile u32 *)(isp0_base + 0x092*4))
#define isp0_stc_ae_base3       (*(volatile u32 *)(isp0_base + 0x093*4))
#define isp0_stc_ae_basex       (*(volatile u32 *)(isp0_base + 0x094*4))
#define isp0_stc_ae_en          (*(volatile u32 *)(isp0_base + 0x095*4))
#define isp0_stc_ae_lv1         (*(volatile u32 *)(isp0_base + 0x096*4))
#define isp0_stc_ae_lv2         (*(volatile u32 *)(isp0_base + 0x097*4))
#define isp0_stc_ae_lv3         (*(volatile u32 *)(isp0_base + 0x098*4))
#define isp0_stc_ae_lv4         (*(volatile u32 *)(isp0_base + 0x099*4))
#define isp0_stc_ae_lv5         (*(volatile u32 *)(isp0_base + 0x09a*4))
#define isp0_stc_ae_lv6         (*(volatile u32 *)(isp0_base + 0x09b*4))
#define isp0_stc_ae_lv7         (*(volatile u32 *)(isp0_base + 0x09c*4))

#define isp0_stc_wb_base0       (*(volatile u32 *)(isp0_base + 0x0a0*4))
#define isp0_stc_wb_base1       (*(volatile u32 *)(isp0_base + 0x0a1*4))
#define isp0_stc_wb_base2       (*(volatile u32 *)(isp0_base + 0x0a2*4))
#define isp0_stc_wb_base3       (*(volatile u32 *)(isp0_base + 0x0a3*4))
#define isp0_stc_wb_basex       (*(volatile u32 *)(isp0_base + 0x0a4*4))
#define isp0_stc_wb_en          (*(volatile u32 *)(isp0_base + 0x0a5*4))
#define isp0_stc_wb_r_th        (*(volatile u32 *)(isp0_base + 0x0a6*4))
#define isp0_stc_wb_g_th        (*(volatile u32 *)(isp0_base + 0x0a7*4))
#define isp0_stc_wb_b_th        (*(volatile u32 *)(isp0_base + 0x0a8*4))
#define isp0_stc_wb_w_th        (*(volatile u32 *)(isp0_base + 0x0a9*4))
#define isp0_stc_wb_y_min       (*(volatile u32 *)(isp0_base + 0x0aa*4))
#define isp0_stc_wb_y_max       (*(volatile u32 *)(isp0_base + 0x0ab*4))
#define isp0_stc_wb_rg_min      (*(volatile u32 *)(isp0_base + 0x0ac*4))
#define isp0_stc_wb_rg_max      (*(volatile u32 *)(isp0_base + 0x0ad*4))
#define isp0_stc_wb_bg_min      (*(volatile u32 *)(isp0_base + 0x0ae*4))
#define isp0_stc_wb_bg_max      (*(volatile u32 *)(isp0_base + 0x0af*4))

//==============================================================//
//  isp1
//==============================================================//
#define isp1_base   eva_base + 0x1800
#define isp1_pnd_con            (*(volatile u32 *)(isp1_base + 0x000*4))
#define isp1_src_con            (*(volatile u32 *)(isp1_base + 0x001*4))
#define isp1_src_haw            (*(volatile u32 *)(isp1_base + 0x002*4))
#define isp1_src_vaw            (*(volatile u32 *)(isp1_base + 0x003*4))

//==============================================================//
//  imc
//==============================================================//
#define imc_base    eva_base + 0x2000
#define imc_pnd_con             (*(volatile u32 *)(imc_base + 0x000*4))
#define imc_src0_con            (*(volatile u32 *)(imc_base + 0x001*4))
#define imc_src1_con            (*(volatile u32 *)(imc_base + 0x002*4))
#define imc_src2_con            (*(volatile u32 *)(imc_base + 0x003*4))
#define imc_src2_h_cfg          (*(volatile u32 *)(imc_base + 0x004*4))
#define imc_src2_v_cfg          (*(volatile u32 *)(imc_base + 0x005*4))
#define imc_src2_y_base         (*(volatile u32 *)(imc_base + 0x006*4))
#define imc_src2_u_base         (*(volatile u32 *)(imc_base + 0x007*4))
#define imc_src2_v_base         (*(volatile u32 *)(imc_base + 0x008*4))
#define imc_scn_con             (*(volatile u32 *)(imc_base + 0x009*4))

#define imc_ch0_sca0_con        (*(volatile u32 *)(imc_base + 0x010*4))
#define imc_ch0_sca0_step       (*(volatile u32 *)(imc_base + 0x011*4))
#define imc_ch0_sca0_phase      (*(volatile u32 *)(imc_base + 0x012*4))
#define imc_ch0_sca0_width      (*(volatile u32 *)(imc_base + 0x013*4))
#define imc_ch0_sca1_con        (*(volatile u32 *)(imc_base + 0x014*4))
#define imc_ch0_sca1_step       (*(volatile u32 *)(imc_base + 0x015*4))
#define imc_ch0_sca1_phase      (*(volatile u32 *)(imc_base + 0x016*4))
#define imc_ch0_sca1_high       (*(volatile u32 *)(imc_base + 0x017*4))
#define imc_ch0_enc_con         (*(volatile u32 *)(imc_base + 0x018*4))
#define imc_ch0_enc_cnt         (*(volatile u32 *)(imc_base + 0x019*4))
#define imc_ch0_enc_y_base0     (*(volatile u32 *)(imc_base + 0x01a*4))
#define imc_ch0_enc_u_base0     (*(volatile u32 *)(imc_base + 0x01b*4))
#define imc_ch0_enc_v_base0     (*(volatile u32 *)(imc_base + 0x01c*4))
#define imc_ch0_enc_y_base1     (*(volatile u32 *)(imc_base + 0x01d*4))
#define imc_ch0_enc_u_base1     (*(volatile u32 *)(imc_base + 0x01e*4))
#define imc_ch0_enc_v_base1     (*(volatile u32 *)(imc_base + 0x01f*4))
#define imc_ch0_enc_o_base      (*(volatile u32 *)(imc_base + 0x020*4))
#define imc_ch0_osd_color       (*(volatile u32 *)(imc_base + 0x021*4))
#define imc_ch0_osd_h_cfg       (*(volatile u32 *)(imc_base + 0x022*4))
#define imc_ch0_osd_v_cfg       (*(volatile u32 *)(imc_base + 0x023*4))

#define imc_psca0_con           (*(volatile u32 *)(imc_base + 0x024*4))
#define imc_psca0_h_cfg         (*(volatile u32 *)(imc_base + 0x025*4))
#define imc_psca0_v_cfg         (*(volatile u32 *)(imc_base + 0x026*4))
#define imc_psca0_gs_parma      (*(volatile u32 *)(imc_base + 0x027*4))
#define imc_psca0_gs_parmb      (*(volatile u32 *)(imc_base + 0x028*4))
#define imc_psca0_gs_parmc      (*(volatile u32 *)(imc_base + 0x029*4))
#define imc_psca0_base          (*(volatile u32 *)(imc_base + 0x02a*4))
#define imc_psca0_add_d_num     (*(volatile u32 *)(imc_base + 0x02b*4))
#define imc_psca0_gs_parmd      (*(volatile u32 *)(imc_base + 0x02c*4))

#define imc_ch1_sca0_con        (*(volatile u32 *)(imc_base + 0x030*4))
#define imc_ch1_sca0_step       (*(volatile u32 *)(imc_base + 0x031*4))
#define imc_ch1_sca0_phase      (*(volatile u32 *)(imc_base + 0x032*4))
#define imc_ch1_sca0_width      (*(volatile u32 *)(imc_base + 0x033*4))
#define imc_ch1_sca1_con        (*(volatile u32 *)(imc_base + 0x034*4))
#define imc_ch1_sca1_step       (*(volatile u32 *)(imc_base + 0x035*4))
#define imc_ch1_sca1_phase      (*(volatile u32 *)(imc_base + 0x036*4))
#define imc_ch1_sca1_high       (*(volatile u32 *)(imc_base + 0x037*4))
#define imc_ch1_enc_con         (*(volatile u32 *)(imc_base + 0x038*4))
#define imc_ch1_enc_cnt         (*(volatile u32 *)(imc_base + 0x039*4))
#define imc_ch1_enc_y_base0     (*(volatile u32 *)(imc_base + 0x03a*4))
#define imc_ch1_enc_u_base0     (*(volatile u32 *)(imc_base + 0x03b*4))
#define imc_ch1_enc_v_base0     (*(volatile u32 *)(imc_base + 0x03c*4))
#define imc_ch1_enc_y_base1     (*(volatile u32 *)(imc_base + 0x03d*4))
#define imc_ch1_enc_u_base1     (*(volatile u32 *)(imc_base + 0x03e*4))
#define imc_ch1_enc_v_base1     (*(volatile u32 *)(imc_base + 0x03f*4))
#define imc_ch1_enc_o_base      (*(volatile u32 *)(imc_base + 0x040*4))
#define imc_ch1_osd_color       (*(volatile u32 *)(imc_base + 0x041*4))
#define imc_ch1_osd_h_cfg       (*(volatile u32 *)(imc_base + 0x042*4))
#define imc_ch1_osd_v_cfg       (*(volatile u32 *)(imc_base + 0x043*4))

#define imc_psca1_con           (*(volatile u32 *)(imc_base + 0x044*4))
#define imc_psca1_h_cfg         (*(volatile u32 *)(imc_base + 0x045*4))
#define imc_psca1_v_cfg         (*(volatile u32 *)(imc_base + 0x046*4))
#define imc_psca1_gs_parma      (*(volatile u32 *)(imc_base + 0x047*4))
#define imc_psca1_gs_parmb      (*(volatile u32 *)(imc_base + 0x048*4))
#define imc_psca1_gs_parmc      (*(volatile u32 *)(imc_base + 0x049*4))
#define imc_psca1_base          (*(volatile u32 *)(imc_base + 0x04a*4))
#define imc_psca1_add_d_num     (*(volatile u32 *)(imc_base + 0x04b*4))
#define imc_psca1_gs_parmd      (*(volatile u32 *)(imc_base + 0x04c*4))

#define imc_ch2_sca0_con        (*(volatile u32 *)(imc_base + 0x050*4))
#define imc_ch2_sca0_step       (*(volatile u32 *)(imc_base + 0x051*4))
#define imc_ch2_sca0_phase      (*(volatile u32 *)(imc_base + 0x052*4))
#define imc_ch2_sca0_width      (*(volatile u32 *)(imc_base + 0x053*4))
#define imc_ch2_sca1_con        (*(volatile u32 *)(imc_base + 0x054*4))
#define imc_ch2_sca1_step       (*(volatile u32 *)(imc_base + 0x055*4))
#define imc_ch2_sca1_phase      (*(volatile u32 *)(imc_base + 0x056*4))
#define imc_ch2_sca1_high       (*(volatile u32 *)(imc_base + 0x057*4))
#define imc_ch2_dis_con         (*(volatile u32 *)(imc_base + 0x058*4))
#define imc_ch2_dis_y_base      (*(volatile u32 *)(imc_base + 0x059*4))
#define imc_ch2_dis_u_base      (*(volatile u32 *)(imc_base + 0x05a*4))
#define imc_ch2_dis_v_base      (*(volatile u32 *)(imc_base + 0x05b*4))

#define imc_ch3_sca0_con        (*(volatile u32 *)(imc_base + 0x060*4))
#define imc_ch3_sca0_step       (*(volatile u32 *)(imc_base + 0x061*4))
#define imc_ch3_sca0_phase      (*(volatile u32 *)(imc_base + 0x062*4))
#define imc_ch3_sca0_width      (*(volatile u32 *)(imc_base + 0x063*4))
#define imc_ch3_sca1_con        (*(volatile u32 *)(imc_base + 0x064*4))
#define imc_ch3_sca1_step       (*(volatile u32 *)(imc_base + 0x065*4))
#define imc_ch3_sca1_phase      (*(volatile u32 *)(imc_base + 0x066*4))
#define imc_ch3_sca1_high       (*(volatile u32 *)(imc_base + 0x067*4))
#define imc_ch3_dis_con         (*(volatile u32 *)(imc_base + 0x068*4))
#define imc_ch3_dis_y_base      (*(volatile u32 *)(imc_base + 0x069*4))
#define imc_ch3_dis_u_base      (*(volatile u32 *)(imc_base + 0x06a*4))
#define imc_ch3_dis_v_base      (*(volatile u32 *)(imc_base + 0x06b*4))

#define imc_ch4_sca0_con        (*(volatile u32 *)(imc_base + 0x070*4))
#define imc_ch4_sca0_step       (*(volatile u32 *)(imc_base + 0x071*4))
#define imc_ch4_sca0_phase      (*(volatile u32 *)(imc_base + 0x072*4))
#define imc_ch4_sca0_width      (*(volatile u32 *)(imc_base + 0x073*4))
#define imc_ch4_sca1_con        (*(volatile u32 *)(imc_base + 0x074*4))
#define imc_ch4_sca1_step       (*(volatile u32 *)(imc_base + 0x075*4))
#define imc_ch4_sca1_phase      (*(volatile u32 *)(imc_base + 0x076*4))
#define imc_ch4_sca1_high       (*(volatile u32 *)(imc_base + 0x077*4))
#define imc_ch4_dis_con         (*(volatile u32 *)(imc_base + 0x078*4))
#define imc_ch4_dis_y_base      (*(volatile u32 *)(imc_base + 0x079*4))
#define imc_ch4_dis_u_base      (*(volatile u32 *)(imc_base + 0x07a*4))
#define imc_ch4_dis_v_base      (*(volatile u32 *)(imc_base + 0x07b*4))

//==============================================================//
//  imb
//==============================================================//
#define imb_base    eva_base + 0x2800
#define imb_con                 (*(volatile u32 *)(imb_base + 0x00*4))
#define imb_htt_with            (*(volatile u32 *)(imb_base + 0x01*4))
#define imb_vtt_high            (*(volatile u32 *)(imb_base + 0x02*4))
#define imb_buf_con             (*(volatile u32 *)(imb_base + 0x03*4))
#define imb_mix_con             (*(volatile u32 *)(imb_base + 0x04*4))
#define imb_o1b_dat             (*(volatile u32 *)(imb_base + 0x05*4))
#define imb_grp0_d0             (*(volatile u32 *)(imb_base + 0x06*4))
#define imb_grp0_d1             (*(volatile u32 *)(imb_base + 0x07*4))
#define imb_grp0_d2             (*(volatile u32 *)(imb_base + 0x08*4))
#define imb_grp1_d0             (*(volatile u32 *)(imb_base + 0x09*4))
#define imb_grp1_d1             (*(volatile u32 *)(imb_base + 0x0a*4))
#define imb_grp1_d2             (*(volatile u32 *)(imb_base + 0x0b*4))
#define imb_osd_tab0            (*(volatile u32 *)(imb_base + 0x0c*4))
#define imb_osd_tab1            (*(volatile u32 *)(imb_base + 0x0d*4))
#define imb_rot_con             (*(volatile u32 *)(imb_base + 0x0e*4))
#define imb_rot_badr            (*(volatile u32 *)(imb_base + 0x0f*4))

#define imb_l0_con              (*(volatile u32 *)(imb_base + 0x20*4))
#define imb_l0_h_cfg0           (*(volatile u32 *)(imb_base + 0x21*4))
#define imb_l0_h_cfg1           (*(volatile u32 *)(imb_base + 0x22*4))
#define imb_l0_v_cfg0           (*(volatile u32 *)(imb_base + 0x23*4))
#define imb_l0_v_cfg1           (*(volatile u32 *)(imb_base + 0x24*4))
#define imb_l0_ha_with          (*(volatile u32 *)(imb_base + 0x25*4))
#define imb_l0_aph              (*(volatile u32 *)(imb_base + 0x26*4))
#define imb_l0_badr0            (*(volatile u32 *)(imb_base + 0x27*4))
#define imb_l0_badr1            (*(volatile u32 *)(imb_base + 0x28*4))
#define imb_l0_badr2            (*(volatile u32 *)(imb_base + 0x29*4))
#define imb_l0_sc_dat           (*(volatile u32 *)(imb_base + 0x2a*4))
#define imb_l0_ht_with          (*(volatile u32 *)(imb_base + 0x2b*4))

#define imb_l1_con              (*(volatile u32 *)(imb_base + 0x30*4))
#define imb_l1_h_cfg0           (*(volatile u32 *)(imb_base + 0x31*4))
#define imb_l1_h_cfg1           (*(volatile u32 *)(imb_base + 0x32*4))
#define imb_l1_v_cfg0           (*(volatile u32 *)(imb_base + 0x33*4))
#define imb_l1_v_cfg1           (*(volatile u32 *)(imb_base + 0x34*4))
#define imb_l1_ha_with          (*(volatile u32 *)(imb_base + 0x35*4))
#define imb_l1_aph              (*(volatile u32 *)(imb_base + 0x36*4))
#define imb_l1_badr0            (*(volatile u32 *)(imb_base + 0x37*4))
#define imb_l1_badr1            (*(volatile u32 *)(imb_base + 0x38*4))
#define imb_l1_badr2            (*(volatile u32 *)(imb_base + 0x39*4))
#define imb_l1_sc_dat           (*(volatile u32 *)(imb_base + 0x3a*4))
#define imb_l1_ht_with          (*(volatile u32 *)(imb_base + 0x3b*4))

#define imb_l2_con              (*(volatile u32 *)(imb_base + 0x40*4))
#define imb_l2_h_cfg0           (*(volatile u32 *)(imb_base + 0x41*4))
#define imb_l2_h_cfg1           (*(volatile u32 *)(imb_base + 0x42*4))
#define imb_l2_v_cfg0           (*(volatile u32 *)(imb_base + 0x43*4))
#define imb_l2_v_cfg1           (*(volatile u32 *)(imb_base + 0x44*4))
#define imb_l2_ha_with          (*(volatile u32 *)(imb_base + 0x45*4))
#define imb_l2_aph              (*(volatile u32 *)(imb_base + 0x46*4))
#define imb_l2_badr0            (*(volatile u32 *)(imb_base + 0x47*4))
#define imb_l2_badr1            (*(volatile u32 *)(imb_base + 0x48*4))
#define imb_l2_badr2            (*(volatile u32 *)(imb_base + 0x49*4))
#define imb_l2_sc_dat           (*(volatile u32 *)(imb_base + 0x4a*4))
#define imb_l2_ht_with          (*(volatile u32 *)(imb_base + 0x4b*4))

#define imb_l3_con              (*(volatile u32 *)(imb_base + 0x50*4))
#define imb_l3_h_cfg0           (*(volatile u32 *)(imb_base + 0x51*4))
#define imb_l3_h_cfg1           (*(volatile u32 *)(imb_base + 0x52*4))
#define imb_l3_v_cfg0           (*(volatile u32 *)(imb_base + 0x53*4))
#define imb_l3_v_cfg1           (*(volatile u32 *)(imb_base + 0x54*4))
#define imb_l3_ha_with          (*(volatile u32 *)(imb_base + 0x55*4))
#define imb_l3_aph              (*(volatile u32 *)(imb_base + 0x56*4))
#define imb_l3_badr0            (*(volatile u32 *)(imb_base + 0x57*4))
#define imb_l3_badr1            (*(volatile u32 *)(imb_base + 0x58*4))
#define imb_l3_badr2            (*(volatile u32 *)(imb_base + 0x59*4))
#define imb_l3_sc_dat           (*(volatile u32 *)(imb_base + 0x5a*4))
#define imb_l3_ht_with          (*(volatile u32 *)(imb_base + 0x5b*4))

#define imb_l4_con              (*(volatile u32 *)(imb_base + 0x60*4))
#define imb_l4_h_cfg0           (*(volatile u32 *)(imb_base + 0x61*4))
#define imb_l4_h_cfg1           (*(volatile u32 *)(imb_base + 0x62*4))
#define imb_l4_v_cfg0           (*(volatile u32 *)(imb_base + 0x63*4))
#define imb_l4_v_cfg1           (*(volatile u32 *)(imb_base + 0x64*4))
#define imb_l4_ha_with          (*(volatile u32 *)(imb_base + 0x65*4))
#define imb_l4_aph              (*(volatile u32 *)(imb_base + 0x66*4))
#define imb_l4_badr0            (*(volatile u32 *)(imb_base + 0x67*4))
#define imb_l4_badr1            (*(volatile u32 *)(imb_base + 0x68*4))
#define imb_l4_badr2            (*(volatile u32 *)(imb_base + 0x69*4))
#define imb_l4_sc_dat           (*(volatile u32 *)(imb_base + 0x6a*4))
#define imb_l4_ht_with          (*(volatile u32 *)(imb_base + 0x6b*4))

#define imb_l5_con              (*(volatile u32 *)(imb_base + 0x70*4))
#define imb_l5_h_cfg0           (*(volatile u32 *)(imb_base + 0x71*4))
#define imb_l5_h_cfg1           (*(volatile u32 *)(imb_base + 0x72*4))
#define imb_l5_v_cfg0           (*(volatile u32 *)(imb_base + 0x73*4))
#define imb_l5_v_cfg1           (*(volatile u32 *)(imb_base + 0x74*4))
#define imb_l5_ha_with          (*(volatile u32 *)(imb_base + 0x75*4))
#define imb_l5_aph              (*(volatile u32 *)(imb_base + 0x76*4))
#define imb_l5_badr0            (*(volatile u32 *)(imb_base + 0x77*4))
#define imb_l5_badr1            (*(volatile u32 *)(imb_base + 0x78*4))
#define imb_l5_badr2            (*(volatile u32 *)(imb_base + 0x79*4))
#define imb_l5_sc_dat           (*(volatile u32 *)(imb_base + 0x7a*4))
#define imb_l5_ht_with          (*(volatile u32 *)(imb_base + 0xbb*4))

#define imb_l6_con              (*(volatile u32 *)(imb_base + 0x80*4))
#define imb_l6_h_cfg0           (*(volatile u32 *)(imb_base + 0x81*4))
#define imb_l6_h_cfg1           (*(volatile u32 *)(imb_base + 0x82*4))
#define imb_l6_v_cfg0           (*(volatile u32 *)(imb_base + 0x83*4))
#define imb_l6_v_cfg1           (*(volatile u32 *)(imb_base + 0x84*4))
#define imb_l6_ha_with          (*(volatile u32 *)(imb_base + 0x85*4))
#define imb_l6_aph              (*(volatile u32 *)(imb_base + 0x86*4))
#define imb_l6_badr0            (*(volatile u32 *)(imb_base + 0x87*4))
#define imb_l6_badr1            (*(volatile u32 *)(imb_base + 0x88*4))
#define imb_l6_badr2            (*(volatile u32 *)(imb_base + 0x89*4))
#define imb_l6_sc_dat           (*(volatile u32 *)(imb_base + 0x8a*4))
#define imb_l6_ht_with          (*(volatile u32 *)(imb_base + 0x8b*4))

#define imb_l7_con              (*(volatile u32 *)(imb_base + 0x90*4))
#define imb_l7_h_cfg0           (*(volatile u32 *)(imb_base + 0x91*4))
#define imb_l7_h_cfg1           (*(volatile u32 *)(imb_base + 0x92*4))
#define imb_l7_v_cfg0           (*(volatile u32 *)(imb_base + 0x93*4))
#define imb_l7_v_cfg1           (*(volatile u32 *)(imb_base + 0x94*4))
#define imb_l7_ha_with          (*(volatile u32 *)(imb_base + 0x95*4))
#define imb_l7_aph              (*(volatile u32 *)(imb_base + 0x96*4))
#define imb_l7_badr0            (*(volatile u32 *)(imb_base + 0x97*4))
#define imb_l7_badr1            (*(volatile u32 *)(imb_base + 0x98*4))
#define imb_l7_badr2            (*(volatile u32 *)(imb_base + 0x99*4))
#define imb_l7_sc_dat           (*(volatile u32 *)(imb_base + 0x9a*4))
#define imb_l7_ht_with          (*(volatile u32 *)(imb_base + 0x9b*4))

#define imb_scn_con             (*(volatile u32 *)(imb_base + 0xa0*4))

//==============================================================//
//  imd
//==============================================================//
#define imd_dmm_base    eva_base + 0x3000
#define imd_dmm_con             (*(volatile u32 *)(imd_dmm_base + 0x00*4))
#define imd_dmm_haw             (*(volatile u32 *)(imd_dmm_base + 0x01*4))
#define imd_dmm_vaw             (*(volatile u32 *)(imd_dmm_base + 0x02*4))
#define imd_dmm_badr            (*(volatile u32 *)(imd_dmm_base + 0x03*4))
#define imd_dmm_tdat            (*(volatile u32 *)(imd_dmm_base + 0x04*4))
#define imd_dmm_y_gain          (*(volatile u32 *)(imd_dmm_base + 0x05*4))
#define imd_dmm_u_gain          (*(volatile u32 *)(imd_dmm_base + 0x06*4))
#define imd_dmm_v_gain          (*(volatile u32 *)(imd_dmm_base + 0x07*4))
#define imd_dmm_y_offs          (*(volatile u32 *)(imd_dmm_base + 0x08*4))
#define imd_dmm_u_offs          (*(volatile u32 *)(imd_dmm_base + 0x09*4))
#define imd_dmm_v_offs          (*(volatile u32 *)(imd_dmm_base + 0x0a*4))
#define imd_dmm_r_gain          (*(volatile u32 *)(imd_dmm_base + 0x0b*4))
#define imd_dmm_g_gain          (*(volatile u32 *)(imd_dmm_base + 0x0c*4))
#define imd_dmm_b_gain          (*(volatile u32 *)(imd_dmm_base + 0x0d*4))
#define imd_dmm_r_coe0          (*(volatile u32 *)(imd_dmm_base + 0x0e*4))
#define imd_dmm_r_coe1          (*(volatile u32 *)(imd_dmm_base + 0x0f*4))
#define imd_dmm_r_coe2          (*(volatile u32 *)(imd_dmm_base + 0x10*4))
#define imd_dmm_g_coe0          (*(volatile u32 *)(imd_dmm_base + 0x11*4))
#define imd_dmm_g_coe1          (*(volatile u32 *)(imd_dmm_base + 0x12*4))
#define imd_dmm_g_coe2          (*(volatile u32 *)(imd_dmm_base + 0x13*4))
#define imd_dmm_b_coe0          (*(volatile u32 *)(imd_dmm_base + 0x14*4))
#define imd_dmm_b_coe1          (*(volatile u32 *)(imd_dmm_base + 0x15*4))
#define imd_dmm_b_coe2          (*(volatile u32 *)(imd_dmm_base + 0x16*4))
#define imd_dmm_r_offs          (*(volatile u32 *)(imd_dmm_base + 0x17*4))
#define imd_dmm_g_offs          (*(volatile u32 *)(imd_dmm_base + 0x18*4))
#define imd_dmm_b_offs          (*(volatile u32 *)(imd_dmm_base + 0x19*4))
#define imd_dmm_r_gma           (*(volatile u32 *)(imd_dmm_base + 0x1a*4))
#define imd_dmm_g_gma           (*(volatile u32 *)(imd_dmm_base + 0x1b*4))
#define imd_dmm_b_gma           (*(volatile u32 *)(imd_dmm_base + 0x1c*4))
#define imd_dmm_scn_con         (*(volatile u32 *)(imd_dmm_base + 0x1d*4))

#define imd_dpi_base    eva_base + 0x3080
#define imd_dpi_clk_con         (*(volatile u32 *)(imd_dpi_base + 0x00*4))
#define imd_dpi_io_con          (*(volatile u32 *)(imd_dpi_base + 0x01*4))
#define imd_dpi_con             (*(volatile u32 *)(imd_dpi_base + 0x02*4))
#define imd_dpi_fmt             (*(volatile u32 *)(imd_dpi_base + 0x03*4))
#define imd_dpi_emi             (*(volatile u32 *)(imd_dpi_base + 0x04*4))
#define imd_dpi_htt_cfg         (*(volatile u32 *)(imd_dpi_base + 0x05*4))
#define imd_dpi_hsw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x06*4))
#define imd_dpi_hst_cfg         (*(volatile u32 *)(imd_dpi_base + 0x07*4))
#define imd_dpi_haw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x08*4))
#define imd_dpi_vtt_cfg         (*(volatile u32 *)(imd_dpi_base + 0x09*4))
#define imd_dpi_vsw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x0a*4))
#define imd_dpi_vst_o_cfg       (*(volatile u32 *)(imd_dpi_base + 0x0b*4))
#define imd_dpi_vst_e_cfg       (*(volatile u32 *)(imd_dpi_base + 0x0c*4))
#define imd_dpi_vaw_cfg         (*(volatile u32 *)(imd_dpi_base + 0x0d*4))

//==============================================================//
//  avo
//==============================================================//
#define avo_base0   eva_base + 0x3800
#define avo_con0                (*(volatile u32 *)(avo_base0 + 0x00*4))
#define avo_con1                (*(volatile u32 *)(avo_base0 + 0x01*4))
#define avo_con2                (*(volatile u32 *)(avo_base0 + 0x02*4))

#define avo_base1   eva_base + 0x3a00
#define avo_soft_reset          (*(volatile u32 *)(avo_base1 + 4*62))
#define avo_system_con          (*(volatile u32 *)(avo_base1 + 4*52))
#define avo_slave_mode          (*(volatile u32 *)(avo_base1 + 4*13))
#define avo_clrbar_mode         (*(volatile u32 *)(avo_base1 + 4*5 ))

#define avo_chroma_freq0        (*(volatile u32 *)(avo_base1 + 4*0 ))
#define avo_chroma_freq1        (*(volatile u32 *)(avo_base1 + 4*1 ))
#define avo_chroma_freq2        (*(volatile u32 *)(avo_base1 + 4*2 ))
#define avo_chroma_freq3        (*(volatile u32 *)(avo_base1 + 4*3 ))
#define avo_chroma_phase        (*(volatile u32 *)(avo_base1 + 4*4 ))

#define avo_black_level_h       (*(volatile u32 *)(avo_base1 + 4*14))
#define avo_black_level_l       (*(volatile u32 *)(avo_base1 + 4*15))
#define avo_blank_level_h       (*(volatile u32 *)(avo_base1 + 4*16))
#define avo_blank_level_l       (*(volatile u32 *)(avo_base1 + 4*17))
#define avo_white_level_h       (*(volatile u32 *)(avo_base1 + 4*30))
#define avo_white_level_l       (*(volatile u32 *)(avo_base1 + 4*31))

#define avo_hsync_width         (*(volatile u32 *)(avo_base1 + 4*8 ))
#define avo_burst_width         (*(volatile u32 *)(avo_base1 + 4*9 ))
#define avo_back_porch          (*(volatile u32 *)(avo_base1 + 4*10))
#define avo_breeze_way          (*(volatile u32 *)(avo_base1 + 4*41))
#define avo_front_porch         (*(volatile u32 *)(avo_base1 + 4*44))

#define avo_start_line          (*(volatile u32 *)(avo_base1 + 4*51))
#define avo_num_lines_h         (*(volatile u32 *)(avo_base1 + 4*23))
#define avo_num_lines_l         (*(volatile u32 *)(avo_base1 + 4*24))
#define avo_line_period_h       (*(volatile u32 *)(avo_base1 + 4*49))
#define avo_line_period_l       (*(volatile u32 *)(avo_base1 + 4*50))

#define avo_cb_burst_amp        (*(volatile u32 *)(avo_base1 + 4*11))
#define avo_cr_burst_amp        (*(volatile u32 *)(avo_base1 + 4*12))
#define avo_cb_gain             (*(volatile u32 *)(avo_base1 + 4*32))
#define avo_cr_gain             (*(volatile u32 *)(avo_base1 + 4*34))

//==============================================================//
//  mipi
//==============================================================//
#define mipi_base   eva_base + 0x4000
#define csi_base    (mipi_base + 0x0000)
#define dsi_s_base  (mipi_base + 0x0200)
#define dsi_d_base  (mipi_base + 0x0300)
#define mp_phy_base (mipi_base + 0x0400)

#define csi_sys_con                 (*(volatile u32 *)(csi_base + 0x00*4))
#define csi_rmap_con                (*(volatile u32 *)(csi_base + 0x01*4))
#define csi_lane_con                (*(volatile u32 *)(csi_base + 0x02*4))
#define csi_tval_con                (*(volatile u32 *)(csi_base + 0x03*4))
#define csi_task_con                (*(volatile u32 *)(csi_base + 0x04*4))
#define csi_task_haw                (*(volatile u32 *)(csi_base + 0x05*4))

#define dsi_sys_con                 (*(volatile u32 *)(dsi_s_base + 0x00*4))
#define dsi_vdo_cfg                 (*(volatile u32 *)(dsi_s_base + 0x10*4))
#define dsi_vdo_vsa                 (*(volatile u32 *)(dsi_s_base + 0x11*4))
#define dsi_vdo_vbp                 (*(volatile u32 *)(dsi_s_base + 0x12*4))
#define dsi_vdo_vact                (*(volatile u32 *)(dsi_s_base + 0x13*4))
#define dsi_vdo_vfp                 (*(volatile u32 *)(dsi_s_base + 0x14*4))
#define dsi_vdo_hsa                 (*(volatile u32 *)(dsi_s_base + 0x15*4))
#define dsi_vdo_hbp                 (*(volatile u32 *)(dsi_s_base + 0x16*4))
#define dsi_vdo_hact                (*(volatile u32 *)(dsi_s_base + 0x17*4))
#define dsi_vdo_hfp                 (*(volatile u32 *)(dsi_s_base + 0x18*4))
#define dsi_vdo_null                (*(volatile u32 *)(dsi_s_base + 0x19*4))
#define dsi_vdo_bllp                (*(volatile u32 *)(dsi_s_base + 0x1a*4))
#define dsi_tval_con0               (*(volatile u32 *)(dsi_s_base + 0x20*4))
#define dsi_tval_con1               (*(volatile u32 *)(dsi_s_base + 0x21*4))
#define dsi_tval_con2               (*(volatile u32 *)(dsi_s_base + 0x22*4))
#define dsi_tval_con3               (*(volatile u32 *)(dsi_s_base + 0x23*4))
#define dsi_tval_con4               (*(volatile u32 *)(dsi_s_base + 0x24*4))
#define dsi_rmap_con                (*(volatile u32 *)(dsi_s_base + 0x25*4))

#define dsi_task_con                (*(volatile u32 *)(dsi_d_base + 0x00*4))
#define dsi_bus_con                 (*(volatile u32 *)(dsi_d_base + 0x01*4))
#define dsi_cmd_con0                (*(volatile u32 *)(dsi_d_base + 0x02*4))
#define dsi_cmd_con1                (*(volatile u32 *)(dsi_d_base + 0x03*4))
#define dsi_cmd_con2                (*(volatile u32 *)(dsi_d_base + 0x04*4))
#define dsi_cmd_fifo                (*(volatile u32 *)(dsi_d_base + 0x05*4))
#define dsi_lane_con                (*(volatile u32 *)(dsi_d_base + 0x06*4))

#define mipi_pll_con0               (*(volatile u32 *)(mp_phy_base + 0x00*4))
#define mipi_pll_con1               (*(volatile u32 *)(mp_phy_base + 0x01*4))
#define mipi_phy_con0               (*(volatile u32 *)(mp_phy_base + 0x10*4))
#define mipi_phy_con1               (*(volatile u32 *)(mp_phy_base + 0x11*4))
#define mipi_phy_con2               (*(volatile u32 *)(mp_phy_base + 0x12*4))
#define mipi_phy_con3               (*(volatile u32 *)(mp_phy_base + 0x13*4))
#define mipi_phy_con4               (*(volatile u32 *)(mp_phy_base + 0x14*4))
#define mipi_phy_con5               (*(volatile u32 *)(mp_phy_base + 0x15*4))
#define mipi_phy_con6               (*(volatile u32 *)(mp_phy_base + 0x16*4))
#define mipi_phy_con7               (*(volatile u32 *)(mp_phy_base + 0x17*4))
#define mipi_phy_con8               (*(volatile u32 *)(mp_phy_base + 0x18*4))
#define mipi_phy_con9               (*(volatile u32 *)(mp_phy_base + 0x19*4))
#define mipi_phy_con10              (*(volatile u32 *)(mp_phy_base + 0x1a*4))
#define mipi_phy_con11              (*(volatile u32 *)(mp_phy_base + 0x1b*4))
#define mipi_phy_con12              (*(volatile u32 *)(mp_phy_base + 0x1c*4))
#define mipi_phy_con13              (*(volatile u32 *)(mp_phy_base + 0x1d*4))
#define mipi_phy_con14              (*(volatile u32 *)(mp_phy_base + 0x1e*4))
#define mipi_phy_con15              (*(volatile u32 *)(mp_phy_base + 0x1f*4))

#define OETH_REG_BASE           0x3f88000
#define OETH_REG_BASE_ADD       0x400
#define OETH_BD_BASE            (OETH_REG_BASE + OETH_REG_BASE_ADD)
#define OETH_TOTAL_BD           128
#endif





