#ifndef TIMER_H_H___
#define TIMER_H_H___
#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define MAX_TIMER 500

#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMER
{
	struct TIMER *next;// 保存下一个定时器地址
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	unsigned char data;
};
struct TIMERCTL
{
	unsigned int count, next, using;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};
void init_pit();
void inthandler20(int *esp);
// void settimer(unsigned int timeout, struct FIFO32 *fifo, int data);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);


#endif

