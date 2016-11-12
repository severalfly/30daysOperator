#include "timer.h"
#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);// 与下面语句设置中断周期为11932，频率为100Hz
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
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
	return;
}