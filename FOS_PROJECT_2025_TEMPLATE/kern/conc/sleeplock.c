// Sleeping locks

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_sleeplock(struct sleeplock *lk, char *name)
{
	init_channel(&(lk->chan), "sleep lock channel");
	char prefix[30] = "lock of sleeplock - ";
	char guardName[30+NAMELEN];
	strcconcat(prefix, name, guardName);
	init_kspinlock(&(lk->lk), guardName);
	strcpy(lk->name, name);
	lk->locked = 0;
	lk->pid = 0;
}

void acquire_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #4 SLEEP LOCK - acquire_sleeplock
	//Your code is here
	//Comment the following line
	//panic("acquire_sleeplock() is not implemented yet...!!");
	acquire_kspinlock(&(lk->lk));
	while (lk->locked) {
		 // Sleep
	 sleep(&(lk->chan), &(lk->lk));
		}

		 //freeee
     lk->locked = 1;
	lk->pid = get_cpu_proc()->env_id;
    release_kspinlock(&(lk->lk));

}

void release_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #5 SLEEP LOCK - release_sleeplock
	//Your code is here
	//Comment the following line
	//panic("release_sleeplock() is not implemented yet...!!");
	acquire_kspinlock(&(lk->lk));
	struct Env* x = get_cpu_proc();
	if (lk->locked && lk->pid == x->env_id) {
	// Free
    lk->pid = 0; lk->locked = 0;
    wakeup_all(&lk->chan);}
   release_kspinlock(&(lk->lk));
}

int holding_sleeplock(struct sleeplock *lk)
{
	int r;
	acquire_kspinlock(&(lk->lk));
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id);
	release_kspinlock(&(lk->lk));
	return r;
}



