/*
 * chunk_operations.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include <kern/trap/fault_handler.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/proc/user_environment.h>
#include "kheap.h"
#include "memory_manager.h"
#include <inc/queue.h>
//#include <kern/mem/working_set_manager.h>
//extern void inctst();

/******************************/
/*[1] RAM CHUNKS MANIPULATION */
/******************************/

//===============================
// 1) CUT-PASTE PAGES IN RAM:
//===============================
//This function should cut-paste the given number of pages from source_va to dest_va on the given page_directory
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, cut-paste the number of pages and return 0
//	ALL 12 permission bits of the destination should be TYPICAL to those of the source
//	The given addresses may be not aligned on 4 KB
int cut_paste_pages(uint32* page_directory, uint32 source_va, uint32 dest_va,uint32 num_of_pages) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("cut_paste_pages() is not implemented yet...!!");
}

//===============================
// 2) COPY-PASTE RANGE IN RAM:
//===============================
//This function should copy-paste the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	If ANY of the destination pages exists with READ ONLY permission, deny the entire process and return -1.
//	If the page table at any destination page in the range is not exist, it should create it
//	If ANY of the destination pages doesn't exist, create it with the following permissions then copy.
//	Otherwise, just copy!
//		1. WRITABLE permission
//		2. USER/SUPERVISOR permission must be SAME as the one of the source
//	The given range(s) may be not aligned on 4 KB
int copy_paste_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va,uint32 size) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("copy_paste_chunk() is not implemented yet...!!");
}

//===============================
// 3) SHARE RANGE IN RAM:
//===============================
//This function should share the given size from source_va to dest_va on the given page_directory
//	Ranges DO NOT overlapped.
//	It should set the permissions of the second range by the given perms
//	If ANY of the destination pages exists, deny the entire process and return -1.
//	Otherwise, share the required range and return 0
//	During the share process:
//		1. If the page table at any destination page in the range is not exist, it should create it
//	The given range(s) may be not aligned on 4 KB
int share_chunk(uint32* page_directory, uint32 source_va, uint32 dest_va,uint32 size, uint32 perms) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("share_chunk() is not implemented yet...!!");
}

//===============================
// 4) ALLOCATE CHUNK IN RAM:
//===============================
//This function should allocate the given virtual range [<va>, <va> + <size>) in the given address space  <page_directory> with the given permissions <perms>.
//	If ANY of the destination pages exists, deny the entire process and return -1. Otherwise, allocate the required range and return 0
//	If the page table at any destination page in the range is not exist, it should create it
//	Allocation should be aligned on page boundary. However, the given range may be not aligned.
int allocate_chunk(uint32* page_directory, uint32 va, uint32 size, uint32 perms) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("allocate_chunk() is not implemented yet...!!");
}

//=====================================
// 5) CALCULATE FREE SPACE:
//=====================================
//It should count the number of free pages in the given range [va1, va2)
//(i.e. number of pages that are not mapped).
//Addresses may not be aligned on page boundaries
uint32 calculate_free_space(uint32* page_directory, uint32 sva, uint32 eva) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("calculate_free_space() is not implemented yet...!!");
}

//=====================================
// 6) CALCULATE ALLOCATED SPACE:
//=====================================
void calculate_allocated_space(uint32* page_directory, uint32 sva, uint32 eva,uint32 *num_tables, uint32 *num_pages) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("calculate_allocated_space() is not implemented yet...!!");
}

//=====================================
// 7) CALCULATE REQUIRED FRAMES IN RAM:
//=====================================
//This function should calculate the required number of pages for allocating and mapping the given range [start va, start va + size) (either for the pages themselves or for the page tables required for mapping)
//	Pages and/or page tables that are already exist in the range SHOULD NOT be counted.
//	The given range(s) may be not aligned on 4 KB
uint32 calculate_required_frames(uint32* page_directory, uint32 sva,uint32 size) {
	//TODO: PRACTICE: fill this function.
	//Comment the following line
	panic("calculate_required_frames() is not implemented yet...!!");
}

//=================================================================================//
//===========================END RAM CHUNKS MANIPULATION ==========================//
//=================================================================================//

/*******************************/
/*[2] USER CHUNKS MANIPULATION */
/*******************************/

//======================================================
/// functions used for USER HEAP (malloc, free, ...)
//======================================================
//=====================================
/* DYNAMIC ALLOCATOR SYSTEM CALLS */
//=====================================
/*2024*/
void* sys_sbrk(int numOfPages) {
	panic("not implemented function");
}

//=====================================
// 1) ALLOCATE USER MEMORY:
//=====================================
void allocate_user_mem(struct Env* e, uint32 virtual_address, uint32 size) {

	uint32 *Elpagetable_POINTER;
	Elpagetable_POINTER = NULL;

	if (virtual_address == 0 || size == 0)
		return;

	uint32 Bdaytna = ROUNDDOWN(virtual_address, PAGE_SIZE);
	uint32 A5erna = ROUNDUP(virtual_address + size, PAGE_SIZE);

	for (uint32 EladdElvertual = Bdaytna; EladdElvertual < A5erna;EladdElvertual += PAGE_SIZE)
		{
		get_page_table(e->env_page_directory, EladdElvertual,&Elpagetable_POINTER);

		if (EladdElvertual > USER_HEAP_MAX|| EladdElvertual < uheapPageAllocStart|| EladdElvertual > USER_HEAP_MAX) {
			continue;
		}

		if (Elpagetable_POINTER == NULL) {
			if (create_page_table(e->env_page_directory, EladdElvertual) < 0) {
				continue;
			}
			get_page_table(e->env_page_directory, EladdElvertual,&Elpagetable_POINTER);
		}

		if (Elpagetable_POINTER != NULL) {
			uint32 ElIndexx;
			ElIndexx = PTX(EladdElvertual);

			uint32 var = Elpagetable_POINTER[ElIndexx];
			if ((var & PERM_PRESENT) == 0) {
				Elpagetable_POINTER[ElIndexx] = PERM_USER | PERM_WRITEABLE| PERM_UHPAGE;
			} else {
				Elpagetable_POINTER[ElIndexx] |= PERM_UHPAGE;
			}
		}
	}
}
//=====================================
// 2) FREE USER MEMORY:
//=====================================
void free_user_mem(struct Env* e, uint32 virtual_address, uint32 size) {
	uint32 Bdaytna = ROUNDDOWN(virtual_address, PAGE_SIZE);
	uint32 A5erna = ROUNDUP(virtual_address + size, PAGE_SIZE);
	uint32 var = A5erna - Bdaytna;
	uint32 KolelPages = var / PAGE_SIZE;

	for (uint32 EladdElvertual = Bdaytna; EladdElvertual < A5erna;EladdElvertual += PAGE_SIZE)
		{
		uint32 *IamPoinerrrr = NULL;
		int x = (get_page_table(e->env_page_directory, EladdElvertual,&IamPoinerrrr) == 0);

		env_page_ws_invalidate(e, EladdElvertual);
		pf_remove_env_page(e, EladdElvertual);

		uint32 xx = PTX(EladdElvertual);
		IamPoinerrrr[xx] &= ~PERM_UHPAGE;

		tlb_invalidate(e->env_page_directory, (void*) EladdElvertual);
	}
}

//=====================================
// 4) FREE USER MEMORY (BUFFERING):
//=====================================
void __free_user_mem_with_buffering(struct Env* e, uint32 virtual_address,uint32 size) {
	// your code is here, remove the panic and write your code
	panic("__free_user_mem_with_buffering() is not implemented yet...!!");
}

//=====================================
// 3) MOVE USER MEMORY:
//=====================================
void move_user_mem(struct Env* e, uint32 src_virtual_address,uint32 dst_virtual_address, uint32 size) {
	panic("move_user_mem() is not implemented yet...!!");
}

//=================================================================================//
//========================== END USER CHUNKS MANIPULATION =========================//
//=================================================================================//

