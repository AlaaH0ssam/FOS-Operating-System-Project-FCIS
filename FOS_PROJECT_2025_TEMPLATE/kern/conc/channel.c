/*
 * channel.c
 *
 *  Created on: Sep 22, 2024
 *      Author: HP
 */
#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <inc/string.h>
#include <inc/disk.h>

//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}

//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct kspinlock* lk)
{
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #1 CHANNEL - sleep
	//Your code is here
	//Comment the following line
	//panic("sleep() is not implemented yet...!!");

	 acquire_kspinlock(&ProcessQueues.qlock);
	 release_kspinlock(lk);
     get_cpu_proc()->env_status = ENV_BLOCKED;
	 enqueue(&chan->queue, get_cpu_proc());
	 sched();
	 acquire_kspinlock(lk);

	 release_kspinlock(&ProcessQueues.qlock);
}

//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_one(struct Channel *chan)
{
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #2 CHANNEL - wakeup_one
	//Your code is here
	//Comment the following line
	//panic("wakeup_one() is not implemented yet...!!");

	 acquire_kspinlock(&ProcessQueues.qlock);
     struct Env* i = dequeue(&chan->queue);
	  if (i){
	    sched_insert_ready(i);
	 }

     release_kspinlock(&ProcessQueues.qlock);
}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes

void wakeup_all(struct Channel *chan)
{
	//TODO: [PROJECT'25.IM#5] KERNEL PROTECTION: #3 CHANNEL - wakeup_all
	//Your code is here
	//Comment the following line
	//panic("wakeup_all() is not implemented yet...!!");

	acquire_kspinlock(&ProcessQueues.qlock);
		struct Env* i;
		while ((i = dequeue(&chan->queue)) != NULL)
		{
		i->env_status=ENV_READY;
		sched_insert_ready(i);
		}

     release_kspinlock(&ProcessQueues.qlock);
}

