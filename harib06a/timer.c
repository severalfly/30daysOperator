#include "timer.h"
#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit()
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);// 与下面语句设置中断周期为11932，频率为100Hz
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timer[i].flags = 0;
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(timerctl.timer[i].flags == 0)
		{
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0;// 还原为未使用
	return;
}
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	timer->timeout = timeout;
	timer->flags = TIMER_FLAGS_USING;
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
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if(timerctl.timer[i].flags == TIMER_FLAGS_USING)
		{
			timerctl.timer[i].timeout--;
			if(timerctl.timer[i].timeout ==0)
			{
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}
		}
	}

	return;
}
// void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data)
// {
// 	int eflags;
// 	eflags = io_load_eflags();
// 	io_cli();
// 	timerctl.timeout = timeout;
// 	timerctl.fifo = fifo;
// 	timerctl.data = data;
// 	io_store_eflags(eflags);
// 	return;
// }


