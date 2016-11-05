#include "memory.h"
#include "sheet.h"



struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL*) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0)
	{
		goto error;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; // 设置为无图层
	for (i = 0; i < MAX_SHEETS; ++i)
	{
		ctl->sheet0[i].flags = 0;// 标记为未使用
	}
	err:
	return ctl
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i ;
	for (int i = 0; i < MAX_SHEETS; ++i)
	{
		if(ctl->sheet0[i].flags == 0)
		{
			sht = &ctl->sheet0[i];
			sht->flags = SHEET_USE;
			sht->height = -1; // 新申请的图层默认为隐藏
			return sht;
		}
	}
	return 0;//当前所有的图层都在使用，这种情况应该不会出现的，
}

void sheet_setbf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize , int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}
