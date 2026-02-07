/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/mem/memory_manager.h>
#include <kern/mem/kheap.h>

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE) {
	assert(
			LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE;
}
void setPageReplacmentAlgorithmCLOCK() {
	_PageRepAlgoType = PG_REP_CLOCK;
}
void setPageReplacmentAlgorithmFIFO() {
	_PageRepAlgoType = PG_REP_FIFO;
}
void setPageReplacmentAlgorithmModifiedCLOCK() {
	_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;
}
/*2018*/void setPageReplacmentAlgorithmDynamicLocal() {
	_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;
}
/*2021*/void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps) {
	_PageRepAlgoType = PG_REP_NchanceCLOCK;
	page_WS_max_sweeps = PageWSMaxSweeps;
}
/*2024*/void setFASTNchanceCLOCK(bool fast) {
	FASTNchanceCLOCK = fast;
}
;
/*2025*/void setPageReplacmentAlgorithmOPTIMAL() {
	_PageRepAlgoType = PG_REP_OPTIMAL;
}
;

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE) {
	return _PageRepAlgoType == LRU_TYPE ? 1 : 0;
}
uint32 isPageReplacmentAlgorithmCLOCK() {
	if (_PageRepAlgoType == PG_REP_CLOCK)
		return 1;
	return 0;
}
uint32 isPageReplacmentAlgorithmFIFO() {
	if (_PageRepAlgoType == PG_REP_FIFO)
		return 1;
	return 0;
}
uint32 isPageReplacmentAlgorithmModifiedCLOCK() {
	if (_PageRepAlgoType == PG_REP_MODIFIEDCLOCK)
		return 1;
	return 0;
}
/*2018*/uint32 isPageReplacmentAlgorithmDynamicLocal() {
	if (_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL)
		return 1;
	return 0;
}
/*2021*/uint32 isPageReplacmentAlgorithmNchanceCLOCK() {
	if (_PageRepAlgoType == PG_REP_NchanceCLOCK)
		return 1;
	return 0;
}
/*2021*/uint32 isPageReplacmentAlgorithmOPTIMAL() {
	if (_PageRepAlgoType == PG_REP_OPTIMAL)
		return 1;
	return 0;
}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt) {
	_EnableModifiedBuffer = enableIt;
}
uint8 isModifiedBufferEnabled() {
	return _EnableModifiedBuffer;
}

void enableBuffering(uint32 enableIt) {
	_EnableBuffering = enableIt;
}
uint8 isBufferingEnabled() {
	return _EnableBuffering;
}

void setModifiedBufferLength(uint32 length) {
	_ModifiedBufferLength = length;
}
uint32 getModifiedBufferLength() {
	return _ModifiedBufferLength;
}

//===============================
// FAULT HANDLERS
//===============================

//==================
// [0] INIT HANDLER:
//==================
void fault_handler_init() {
	//setPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX);
	//setPageReplacmentAlgorithmOPTIMAL();
	setPageReplacmentAlgorithmCLOCK();
	//setPageReplacmentAlgorithmModifiedCLOCK();
	enableBuffering(0);
	enableModifiedBuffer(0);
	setModifiedBufferLength(1000);
}
//==================
// [1] MAIN HANDLER:
//==================
/*2022*/
uint32 last_eip = 0;
uint32 before_last_eip = 0;
uint32 last_fault_va = 0;
uint32 before_last_fault_va = 0;
int8 num_repeated_fault = 0;
extern uint32 sys_calculate_free_frames();

struct Env* last_faulted_env = NULL;
void fault_handler(struct Trapframe *tf) {
	/******************************************************/
	// Read processor's CR2 register to find the faulting address
	uint32 fault_va = rcr2();
	//cprintf("************Faulted VA = %x************\n", fault_va);
	//	print_trapframe(tf);
	/******************************************************/

	//If same fault va for 3 times, then panic
	//UPDATE: 3 FAULTS MUST come from the same environment (or the kernel)
	struct Env* cur_env = get_cpu_proc();
	if (last_fault_va == fault_va && last_faulted_env == cur_env) {
		num_repeated_fault++;
		if (num_repeated_fault == 3) {
			print_trapframe(tf);
			panic(
					"Failed to handle fault! fault @ at va = %x from eip = %x causes va (%x) to be faulted for 3 successive times\n",
					before_last_fault_va, before_last_eip, fault_va);
		}
	} else {
		before_last_fault_va = last_fault_va;
		before_last_eip = last_eip;
		num_repeated_fault = 0;
	}
	last_eip = (uint32) tf->tf_eip;
	last_fault_va = fault_va;
	last_faulted_env = cur_env;
	/******************************************************/
	//2017: Check stack overflow for Kernel
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	if (!userTrap) {
		struct cpu* c = mycpu();
		//cprintf("trap from KERNEL\n");
		if (cur_env&& fault_va>= (uint32) cur_env->kstack&& fault_va < (uint32)cur_env->kstack + PAGE_SIZE)
			panic("User Kernel Stack: overflow exception!");
		else if (fault_va>= (uint32) c->stack&& fault_va < (uint32)c->stack + PAGE_SIZE)
			panic("Sched Kernel Stack of CPU #%d: overflow exception!",c - CPUS);
#if USE_KHEAP
		if (fault_va >= KERNEL_HEAP_MAX)
			panic("Kernel: heap overflow exception!");
#endif
	}
	//2017: Check stack underflow for User
	else {
		//cprintf("trap from USER\n");
		if (fault_va >= USTACKTOP && fault_va < USER_TOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	//cprintf("curenv = %x\n", curenv);
	struct Env* faulted_env = cur_env;
	if (faulted_env == NULL) {
		cprintf("\nFaulted VA = %x\n", fault_va);
		print_trapframe(tf);
		panic("faulted env == NULL!"); //not ours, it is for checking
	}
	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ((faulted_env->env_page_directory[PDX(fault_va)] & PERM_PRESENT)!= PERM_PRESENT) {
		faulted_env->tableFaultsCounter++;
		table_fault_handler(faulted_env, fault_va);
	} else {
		if (userTrap)
		{
			/*============================================================================================*/
			//TODO: [PROJECT'25.GM#3] FAULT HANDLER I - #2 Check for invalid pointers
			//(e.g. pointing to unmarked user heap page, kernel or wrong access rights),
			//your code is here
			// Fixed and Complete Fault Handler Checks
			//0. Kernel Guard:

			////el description bta3 el function////
			//Pointing to UNMARKED page in user heap (i.e. its PERM_UHPAGE should equals 0)
			//Pointing to kernel
			//Exist with read-only permissions
			//If invalid (any of above occur): it must be rejected without harm to the kernel or other running processes, by exiting the process using env_exit()

			//Check if pointing to kernel
			if (fault_va >= USER_TOP) {
				cprintf("Invalid!!! Because User Tried To Access Kernel Memory");
				//If invalid (any of above occur): it must be rejected without harm to the kernel or other running processes, by exiting the process using env_exit()
				env_exit();
				return;
			}
			//Check if pointing to unmarked page in user heap
			if (fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX) {
				int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
				if (!(perms & PERM_UHPAGE)) {
					cprintf("Invalid!!! Because User attempt to use an Unmarked Page In User Heap");
					//If invalid (any of above occur): it must be rejected without harm to the kernel or other running processes, by exiting the process using env_exit()
					env_exit();
					return;
				}
			}
			//Check if trying to write to read-only page
			if (tf->tf_err & FEC_WR) {
				int perms = pt_get_page_permissions(faulted_env->env_page_directory, fault_va);
				if ((perms & PERM_PRESENT) && !(perms & PERM_WRITEABLE)) {
					cprintf("Invalid!!! Because User Trying To Write to Read-only Page");
					//If invalid (any of above occur): it must be rejected without harm to the kernel or other running processes, by exiting the process using env_exit()
					env_exit();
					return;
				}
			}
			//Check illegal low addresses
			if (fault_va < UTEXT) {
				cprintf("Invalid!!! Illegal Low Addresses At VA %x",fault_va);
				//If invalid (any of above occur): it must be rejected without harm to the kernel or other running processes, by exiting the process using env_exit()
				env_exit();
				return;
			}
			/*============================================================================================*/
		}

		/*2022: Check if fault due to Access Rights */
		int perms = pt_get_page_permissions(faulted_env->env_page_directory,fault_va);
		if (perms & PERM_PRESENT)
			panic("Page @va=%x is exist! page fault due to violation of ACCESS RIGHTS\n",fault_va);
		/*============================================================================================*/

		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter++;

//				cprintf("[%08s] user PAGE fault va %08x\n", faulted_env->prog_name, fault_va);
//				cprintf("\nPage working set BEFORE fault handler...\n");
//				env_page_ws_print(faulted_env);
		//int ffb = sys_calculate_free_frames();

		if (isBufferingEnabled()) {
			__page_fault_handler_with_buffering(faulted_env, fault_va);
		} else {
			page_fault_handler(faulted_env, fault_va);
		}

		//		cprintf("\nPage working set AFTER fault handler...\n");
		//		env_page_ws_print(faulted_env);
		//		int ffa = sys_calculate_free_frames();
		//		cprintf("fault handling @%x: difference in free frames (after - before = %d)\n", fault_va, ffa - ffb);
	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/
}

//=========================
// [2] TABLE FAULT HANDLER:
//=========================
void table_fault_handler(struct Env * curenv, uint32 fault_va) {
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory,(uint32) fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//=========================
// [3] PAGE FAULT HANDLER:
//=========================
/* Calculate the number of page faults according th the OPTIMAL replacement strategy
 * Given:
 * 	1. Initial Working Set List (that the process started with)
 * 	2. Max Working Set Size
 * 	3. Page References List (contains the stream of referenced VAs till the process finished)
 *
 * 	IMPORTANT: This function SHOULD NOT change any of the given lists
 */
int get_optimal_num_faults(struct WS_List *initWorkingSet, int maxWSSize,struct PageRef_List *pageReferences) {

	//TODO: [PROJECT'25.IM#1] FAULT HANDLER II - #2 get_optimal_num_faults
	//Your code is here
	//Comment the following line
	//panic("get_optimal_num_faults() is not implemented yet...!!");

	int elfaults = 0;
	struct WS_List eltempWset;
	LIST_INIT(&eltempWset);
	struct WorkingSetElement* ws_Element;
	LIST_FOREACH (ws_Element, initWorkingSet)
	{
		struct WorkingSetElement* N_ws_Element =(struct WorkingSetElement *) kmalloc(sizeof(struct WorkingSetElement));
		N_ws_Element->virtual_address = ws_Element->virtual_address;
		LIST_INSERT_TAIL(&eltempWset, N_ws_Element);
	}
	struct PageRefElement* elReference;
	struct PageRefElement* elfutureReference;
	LIST_FOREACH ( elReference, pageReferences )
	{
		bool found_toggle = 0;
		struct WorkingSetElement* eltempWSelement;
		LIST_FOREACH ( eltempWSelement, & eltempWset)
		{
		if ( ROUNDDOWN(eltempWSelement->virtual_address,PAGE_SIZE) == ROUNDDOWN ( elReference->virtual_address, PAGE_SIZE)) {
				found_toggle = 1;
				break;
			}
		}
		if (!found_toggle) {
			elfaults++;
			if ( LIST_SIZE (&eltempWset) < maxWSSize) {
				struct WorkingSetElement* localScope_newWSE =(struct WorkingSetElement*) kmalloc(sizeof(struct WorkingSetElement));
				localScope_newWSE->virtual_address =elReference->virtual_address;
				LIST_INSERT_TAIL(&eltempWset, localScope_newWSE);
			} else {
				struct WorkingSetElement* el_evicted = NULL;
				int distance_maximum = -1;
				LIST_FOREACH ( eltempWSelement, &eltempWset )
				{
					int distance = 0;
					bool future_found = 0;

					struct PageRefElement* trailing_ahead = LIST_NEXT(elReference);
					while (trailing_ahead != NULL) {
						distance++;
						if (ROUNDDOWN(trailing_ahead->virtual_address,PAGE_SIZE) == ROUNDDOWN(eltempWSelement->virtual_address, PAGE_SIZE)) {
							future_found = 1;
							break;
						}
						trailing_ahead = LIST_NEXT(trailing_ahead);
					}

					if (!future_found) {
						el_evicted = eltempWSelement;
						break;
					} else {
						if (distance > distance_maximum) {
							distance_maximum = distance;
							el_evicted = eltempWSelement;
						}
					}
				}
				if (el_evicted != NULL) {
					el_evicted->virtual_address = elReference->virtual_address;
				}
			}
		}
}
while(!LIST_EMPTY( & eltempWset) ) {
    struct WorkingSetElement* elm = LIST_FIRST ( &eltempWset );
	LIST_REMOVE(&eltempWset, elm);
	kfree(elm);
}
return elfaults;
}

void page_fault_handler(struct Env * faulted_env, uint32 fault_va) {
#if USE_KHEAP
	if (isPageReplacmentAlgorithmOPTIMAL()) {
		//TODO: [PROJECT'25.IM#1] FAULT HANDLER II - #1 Optimal Reference Stream
		//Your code is here
		//Comment the following line
		//panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");

		// to force faults for all subsequent accesses (re-discovering the WS).
		if (LIST_EMPTY(&(faulted_env->referenceStreamList))) {
			struct WorkingSetElement* wsElement;
			LIST_FOREACH(wsElement, &( faulted_env->page_WS_list ) )
			{
				pt_set_page_permissions(faulted_env->env_page_directory,wsElement->virtual_address, 0, PERM_PRESENT);
			}
		}
		uint32 elFaultVAElRounded = ROUNDDOWN(fault_va, PAGE_SIZE);
		struct WorkingSetElement* elActiveWse = NULL;
		bool presentFilActive_toggle = 0;
		LIST_FOREACH ( elActiveWse, & (faulted_env->ActiveWorkingSet))
		{
			if ( ROUNDDOWN(elActiveWse->virtual_address, PAGE_SIZE)== elFaultVAElRounded) {
				presentFilActive_toggle = 1;
				break;
			}
		}
		if (presentFilActive_toggle) {
			pt_set_page_permissions(faulted_env->env_page_directory, fault_va,PERM_PRESENT, 0);
		}else {
			if (LIST_SIZE(&(faulted_env->ActiveWorkingSet))>= faulted_env->page_WS_max_size) {
				while (!LIST_EMPTY(&(faulted_env->ActiveWorkingSet))) {
					struct WorkingSetElement* optimalsElm = LIST_FIRST(&(faulted_env->ActiveWorkingSet));
					pt_set_page_permissions(faulted_env->env_page_directory,optimalsElm->virtual_address, 0, PERM_PRESENT);
					LIST_REMOVE ( & ( faulted_env->ActiveWorkingSet), optimalsElm );
					kfree(optimalsElm);
				}
			}
			struct WorkingSetElement* elNew_activeSet =(struct WorkingSetElement*) kmalloc(sizeof(struct WorkingSetElement));
			elNew_activeSet->virtual_address = elFaultVAElRounded;
			LIST_INSERT_TAIL(&(faulted_env->ActiveWorkingSet), elNew_activeSet);

			struct PageRefElement* ElNew_Ref =(struct PageRefElement *) kmalloc(sizeof(struct PageRefElement));
			ElNew_Ref->virtual_address = fault_va;
			LIST_INSERT_TAIL(&(faulted_env->referenceStreamList), ElNew_Ref);

			uint32* ptr_table;
			struct FrameInfo* ptr_frame_info = get_frame_info(faulted_env->env_page_directory, elFaultVAElRounded,&ptr_table);
			if (ptr_frame_info != NULL) {
				pt_set_page_permissions(faulted_env->env_page_directory,elFaultVAElRounded,PERM_PRESENT | PERM_USER | PERM_WRITEABLE, 0);
			}
			else {
				struct FrameInfo* elPnewFrame = NULL;
				if (allocate_frame(&elPnewFrame) == E_NO_MEM) {
					panic("page_fault_handler().OPTIMAL placement failed, no free frames");
				}
				map_frame(faulted_env->env_page_directory, elPnewFrame,elFaultVAElRounded, PERM_USER | PERM_WRITEABLE);
				int iret = pf_read_env_page(faulted_env,(void*) elFaultVAElRounded);
				if (iret == E_PAGE_NOT_EXIST_IN_PF) {
					if (!((elFaultVAElRounded >= USTACKBOTTOM&& elFaultVAElRounded < USTACKTOP)|| (elFaultVAElRounded >= USER_HEAP_START&& elFaultVAElRounded < USER_HEAP_MAX))) {
						env_exit();
						return;
					}
					memset((void *) elFaultVAElRounded, 0, PAGE_SIZE);
				}
			}
		}
	} else {
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(faulted_env->page_WS_list));
		if (wsSize < (faulted_env->page_WS_max_size)) {
			//TODO: [PROJECT'25.GM#3] FAULT HANDLER I - #3 placement
			//Your code is here
			//Comment the following line
			//panic("page_fault_handler().PLACEMENT is not implemented yet...!!");

			  ////el description bta3 el function////
			//Allocate space for the faulted page
			//Read the faulted page from page file to memory
			//If the page does not exist on page file, then
			//If it is a stack or a heap page, then, it’s OK.
			//Else, it must be rejected without harm to the kernel or other running processes, by exiting the process.
			//Reflect the changes in the page working set list (i.e. add new element to list & update its last one)

			uint32 elVAElRounded = ROUNDDOWN(fault_va, PAGE_SIZE);
			struct FrameInfo* allocateNewFrameFaultFunc = NULL;
			if (allocate_frame(&allocateNewFrameFaultFunc) == E_NO_MEM) {
				panic("page_fault_handler().PLACEMENT failed, no free frames available to allocate");
			}
			map_frame(faulted_env->env_page_directory, allocateNewFrameFaultFunc, elVAElRounded, PERM_PRESENT | PERM_USER | PERM_WRITEABLE);

			int ret = pf_read_env_page(faulted_env, (void*) elVAElRounded);
			if (ret == E_PAGE_NOT_EXIST_IN_PF) {
				if (!((elVAElRounded >= USTACKBOTTOM && elVAElRounded < USTACKTOP) || (elVAElRounded >= USER_HEAP_START && elVAElRounded < USER_HEAP_MAX))) {
					cprintf("[%s] ILLEGAL MEMORY ACCESS at address %x\n",faulted_env->prog_name, elVAElRounded);
					env_exit();
					return;
				}
				memset((void *) elVAElRounded, 0, PAGE_SIZE);
			}
			struct WorkingSetElement *newWSFaultFuncPlacment = env_page_ws_list_create_element(faulted_env, elVAElRounded);
			LIST_INSERT_TAIL(&(faulted_env->page_WS_list), newWSFaultFuncPlacment);

			uint32 elSizeBta3ElWorkingSet = LIST_SIZE(&(faulted_env->page_WS_list));
			if (elSizeBta3ElWorkingSet == faulted_env->page_WS_max_size) {
				faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
			}

		} else {
			if (isPageReplacmentAlgorithmCLOCK()) {
				//TODO: [PROJECT'25.IM#1] FAULT HANDLER II - #3 Clock Replacement
				//Your code is here
				//Comment the following line
				//panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");

			if (LIST_SIZE(&(faulted_env->page_WS_list))< faulted_env->page_WS_max_size) {
			    struct WorkingSetElement* elNewWse_clk =env_page_ws_list_create_element(faulted_env,fault_va);
				LIST_INSERT_TAIL(&(faulted_env->page_WS_list),elNewWse_clk);
				if (LIST_SIZE(&(faulted_env->page_WS_list))== faulted_env->page_WS_max_size) {
					if (faulted_env->page_last_WS_element != NULL) {
							while( LIST_FIRST(&(faulted_env->page_WS_list))!= faulted_env->page_last_WS_element) {
								struct WorkingSetElement *theOldTrailinghead =LIST_FIRST(&(faulted_env->page_WS_list));
								LIST_REMOVE(&(faulted_env->page_WS_list),theOldTrailinghead);
								LIST_INSERT_TAIL(&(faulted_env->page_WS_list),theOldTrailinghead);
							}
						} else {
							faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
						}
					}
					uint32 ElfaultElRounded_clk = ROUNDDOWN(fault_va,PAGE_SIZE);
					struct FrameInfo* pt_TO_neframe = NULL;
					if (allocate_frame(&pt_TO_neframe) == E_NO_MEM) {
						panic("page_fault_handler().CLOCK placement failed, no free frames");
					}
					map_frame(faulted_env->env_page_directory, pt_TO_neframe,ElfaultElRounded_clk, PERM_USER | PERM_WRITEABLE);
					int ret = pf_read_env_page(faulted_env,(void*) ElfaultElRounded_clk);
					if (ret == E_PAGE_NOT_EXIST_IN_PF) {
					  if (!((ElfaultElRounded_clk >= USTACKBOTTOM&& ElfaultElRounded_clk < USTACKTOP)|| (ElfaultElRounded_clk >= USER_HEAP_START	&& ElfaultElRounded_clk < USER_HEAP_MAX))) {
							env_exit();
							return;
						}
					  memset((void *) ElfaultElRounded_clk, 0, PAGE_SIZE);
					}
				} else {
					if (faulted_env->page_last_WS_element == NULL) {
						faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
					}
					struct WorkingSetElement* elVictim_clk = NULL;
					while (1) {
						struct WorkingSetElement* elCurrent_ws_element_clk =faulted_env->page_last_WS_element;
						uint32 perms_clk = pt_get_page_permissions(faulted_env->env_page_directory,elCurrent_ws_element_clk->virtual_address);
						if (perms_clk & PERM_USED) {
							pt_set_page_permissions(faulted_env->env_page_directory,elCurrent_ws_element_clk->virtual_address,0, PERM_USED);
							faulted_env->page_last_WS_element = LIST_NEXT(elCurrent_ws_element_clk);
							if (faulted_env->page_last_WS_element == NULL) {
								faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
							}
						} else {
							elVictim_clk = elCurrent_ws_element_clk;
							uint32 VICTIM_VA_CLK = elVictim_clk->virtual_address;
							uint32 VICTIM_PERMS_CLK = pt_get_page_permissions(faulted_env->env_page_directory,VICTIM_VA_CLK);

							if (VICTIM_PERMS_CLK & PERM_MODIFIED) {
								uint32 *ptr_table_CLK;
								struct FrameInfo *ptr_frame_info_CLK =get_frame_info(faulted_env->env_page_directory,VICTIM_VA_CLK, &ptr_table_CLK);
								if (ptr_frame_info_CLK != NULL) {
									pf_update_env_page(faulted_env,VICTIM_VA_CLK, ptr_frame_info_CLK);
								}
							}
							unmap_frame(faulted_env->env_page_directory,VICTIM_VA_CLK);
							break;
						}
					}
					uint32 ELFAULTVA_ELROUNDED_CLK = ROUNDDOWN(fault_va,PAGE_SIZE);
					elVictim_clk->virtual_address = ELFAULTVA_ELROUNDED_CLK;
					struct FrameInfo *p_new_frame = NULL;
					if (allocate_frame(&p_new_frame) == E_NO_MEM) {
						panic("page_fault_handler().CLOCK replacement failed, no free frames");
					}
					map_frame(faulted_env->env_page_directory, p_new_frame,ELFAULTVA_ELROUNDED_CLK,PERM_USER | PERM_WRITEABLE);

					int ret = pf_read_env_page(faulted_env,(void*) ELFAULTVA_ELROUNDED_CLK);
					if (ret == E_PAGE_NOT_EXIST_IN_PF) {
						if (!((ELFAULTVA_ELROUNDED_CLK >= USTACKBOTTOM&& ELFAULTVA_ELROUNDED_CLK < USTACKTOP)|| (ELFAULTVA_ELROUNDED_CLK >= USER_HEAP_START&& ELFAULTVA_ELROUNDED_CLK< USER_HEAP_MAX))) {
							env_exit();
							return;
						}
						memset((void *) ELFAULTVA_ELROUNDED_CLK, 0, PAGE_SIZE);
					}

					struct WorkingSetElement *elmNEXT_TO_VICTIM;
					if (elVictim_clk == LIST_LAST(&(faulted_env->page_WS_list))) {
						elmNEXT_TO_VICTIM = LIST_FIRST(&(faulted_env->page_WS_list));
					}else {
						elmNEXT_TO_VICTIM = LIST_NEXT(elVictim_clk);
					}

					while ( LIST_FIRST(&(faulted_env->page_WS_list))!= elmNEXT_TO_VICTIM) {
						struct WorkingSetElement *el_old_head_clk = LIST_FIRST(&(faulted_env->page_WS_list));
						LIST_REMOVE(&(faulted_env->page_WS_list),el_old_head_clk);
						LIST_INSERT_TAIL(&(faulted_env->page_WS_list),el_old_head_clk);
					}

					faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
				}

			} else if (isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX)) {
				//TODO: [PROJECT'25.IM#6] FAULT HANDLER II - #2 LRU Aging Replacement
				//Your code is here
				//Comment the following line
				//panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");

				struct WorkingSetElement *victimWSElement = NULL;
				struct WorkingSetElement *curr_wse = NULL;
				uint32 min_time_stamp = 0xFFFFFFFF;
				LIST_FOREACH(curr_wse, &(faulted_env->page_WS_list))
				{
					if (curr_wse->time_stamp < min_time_stamp) {
						min_time_stamp = curr_wse->time_stamp;
						victimWSElement = curr_wse;
					}
				}

				if (victimWSElement == NULL)
					victimWSElement = LIST_FIRST(&(faulted_env->page_WS_list));
				if (victimWSElement == NULL)
					panic("LRU Replacement: WS empty!");

				uint32 victim_va = victimWSElement->virtual_address;
				uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, victim_va);
				if (perms & PERM_MODIFIED) {
					uint32 *ptr_table;
					struct FrameInfo *ptr_frame_info = get_frame_info(faulted_env->env_page_directory, victim_va, &ptr_table);
					if (ptr_frame_info != NULL) {
						pf_update_env_page(faulted_env, victim_va, ptr_frame_info);
						unmap_frame(faulted_env->env_page_directory,(uint32) PGFLTEMP);
					}
				}

				unmap_frame(faulted_env->env_page_directory, victim_va);

				uint32 rounded_fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);
				victimWSElement->virtual_address = rounded_fault_va;
				victimWSElement->time_stamp = 0;

				struct FrameInfo *p_new_frame = NULL;
				if (allocate_frame(&p_new_frame) == E_NO_MEM)
					panic("page_fault_handler().REPLACEMENT (LRU) failed, no free frames");

				map_frame(faulted_env->env_page_directory, p_new_frame,rounded_fault_va, PERM_PRESENT|PERM_USER | PERM_WRITEABLE);

				int ret = pf_read_env_page(faulted_env,(void*) rounded_fault_va);
				if (ret == E_PAGE_NOT_EXIST_IN_PF) {
					if (!((rounded_fault_va >= USTACKBOTTOM && rounded_fault_va < USTACKTOP) || (rounded_fault_va >= USER_HEAP_START && rounded_fault_va < USER_HEAP_MAX))) {
						env_exit();
						return;
					}
					memset((void *) rounded_fault_va, 0, PAGE_SIZE);
				}

				if (LIST_SIZE(&(faulted_env->page_WS_list))== faulted_env->page_WS_max_size) {
					faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
				} else {
					faulted_env->page_last_WS_element = NULL;
				}

			} else if (isPageReplacmentAlgorithmModifiedCLOCK()) {
				//TODO: [PROJECT'25.IM#6] FAULT HANDLER II - #3 Modified Clock Replacement
				//Your code is here
				//Comment the following line
				//panic("page_fault_handler().REPLACEMENT is not implemented yet...!!");

				struct WorkingSetElement *victimWSElement = NULL;
				struct WorkingSetElement *iterator = faulted_env->page_last_WS_element;

				if (iterator == NULL) {
					iterator = LIST_FIRST(&(faulted_env->page_WS_list));
				}

				while (1) {
					struct WorkingSetElement *start_node = iterator;
					int found = 0;

					do {
						uint32 va = iterator->virtual_address;
						uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, va);
						bool used = perms & PERM_USED;
						bool modified = perms & PERM_MODIFIED;

						if (!used && !modified) {
							victimWSElement = iterator;
							found = 1;
							break;
						}
						if (iterator == LIST_LAST(&(faulted_env->page_WS_list))) {
							iterator = LIST_FIRST(&(faulted_env->page_WS_list));
						} else {
							iterator = LIST_NEXT(iterator);
						}
					} while (iterator != start_node);

					if (found)
						break;
					do {
						uint32 va = iterator->virtual_address;
						uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, va);
						bool used = perms & PERM_USED;

						if (!used) {
							victimWSElement = iterator;
							found = 1;
							break;
						} else {
							pt_set_page_permissions(faulted_env->env_page_directory, va, 0,PERM_USED);
						}

						if (iterator == LIST_LAST(&(faulted_env->page_WS_list))) {
							iterator = LIST_FIRST(&(faulted_env->page_WS_list));
						} else {
							iterator = LIST_NEXT(iterator);
						}
					} while (iterator != start_node);

					if (found)
						break;
				}
				struct WorkingSetElement *next_node;
				if (victimWSElement == LIST_LAST(&(faulted_env->page_WS_list))) {
					next_node = LIST_FIRST(&(faulted_env->page_WS_list));
				} else {
					next_node = LIST_NEXT(victimWSElement);
				}

				while (LIST_FIRST(&(faulted_env->page_WS_list)) != next_node) {
					struct WorkingSetElement *head = LIST_FIRST(&(faulted_env->page_WS_list));
					LIST_REMOVE(&(faulted_env->page_WS_list), head);
					LIST_INSERT_TAIL(&(faulted_env->page_WS_list), head);
				}
				uint32 victim_va = victimWSElement->virtual_address;
				uint32 perms = pt_get_page_permissions(faulted_env->env_page_directory, victim_va);
				if (perms & PERM_MODIFIED) {
					struct FrameInfo *ptr_frame_info;
					uint32 *ptr_table;
					ptr_frame_info = get_frame_info(faulted_env->env_page_directory, victim_va,&ptr_table);

					pf_update_env_page(faulted_env, victim_va, ptr_frame_info);

					unmap_frame(ptr_page_directory, (uint32) PGFLTEMP);

					tlb_invalidate(ptr_page_directory, (void*) PGFLTEMP);
				}
				unmap_frame(faulted_env->env_page_directory, victim_va);

				uint32 rounded_fault_va = ROUNDDOWN(fault_va, PAGE_SIZE);
				victimWSElement->virtual_address = rounded_fault_va;
				victimWSElement->time_stamp = 0;

				struct FrameInfo *p_new_frame = NULL;
				if (allocate_frame(&p_new_frame) == E_NO_MEM) {
					panic("Modified CLOCK: No free frames!");
				}
				int map_ret = map_frame(faulted_env->env_page_directory,p_new_frame, rounded_fault_va,PERM_PRESENT|PERM_USER | PERM_WRITEABLE);
				if (map_ret != 0)
					panic("Modified CLOCK: map_frame failed");

				tlb_invalidate(faulted_env->env_page_directory,(void*) rounded_fault_va);

				int ret = pf_read_env_page(faulted_env,(void*) rounded_fault_va);
				if (ret == E_PAGE_NOT_EXIST_IN_PF) {
					if (!((rounded_fault_va >= USTACKBOTTOM && rounded_fault_va < USTACKTOP)|| (rounded_fault_va >= USER_HEAP_START && rounded_fault_va < USER_HEAP_MAX))) {
						env_exit();
						return;
					}
					memset((void *) rounded_fault_va, 0, PAGE_SIZE);
				}
				faulted_env->page_last_WS_element = LIST_FIRST(&(faulted_env->page_WS_list));
			}
		}
	}
#endif
}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va) {
	panic("this function is not required...!!");
}

