// #include "memory.h"
#include "sheet.h"



struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL*) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0)
	{
		goto err;
	}
	ctl->map = (unsigned char*)memman_alloc_4k(memman, xsize * ysize);
	if( ctl->map == 0);
	{
		memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; // 设置为无图层
	for (i = 0; i < MAX_SHEETS; ++i)
	{
		ctl->sheet0[i].flags = 0;// 标记为未使用
		ctl->sheet0[i].ctl = ctl;
	}
	err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i ;
	for (i = 0; i < MAX_SHEETS; ++i)
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

/**
 * 设置缓存信息
 * @param
 * @param
 * @param
 * @param
 * @param
 */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize , int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

/**
* 程序主要做的是将sht 图层移动到height 层
* 所以代码中首先要做的就是存储sht 的高度信息，
*/
void sheet_updown(struct SHEET *sht, int height)
{
	int h, old = sht->height;
	struct SHTCTL *ctl = sht->ctl;
	// 指定高度出错，此处做校验
	if(height > ctl->top + 1)
	{
		height = ctl->top + 1;
	}
	if(height < -1 )
	{
		height = -1;
	}
	sht->height = height;
	// 下面主要是对sheets[] 重新排列，目测应该是插入到合适位置就可以
	if(old > height)
	{
		if (height >= 0)
		{
			for( h = old; h > height; --h)
			{
				ctl->sheets[h] = ctl->sheets[h -1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;// 已经找到合适位置
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
		}
		else
		{
			// height 为负，则隐藏之
			for(h = old; h < ctl->top; ++h)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->top --;// 当前最大图层数减1， 因为图层少了一个
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
		}
		/////////////////////////
		// sheet_refresh(ctl); //
		/////////////////////////
	}
	else if (old < height)
	{
		if (old >= 0)
		{
			//  把中间的拉下去
			for (h = old; h < height; h ++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else // 由隐藏状态转为显示状态
		{
			// 将已在上面的提上去
			for(h = ctl->top; h >= height ; h --)
			{
				ctl->sheets[h+1] = ctl->sheets[h];
				ctl->sheets[h+1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top ++;// 多了一层
		}
		// sheet_refresh(ctl);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
	}
	return;
}


void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if(sht->height >= 0)
	{
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0+bx1, sht->vy0 +by1, sht->height, sht->height);
	}
}


/**
 * 指定刷新位置
 * @param ctl [description]
 * @param vx0 [description]
 * @param vy0 [description]
 * @param vx1 [description]
 * @param vy1 [description]
 */
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;
	if(vx0 < 0) { vx0 = 0;}
	if(vy0 < 0) { vy0 = 0;}
	if(vx1 > ctl->xsize) { vx1 = ctl->xsize;}
	if(vy1 > ctl->ysize) { vy1 = ctl->ysize;}
	for(h = h0; h <= h1; h ++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht->ctl->sheet0;

		// 使用 vx0-vy1, 对 bx0 - by1 进行倒推
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0){ bx0 = 0;}
		if(by0 < 0) {by0 = 0;}
		if(bx1 > sht->bxsize ){bx1 = sht->bxsize;}
		if(by1 > sht->bysize ){by1 = sht->bysize;}
		for(by = by0;  by < by1; by++)
		{
			vy = sht->vy0 + by;
			for(bx = bx0; bx <bx1; bx ++)
			{
				vx = sht->vx0 + bx;
				if(map[vy*ctl->xsize + vx] ==sid)
				{
					vram[vy * ctl->xsize+ vx] = buf[by*sht->bxsize +bx];
				}
			}
		}
	}
}

/**
 * 移动位置
 * @param
 * @param
 * @param
 * @param
 */
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->height > 0)
	{
		/////////////////////////
		// sheet_refresh(ctl); //
		/////////////////////////
		sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
	}
	return ;
}

/**
 * 释放一个
 * @param
 * @param
 */
void sheet_free( struct SHEET *sht)
{
	struct SHTCTL *ctl = sht->ctl;
	if(sht->height >= 0)
	{
		sheet_updown(sht, -1);
	}
	sht->flags = 0;
	return ;
}

/**
 * 刷新map 
 * @param ctl [description]
 * @param vx0 [description]
 * @param vy0 [description]
 * @param vx1 [description]
 * @param vy1 [description]
 * @param h0 此图层以上的刷新，以下的不需要刷新
 */
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if(vx0 < 0){ vx0 = 0;}
	if(vy0 < 0){ vy0 = 0;}
	if(vx1 > ctl->xsize){vx1 = ctl->xsize;}
	if(vy1 > ctl->ysize){vy1 = ctl->ysize;}

	for(h = h0; h <= ctl->top; h++)
	{
		sht = ctl->sheets[h];
		sid = sht - ctl->sheet0;// 将进行了减法计算的地址作为图层号码使用。这里不明白
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		if(bx1 > sht->bxsize) {bx1 = sht->bxsize;}
		if(by1 > sht->bysize) {by1 = sht->bysize;}
		for(by = by0; by < by1; by++)
		{
			vy = sht->vy0 + by;
			for(bx = bx0; bx < bx1; bx++)
			{
				vx = sht->vx0 + bx;
				if(buf[by*sht->bxsize + bx] != sht->col_inv)
				{
					map[vy * ctl->xsize + vx] = sid;
				}
			}
		}
	}
	return;
}

