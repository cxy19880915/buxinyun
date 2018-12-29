#ifndef SYS_TIMER_H
#define SYS_TIMER_H


#include "typedef.h"
#include "list.h"





struct static_sys_timer {
    void (*func)(void *priv);
    void *priv;
    u32 msec;
    u32 jiffies;
};


#define SYS_HI_TIMER_ADD(_func, _priv, _msec) \
	static struct static_sys_timer hi_timer sec(.hi_timer) = { \
		.func = _func, \
		.priv = _priv, \
		.msec = _msec, \
	}

extern struct static_sys_timer static_hi_timer_begin[];
extern struct static_sys_timer static_hi_timer_end[];

#define list_for_each_static_hi_timer(p) \
	for (p=static_hi_timer_begin; p<static_hi_timer_end; p++)



struct sys_cpu_timer {
    u8 busy;
    void *priv;
    void (*set)(u32 usec);
    void (*unset)();
};

#define DEFINE_SYS_CPU_TIMER(t) \
    struct sys_cpu_timer t sec(.sys_cpu_timer);

#define REGISTER_SYS_CPU_TIMER(t) \
    struct sys_cpu_timer t sec(.sys_cpu_timer)


extern struct sys_cpu_timer sys_cpu_timer_begin[];
extern struct sys_cpu_timer sys_cpu_timer_end[];

#define list_for_each_cpu_usec_timer(p) \
    for (p = sys_cpu_timer_begin; p < sys_cpu_timer_end; p++)



int sys_timer_add(void *priv, void (*func)(void *priv), u32 msec);

void sys_timer_del(int);

int sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);

void sys_timeout_del(int);

void sys_timer_schedule();

void sys_hi_timer_del(int);

int sys_hi_timer_add(void *priv, void (*func)(void *priv), u32 msec);

void sys_hi_timer_schedule();

void sys_hi_timeout_del(int);

int sys_hi_timeout_add(void *priv, void (*func)(void *priv), u32 msec);


int sys_usec_timer_add(void *priv, void (*func)(void *priv), u32 usec);

void sys_usec_timer_schedule(struct sys_cpu_timer *);

void sys_usec_timer_set(int _t, u32 usec);

void sys_usec_timer_del(int);

#endif


