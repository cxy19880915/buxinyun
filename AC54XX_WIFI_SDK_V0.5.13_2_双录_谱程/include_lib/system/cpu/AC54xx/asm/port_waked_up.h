#ifndef  __PORT_WAKED_UP_H__
#define  __PORT_WAKED_UP_H__

#include "typedef.h"

#define CLEAR_PORT_PEND()    WKUP_CON2 = 0xffff
#define READ_PORT_PEND()     WKUP_CON3
#define WKUP_ENABLE(x)       WKUP_CON0 |= BIT(x)
#define WKUP_DISABLE(x)      WKUP_CON0 &= (~BIT(x))
#define WKUP_EDGE(x,edg)     WKUP_CON1 = ((WKUP_CON1 & (~BIT(x))) | (edg? BIT(x): 0))
#define WKUP_CLEAR_PEND(x)   (WKUP_CON2 |= BIT(x))


typedef enum {
    EVENT_IO_0 = 0,
    EVENT_IO_1,
    EVENT_SDC0_DAT1,
    EVENT_SDC1_DAT1,
    EVENT_SDC2_DAT1,
    EVENT_UT0_RX,
    EVENT_UT1_RX,
    EVENT_UT2_RX,
    EVENT_UT3_RX,
    EVENT_IO_2,
    EVENT_PB03,
    EVENT_PB14,
    EVENT_PE4,
    EVENT_PE5,
    EVENT_PH7,
    EVENT_PH10,
    EVENT_PH14,
    EVENT_PH15,
} PORT_EVENT_E;

typedef enum {
    EDGE_POSITIVE = 0,
    EDGE_NEGATIVE,
} PORT_EDGE_E;

int port_wakeup_init(void);
void  *port_wakeup_reg(void (*handler)(void *priv), void *priv);

void port_wakeup_unreg(void *hdl);

void *Port_Wakeup_Reg(PORT_EVENT_E event, unsigned int gpio, PORT_EDGE_E edge, void (*handler)(void *priv), void *priv);
void Port_Wakeup_UnReg(void *hdl);


#endif  /*PORT_WAKED_UP_H*/
