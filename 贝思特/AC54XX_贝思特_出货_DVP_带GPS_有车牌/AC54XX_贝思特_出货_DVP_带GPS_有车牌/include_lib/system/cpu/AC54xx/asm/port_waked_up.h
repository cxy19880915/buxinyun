#ifndef  __PORT_WAKED_UP_H__
#define  __PORT_WAKED_UP_H__

void port_wakeup_init(void);

void  *port_wakeup_reg(void (*handler)(void *priv), void *priv);

void port_wakeup_unreg(void *hdl);



#endif  /*PORT_WAKED_UP_H*/
