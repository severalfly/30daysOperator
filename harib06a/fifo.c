/* FIFOƒ‰ƒCƒuƒ‰ƒŠ */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001




// 以下是32位处理
void fifo32_init(struct FIFO32 *fifo, int size, int *buf)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* 空 */
	fifo->flags = 0;
	fifo->p = 0; /* 开始写入位置 */
	fifo->q = 0; /* 结束位置 */
	return;
}
int fifo32_put(struct FIFO32 *fifo, int data)
{
	// 给fifo 发送数据，并保存在fifo  中
	if (fifo->free == 0) {
	/* 已经没了剩余数据 */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}
int fifo32_get(struct FIFO32 *fifo)
{
	// 从fifo 获取一个数据
	int data;
	if (fifo->free == fifo->size) {
		/* 缓冲区为空的情况下返回-1 */
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
{
	// 返回已经存储的数据数
	return fifo->size - fifo->free;
}
