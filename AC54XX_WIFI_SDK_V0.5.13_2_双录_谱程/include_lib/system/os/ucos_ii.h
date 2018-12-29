/***********************************Jieli tech************************************************
  File : ucos_ii.h
  By   : Juntham
  date : 2014-07-03 09:09
********************************************************************************************/

#ifndef   OS_uCOS_II_H
#define   OS_uCOS_II_H


#include "typedef.h"
#include "os/os_cpu.h"
#include "os/os_cfg.h"
#include "os/os_api.h"

/*
 *********************************************************************************************************
 *                                           INCLUDE HEADER FILES
 *********************************************************************************************************
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *********************************************************************************************************
 *                                             MISCELLANEOUS
 *********************************************************************************************************
 */

#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif
/*
*********************************************************************************************************
*                                          EVENT CONTROL BLOCK
*********************************************************************************************************
*/
#ifdef OS_CREATE_LIB
#if OS_EVENT_EN
struct event_cnt {
    u16 cnt;
};
struct event_mutex {
    u8 value;
    u8 prio;
    u16 OwnerNestingCtr;
};

struct os_tcb;

typedef struct os_event {
    u8 OSEventType;                    /* Type of event control block (see OS_EVENT_TYPE_xxxx)    */
#if OS_TIME_SLICE_EN > 0
    struct   os_tcb  *OSTCBList;                      /* TCB List */
#else
    OS_CPU_DATA   OSTCBList;
#endif
    void    *OSEventPtr;                  /* Pointer to message or queue structure                   */
    union {
        struct event_cnt OSEvent;
        struct event_mutex OSMutex;
    };
} OS_EVENT;
#endif


/*$PAGE*/

/*
 *********************************************************************************************************
 *                                          MESSAGE QUEUE DATA
 *********************************************************************************************************
 */

#if (OS_Q_EN > 0) || (OS_TASKQ_EN > 0)
typedef struct os_q {                   /* QUEUE CONTROL BLOCK                                         */
    QS         OSQIn;
    QS         OSQOut;
    QS         OSQSize;             /* Size of queue (maximum number of entries)                   */
    QS         OSQEntries;          /* Current number of entries in the queue                      */
    void       **OSQStart;            /* Pointer to start of queue data                              */
} OS_Q;
#endif

/*$PAGE*/

/*
 *********************************************************************************************************
 *                                          TASK CONTROL BLOCK
 *********************************************************************************************************
*/
typedef struct os_tcb {
    OS_STK        *OSTCBStkPtr;      /* Pointer to current top of stack                              */

#if OS_CPU_MMU > 0
    u8            *frame;
#endif

#if OS_TIME_SLICE_EN > 0
    u8             slice_quanta;         /* 时间片初始值 */
    u8             slice_cnt;     /* 时间片模式下的计数值 */
    u16            OSTCBDly;         /* Nbr ticks to delay task or, timeout waiting for event        */
#endif

#if OS_TASKQ_EN > 0
    OS_Q           task_q;
#endif

#if OS_PARENT_TCB > 0
    struct os_tcb   *OSTCBParent;
#endif

#if OS_CHILD_TCB > 0
    struct os_tcb   *OSTCBChildNext;
#endif

#if OS_TIME_SLICE_EN > 0
    struct os_tcb   *OSTCBEventNext; /* Pointer to next     TCB in the event waiting TCB list     */
    struct os_tcb   *OSTCBEventPrev; /* Pointer to previous TCB in the event waiting TCB list     */
    struct os_tcb   *OSTCBSliceNext; /* Pointer to next     TCB in the time slice TCB list        */
    struct os_tcb   *OSTCBSlicePrev; /* Pointer to previous TCB in the time slice TCB list        */
#endif

#if OS_EVENT_EN
    OS_EVENT        *OSTCBEventPtr;    /* Pointer to event control block                               */
#endif

#if (OS_Q_EN > 0) || (OS_TASKQ_EN > 0)
    void            *OSTCBMsg;         /* Message received from OSMboxPost() or OSQPost()              */
#endif

#if OS_TIME_SLICE_EN == 0
    u16           OSTCBDly;         /* Nbr ticks to delay task or, timeout waiting for event        */
#endif
    u8            OSTCBPrio;        /* Task priority (0 == highest)                                 */
    u8            OSTCBStat;        /* Task status                                                  */
    bit1          OSTCBPendTO;      /* Flag indicating PEND timed out (TRUE == timed out)           */


#if OS_TASK_DEL_EN > 0
    u8            OSTCBDelReq;      /* Indicates whether a task needs to delete itself              */
#endif


    int *pid;
    OS_STK stk_size;
    OS_STK *p_stk_base;
    char          *name;
    u32 timeout;
} OS_TCB;


typedef struct os_tcb_list {
    OS_TCB *ptcb;
#if OS_TIME_SLICE_EN > 0
    OS_TCB *idle_ptcb;
#endif
} OS_TCB_LIST;

/*
 *********************************************************************************************************
 *                                MUTUAL EXCLUSION SEMAPHORE MANAGEMENT
 *********************************************************************************************************
 */

#if OS_MUTEX_EN > 0

#if OS_MUTEX_ACCEPT_EN > 0
u8         OSMutexAccept(OS_EVENT *pevent);
#endif

u8     OSMutexCreate(OS_EVENT *pevent);

#if OS_MUTEX_DEL_EN > 0
u8     OSMutexDel(OS_EVENT *pevent, u8 opt);
#endif

u8         OSMutexPend(OS_EVENT *pevent, u16 timeout);
u8         OSMutexPost(OS_EVENT *pevent);

#if OS_MUTEX_QUERY_EN > 0
u8         OSMutexQuery(OS_EVENT *pevent, OS_MUTEX_DATA *p_mutex_data);
#endif

#endif
/*
 *********************************************************************************************************
 *                                         MESSAGE QUEUE MANAGEMENT
 *********************************************************************************************************
 */

#if (OS_Q_EN > 0)

#if OS_Q_ACCEPT_EN > 0
u8         OSQAccept(OS_EVENT *pevent, void *msg);
#endif

u8 OSQCreate(OS_EVENT *pevent, /*void **start, */QS size);

#if OS_Q_DEL_EN > 0
u8         OSQDel(OS_EVENT *pevent, u8 opt);
#endif

#if OS_Q_FLUSH_EN > 0
u8         OSQFlush(OS_EVENT *pevent);
#endif

u8         OSQPend(OS_EVENT *pevent, u16 timeout, void *msg);

#if OS_Q_POST_EN > 0
u8         OSQPost(OS_EVENT *pevent, void *msg);
#endif

#if OS_Q_POST_FRONT_EN > 0
u8         OSQPostFront(OS_EVENT *pevent, void *msg);
#endif

#if OS_Q_POST_OPT_EN > 0
u8         OSQPostOpt(OS_EVENT *pevent, void *msg, u8 opt);
#endif

#if OS_Q_QUERY_EN > 0
u16  OSQQuery(OS_EVENT *pevent);
#endif

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          SEMAPHORE MANAGEMENT
*********************************************************************************************************
*/
#if OS_SEM_EN > 0

#if OS_SEM_ACCEPT_EN > 0
u16    OSSemAccept(OS_EVENT *pevent);
#endif

u8     OSSemCreate(OS_EVENT *pevent, u16 cnt);

#if OS_SEM_DEL_EN > 0
u8     OSSemDel(OS_EVENT *pevent, u8 opt);
#endif

u8     OSSemPend(OS_EVENT *pevent, u16 timeout);
u8     OSSemPost(OS_EVENT *pevent);

#if OS_SEM_QUERY_EN > 0
u16  OSSemQuery(OS_EVENT *pevent);
#endif

#if OS_SEM_SET_EN > 0
u8     OSSemSet(OS_EVENT *pevent, u16 cnt);
#endif

#endif

/*$PAGE*/
/*
 *********************************************************************************************************
 *                                            TASK MANAGEMENT
 *********************************************************************************************************
 */
#if OS_TASK_CHANGE_PRIO_EN > 0
u8         OSTaskChangePrio(char *name, u8 newprio);
#endif

#if OS_TASK_CREATE_EN > 0
u8  OSTaskCreate(void (*task)(void *p_arg), OS_TCB *task_tcb, void *p_arg
#if OS_CPU_MMU == 0
                 , OS_STK *ptos
#endif
                 , u8 prio
#if OS_TASKQ_EN > 0
                 , void **start, QS qsize
#endif
#if OS_TIME_SLICE_EN > 0
                 , u8 time_quanta
#endif
                 , s8 *name
                );
#endif

u8 OSTaskQAccept(int argc, int *argv);

u8 OSTaskQPend(u16 timeout, int argc, int *argv);

u8  OSTaskQPost(const char *name, int argc, ...);

u8  OSTaskQFlush(const char *name);

u8  OSTaskQPostFront(const char *name, int argc, ...);

u8  OSTaskQQuery(const char *name, u8 *task_q_entries);

#if OS_TASK_DEL_EN > 0
u8   OSTaskDel(const char *name);
u8   OSTaskDelReq(const char *name);
void OSTaskDelRes(const char *name);
#endif


#if OS_TASK_SUSPEND_EN > 0
u8   OSTaskResume(const char *name);
u8   OSTaskSuspend(const char *name);
#endif

#if OS_TASK_QUERY_EN > 0
u8   OSTaskQuery(const char *name, OS_TCB *p_task_data);
#endif

/*$PAGE*/
/*
 *********************************************************************************************************
 *                                            TIME MANAGEMENT
 *********************************************************************************************************
 */

void          OSTimeDly(u16 ticks);


#if OS_TIME_GET_SET_EN > 0
u32  OSTimeGet(void);
void OSTimeSet(u32 ticks);

#endif

void OSTimeTick(void);

/*
 *********************************************************************************************************
 *                                             MISCELLANEOUS
 *********************************************************************************************************
 */

void OSInit(void);

void OSStart();

u16  OSVersion(void);
#endif
/*
 *********************************************************************************************************
 *                                            GLOBAL VARIABLES
 *********************************************************************************************************
*/

OS_EXT  volatile u8    OSRunning;                       /* Flag indicating that kernel is running   */
OS_EXT  volatile OS_CPU_DATA    OSRdyTbl;
OS_EXT  volatile  u32  OSIdleCtr;                       /* Idle counter                   */
OS_EXT  OS_TCB         *OSTCBCur[OS_CPU_CORE];          /* Pointer to currently running TCB         */
OS_EXT  OS_TCB         *OSTCBHighRdy[OS_CPU_CORE];      /* Pointer to highest priority TCB R-to-R   */
OS_EXT  OS_TCB_LIST    OSTCBPrioTbl[OS_MAX_TASKS + 1];  /* Table of pointers to created TCBs        */


#if (OS_INT_NESTING == 1)
OS_EXT  u8 OSIntNesting;
#elif (OS_INT_NESTING == 2)
extern int is_cpu_int_nesting();
#define OSIntNesting 		is_cpu_int_nesting()
#endif

#if OS_TIME_GET_SET_EN > 0
OS_EXT  volatile  u32  OSTime;                   /* Current value of system time (in ticks)         */
#endif


/*
 *********************************************************************************************************
 *                                             MISCELLANEOUS
 *********************************************************************************************************
 */

/* void          OSInit(void); */

void          OSIntEnter(void);
void          OSIntExit(void);

#if OS_SCHED_LOCK_EN > 0
void          OSSchedLock(void);
void          OSSchedUnlock(void);
#endif

u32  OS_HighPrio(OS_CPU_DATA table);

void OS_SchedRoundRobin(OS_TCB *ptcb);

void OS_InsertRdyListHead(OS_TCB *ptcb);

void OS_InsertRdyListTail(OS_TCB *ptcb);

void OS_InsertIdleList(OS_TCB *ptcb);

void OS_RemoveRdyList(OS_TCB *ptcb);

void OS_RemoveIdleList(OS_TCB *ptcb);

void OS_InsertListHead(OS_TCB *ptcb);

void OS_RemoveList(OS_TCB *ptcb);

/* void OSStart(); */

void OSStatInit(void);

/* u16  OSVersion(void); */

/*$PAGE*/

/*
 *********************************************************************************************************
 *                                      INTERNAL FUNCTION PROTOTYPES
 *                            (Your application MUST NOT call these functions)
 *********************************************************************************************************
 */

#if OS_TASK_DEL_EN > 0
void          OS_Dummy(void);
#endif

#if OS_EVENT_EN
void          OS_ExchangePrio(OS_TCB *ptcba, OS_TCB *ptcbb);

#if OS_TIME_SLICE_EN > 0
OS_TCB        *OS_EventHighestTask(OS_TCB *ptcb, u8 prio);
void          OS_EventTaskRdy(OS_EVENT *pevent, OS_TCB *ptcb, void *msg, u8 msk);
#else
OS_TCB *OS_EventTaskRdy(OS_EVENT *pevent, void *msg, u8 msk);
#endif

void          OS_EventTaskWait(OS_EVENT *pevent, OS_TCB *OSTCB);
void          OS_EventTO(OS_EVENT *pevent, OS_TCB *OSTCB);
void          OS_EventWaitListInit(OS_EVENT *pevent);
#endif

void          OS_MemClr(u8 *pdest, u16 size);
void          OS_MemCopy(u8 *pdest, u8 *psrc, u16 size);

#if OS_Q_EN > 0
void          OS_QInit(void);
#endif

int          OS_Sched(void);

void          OS_TaskIdle(void *p_arg);

/*$PAGE*/

/*
 *********************************************************************************************************
 *                                          FUNCTION PROTOTYPES
 *                                      (Target Specific Functions)
 *********************************************************************************************************
 */

#if OS_VERSION >= 204
void          OSInitHookBegin(void);
void          OSInitHookEnd(void);
#endif

#ifndef OS_ISR_PROTO_EXT
void          OSIntCtxSw(void);
void          OSStartHighRdy(void);
#endif

void          OSTaskCreateHook(OS_TCB *ptcb);
void          OSTaskDelHook(OS_TCB *ptcb);

#if OS_VERSION >= 251
void          OSTaskIdleHook(void);
#endif

void          OSTaskStatHook(void);
OS_STK *OSTaskStkInit(void(*p_task)(void *pd), void *p_arg, OS_STK *p_stk_base, u16 opt);

#if OS_TASK_SW_HOOK_EN > 0
void          OSTaskSwHook(void);
#endif

#if OS_VERSION >= 204
void          OSTCBInitHook(OS_TCB *ptcb);
#endif

#if OS_TIME_TICK_HOOK_EN > 0
void          OSTimeTickHook(void);
#endif
extern void OS_TASK_DEL_HOOK(OS_TCB *ptcb) ;
/*$PAGE*/
/*
 *********************************************************************************************************
 *                                   LOOK FOR MISSING #define CONSTANTS
 *
 * This section is used to generate ERROR messages at compile time if certain #define constants are
 * MISSING in OS_CFG.H.  This allows you to quickly determine the source of the error.
 *
 * You SHOULD NOT change this section UNLESS you would like to add more comments as to the source of the
 * compile time error.
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 *                                       MUTUAL EXCLUSION SEMAPHORES
 *********************************************************************************************************
 */

#ifndef OS_MUTEX_EN
#error  "OS_CFG.H, Missing OS_MUTEX_EN: Enable (1) or Disable (0) code generation for MUTEX"
#else
#ifndef OS_MUTEX_ACCEPT_EN
#error  "OS_CFG.H, Missing OS_MUTEX_ACCEPT_EN: Include code for OSMutexAccept()"
#endif

#ifndef OS_MUTEX_DEL_EN
#error  "OS_CFG.H, Missing OS_MUTEX_DEL_EN: Include code for OSMutexDel()"
#endif

#ifndef OS_MUTEX_QUERY_EN
#error  "OS_CFG.H, Missing OS_MUTEX_QUERY_EN: Include code for OSMutexQuery()"
#endif
#endif

/*
 *********************************************************************************************************
 *                                              SEMAPHORES
 *********************************************************************************************************
 */

#ifndef OS_SEM_EN
#error  "OS_CFG.H, Missing OS_SEM_EN: Enable (1) or Disable (0) code generation for SEMAPHORES"
#else
#ifndef OS_SEM_ACCEPT_EN
#error  "OS_CFG.H, Missing OS_SEM_ACCEPT_EN: Include code for OSSemAccept()"
#endif

#ifndef OS_SEM_DEL_EN
#error  "OS_CFG.H, Missing OS_SEM_DEL_EN: Include code for OSSemDel()"
#endif

#ifndef OS_SEM_QUERY_EN
#error  "OS_CFG.H, Missing OS_SEM_QUERY_EN: Include code for OSSemQuery()"
#endif

#ifndef OS_SEM_SET_EN
#error  "OS_CFG.H, Missing OS_SEM_SET_EN: Include code for OSSemSet()"
#endif
#endif

/*
 *********************************************************************************************************
 *                                             TASK MANAGEMENT
 *********************************************************************************************************
 */

#ifndef OS_MAX_TASKS
#error  "OS_CFG.H, Missing OS_MAX_TASKS: Max. number of tasks in your application"
#else
#if     OS_MAX_TASKS < 2
#error  "OS_CFG.H,         OS_MAX_TASKS must be >= 2"
#endif

/* #if     OS_MAX_TASKS >  (OS_LOWEST_PRIO - OS_N_SYS_TASKS + 1) */
/* #error  "OS_CFG.H,         OS_MAX_TASKS must be <= OS_LOWEST_PRIO - OS_N_SYS_TASKS + 1" */
/* #endif */

#endif

#if     OS_VERSION     <  280
#if     OS_LOWEST_PRIO >   63
#error  "OS_CFG.H,         OS_LOWEST_PRIO must be <= 63 in V2.7x or lower"
#endif
#endif

#if     OS_VERSION     >= 280
#if     OS_LOWEST_PRIO >  254
#error  "OS_CFG.H,         OS_LOWEST_PRIO must be <= 254 in V2.8x and higher"
#endif
#endif

#ifndef OS_TASK_CHANGE_PRIO_EN
#error  "OS_CFG.H, Missing OS_TASK_CHANGE_PRIO_EN: Include code for OSTaskChangePrio()"
#endif

#ifndef OS_TASK_CREATE_EN
#error  "OS_CFG.H, Missing OS_TASK_CREATE_EN: Include code for OSTaskCreate()"
#endif

#ifndef OS_TASK_DEL_EN
#error  "OS_CFG.H, Missing OS_TASK_DEL_EN: Include code for OSTaskDel()"
#endif

#ifndef OS_TASK_SUSPEND_EN
#error  "OS_CFG.H, Missing OS_TASK_SUSPEND_EN: Include code for OSTaskSuspend() and OSTaskResume()"
#endif

#ifndef OS_TASK_QUERY_EN
#error  "OS_CFG.H, Missing OS_TASK_QUERY_EN: Include code for OSTaskQuery()"
#endif

/*
 *********************************************************************************************************
 *                                             TIME MANAGEMENT
 *********************************************************************************************************
 */

#ifndef OS_TICKS_PER_SEC
#error  "OS_CFG.H, Missing OS_TICKS_PER_SEC: Sets the number of ticks in one second"
#endif

#ifndef OS_TIME_DLY_HMSM_EN
#error  "OS_CFG.H, Missing OS_TIME_DLY_HMSM_EN: Include code for OSTimeDlyHMSM()"
#endif

#ifndef OS_TIME_DLY_RESUME_EN
#error  "OS_CFG.H, Missing OS_TIME_DLY_RESUME_EN: Include code for OSTimeDlyResume()"
#endif

#ifndef OS_TIME_GET_SET_EN
#error  "OS_CFG.H, Missing OS_TIME_GET_SET_EN: Include code for OSTimeGet() and OSTimeSet()"
#endif

/*
 *********************************************************************************************************
 *                                            MISCELLANEOUS
 *********************************************************************************************************
 */

#ifndef OS_ARG_CHK_EN
#error  "OS_CFG.H, Missing OS_ARG_CHK_EN: Enable (1) or Disable (0) argument checking"
#endif


#ifndef OS_CPU_HOOKS_EN
#error  "OS_CFG.H, Missing OS_CPU_HOOKS_EN: uC/OS-II hooks are found in the processor port files when 1"
#endif


#ifndef OS_LOWEST_PRIO
#error  "OS_CFG.H, Missing OS_LOWEST_PRIO: Defines the lowest priority that can be assigned"
#endif


#ifndef OS_SCHED_LOCK_EN
#error  "OS_CFG.H, Missing OS_SCHED_LOCK_EN: Include code for OSSchedLock() and OSSchedUnlock()"
#endif



#ifndef OS_TASK_SW_HOOK_EN
#error  "OS_CFG.H, Missing OS_TASK_SW_HOOK_EN: Allows you to include the code for OSTaskSwHook() or not"
#endif


#ifndef OS_TIME_TICK_HOOK_EN
#error  "OS_CFG.H, Missing OS_TIME_TICK_HOOK_EN: Allows you to include the code for OSTimeTickHook() or not"
#endif

#ifdef __cplusplus
}
#endif
#endif
