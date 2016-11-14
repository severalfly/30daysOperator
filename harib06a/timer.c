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
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	timerctl.using ++;
	if (timerctl.using == 1)
	{
		timerctl.t0 = timer;
		timer->next = 0;// 设置为没有下一个
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	t = timerctl.t0;
	if(timer->timeout <= t->timeout)
	{
		// 此时插入到最前面
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}

	// 搜寻插入位置
	for (;;)
	{
		s = t;
		t = t->next;
		if(t == 0)
		{
			break;// 最后面了，不用处理啦，前面已经t=t->next
		}
		if(timer->timeout <= t->timeout)
		{
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
	s->next = timer;
	timer->next = 0;
	io_store_eflags(e);
	return;
}

/**
 * 由前面设置的中断周期可知，此处中断100次每秒
 * @param esp [description]
 */
void inthandler20(int *esp)
{
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);// 把IRQ-00 信号接收完了的信息通知PIC
	timerctl.count ++; // 计数器加1 
	if(timerctl.next > timerctl.count)
	{
		// 还没到下一次时间
		return;
	}
	timer = timerctl.t0;
	int i;
	for(i = 0; i < timerctl.using; i++)
	{
		// timers 的定时器处于动作中，所以不确认flags
		if(timer->timeout > timerctl.count)
		{
			// 一直找到未超时的，跳出循环
			break;
		}
		// 超时了
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next;
		// 这个for 循环中是没有使用到i 的，也就是说i 仅用来记数
	}

	// 正好有i 个定时器超时了，其他的进行移位
	timerctl.using -= i;

	// 新移位
	timerctl.t0 = timer;
	if(timerctl.using > 0)
	{
		timerctl.next = timerctl.t0->timeout;
	}
	else
	{
		timerctl.next = 0xffffffff;
	}

	return;
}


