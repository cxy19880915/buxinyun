#ifndef SYS_EVENT_H
#define SYS_EVENT_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/rect.h"


#define     KEY_POWER           12
#define 	KEY_OK 				13
#define 	KEY_CANCLE 			14
#define 	KEY_MENU 			15
#define 	KEY_MODE 			16
#define     KEY_PHOTO           17


#define  	KEY_LEFT 			37
#define  	KEY_UP 				38
#define  	KEY_RIGHT 			39
#define  	KEY_DOWN 			40

#define 	KEY_0 				48
#define 	KEY_1 				49
#define 	KEY_2 				50
#define 	KEY_3 				51
#define 	KEY_4 				52
#define 	KEY_5 				53
#define 	KEY_6 				54
#define 	KEY_7 				55
#define 	KEY_8 				56
#define 	KEY_9 				57


#define 	KEY_F1 				60


#define SYS_KEY_EVENT 			0x01
#define SYS_TOUCH_EVENT 		0x02
#define SYS_DEVICE_EVENT 		0x04
#define SYS_NET_EVENT 		    0x08


enum {
    KEY_EVENT_CLICK,
    KEY_EVENT_LONG,
    KEY_EVENT_HOLD,
    KEY_EVENT_DOWN,
    KEY_EVENT_UP,
};


enum {
    DEVICE_EVENT_IN,
    DEVICE_EVENT_OUT,
    DEVICE_EVENT_ONLINE,
    DEVICE_EVENT_OFFLINE,
    DEVICE_EVENT_CHANGE,
    DEVICE_EVENT_POWEROFF,
};

enum {
    TOUCH_EVENT_DOWN,
    TOUCH_EVENT_MOVE,
    TOUCH_EVENT_HOLD,
    TOUCH_EVENT_UP,
    TOUCH_EVENT_CLICK,
    TOUCH_EVENT_DOUBLE_CLICK,
};

enum {
    NET_EVENT_CMD,
    NET_EVENT_DATA,

    YI_NET_EVENT_CMD,
    YI_NET_EVENT_DATA,
};

struct key_event {
    u8 event;
    u8 value;
};


struct touch_event {
    u8 event;
    struct position pos;
};

struct device_event {
    u8 event;
    u8 value;
};

struct net_event {
    u8 event;
    u8 value;
};
struct sys_event {
    u8 type;
    u8 consumed;
    void *arg;
    union {
        struct key_event 	key;
        struct touch_event 	touch;
        struct device_event dev;
        struct net_event 	net;
    } u;
};




struct static_event_handler {
    int event_type;
    void (*handler)(struct sys_event *);
};


#define SYS_EVENT_HANDLER(type, fn, pri) \
	const struct static_event_handler __event_handler_##fn sec(.sys_event.pri.handler) = { \
		.event_type = type, \
		.handler = fn, \
	}

extern struct static_event_handler sys_event_handler_begin[];
extern struct static_event_handler sys_event_handler_end[];

#define list_for_each_static_event_handler(p) \
	for (p = sys_event_handler_begin; p < sys_event_handler_end; p++)



u16 register_sys_event_handler(int event_type, u8 priority, void *priv,
                               void (*handler)(struct sys_event *, void *));


void unregister_sys_event_handler(u16 id);


/*
 * 事件通知函数,系统有事件发生时调用此函数
 */
void sys_event_notify(struct sys_event *event);


void sys_key_event_disable();


void sys_key_event_enable();

void sys_touch_event_disable();


void sys_touch_event_enable();

/*
 *下面四个为系统事件消耗函数，调用此函数后则对应的事件不在分发给其它任务
 *
 */
void sys_event_consume(struct sys_event *event);

void sys_key_event_consume(struct key_event *e);

void sys_touch_event_consume(struct touch_event *e);

void sys_device_event_consume(struct device_event *e);


/*
 * 下面两个函数为按键和触摸事件接管函数，调用此函数后则对应的事件只发到当前任务
 *
 * on=true: 开始接管， on=false: 取消接管
 *
 * once:  on = false 时有效，当前这次不接管, 事件可以继续发送到其它任务
 *
 */
void sys_key_event_takeover(bool on, bool once);

void sys_touch_event_takeover(bool on, bool once);


int sys_key_event_map(struct key_event *org, struct key_event *new);
int sys_key_event_unmap(struct key_event *org, struct key_event *new);


#endif

