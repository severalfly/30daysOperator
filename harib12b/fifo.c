/* FIFOƒ‰ƒCƒuƒ‰ƒŠ */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
/* FIFOƒoƒbƒtƒ@‚Ì‰Šú‰» */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* ‹ó‚« */
	fifo->flags = 0;
	fifo->p = 0; /* ‘‚«ž‚ÝˆÊ’u */
	fifo->q = 0; /* “Ç‚Ýž‚ÝˆÊ’u */
	fifo->task = task; // 有数据写入时，需要唤醒的任务
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* FIFO‚Öƒf[ƒ^‚ð‘—‚èž‚ñ‚Å’~‚¦‚é */
{
	if (fifo->free == 0) {
		/* ‹ó‚«‚ª‚È‚­‚Ä‚ ‚Ó‚ê‚½ */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if(fifo->task != 0)
	{
		if(fifo->task->flags != 2)// 如果任务处于休眠状态
		{
			task_run(fifo->task);
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/* FIFO‚©‚çƒf[ƒ^‚ðˆê‚Â‚Æ‚Á‚Ä‚­‚é */
{
	int data;
	if (fifo->free == fifo->size) {
		/* ƒoƒbƒtƒ@‚ª‹ó‚Á‚Û‚Ì‚Æ‚«‚ÍA‚Æ‚è‚ ‚¦‚¸-1‚ª•Ô‚³‚ê‚é */
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/* ‚Ç‚Ì‚­‚ç‚¢ƒf[ƒ^‚ª—­‚Ü‚Á‚Ä‚¢‚é‚©‚ð•ñ‚·‚é */
{
	return fifo->size - fifo->free;
}
