#include "bootpack.h"

struct TIMER *mt_timer;
struct TIMER *task_timer;
struct TASKCTL *taskctl;
int mt_tr;

void mt_init(void)
{
	mt_timer = timer_alloc();
	timer_settime(mt_timer, 2);
	mt_tr = 3* 8;
	return;
}

// void mt_taskswitch(void)
// {
// 	if (mt_tr == 3 * 8)
// 	{
// 		mt_tr = 4 * 8;
// 	}
// 	else
// 	{
// 		mt_tr = 3 * 8;
// 	}
// 	timer_settime(mt_timer, 2);
// 	farjmp(0, mt_tr);
// 	return;
// }

void task_switch(void)
{
	struct TASK *task;
	taskctl->now ++;
	if(taskctl->now == taskctl->running)
	{
		taskctl->now = 0;
	}
	task = taskctl->tasks[taskctl->now];
	timer_settime(task_timer, task->priority);
	if(taskctl->running >= 2)
	{
		farjmp(0, task->sel);
	}
	return;
}

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
	taskctl = (struct TASKCTL*)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	for(i = 0; i < MAX_TASKS; i ++)
	{
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; // 活动中的标志
	task->priority = 2; // 0.02 秒
	taskctl->running = 1;
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, 2);
	return task;
}

struct TASK *task_alloc(void){
	int i;
	struct TASK *task;
	for(i = 0;i < MAX_TASKS; i ++)
	{
		if(taskctl->tasks0[i].flags == 0)
		{
			task = &taskctl->tasks0[i];
			task->flags = 1;
			task->tss.eflags = 0x00000202; // IF = 1
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x4000000;
			return task;
		}
	}
	return 0;
}


void task_run(struct TASK *task, int priority){
	if (priority > 0)
	{
		task->priority = priority;
	}
	task->flags = 2;
	taskctl->tasks[taskctl->running] = task;
	taskctl->running ++;
	return;
}

void task_sleep(struct TASK *task)
{
	int i;
	char ts = 0;
	if(task->flags == 2){ // 如果指定的任务处于唤醒状态
		if(task == taskctl->tasks[taskctl->now]){
			ts = 1; // 让自己休眠的话，稍后需要进行任务切换
		}
		for(i = 0 ;i < taskctl->running; i ++)
		{
			if(taskctl->tasks[i] == task)
			{
				// 在这里
				break;
			}
		}
		taskctl->running --;
		if(i < taskctl->now)
		{
			taskctl->now --; // 需要移动成员，要相应地处理
		}
		// 移动成员
		for(; i < taskctl->running; i ++)
		{
			taskctl->tasks[i] = taskctl->tasks[i + 1];
		}
		task->flags = 1;
		if(ts != 0)
		{
			if(taskctl->now >= taskctl->running)
			{
				taskctl->now = 0;
			}
			farjmp(0, taskctl->tasks[taskctl->now]->sel);
		}

	}
	return;
}
