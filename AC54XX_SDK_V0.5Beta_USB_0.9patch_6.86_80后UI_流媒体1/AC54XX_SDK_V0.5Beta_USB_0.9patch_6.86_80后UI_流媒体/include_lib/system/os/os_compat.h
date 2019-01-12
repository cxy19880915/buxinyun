#ifndef _OS_COMPAT_H_
#define _OS_COMPAT_H_

#include "os_api.h"

enum kill_mode {
    KILL_WAIT,
    KILL_REQ,
    KILL_FORCE,
};

int thread_fork_init(void);
int thread_fork(char *thread_name, int prio, int stk_size, u32 q_size, int *pid, void (*func)(void *), void *parm);
void thread_kill(volatile int *pid, enum kill_mode mode);
int thread_kill_req(void);
u16 thread_put_msg_fifo(u32 msg, u8 type, int *pid);
u16 thread_put_msg_lifo(u32 msg, u8 type, int *pid);
int *get_cur_thread_pid(void);
int thread_rpc(bool wait_finish, void (*rpc_finish_cb)(void *priv), int prio, int stk_size, void (*thread_func)(void *priv), void *priv);

void udelay(unsigned int t);

void usleep(unsigned int t);

void mdelay(unsigned int ms);

void msleep(unsigned int ms);
int pthread_mutexattr_init(int *attr);
int pthread_mutex_init(int *mutex, const int *attr);
int pthread_mutex_lock(int *mutex);
int pthread_mutex_unlock(int *mutex);
int pthread_mutex_destroy(int *mutex);

#endif //_OS_COMPAT_H_
