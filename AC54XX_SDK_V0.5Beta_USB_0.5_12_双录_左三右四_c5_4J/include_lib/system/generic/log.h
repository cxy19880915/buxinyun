#ifndef __LOG_H
#define __LOG_H


#include "system/generic/printf.h"

#define   __LOG_VERBOSE   0
#define   __LOG_DEBUG     1
#define   __LOG_INFO      2
#define   __LOG_WARN      3
#define   __LOG_ERROR     4

struct logbuf {
    u16 len;
    u16 buf_len;
    char buf[0];
};

#define __LOG_ENABLE


#if __LOG_LEVEL > __LOG_VERBOSE
#define log_v(...) do {} while (0)
#elif defined __LOG_ENABLE
#define log_v(...) log_print(__LOG_VERBOSE, NULL, __VA_ARGS__)
#else
#define log_v(...) printf(__VA_ARGS__)
#endif



#if __LOG_LEVEL > __LOG_DEBUG
#define log_d(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_d(...)  log_print(__LOG_DEBUG, NULL, __VA_ARGS__);
#else
#define log_d(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_INFO
#define log_i(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_i(...) log_print(__LOG_INFO, NULL, __VA_ARGS__);
#else
#define log_i(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_WARN
#define log_w(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_w(...) log_print(__LOG_WARN, NULL, __VA_ARGS__);
#else
#define log_w(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_ERROR
#define log_e(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_e(...) log_print(__LOG_ERROR, NULL, __VA_ARGS__);
#else
#define log_e(...) printf(__VA_ARGS__)
#endif


#ifndef __LOG_ENABLE
#define log_dump(a, b)              do {} while(0)
#define log_putchar()               do {} while(0)
#define log_early_init(a)           do {} while(0)
#define log_level(a)                do {} while(0)
#else
void log_early_init(int buf_size);

void log_level(int level);

void log_print(int level, const char *tag, const char *format, ...);


void log_dump(const u8 *buf, int len);

struct logbuf *__log_output_start(int len);

void __log_output_end(struct logbuf *);

void log_putchar(struct logbuf *lb, char c);

#endif

int log_send_completed_cb();

void log_flush();









#endif
