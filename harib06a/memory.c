#include "bootpack.h"

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		// 可用信息数目
	man->maxfrees = 0; 	// 用于观察可用状况
	man->lostsize = 0;	// 释放失败的内存的大小总和
	man->losts = 0;		// 释放失败次数
	return;
}

// 获取空余内存大小的合计
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for(i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for(i = 0; i < man->frees; i ++)
	{
		if(man->free[i].size >= size)
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size ==0)
			{
				for(; i < man->frees; i++)
				{
					// man->free[i].addr = man->free[i+1].addr;
					// man->free[i].size = man->free[i+1].size;
					// 下面代码为下面两行的简化版本
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	// 为便于归纳内存，将free[] 按照Addr 的顺序排列
	// 所以，等决定应该放在哪里
	for(i = 0; i < man->frees; i++)
	{
		if(man->free[i].addr > addr)
		{
			break;
		}
	}

	if (i > 0)
	{
		if(man->free[i-1].addr + man->free[i -1].size == addr)
		{
			// 此时可以与前面的归纳到一起
			man->free[i-1].size += size;
			if(i < man->frees)
			{
				// 此时后面还有尝试与后面的归纳到一起
				if(addr + size == man->free[i].addr)
				{
					man->free[i-1].size += man->free[i].size;
					// man->free[i] 删除
					man->frees --;
					for(; i < man->frees; i++)
					{
						man->free[i] = man->free[i+1];
					}
				}
			}
			return 0;
		}
	}
	if (i < man->frees)
	{
		if(addr + size == man->free[i].addr)
		{
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}

	// 即不能与前面合并，也不能与后面合并，
	if(man->frees < MEMMAN_FREES)
	{
		for(j = man->frees; j > i; --j)
		{
			man->free[j] = man->free[j-1];
		}
		man->frees ++;
		if(man->maxfrees < man->frees)
		{
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	man->losts ++;
	man->lostsize += size;
	return -1;
}

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflag, cr0, i;
	// 确认CPU 是386 还是486以上的
	eflag = io_load_eflags();
	eflag |= EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if((eflag & EFLAGS_AC_BIT) != 0)
	{
		flg486 = 1;
	}
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISBALE; // 禁止缓存
		store_cr0(cr0);
	}
	i = memtest_sub(start, end);

	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISBALE;// 允许缓存
		store_cr0(cr0);
	}
	return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for( i = start; i<= end; i+= 0x1000)
	{
		p = (unsigned int *) i;
		old = *p;    // 先保存数据
		*p = pat0;   // 尝试写入
		
		*p ^= 0xffffffff; // 异或反转
		if(*p != pat1)       // 检查反转结果
		{
not_memory:
			*p = old; // 出问题了，恢复数据，并退出
			break;
		}
		*p ^= 0xffffffff;   // 再次反转
		if( *p != pat0)        // 再次检查
		{
			goto not_memory;
		}
		*p = old; 		// 每一步都要恢复数据

	}
	return i;
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xffffff000;
	a = memman_alloc(man, size);
	return a;
}

unsigned int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	unsigned int i;
	size = (size + 0xfff) & 0xffffff000;
	i = memman_free(man, addr, size);
	return i;
}
