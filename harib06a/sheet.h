#ifndef SHEET_H_H_
#define SHEET_H_H_
#include "bootpack.h"
#define MAX_SHEETS 256
#define SHEET_USE 1

struct SHEET
{
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};

struct SHTCTL
{
	unsigned char *vram;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheet0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
// void sheet_setbuf(struct SHET *sht, unsigned char *buf, int xsize, int ysize , int col_inv);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize , int col_inv);
void sheet_updown( struct SHEET *sht, int height);
// void sheet_refresh(struct SHTCTL *ctl);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
#endif

