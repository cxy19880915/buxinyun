#ifndef _PTHREAD_H
#define _PTHREAD_H	1

#include "os/os_compat.h"
#include <time.h>

typedef int pthread_cond_t;
typedef int pthread_condattr_t;
typedef int pthread_mutex_t;
typedef int pthread_attr_t;
typedef int pthread_t;

struct sched_param {
    int sched_priority;
};

int pthread_cond_init(pthread_cond_t *__cond,
                      const pthread_condattr_t *__cond_attr);

int pthread_cond_destroy(pthread_cond_t *__cond);

int pthread_cond_wait(pthread_cond_t *__cond,
                      pthread_mutex_t *__mutex);

int pthread_cond_signal(pthread_cond_t *__cond);

int pthread_attr_init(pthread_attr_t *__attr);

int pthread_attr_destroy(pthread_attr_t *__attr);

int pthread_attr_setstacksize(pthread_attr_t *__attr,
                              size_t __stacksize);

int pthread_attr_getschedpolicy(const pthread_attr_t *
                                __attr, int *__policy);

int pthread_attr_setschedpolicy(pthread_attr_t *__attr, int __policy);

int pthread_attr_getschedparam(const pthread_attr_t *__attr,
                               struct sched_param *__param);

int pthread_attr_setschedparam(pthread_attr_t *__attr,
                               const struct sched_param *__param);

int pthread_detach(pthread_t __th);

int pthread_join(pthread_t __th, void **__thread_return);

int pthread_condattr_init(pthread_condattr_t *__attr);

int pthread_mutex_trylock(pthread_mutex_t *__mutex);

int pthread_cond_timedwait(pthread_cond_t *__cond,
                           pthread_mutex_t *__mutex,
                           const struct timespec *__abstime);

int pthread_cond_broadcast(pthread_cond_t *__cond);

#endif	/* pthread.h */
