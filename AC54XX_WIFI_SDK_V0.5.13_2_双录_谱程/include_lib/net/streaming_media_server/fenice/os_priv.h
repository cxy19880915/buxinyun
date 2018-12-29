#ifndef __OS_PRIV_H__
#define __OS_PRIV_H__

#define SCHEDULE_DO_PRIO 26
#define SCHEDULE_DO_STK_SIZE 4000

#define STREAM_MEDIA_SERVER_PRIO 25
#define STREAM_MEDIA_SERVER_STK_SIZE  4000

typedef int pthread_mutex_t, pthread_mutexattr_t;

unsigned int reset_time_stamp(void);
unsigned int get_time_stamp(uint32 delta_mtime);

#define MSG_NOSIGNAL 0

#define SOMAXCONN TCP_DEFAULT_LISTEN_BACKLOG


#endif

