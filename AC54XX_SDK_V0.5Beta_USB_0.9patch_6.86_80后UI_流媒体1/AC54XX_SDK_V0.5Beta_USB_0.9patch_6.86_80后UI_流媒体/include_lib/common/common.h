#ifndef  __COMMON_H__
#define  __COMMON_H__

//用于放common_lib外部调用的函数
#include "os/os_cfg.h"
#include "asm/clock.h"
#include "generic/jiffies.h"

unsigned int time_lapse(unsigned int *handle, unsigned int time_out);//2^32/1000/60/60/24 后超时

int asprintf(char **ret, const char *format, ...);

static __inline u32 timer_get_sec(void)
{
    return (unsigned long long)jiffies * (unsigned long long)T1_PRD / (unsigned long long)get_apb_clk();
}
static __inline  u32 timer_get_ms(void)
{
    return (unsigned long long)jiffies * ((unsigned long long)1000 * (unsigned long long)T1_PRD / (unsigned long long)get_apb_clk());
}
int gettimeofday(struct timeval *__restrict __tv, void *__tz);

size_t strftime_2(char *ptr, size_t maxsize, const char *format, const struct tm *timeptr);
int rand(void);
// int vprintf(const char *fmt, __builtin_va_list va);
// int vsnprintf(char *, unsigned long, const char *, __builtin_va_list);
// int snprintf(char *buf, unsigned int size, const char *fmt, ...);
// //int snprintf(char *, unsigned long, const char *, ...);

// int sscanf(const char *buf, const char *fmt, ...);   //BUG: 多个参数? 最后又空格?

#endif  /*COMMON_H*/
