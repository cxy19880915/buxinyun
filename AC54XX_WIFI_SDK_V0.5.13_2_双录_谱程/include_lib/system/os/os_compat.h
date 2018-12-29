#ifndef _OS_COMPAT_H_
#define _OS_COMPAT_H_


#include "os_api.h"

/*! \addtogroup OS_THREAD
 *  @ingroup OS
 *  @brief	 OS thread api
 *  @{
 */


/**
 *  @brief enum kill_mode
 *  used by thread_kill
 */
enum kill_mode {
    KILL_WAIT,		/*!<WAIT */
    KILL_REQ,		/*!<REQUEST	*/
    KILL_FORCE,		/*!<FORCE ON */
};



/*----------------------------------------------------------------------------*/
/**@brief  创建thread_fork_kill线程
   @return 0: 成功
   @return other: enum OS_ERROR_CODES
   @note   该线程用来Kill掉其他线程，其任务优先级最高
*/
/*----------------------------------------------------------------------------*/
int thread_fork_init(void);

/*----------------------------------------------------------------------------*/
/**@brief  创建一个线程
   @param[in] thread_name: 标识此线程的字符串
   @param[in] prio: 优先级
   @param[in] stk_size: 堆栈空间的大小（以四个字节为一个单位）
   @param[in] q_size: 消息队列空间的大小（以四个字节为一个单位）
   @param[out] pid: 保存新线程的pid
   @param[in] func: 入口功能函数
   @param[in] parm: 入口函数的传入参数
   @return 0: 成功
   @return -1: 参数错误
   @return other: enum OS_ERROR_CODES
   @note   由于系统靠thread_name区分不同任务，所以请保证thread_name的唯一
   @note   replace pthread_create function
*/
/*----------------------------------------------------------------------------*/
int thread_fork(const char *thread_name, int prio, int stk_size, u32 q_size, int *pid, void (*func)(void *), void *parm);

/*----------------------------------------------------------------------------*/
/**@brief  杀死一个线程
   @param[in]  pid: 指向需要杀死的pid号的指针
   @param[in]  mode: enum kill_mode
   @return 0: 成功
   @return -1: 参数错误
   @return other: enum OS_ERROR_CODES
   @note   推荐使用KILL_WAIT
   @note   replace pthread_kill function
*/
/*----------------------------------------------------------------------------*/
void thread_kill(volatile int *pid, enum kill_mode mode);

/*----------------------------------------------------------------------------*/
/**@brief  获取正在运行的线程是否有其他线程请求自己杀死
   @return 0: 没有杀死请求
   @return 1: 有杀死请求
   @note
*/
/*----------------------------------------------------------------------------*/
int thread_kill_req(void);

/*----------------------------------------------------------------------------*/
/**@brief  获取指向当前线程pid的指针
   @return 指向当前线程pid的指针
   @note
*/
/*----------------------------------------------------------------------------*/
int *get_cur_thread_pid(void);

/*----------------------------------------------------------------------------*/
/**@brief  微秒级延时函数
   @param[in] t: us
   @note   TODO
*/
/*----------------------------------------------------------------------------*/
void udelay(unsigned int t);

/*----------------------------------------------------------------------------*/
/**@brief  微秒级延时函数
   @param[in] t: us
   @note   TODO
*/
/*----------------------------------------------------------------------------*/
void usleep(unsigned int t);

/*----------------------------------------------------------------------------*/
/**@brief  毫秒级延时函数
   @param[in] ms: ms
   @note
*/
/*----------------------------------------------------------------------------*/
void mdelay(unsigned int ms);

/*----------------------------------------------------------------------------*/
/**@brief  毫秒级延时函数
   @param[in] ms: ms
   @note
*/
/*----------------------------------------------------------------------------*/
void msleep(unsigned int ms);

/**
 * @brief pthread_mutexattr_init
 *
 * @param attr
 *
 * @return 0
 */
int pthread_mutexattr_init(int *attr);

/**
 * @brief pthread_mutexattr_destroy
 *
 * @param mattr
 *
 * @return 0
 */
int pthread_mutexattr_destroy(int *mattr);

/*----------------------------------------------------------------------------*/
/**@brief  初始化互斥量
   @param[out] mutex: 保存新建互斥量的信息
   @param[in]  attr: NULL
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int pthread_mutex_init(int *mutex, const int *attr);

/*----------------------------------------------------------------------------*/
/**@brief  互斥量上锁
   @param[in]  mutex: 指向需要上锁的互斥量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int pthread_mutex_lock(int *mutex);

/*----------------------------------------------------------------------------*/
/**@brief  互斥量解锁
   @param[in]  mutex: 指向需要解锁的互斥量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int pthread_mutex_unlock(int *mutex);

/*----------------------------------------------------------------------------*/
/**@brief  销毁互斥量
   @param[in]  mutex: 指向需要销毁的互斥量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int pthread_mutex_destroy(int *mutex);

/*----------------------------------------------------------------------------*/
/**@brief  初始化信号量
   @param[out]  sem: 保存新建信号量的信息
   @param[in]  cnt: 计数器的初始值
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_init(int *sem, int cnt);

/*----------------------------------------------------------------------------*/
/**@brief  等待信号量
   @param[in]  sem: 指向需要等待的信号量的指针
   @param[in]  timeout: 等待超时（时钟节拍），取0时无限等待
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_pend(int *sem, unsigned int timeout);

/*----------------------------------------------------------------------------*/
/**@brief  发送信号量
   @param[in]  sem: 指向需要发送的信号量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_post(int *sem);

/*----------------------------------------------------------------------------*/
/**@brief  设置信号量的计数器的值
   @param[in]  sem: 指向需要设置的信号量的指针
   @param[in]  init_val: 计数器的设置值
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_set(int *sem, int init_val);

/*----------------------------------------------------------------------------*/
/**@brief  销毁信号量
   @param[in]  sem: 指向需要销毁的信号量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_del(int *sem);

/*----------------------------------------------------------------------------*/
/**@brief  查询信号量计数值
   @param[in]  sem: 指向需要查询的信号量的指针
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int sem_query(int *sem);

/*----------------------------------------------------------------------------*/
/**@brief  获取一个队列消息
   @param[out]  __msg: 保存获取到的消息
   @return 0: 成功
   @return -1: 失败
   @note   无限等待
*/
/*----------------------------------------------------------------------------*/
int get_msg(u32 *__msg);

/*----------------------------------------------------------------------------*/
/**@brief  放进一个队列消息
   @param[in]  msg: 消息
   @param[in]  type: 消息种类
   @param[in]  parm: 线程名称
   @return enum OS_ERROR_CODES
   @note
*/
/*----------------------------------------------------------------------------*/
int put_msg_fifo(u32 msg, u8 type, void *parm);

/*----------------------------------------------------------------------------*/
/**@brief  获取一个队列消息(等待超时退出)
   @param[in]  timeout_val: 等待时间
   @param[out] to_flag: 超时标志
   @return 消息
   @note
*/
/*----------------------------------------------------------------------------*/
u32 get_msg_timeout(u32 timeout_val, u8 *to_flag);


/*! @}*/

#endif //_OS_COMPAT_H_
