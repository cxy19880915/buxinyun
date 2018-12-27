#ifndef __USB_INC_H__
#define __USB_INC_H__
#include "asm/cpu.h"

#if 0
#define HUSB0_EP5_TADR           (*(volatile u32 *)(ls_husb_base + 0x0f*4))      //26bit write only;
#define HUSB0_EP5_RADR           (*(volatile u32 *)(ls_husb_base + 0x10*4))      //26bit write only;

#define HUSB0_EP6_TADR           (*(volatile u32 *)(ls_husb_base + 0x11*4))      //26bit write only;
#define HUSB0_EP6_RADR           (*(volatile u32 *)(ls_husb_base + 0x12*4))      //26bit write only;

#endif
#define H0_EP5TXMAXP     (*(volatile u16 *)(husb0_base + 0x150))
#define H0_EP5TXCSR      (*(volatile u16 *)(husb0_base + 0x152))
#define H0_EP5RXMAXP     (*(volatile u16 *)(husb0_base + 0x154))
#define H0_EP5RXCSR      (*(volatile u16 *)(husb0_base + 0x156))
#define H0_EP5RXCOUNT    (*(volatile u16 *)(husb0_base + 0x158))
#define H0_EP5TXTYPE     (*(volatile u8  *)(husb0_base + 0x15a))
#define H0_EP5TXINTERVAL (*(volatile u8  *)(husb0_base + 0x15b))
#define H0_EP5RXTYPE     (*(volatile u8  *)(husb0_base + 0x15c))
#define H0_EP5RXINTERVAL (*(volatile u8  *)(husb0_base + 0x15d))
#define H0_EP5FIFOSIZE   (*(volatile u8  *)(husb0_base + 0x15f))




#define H0_EP6TXMAXP     (*(volatile u16 *)(husb0_base + 0x160))
#define H0_EP6TXCSR      (*(volatile u16 *)(husb0_base + 0x162))
#define H0_EP6RXMAXP     (*(volatile u16 *)(husb0_base + 0x164))
#define H0_EP6RXCSR      (*(volatile u16 *)(husb0_base + 0x166))
#define H0_EP6RXCOUNT    (*(volatile u16 *)(husb0_base + 0x168))
#define H0_EP6TXTYPE     (*(volatile u8  *)(husb0_base + 0x16a))
#define H0_EP6TXINTERVAL (*(volatile u8  *)(husb0_base + 0x16b))
#define H0_EP6RXTYPE     (*(volatile u8  *)(husb0_base + 0x16c))
#define H0_EP6RXINTERVAL (*(volatile u8  *)(husb0_base + 0x16d))
#define H0_EP6FIFOSIZE   (*(volatile u8  *)(husb0_base + 0x16f))



#define H0_EP0_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x080))
#define H0_EP0_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x082))
#define H0_EP0_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x083))
#define H0_EP0_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x084))
#define H0_EP0_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x086))
#define H0_EP0_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x087))

#define H0_EP1_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x088))
#define H0_EP1_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x08a))
#define H0_EP1_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x08b))
#define H0_EP1_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x08c))
#define H0_EP1_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x08e))
#define H0_EP1_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x08f))

#define H0_EP2_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x090))
#define H0_EP2_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x092))
#define H0_EP2_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x093))
#define H0_EP2_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x094))
#define H0_EP2_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x096))
#define H0_EP2_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x097))

#define H0_EP3_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x098))
#define H0_EP3_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x09a))
#define H0_EP3_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x09b))
#define H0_EP3_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x09c))
#define H0_EP3_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x09e))
#define H0_EP3_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x09f))

#define H0_EP4_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x0a0))

#define H0_EP4_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x0a2))
#define H0_EP4_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x0a3))
#define H0_EP4_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x0a4))
#define H0_EP4_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x0a6))
#define H0_EP4_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x0a7))

#define H0_EP5_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x0a8))

#define H0_EP5_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x0aa))
#define H0_EP5_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x0ab))
#define H0_EP5_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x0ac))
#define H0_EP5_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x0ae))
#define H0_EP5_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x0af))

#define H0_EP6_TXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x0b0))

#define H0_EP6_TXHUBADDR    (*(volatile u8  *)(husb0_base + 0x0b2))
#define H0_EP6_TXHUBPORT    (*(volatile u8  *)(husb0_base + 0x0b3))
#define H0_EP6_RXFUNCADDR   (*(volatile u8  *)(husb0_base + 0x0b4))
#define H0_EP6_RXHUBADDR    (*(volatile u8  *)(husb0_base + 0x0b6))
#define H0_EP6_RXHUBPORT    (*(volatile u8  *)(husb0_base + 0x0b7))

//...........  High Speed USB .....................

#define HUSB0_FADDR                        &H0_FADDR
#define HUSB0_POWER                        &H0_POWER
#define HUSB0_INTRTX                       &H0_INTRTX
#define HUSB0_INTRRX                       &H0_INTRRX
#define HUSB0_INTRTXE                      &H0_INTRTXE
#define HUSB0_INTRRXE                      &H0_INTRRXE
#define HUSB0_INTRUSB                      &H0_INTRUSB
#define HUSB0_INTRUSBE                     &H0_INTRUSBE
#define HUSB0_FRAME                        &H0_FRAME
#define HUSB0_INDEX                        &H0_INDEX
#define HUSB0_TESTMODE                     &H0_TESTMODE

#define HUSB0_FIFO0                        &H0_FIFO0
#define HUSB0_FIFO1                        &H0_FIFO1
#define HUSB0_FIFO2                        &H0_FIFO2
#define HUSB0_FIFO3                        &H0_FIFO3
#define HUSB0_FIFO4                        &H0_FIFO4
#define HUSB0_DEVCTL                       &H0_DEVCTL
#define HUSB0_TYPE0						   &H0_TYPE0

#define HUSB0_EP0_TXFUNCADDR                 &H0_EP0_TXFUNCADDR
#define HUSB0_EP0_TXHUBADDR             	 &H0_EP0_TXHUBADDR
#define HUSB0_EP0_TXHUBPORT              	 &H0_EP0_TXHUBPORT
#define HUSB0_EP0_RXFUNCADDR             	 &H0_EP0_RXFUNCADDR
#define HUSB0_EP0_RXHUBADDR              	 &H0_EP0_RXHUBADDR
#define HUSB0_EP0_RXHUBPORT              	 &H0_EP0_RXHUBPORT

#define HUSB0_EP1_TXFUNCADDR                 &H0_EP1_TXFUNCADDR
#define HUSB0_EP1_TXHUBADDR             	 &H0_EP1_TXHUBADDR
#define HUSB0_EP1_TXHUBPORT              	 &H0_EP1_TXHUBPORT
#define HUSB0_EP1_RXFUNCADDR             	 &H0_EP1_RXFUNCADDR
#define HUSB0_EP1_RXHUBADDR              	 &H0_EP1_RXHUBADDR
#define HUSB0_EP1_RXHUBPORT              	 &H0_EP1_RXHUBPORT

#define HUSB0_EP2_TXFUNCADDR                 &H0_EP2_TXFUNCADDR
#define HUSB0_EP2_TXHUBADDR             	 &H0_EP2_TXHUBADDR
#define HUSB0_EP2_TXHUBPORT              	 &H0_EP2_TXHUBPORT
#define HUSB0_EP2_RXFUNCADDR             	 &H0_EP2_RXFUNCADDR
#define HUSB0_EP2_RXHUBADDR              	 &H0_EP2_RXHUBADDR
#define HUSB0_EP2_RXHUBPORT              	 &H0_EP2_RXHUBPORT

#define HUSB0_EP3_TXFUNCADDR                 &H0_EP3_TXFUNCADDR
#define HUSB0_EP3_TXHUBADDR             	 &H0_EP3_TXHUBADDR
#define HUSB0_EP3_TXHUBPORT              	 &H0_EP3_TXHUBPORT
#define HUSB0_EP3_RXFUNCADDR             	 &H0_EP3_RXFUNCADDR
#define HUSB0_EP3_RXHUBADDR              	 &H0_EP3_RXHUBADDR
#define HUSB0_EP3_RXHUBPORT              	 &H0_EP3_RXHUBPORT

#define HUSB0_EP4_TXFUNCADDR                 &H0_EP4_TXFUNCADDR
#define HUSB0_EP4_TXHUBADDR             	 &H0_EP4_TXHUBADDR
#define HUSB0_EP4_TXHUBPORT              	 &H0_EP4_TXHUBPORT
#define HUSB0_EP4_RXFUNCADDR             	 &H0_EP4_RXFUNCADDR
#define HUSB0_EP4_RXHUBADDR              	 &H0_EP4_RXHUBADDR
#define HUSB0_EP4_RXHUBPORT              	 &H0_EP4_RXHUBPORT

#define HUSB0_EP5_TXFUNCADDR                 &H0_EP5_TXFUNCADDR
#define HUSB0_EP5_TXHUBADDR             	 &H0_EP5_TXHUBADDR
#define HUSB0_EP5_TXHUBPORT              	 &H0_EP5_TXHUBPORT
#define HUSB0_EP5_RXFUNCADDR             	 &H0_EP5_RXFUNCADDR
#define HUSB0_EP5_RXHUBADDR              	 &H0_EP5_RXHUBADDR
#define HUSB0_EP5_RXHUBPORT              	 &H0_EP5_RXHUBPORT

#define HUSB0_EP6_TXFUNCADDR                 &H0_EP6_TXFUNCADDR
#define HUSB0_EP6_TXHUBADDR             	 &H0_EP6_TXHUBADDR
#define HUSB0_EP6_TXHUBPORT              	 &H0_EP6_TXHUBPORT
#define HUSB0_EP6_RXFUNCADDR             	 &H0_EP6_RXFUNCADDR
#define HUSB0_EP6_RXHUBADDR              	 &H0_EP6_RXHUBADDR
#define HUSB0_EP6_RXHUBPORT              	 &H0_EP6_RXHUBPORT

#define HUSB0_CSR0                         &H0_CSR0
#define HUSB0_COUNT0                       &H0_COUNT0
#define HUSB0_NAKLIMIT0                    &H0_NAKLIMIT0
#define HUSB0_CFGDATA                      &H0_CFGDATA

#define H0_EP0TXMAXP			(*(volatile u16 *)(husb0_base + 0x100))
#define HUSB0_EP0TXMAXP                    &H0_EP0TXMAXP

#define HUSB0_EP1TXMAXP                    &H0_EP1TXMAXP
#define HUSB0_EP1TXCSR                     &H0_EP1TXCSR
#define HUSB0_EP1RXMAXP                    &H0_EP1RXMAXP
#define HUSB0_EP1RXCSR                     &H0_EP1RXCSR
#define HUSB0_EP1RXCOUNT                   &H0_EP1RXCOUNT
#define HUSB0_EP1TXTYPE                    &H0_EP1TXTYPE
#define HUSB0_EP1TXINTERVAL                &H0_EP1TXINTERVAL
#define HUSB0_EP1RXTYPE                    &H0_EP1RXTYPE
#define HUSB0_EP1RXINTERVAL                &H0_EP1RXINTERVAL
#define HUSB0_EP1FIFOSIZE                  &H0_EP1FIFOSIZE

#define HUSB0_EP2TXMAXP                    &H0_EP2TXMAXP
#define HUSB0_EP2TXCSR                     &H0_EP2TXCSR
#define HUSB0_EP2RXMAXP                    &H0_EP2RXMAXP
#define HUSB0_EP2RXCSR                     &H0_EP2RXCSR
#define HUSB0_EP2RXCOUNT                   &H0_EP2RXCOUNT
#define HUSB0_EP2TXTYPE                    &H0_EP2TXTYPE
#define HUSB0_EP2TXINTERVAL                &H0_EP2TXINTERVAL
#define HUSB0_EP2RXTYPE                    &H0_EP2RXTYPE
#define HUSB0_EP2RXINTERVAL                &H0_EP2RXINTERVAL
#define HUSB0_EP2FIFOSIZE                  &H0_EP2FIFOSIZE

#define HUSB0_EP3TXMAXP                    &H0_EP3TXMAXP
#define HUSB0_EP3TXCSR                     &H0_EP3TXCSR
#define HUSB0_EP3RXMAXP                    &H0_EP3RXMAXP
#define HUSB0_EP3RXCSR                     &H0_EP3RXCSR
#define HUSB0_EP3RXCOUNT                   &H0_EP3RXCOUNT
#define HUSB0_EP3TXTYPE                    &H0_EP3TXTYPE
#define HUSB0_EP3TXINTERVAL                &H0_EP3TXINTERVAL
#define HUSB0_EP3RXTYPE                    &H0_EP3RXTYPE
#define HUSB0_EP3RXINTERVAL                &H0_EP3RXINTERVAL
#define HUSB0_EP3FIFOSIZE                  &H0_EP3FIFOSIZE

#define HUSB0_EP4TXMAXP                    &H0_EP4TXMAXP
#define HUSB0_EP4TXCSR                     &H0_EP4TXCSR
#define HUSB0_EP4RXMAXP                    &H0_EP4RXMAXP
#define HUSB0_EP4RXCSR                     &H0_EP4RXCSR
#define HUSB0_EP4RXCOUNT                   &H0_EP4RXCOUNT
#define HUSB0_EP4TXTYPE                    &H0_EP4TXTYPE
#define HUSB0_EP4TXINTERVAL                &H0_EP4TXINTERVAL
#define HUSB0_EP4RXTYPE                    &H0_EP4RXTYPE
#define HUSB0_EP4RXINTERVAL                &H0_EP4RXINTERVAL
#define HUSB0_EP4FIFOSIZE                  &H0_EP4FIFOSIZE

#define HUSB0_EP5TXMAXP                    &H0_EP5TXMAXP
#define HUSB0_EP5TXCSR                     &H0_EP5TXCSR
#define HUSB0_EP5RXMAXP                    &H0_EP5RXMAXP
#define HUSB0_EP5RXCSR                     &H0_EP5RXCSR
#define HUSB0_EP5RXCOUNT                   &H0_EP5RXCOUNT
#define HUSB0_EP5TXTYPE                    &H0_EP5TXTYPE
#define HUSB0_EP5TXINTERVAL                &H0_EP5TXINTERVAL
#define HUSB0_EP5RXTYPE                    &H0_EP5RXTYPE
#define HUSB0_EP5RXINTERVAL                &H0_EP5RXINTERVAL
#define HUSB0_EP5FIFOSIZE                  &H0_EP5FIFOSIZE

#define HUSB0_EP6TXMAXP                    &H0_EP6TXMAXP
#define HUSB0_EP6TXCSR                     &H0_EP6TXCSR
#define HUSB0_EP6RXMAXP                    &H0_EP6RXMAXP
#define HUSB0_EP6RXCSR                     &H0_EP6RXCSR
#define HUSB0_EP6RXCOUNT                   &H0_EP6RXCOUNT
#define HUSB0_EP6TXTYPE                    &H0_EP6TXTYPE
#define HUSB0_EP6TXINTERVAL                &H0_EP6TXINTERVAL
#define HUSB0_EP6RXTYPE                    &H0_EP6RXTYPE
#define HUSB0_EP6RXINTERVAL                &H0_EP6RXINTERVAL
#define HUSB0_EP6FIFOSIZE                  &H0_EP6FIFOSIZE

#define HUSB0_C_T_UCH                      &H0_C_T_UCH

#define H1_EP5TXMAXP     (*(volatile u16 *)(husb1_base + 0x150))
#define H1_EP5TXCSR      (*(volatile u16 *)(husb1_base + 0x152))
#define H1_EP5RXMAXP     (*(volatile u16 *)(husb1_base + 0x154))
#define H1_EP5RXCSR      (*(volatile u16 *)(husb1_base + 0x156))
#define H1_EP5RXCOUNT    (*(volatile u16 *)(husb1_base + 0x158))
#define H1_EP5TXTYPE     (*(volatile u8  *)(husb1_base + 0x15a))
#define H1_EP5TXINTERVAL (*(volatile u8  *)(husb1_base + 0x15b))
#define H1_EP5RXTYPE     (*(volatile u8  *)(husb1_base + 0x15c))
#define H1_EP5RXINTERVAL (*(volatile u8  *)(husb1_base + 0x15d))
#define H1_EP5FIFOSIZE   (*(volatile u8  *)(husb1_base + 0x15f))
#define H1_EP6TXMAXP     (*(volatile u16 *)(husb1_base + 0x160))
#define H1_EP6TXCSR      (*(volatile u16 *)(husb1_base + 0x162))
#define H1_EP6RXMAXP     (*(volatile u16 *)(husb1_base + 0x164))
#define H1_EP6RXCSR      (*(volatile u16 *)(husb1_base + 0x166))
#define H1_EP6RXCOUNT    (*(volatile u16 *)(husb1_base + 0x168))
#define H1_EP6TXTYPE     (*(volatile u8  *)(husb1_base + 0x16a))
#define H1_EP6TXINTERVAL (*(volatile u8  *)(husb1_base + 0x16b))
#define H1_EP6RXTYPE     (*(volatile u8  *)(husb1_base + 0x16c))
#define H1_EP6RXINTERVAL (*(volatile u8  *)(husb1_base + 0x16d))
#define H1_EP6FIFOSIZE   (*(volatile u8  *)(husb1_base + 0x16f))

#define H1_EP0_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x080))
#define H1_EP0_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x082))
#define H1_EP0_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x083))
#define H1_EP0_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x084))
#define H1_EP0_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x086))
#define H1_EP0_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x087))
#define H1_EP1_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x088))
#define H1_EP1_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x08a))
#define H1_EP1_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x08b))
#define H1_EP1_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x08c))
#define H1_EP1_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x08e))
#define H1_EP1_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x08f))
#define H1_EP2_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x090))
#define H1_EP2_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x092))
#define H1_EP2_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x093))
#define H1_EP2_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x094))
#define H1_EP2_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x096))
#define H1_EP2_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x097))
#define H1_EP3_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x098))
#define H1_EP3_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x09a))
#define H1_EP3_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x09b))
#define H1_EP3_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x09c))
#define H1_EP3_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x09e))
#define H1_EP3_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x09f))
#define H1_EP4_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x0a0))
#define H1_EP4_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x0a2))
#define H1_EP4_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x0a3))
#define H1_EP4_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x0a4))
#define H1_EP4_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x0a6))
#define H1_EP4_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x0a7))
#define H1_EP5_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x0a8))
#define H1_EP5_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x0aa))
#define H1_EP5_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x0ab))
#define H1_EP5_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x0ac))
#define H1_EP5_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x0ae))
#define H1_EP5_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x0af))
#define H1_EP6_TXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x0b0))
#define H1_EP6_TXHUBADDR    (*(volatile u8  *)(husb1_base + 0x0b2))
#define H1_EP6_TXHUBPORT    (*(volatile u8  *)(husb1_base + 0x0b3))
#define H1_EP6_RXFUNCADDR   (*(volatile u8  *)(husb1_base + 0x0b4))
#define H1_EP6_RXHUBADDR    (*(volatile u8  *)(husb1_base + 0x0b6))
#define H1_EP6_RXHUBPORT    (*(volatile u8  *)(husb1_base + 0x0b7))

#define HUSB1_FADDR                        &H1_FADDR
#define HUSB1_POWER                        &H1_POWER
#define HUSB1_INTRTX                       &H1_INTRTX
#define HUSB1_INTRRX                       &H1_INTRRX
#define HUSB1_INTRTXE                      &H1_INTRTXE
#define HUSB1_INTRRXE                      &H1_INTRRXE
#define HUSB1_INTRUSB                      &H1_INTRUSB
#define HUSB1_INTRUSBE                     &H1_INTRUSBE
#define HUSB1_FRAME                        &H1_FRAME
#define HUSB1_INDEX                        &H1_INDEX
#define HUSB1_TESTMODE                     &H1_TESTMODE

#define HUSB1_FIFO0                        &H1_FIFO0
#define HUSB1_FIFO1                        &H1_FIFO1
#define HUSB1_FIFO2                        &H1_FIFO2
#define HUSB1_FIFO3                        &H1_FIFO3
#define HUSB1_FIFO4                        &H1_FIFO4
#define HUSB1_DEVCTL                       &H1_DEVCTL

#define HUSB1_CSR0                         &H1_CSR0
#define HUSB1_COUNT0                       &H1_COUNT0
#define HUSB1_NAKLIMIT0                    &H1_NAKLIMIT0
#define HUSB1_CFGDATA                      &H1_CFGDATA

#define H1_EP0TXMAXP			(*(volatile u16 *)(husb1_base + 0x100))
#define HUSB1_EP0TXMAXP                    &H1_EP0TXMAXP
#define HUSB1_EP1TXMAXP                    &H1_EP1TXMAXP
#define HUSB1_EP1TXCSR                     &H1_EP1TXCSR
#define HUSB1_EP1RXMAXP                    &H1_EP1RXMAXP
#define HUSB1_EP1RXCSR                     &H1_EP1RXCSR
#define HUSB1_EP1RXCOUNT                   &H1_EP1RXCOUNT
#define HUSB1_EP1TXTYPE                    &H1_EP1TXTYPE
#define HUSB1_EP1TXINTERVAL                &H1_EP1TXINTERVAL
#define HUSB1_EP1RXTYPE                    &H1_EP1RXTYPE
#define HUSB1_EP1RXINTERVAL                &H1_EP1RXINTERVAL
#define HUSB1_EP1FIFOSIZE                  &H1_EP1FIFOSIZE

#define HUSB1_EP2TXMAXP                    &H1_EP2TXMAXP
#define HUSB1_EP2TXCSR                     &H1_EP2TXCSR
#define HUSB1_EP2RXMAXP                    &H1_EP2RXMAXP
#define HUSB1_EP2RXCSR                     &H1_EP2RXCSR
#define HUSB1_EP2RXCOUNT                   &H1_EP2RXCOUNT
#define HUSB1_EP2TXTYPE                    &H1_EP2TXTYPE
#define HUSB1_EP2TXINTERVAL                &H1_EP2TXINTERVAL
#define HUSB1_EP2RXTYPE                    &H1_EP2RXTYPE
#define HUSB1_EP2RXINTERVAL                &H1_EP2RXINTERVAL
#define HUSB1_EP2FIFOSIZE                  &H1_EP2FIFOSIZE

#define HUSB1_EP3TXMAXP                    &H1_EP3TXMAXP
#define HUSB1_EP3TXCSR                     &H1_EP3TXCSR
#define HUSB1_EP3RXMAXP                    &H1_EP3RXMAXP
#define HUSB1_EP3RXCSR                     &H1_EP3RXCSR
#define HUSB1_EP3RXCOUNT                   &H1_EP3RXCOUNT
#define HUSB1_EP3TXTYPE                    &H1_EP3TXTYPE
#define HUSB1_EP3TXINTERVAL                &H1_EP3TXINTERVAL
#define HUSB1_EP3RXTYPE                    &H1_EP3RXTYPE
#define HUSB1_EP3RXINTERVAL                &H1_EP3RXINTERVAL
#define HUSB1_EP3FIFOSIZE                  &H1_EP3FIFOSIZE

#define HUSB1_EP4TXMAXP                    &H1_EP4TXMAXP
#define HUSB1_EP4TXCSR                     &H1_EP4TXCSR
#define HUSB1_EP4RXMAXP                    &H1_EP4RXMAXP
#define HUSB1_EP4RXCSR                     &H1_EP4RXCSR
#define HUSB1_EP4RXCOUNT                   &H1_EP4RXCOUNT
#define HUSB1_EP4TXTYPE                    &H1_EP4TXTYPE
#define HUSB1_EP4TXINTERVAL                &H1_EP4TXINTERVAL
#define HUSB1_EP4RXTYPE                    &H1_EP4RXTYPE
#define HUSB1_EP4RXINTERVAL                &H1_EP4RXINTERVAL
#define HUSB1_EP4FIFOSIZE                  &H1_EP4FIFOSIZE

#define HUSB1_EP5TXMAXP                    &H1_EP5TXMAXP
#define HUSB1_EP5TXCSR                     &H1_EP5TXCSR
#define HUSB1_EP5RXMAXP                    &H1_EP5RXMAXP
#define HUSB1_EP5RXCSR                     &H1_EP5RXCSR
#define HUSB1_EP5RXCOUNT                   &H1_EP5RXCOUNT
#define HUSB1_EP5TXTYPE                    &H1_EP5TXTYPE
#define HUSB1_EP5TXINTERVAL                &H1_EP5TXINTERVAL
#define HUSB1_EP5RXTYPE                    &H1_EP5RXTYPE
#define HUSB1_EP5RXINTERVAL                &H1_EP5RXINTERVAL
#define HUSB1_EP5FIFOSIZE                  &H1_EP5FIFOSIZE

#define HUSB1_EP6TXMAXP                    &H1_EP6TXMAXP
#define HUSB1_EP6TXCSR                     &H1_EP6TXCSR
#define HUSB1_EP6RXMAXP                    &H1_EP6RXMAXP
#define HUSB1_EP6RXCSR                     &H1_EP6RXCSR
#define HUSB1_EP6RXCOUNT                   &H1_EP6RXCOUNT
#define HUSB1_EP6TXTYPE                    &H1_EP6TXTYPE
#define HUSB1_EP6TXINTERVAL                &H1_EP6TXINTERVAL
#define HUSB1_EP6RXTYPE                    &H1_EP6RXTYPE
#define HUSB1_EP6RXINTERVAL                &H1_EP6RXINTERVAL
#define HUSB1_EP6FIFOSIZE                  &H1_EP6FIFOSIZE
#define HUSB1_C_T_UCH

#define HUSB1_EP0_TXFUNCADDR                 &H1_EP0_TXFUNCADDR
#define HUSB1_EP0_TXHUBADDR             	 &H1_EP0_TXHUBADDR
#define HUSB1_EP0_TXHUBPORT              	 &H1_EP0_TXHUBPORT
#define HUSB1_EP0_RXFUNCADDR             	 &H1_EP0_RXFUNCADDR
#define HUSB1_EP0_RXHUBADDR              	 &H1_EP0_RXHUBADDR
#define HUSB1_EP0_RXHUBPORT              	 &H1_EP0_RXHUBPORT

#define HUSB1_EP1_TXFUNCADDR                 &H1_EP1_TXFUNCADDR
#define HUSB1_EP1_TXHUBADDR             	 &H1_EP1_TXHUBADDR
#define HUSB1_EP1_TXHUBPORT              	 &H1_EP1_TXHUBPORT
#define HUSB1_EP1_RXFUNCADDR             	 &H1_EP1_RXFUNCADDR
#define HUSB1_EP1_RXHUBADDR              	 &H1_EP1_RXHUBADDR
#define HUSB1_EP1_RXHUBPORT              	 &H1_EP1_RXHUBPORT

#define HUSB1_EP2_TXFUNCADDR                 &H1_EP2_TXFUNCADDR
#define HUSB1_EP2_TXHUBADDR             	 &H1_EP2_TXHUBADDR
#define HUSB1_EP2_TXHUBPORT              	 &H1_EP2_TXHUBPORT
#define HUSB1_EP2_RXFUNCADDR             	 &H1_EP2_RXFUNCADDR
#define HUSB1_EP2_RXHUBADDR              	 &H1_EP2_RXHUBADDR
#define HUSB1_EP2_RXHUBPORT              	 &H1_EP2_RXHUBPORT

#define HUSB1_EP3_TXFUNCADDR                 &H1_EP3_TXFUNCADDR
#define HUSB1_EP3_TXHUBADDR             	 &H1_EP3_TXHUBADDR
#define HUSB1_EP3_TXHUBPORT              	 &H1_EP3_TXHUBPORT
#define HUSB1_EP3_RXFUNCADDR             	 &H1_EP3_RXFUNCADDR
#define HUSB1_EP3_RXHUBADDR              	 &H1_EP3_RXHUBADDR
#define HUSB1_EP3_RXHUBPORT              	 &H1_EP3_RXHUBPORT

#define HUSB1_EP4_TXFUNCADDR                 &H1_EP4_TXFUNCADDR
#define HUSB1_EP4_TXHUBADDR             	 &H1_EP4_TXHUBADDR
#define HUSB1_EP4_TXHUBPORT              	 &H1_EP4_TXHUBPORT
#define HUSB1_EP4_RXFUNCADDR             	 &H1_EP4_RXFUNCADDR
#define HUSB1_EP4_RXHUBADDR              	 &H1_EP4_RXHUBADDR
#define HUSB1_EP4_RXHUBPORT              	 &H1_EP4_RXHUBPORT

#define HUSB1_EP5_TXFUNCADDR                 &H1_EP5_TXFUNCADDR
#define HUSB1_EP5_TXHUBADDR             	 &H1_EP5_TXHUBADDR
#define HUSB1_EP5_TXHUBPORT              	 &H1_EP5_TXHUBPORT
#define HUSB1_EP5_RXFUNCADDR             	 &H1_EP5_RXFUNCADDR
#define HUSB1_EP5_RXHUBADDR              	 &H1_EP5_RXHUBADDR
#define HUSB1_EP5_RXHUBPORT              	 &H1_EP5_RXHUBPORT

#define HUSB1_EP6_TXFUNCADDR                 &H1_EP6_TXFUNCADDR
#define HUSB1_EP6_TXHUBADDR             	 &H1_EP6_TXHUBADDR
#define HUSB1_EP6_TXHUBPORT              	 &H1_EP6_TXHUBPORT
#define HUSB1_EP6_RXFUNCADDR             	 &H1_EP6_RXFUNCADDR
#define HUSB1_EP6_RXHUBADDR              	 &H1_EP6_RXHUBADDR
#define HUSB1_EP6_RXHUBPORT              	 &H1_EP6_RXHUBPORT
#endif
