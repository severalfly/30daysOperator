#define MAX_SHEETS 256
#define SHEET_USE 1

struct SHEET
{
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, vol_inv, height, flags;
};

struct SHTCTL
{
	unsigned char *vram;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbf(struct SHET *sht, unsigned char *buf, int xsize, int ysize , int col_inv);
