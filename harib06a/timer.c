#include "timer.h"
#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit()
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);// 与下面语句设置中断周期为11932，频率为100Hz
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff; // 因为最开始可能没有定时器

	timerctl.using = 0;
	for(i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timers0[i].flags = 0; // 设置为还未使用
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(timerctl.timers0[i].flags == 0)
		{
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; // 没找到
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0;// 还原为未使用
	return;
}
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e, i, j;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	// 搜索注册位置
	for(i = 0; i < timerctl.using; i++)
	{
		if(timerctl.timers[i]->timeout >= timer->timeout)
		{
			// 找到注册位置，
			break;
		}
	}
	for(j = timerctl.using; j > i; j--)
	{
		timerctl.timers[j] = timerctl.timers[j - 1];
	}
	timerctl.using ++;
	// 插入到空位上
	timerctl.timers[i] = timer;
	timerctl.next = timerctl.timers[0]->timeout;

	io_store_eflags(e);
	return;
}

/**
 * 由前面设置的中断周期可知，此处中断100次每秒
 * @param esp [description]
 */
void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2, 0x60);// 把IRQ-00 信号接收完了的信息通知PIC
	timerctl.count ++; // 计数器加1 
	if(timerctl.next > timerctl.count)
	{
		// 还没到下一次时间
		return;
	}
	timerctl.next = 0xfffffff;
	int i, j;
	for(i = 0; i < timerctl.using; i++)
	{
		// timers 的定时器处于动作中，所以不确认flags
		if(timerctl.timers[i]->timeout > timerctl.count)
		{
			break;
		}
		// 超时了
		timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);

	}

	// 正好有i 个定时器超时了，其他的进行移位
	timerctl.using -= i;
	for(j = 0; j < timerctl.using; j ++)
	{
		// 将后面的移动到前面来
		timerctl.timers[j] = timerctl.timers[i + j];
	}
	if(timerctl.using > 0)
	{
		timerctl.next = timerctl.timers[0]->timeout;
	}
	else
	{
		timerctl.next = 0xffffffff;
	}

	return;
}


