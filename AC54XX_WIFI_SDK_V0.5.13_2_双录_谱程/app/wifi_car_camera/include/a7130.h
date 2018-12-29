#include "printf.h"
#include "cpu.h"
#include "asm/spi.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"
#include "generic/gpio.h"
#include "generic/typedef.h"
#include "asm/port_waked_up.h"

// #include "lbuf.h"
#include "A7130reg.h"
#define A7130_GIO1_PORT  IO_PORTB_11




static const u8 BitCount_Tab[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
static const u8 ID_Tab[8] = {0x34, 0x75, 0xC5, 0x2A, 0xC7, 0x33, 0x45, 0xEA}; //ID code
static const u8  PageTab[8] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70}; //page select
static const u8  PN9_Tab[] = {
    0xFF, 0x83, 0xDF, 0x17, 0x32, 0x09, 0x4E, 0xD1,
    0xE7, 0xCD, 0x8A, 0x91, 0xC6, 0xD5, 0xC4, 0xC4,
    0x40, 0x21, 0x18, 0x4E, 0x55, 0x86, 0xF4, 0xDC,
    0x8A, 0x15, 0xA7, 0xEC, 0x92, 0xDF, 0x93, 0x53,
    0x30, 0x18, 0xCA, 0x34, 0xBF, 0xA2, 0xC7, 0x59,
    0x67, 0x8F, 0xBA, 0x0D, 0x6D, 0xD8, 0x2D, 0x7D,
    0x54, 0x0A, 0x57, 0x97, 0x70, 0x39, 0xD2, 0x7A,
    0xEA, 0x24, 0x33, 0x85, 0xED, 0x9A, 0x1D, 0xE0
};	// This table are 64bytes PN9 pseudo random code.

const u8 A7130Config[] = {
    0x00, //RESET register,			only reset, not use on config
    0x62, //MODE register,
    0x00, //CALIBRATION register,
    // 0x3F, //FIFO1 register,
    0x3F, //FIFO1 register,

    // 0x00, //FIFO2 register,
    0xC0, //FIFO2 register,
    0x00, //FIFO register,			for fifo read/write
    0x00, //IDDATA register,		for idcode
    0x00, //RCOSC1 register,
    0x00, //RCOSC2 register,
    0x00, //RCOSC3 register,
    0x00, //CKO register,
    // 0x12, //CKO register,
    0x01, //GPIO1 register
    0x37, //GPIO2 register,
    // 0x1D, //GPIO2 register,
    0x9F, //DATARATECLOCK register,
    0x00, //PLL1 register,
    0x0E, //PLL2 register, 			RFbase 2400.001MHz
    0x96, //PLL3 register,
    0x00, //PLL4 register,
    0x04, //PLL5 register,
    0x3C, //ChannelGroup1 register,
    0x78, //ChannelGroup1 register,
    0xAF, //TX1 register,
    0x40, //TX2 register,
    0x10, //DELAY1 register,
    // 0x00, //DELAY2 register,
    0x60, //DELAY2 register,
    0x72, //RX register,
    0x6F, //RXGAIN1 register,
    0xC2, //RXGAIN2 register,
    0x3D, //RXGAIN3 register,
    0xE7, //RXGAIN4 register,
    0x00, //RSSI register,
    0xF1, //ADC register,
    0x07, //CODE1 register,
    0x06, //CODE2 register,
    0x2A, //CODE3 register,
    0x60, //IFCAL1 register,
    0xFF, //IFCAL2 register,		**0x7F setting for 17dBm TX power module**
    // 0x7F, //IFCAL2 register,		**0x7F setting for 17dBm TX power module**
    0x80, //VCOCCAL register,
    0xD0, //VCOCAL1 register,
    0x00, //VCOCAL2 register,
    0x70, //VCO deviation 1 register,
    0x00, //VCO deviation 2 register,
    0x00, //DSA register,
    0xDC, //VCO Modulation delay register,
    0xF0, //BATTERY register,
    0x37, //TXTEST register,
    0x47, //RXDEM1 register,
    0xF7, //RXDEM2 register,
    0xF0, //CPC1 register,
    0x37, //CPC2 register,
    0x55, //CRYSTAL register,
    0x15, //PLLTEST register,
    0x15, //VCOTEST register,
    0x00, //RF Analog register,
    0x00, //Key data register,
    0x77, //Channel select register,
    0x00, //ROM register,
    0x00, //DataRate register,
    0x00, //FCR register,
    0x00, //ARD register,
    0x00, //AFEP register,
    0x00, //FCB register,
    0x00, //KEYC register,
    0x00  //USID register,
};

const u8 A7130_Addr2A_Config[] = {
    // 0x74, //page0,					**0x74 setting for 17dBm TX power module**
    0x34, //page0,					**0x74 setting for 17dBm TX power module**
    0x49, //page1,
    0x00, //Page2,
    0x80, //page3,
    0x80, //page4,
    0x00, //page5,
    0x0A, //page6,
    0x00, //page7,
};

const u8 A7130_Addr38_Config[] = {
    0x00, //page0,
    0x10, //page1,
    0x20, //page2,
    0x24, //page3,
    0x20, //page4,
};
