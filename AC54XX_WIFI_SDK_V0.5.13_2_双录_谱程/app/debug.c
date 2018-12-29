#include "app_config.h"
#include "typedef.h"



#ifndef CONFIG_DEBUG_ENABLE

void putchar(char a)
{

}

int puts(char *out)
{
    return 0;
}

int printf(const char *format, ...)
{
    return 0;
}


void put_buf(const u8 *buf, int len)
{

}

void put_u8hex(u8 dat)
{

}

void put_u16hex(u16 dat)
{

}

void put_u32hex(u32 dat)
{

}

void log_print(int level, const char *tag, const char *format, ...)
{

}

#endif
