#include "kheap.h"
#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include <kern/conc/sleeplock.h>
#include <kern/proc/user_environment.h>
#include <kern/mem/memory_manager.h>
#include "../conc/kspinlock.h"

struct kspinlock Lock;
uint32 allocatedSize[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE] = { 0 };
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//==============================================
// [1] INITIALIZE KERNEL HEAP:
//==============================================
//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #0 kheap_init [GIVEN]
//Remember to initialize locks (if any)
void kheap_init() {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		initialize_dynamic_allocator(KERNEL_HEAP_START,KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE);
		set_kheap_strategy(KHP_PLACE_CUSTOMFIT);
		kheapPageAllocStart = dynAllocEnd + PAGE_SIZE;
		kheapPageAllocBreak = kheapPageAllocStart;
		init_kspinlock(&Lock, "kheap_lock");
	}
	//==================================================================================
	//==================================================================================
}

//==============================================
// [2] GET A PAGE FROM THE KERNEL FOR DA:
//==============================================
int get_page(void* va) {
	int ret = alloc_page(ptr_page_directory, ROUNDDOWN((uint32 )va, PAGE_SIZE),PERM_WRITEABLE, 1);
	if (ret < 0)
		panic("get_page() in kern: failed to allocate page from the kernel");
	return 0;
}

//==============================================
// [3] RETURN A PAGE FROM THE DA TO KERNEL:
//==============================================
void return_page(void* va) {
	unmap_frame(ptr_page_directory, ROUNDDOWN((uint32 )va, PAGE_SIZE));
}

struct FreeBlockNode {
	uint32 start_va;
	uint32 num_pages;
	struct FreeBlockNode* next;
};
struct FreeBlockNode* free_head = NULL;
static void MERGEE(void) {
 if (!free_head || !free_head->next){
		return;
	}
 struct FreeBlockNode* Metrateba = NULL;
 struct FreeBlockNode* Delwa2ty = free_head;
 while (Delwa2ty) {
  struct FreeBlockNode* Elgaya = Delwa2ty->next;
  if (!Metrateba || Delwa2ty->start_va < Metrateba->start_va) {
	 Delwa2ty->next = Metrateba;
	 Metrateba = Delwa2ty;
   }
 else{
	struct FreeBlockNode* s = Metrateba;
	while (s->next && s->next->start_va < Delwa2ty->start_va) {
	    s = s->next;
  }
	Delwa2ty->next = s->next;
	s->next = Delwa2ty;
   }
	Delwa2ty = Elgaya;
 }
    free_head = Metrateba;
	Delwa2ty = free_head;
	while (Delwa2ty && Delwa2ty->next) {
	uint32 ENDdelwa2ty = Delwa2ty->start_va + Delwa2ty->num_pages * PAGE_SIZE;
	if (ENDdelwa2ty == Delwa2ty->next->start_va) {
	struct FreeBlockNode* Haytmes7 = Delwa2ty->next;
	Delwa2ty->num_pages += Haytmes7->num_pages;
	Delwa2ty->next = Haytmes7->next;
	kfree(Haytmes7);
	}
	else{
		Delwa2ty = Delwa2ty->next;
	}
  }
}
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===================================
// [1] ALLOCATE SPACE IN KERNEL HEAP:
//===================================

void* kmalloc(unsigned int size) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #1 kmalloc
	//Your code is here
	//Comment the following line
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//TODO: [PROJECT'25.BONUS#3] FAST PAGE ALLOCATOR------> "function run in 4.6s for PAGE ALLOCATOR"

	////el description bta3 el function////
	//If size ≤ DYN_ALLOC_MAX_BLOCK_SIZE:[BLOCK ALLOCATOR]
	//Use dynamic allocator to allocate the required space
	//Else:[PAGE ALLOCATOR]
	//Allocate & map the required space on page-boundaries using CUSTOM FIT strategy
	//If failed to allocate: return NULL

	acquire_kspinlock(&Lock);

	uint32 elpageselly3awzenha = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	uint32 *itspointerrr = NULL;
	uint32 BydaytElcustomm = 0;
	uint32 BydaytElworsT = 0;
	uint32 ElpagesBta3tWorst = 0;
	uint32 ElstratElfadya = 0;
	uint32 ElpagesEllyFadya = 0;

	if (size == 0) {
		release_kspinlock(&Lock);
		return NULL;
	}
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
		void* blockyyellyetallocted;
		blockyyellyetallocted = alloc_block(size);
		release_kspinlock(&Lock);
		return blockyyellyetallocted;
	}
	if (kheapPageAllocBreak + elpageselly3awzenha * PAGE_SIZE >= KERNEL_HEAP_MAX || kheapPageAllocBreak + elpageselly3awzenha * PAGE_SIZE< kheapPageAllocBreak)
	{
		release_kspinlock(&Lock);
		return NULL;
	}
	//CUSTOM FIT Strategy
	//1. Search for EXACT fit
	//2. if not found, search for WORST fit till break
	//3.if not found, extend BREAK if available
	//4.if not available, return NULL
	if (kheapPageAllocBreak == 0) {
		kheapPageAllocStart = ROUNDUP(dynAllocEnd, PAGE_SIZE);
		kheapPageAllocBreak = kheapPageAllocStart;
	}
	uint32 alo2aaAdd = kheapPageAllocStart;
	while (alo2aaAdd < kheapPageAllocBreak) {
		struct FrameInfo *framyyyy;
		uint32 *bas;
		framyyyy = NULL;
		bas = NULL;
		framyyyy = get_frame_info(ptr_page_directory, alo2aaAdd, &bas);
		if (framyyyy == NULL) {
			if (ElpagesEllyFadya == 0)
				ElstratElfadya = alo2aaAdd;
			    ElpagesEllyFadya++;
		} else {
			if (ElpagesEllyFadya > 0) {
				if (ElpagesEllyFadya == elpageselly3awzenha) {
					if (BydaytElcustomm == 0) {
						BydaytElcustomm = ElstratElfadya;
					}
				}
				if (ElpagesEllyFadya >= elpageselly3awzenha) {
					if (ElpagesEllyFadya > ElpagesBta3tWorst) {
						ElpagesBta3tWorst = ElpagesEllyFadya;
						BydaytElworsT = ElstratElfadya;
					}
				}

				ElpagesEllyFadya = 0;
			}
		}
		alo2aaAdd += PAGE_SIZE;
	}
	if (ElpagesEllyFadya > 0) {
		if (ElpagesEllyFadya == elpageselly3awzenha) {
			if (BydaytElcustomm == 0) {
				BydaytElcustomm = ElstratElfadya;
			}
		}

		if (ElpagesEllyFadya >= elpageselly3awzenha) {
			if (ElpagesEllyFadya > ElpagesBta3tWorst) {
				ElpagesBta3tWorst = ElpagesEllyFadya;
				BydaytElworsT = ElstratElfadya;
			}
		}
	}

	uint32 elBedayaBta3tna = 0;
	if (BydaytElcustomm != 0){
		elBedayaBta3tna = BydaytElcustomm;
	}
	else if (BydaytElworsT != 0){
		elBedayaBta3tna = BydaytElworsT;
	}
	if (elBedayaBta3tna != 0) {
		struct FrameInfo *Fofa;
		for (uint32 i = 0; i < elpageselly3awzenha; i++) {
			uint32 elPagEE = elBedayaBta3tna + i * PAGE_SIZE;
			if (allocate_frame(&Fofa) != 0) {
				for (uint32 j = 0; j < i; j++) {
					unmap_frame(ptr_page_directory,elBedayaBta3tna + j * PAGE_SIZE);
					free_frame(get_frame_info(ptr_page_directory, elBedayaBta3tna + j * PAGE_SIZE, &itspointerrr));
				}
				release_kspinlock(&Lock);
				return NULL;
			}
			map_frame(ptr_page_directory, Fofa, elPagEE, PERM_WRITEABLE | PERM_PRESENT);
		}
		uint32 allocated_va = elBedayaBta3tna;
		uint32 allocated_size = elpageselly3awzenha * PAGE_SIZE;

		for (uint32 i = 0; i < elpageselly3awzenha; i++) {
			allocatedSize[(allocated_va >> 12) + i] = allocated_size;
		}

		uint32 *ptr = NULL;
		struct FrameInfo *fi = get_frame_info(ptr_page_directory, allocated_va,&ptr);
		if (fi)
		fi->size = allocated_size;

		release_kspinlock(&Lock);
		return (void*) allocated_va;
	}

	uint32 elBreadEl2deem;
	elBreadEl2deem = kheapPageAllocBreak;
	uint32 elBreadElgdeeed = elBreadEl2deem + elpageselly3awzenha * PAGE_SIZE;
	if (elBreadElgdeeed < elBreadEl2deem || elBreadElgdeeed >= KERNEL_HEAP_MAX) {
		release_kspinlock(&Lock);
		return NULL;
	}
	uint32 ElpagesElExpanded = 0;
	struct FrameInfo *Fr = NULL;
	uint32 EladdExpanded = elBreadEl2deem;
	do {
		for (; ElpagesElExpanded < elpageselly3awzenha; ElpagesElExpanded++) {
			if (allocate_frame(&Fr) != 0)
				break;
			if (map_frame(ptr_page_directory, Fr, EladdExpanded,
					PERM_PRESENT | PERM_WRITEABLE) != 0) {
				free_frame(Fr);
				break;
			}
			EladdExpanded += PAGE_SIZE;
		}
		if (ElpagesElExpanded == elpageselly3awzenha) {
			kheapPageAllocBreak = elBreadElgdeeed;
			uint32 allocated_va = elBreadEl2deem;
			uint32 allocated_size = elpageselly3awzenha * PAGE_SIZE;

			for (uint32 i = 0; i < elpageselly3awzenha; i++) {
				allocatedSize[(allocated_va >> 12) + i] = allocated_size;
			}

			uint32 *ptr = NULL;
			struct FrameInfo *fi = get_frame_info(ptr_page_directory, allocated_va, &ptr);
			if (fi){
				fi->size = allocated_size;
			}

			release_kspinlock(&Lock);
			return (void*) allocated_va;
		}
		EladdExpanded = elBreadEl2deem;
		for (int Bosy = 0; Bosy < ElpagesElExpanded; Bosy++) {
			uint32 *ItsOverrr = NULL;
			struct FrameInfo *fi = get_frame_info(ptr_page_directory,EladdExpanded, &ItsOverrr);
			if (fi) {
				unmap_frame(ptr_page_directory, EladdExpanded);
				free_frame(fi);
			}
			EladdExpanded += PAGE_SIZE;
		}
	} while (0);
	release_kspinlock(&Lock);
	return NULL;

}

//=================================
// [2] FREE SPACE FROM KERNEL HEAP:
//=================================
void kfree(void* virtual_address) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #2 kfree
	//Your code is here
	//Comment the following line
	//panic("kfree() is not implemented yet...!!");------>"function run in 7s for PAGE ALLOCATOR"

	////el description bta3 el function////
	//If virtual address inside the [BLOCK ALLOCATOR] range
	//Use dynamic allocator to free the given address
	//If virtual address inside the [PAGE ALLOCATOR] range
	//FREE the space of the given address from RAM
	//Else (i.e. invalid address): should panic(…)

	acquire_kspinlock(&Lock);

	uint32 blkStartRange = dynAllocStart;
	uint32 blkEndRange = dynAllocEnd;
	uint32 kernelHeapStartAddressBoundry = KERNEL_HEAP_START;
	uint32 pageStart = kheapPageAllocStart;
	uint32 pageEndUsedRange = kheapPageAllocBreak;
	uint32 kernelHeapEndAddressBoundry = KERNEL_HEAP_MAX;

	// If virtual address inside the [BLOCK ALLOCATOR] range
	if ((uint32) virtual_address >= blkStartRange && (uint32) virtual_address < blkEndRange) {
		free_block(virtual_address);
		MERGEE(); //merge adjacent blocks
		release_kspinlock(&Lock);
		return;
	}

	// If virtual address inside the [PAGE ALLOCATOR] range
	if ((uint32) virtual_address >= pageStart && (uint32) virtual_address < kernelHeapEndAddressBoundry) {
		// FREE the space of the given address from RAM
		virtual_address = ROUNDDOWN(virtual_address, PAGE_SIZE);
		uint32 *ptrtable = NULL;
		struct FrameInfo *ptrFrameInfoKfreeFunc = get_frame_info(ptr_page_directory, (uint32) virtual_address, &ptrtable);

		if (ptrFrameInfoKfreeFunc == NULL) {
			release_kspinlock(&Lock);
			panic("trying to free unallocated page");
		}

		uint32 allocatedSizeKfreeFunc = allocatedSize[(uint32) virtual_address >> 12];
		if (allocatedSizeKfreeFunc == 0) {
			release_kspinlock(&Lock);
			panic("size not found for this address");
		}

		int totalNumOfPagesToBeHandled = (allocatedSizeKfreeFunc + PAGE_SIZE - 1) / PAGE_SIZE;
		for (int i = 0; i < totalNumOfPagesToBeHandled; i++) {
			return_page((void *) ((uint32) virtual_address + i * PAGE_SIZE));
			allocatedSize[((uint32) virtual_address >> 12) + i] = 0;
		}

		//Updating kheapPageAllocBreak
		uint32 returnThekheapPageAllocBreakToItsOriginal = kheapPageAllocBreak;
		while (returnThekheapPageAllocBreakToItsOriginal > pageStart) {
			uint32 validationOfVA = returnThekheapPageAllocBreakToItsOriginal- PAGE_SIZE;
			ptrtable = NULL;
			struct FrameInfo *framInfoPointerBreak = get_frame_info(ptr_page_directory, validationOfVA, &ptrtable);

			if (framInfoPointerBreak != NULL&& framInfoPointerBreak->references > 0) {
				break;
			}
			returnThekheapPageAllocBreakToItsOriginal -= PAGE_SIZE;
		}
		kheapPageAllocBreak = returnThekheapPageAllocBreakToItsOriginal;
		release_kspinlock(&Lock);
		return;
	}
	//invalid address? panic!!!!
	if ((uint32) virtual_address < (uint32) kernelHeapStartAddressBoundry || (uint32) virtual_address >= (uint32) kernelHeapEndAddressBoundry) {
		release_kspinlock(&Lock);
		panic("Invalid Address");
	}
}

//=================================
// [3] FIND VA OF GIVEN PA:
//=================================
unsigned int kheap_virtual_address(unsigned int physical_address) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #3 kheap_virtual_address
	//Your code is here
	//Comment the following line
	//panic("kheap_virtual_address() is not implemented yet...!!");

	////el description bta3 el function////
	//return the virtual address corresponding to given physical_address (including offset)
	//If no mapping, return 0.
	//It should work for both [BLOCK ALLOCATOR] and [PAGE ALLOCATOR]
	//It should run in O(1)
	acquire_kspinlock(&Lock);

	if (physical_address >= number_of_frames * PAGE_SIZE){
		release_kspinlock(&Lock);
		return 0;
	}
	struct FrameInfo *frame_info = to_frame_info(physical_address);

	if (frame_info == NULL || frame_info->va == 0) {
		release_kspinlock(&Lock);
		return 0;
	}
	if (frame_info->va < KERNEL_HEAP_START) {
		release_kspinlock(&Lock);
		return 0;
	}
	if (frame_info->va >= KERNEL_HEAP_MAX) {
		release_kspinlock(&Lock);
		return 0;
	}
	unsigned int virtual_address = frame_info->va;
	unsigned int PageOffset = physical_address & (PAGE_SIZE - 1);

	release_kspinlock(&Lock);
	return virtual_address + PageOffset;
	/*EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED */
}

//=================================
// [4] FIND PA OF GIVEN VA:
//=================================
unsigned int kheap_physical_address(unsigned int virtual_address) {
	//TODO: [PROJECT'25.GM#2] KERNEL HEAP - #4 kheap_physical_address
	//Your code is here
	//Comment the following line
	//panic("kheap_physical_address() is not implemented yet...!!");

	////el description bta3 el function////
	//return the physical address corresponding to given virtual_address (including offset)
	//If no mapping, return 0.
	//It should work for both [BLOCK ALLOCATOR] and [PAGE ALLOCATOR]
	//It should run in O(1)
	acquire_kspinlock(&Lock);

	if (virtual_address < KERNEL_HEAP_START) {
		release_kspinlock(&Lock);
		return 0;
	}
	if (virtual_address >= KERNEL_HEAP_MAX) {
		release_kspinlock(&Lock);
		return 0;
	}
	uint32 *ptr_page_table = NULL;
	unsigned int IndexOfDirectory = PDX(virtual_address);
	unsigned int OffsetPage = virtual_address & (PAGE_SIZE - 1);
	uint32 WhatTableGets = get_page_table(ptr_page_directory, virtual_address,&ptr_page_table);
	if (WhatTableGets == TABLE_NOT_EXIST || ptr_page_table == NULL) {
		release_kspinlock(&Lock);
		return 0;
	}
	uint32 page_index = PTX(virtual_address);
	uint32 page_entry = ptr_page_table[page_index];
	if ((page_entry & 0x1) != 0) {
		uint32 physical_address = page_entry & ~(PAGE_SIZE - 1);
		release_kspinlock(&Lock);
		return physical_address + OffsetPage;
	} else {
		release_kspinlock(&Lock);
		return 0;
	}
	/*EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED */
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

extern __inline__ uint32 get_block_size(void *va);

void *krealloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT'25.BONUS#2] KERNEL REALLOC - krealloc
	//Your code is here
	//Comment the following line
	//panic("krealloc() is not implemented yet...!!");

	uint32 blkStartRange = dynAllocStart;
	uint32 blkEndRange = dynAllocEnd;
	uint32 kernelHeapStartAddressBoundry = KERNEL_HEAP_START;
	uint32 pageStart = kheapPageAllocStart;
	uint32 pageEndUsedRange = kheapPageAllocBreak;
	uint32 kernelHeapEndAddressBoundry = KERNEL_HEAP_MAX;

	if(virtual_address==NULL){
		return kmalloc(new_size);
	}
	if(new_size==0){
	    kfree(virtual_address);
		return NULL;
	}
	if ((uint32) virtual_address >= blkStartRange && (uint32) virtual_address < blkEndRange){
		uint32 returnedOldSizeFromGetBlockSize=0;
		returnedOldSizeFromGetBlockSize=get_block_size(virtual_address);
		int oldBlocks=(returnedOldSizeFromGetBlockSize + DYN_ALLOC_MAX_BLOCK_SIZE - 1) / DYN_ALLOC_MAX_BLOCK_SIZE;
		int newBlocks = (new_size + DYN_ALLOC_MAX_BLOCK_SIZE - 1) / DYN_ALLOC_MAX_BLOCK_SIZE;

		if (new_size == returnedOldSizeFromGetBlockSize){
			return virtual_address;
		}
		else if(new_size < DYN_ALLOC_MAX_BLOCK_SIZE){
		    return realloc_block(virtual_address,new_size);
		}
		else{
			uint32 * newAllocatedAdress = kmalloc(new_size);
			if(newAllocatedAdress!=NULL){
			memcpy(newAllocatedAdress, virtual_address,get_block_size(virtual_address));
			free_block(virtual_address);
			return newAllocatedAdress;
			}
		}
	}
	if ((uint32) virtual_address >= pageStart && (uint32) virtual_address < kernelHeapEndAddressBoundry){
		uint32 returnedOldSizeFromGetBlockSize=0;
		virtual_address = ROUNDDOWN(virtual_address,PAGE_SIZE);
		new_size = ROUNDUP(new_size,PAGE_SIZE);
	    uint32 *ptrtable=NULL;
	    struct FrameInfo *ptrFrameInfoKreallocFunc=get_frame_info(ptr_page_directory,(uint32)virtual_address,&ptrtable);
	    returnedOldSizeFromGetBlockSize = ptrFrameInfoKreallocFunc->size;
	    int oldPages = (returnedOldSizeFromGetBlockSize + PAGE_SIZE - 1) / PAGE_SIZE;
	    int newPages = (new_size + PAGE_SIZE - 1) / PAGE_SIZE;

	    if(new_size==returnedOldSizeFromGetBlockSize){
	    	return virtual_address;
	    }
	    else if(new_size < returnedOldSizeFromGetBlockSize){
			if (new_size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
				uint32* newAllocatedAdderKreallocFunc = alloc_block(new_size);
				if (newAllocatedAdderKreallocFunc != NULL) {
					memcpy(newAllocatedAdderKreallocFunc, virtual_address, new_size);
					kfree(virtual_address);
				}
				return newAllocatedAdderKreallocFunc;
			}
			else{
	    	int freeFramesKFunc = oldPages - newPages;
			uint32 startAddressToFreeFrame =(uint32) virtual_address +new_size;
			uint32 endAddressToFreeFrame=(uint32)virtual_address+returnedOldSizeFromGetBlockSize;
			for (int i = startAddressToFreeFrame; i < endAddressToFreeFrame; i+=PAGE_SIZE) {
				uint32* ptrpagetable;
				struct FrameInfo* frameInfoKreallocFunc = get_frame_info(ptr_page_directory, i, &ptrpagetable);
				if(frameInfoKreallocFunc == NULL){
				   unmap_frame(ptr_page_directory, i);
				}else{
					uint32 NumOfFrameKreallocFunc = to_frame_number(frameInfoKreallocFunc);
				}
			}
			ptrFrameInfoKreallocFunc->size=new_size;
			return virtual_address;
		  }
	    }
	    else{
	    	uint32 pagesNeeded=(new_size-returnedOldSizeFromGetBlockSize)/PAGE_SIZE;
	    	uint32 elfar2BeenEL2Sizes=new_size-returnedOldSizeFromGetBlockSize;
	    	uint32 startAddrFrameKrealloc = (uint32)virtual_address + returnedOldSizeFromGetBlockSize;
	    	 int theNextfreePagesKreallocFunc = 0;
	    	 int thePrevfreePagesKreallocFunc = 0;
	    	 bool findIfCanExpand = 0;
	    	 for (uint32 initialVA = startAddrFrameKrealloc; initialVA < kernelHeapEndAddressBoundry; initialVA += PAGE_SIZE) {
	    	     uint32 *ptrtable;
	    	     if (get_frame_info(ptr_page_directory, initialVA, &ptrtable)== NULL){
	    	         theNextfreePagesKreallocFunc++;
	    	     }
	    	     else{
	    	         break;
	    	     }
	    	 }
	    	 for (uint32 initialVA = (uint32)virtual_address - PAGE_SIZE; initialVA >= pageStart; initialVA -= PAGE_SIZE) {
	    	     uint32 *ptr_table;
	    	     if (get_frame_info(ptr_page_directory, initialVA, &ptrtable) == NULL){
	    	         thePrevfreePagesKreallocFunc++;
	    	     }
	    	     else{
	    	       break;
	    	     }
	    	 }
	    	 if (theNextfreePagesKreallocFunc >= pagesNeeded) {
	    	     findIfCanExpand = 1;
	    	     startAddrFrameKrealloc = (uint32)virtual_address + returnedOldSizeFromGetBlockSize;
	    	 }else if(thePrevfreePagesKreallocFunc == pagesNeeded){
	    		 findIfCanExpand = 1;
	    		 startAddrFrameKrealloc = (uint32)virtual_address - elfar2BeenEL2Sizes;
	    	 } else if (thePrevfreePagesKreallocFunc >= pagesNeeded) {
	    	     findIfCanExpand = 1;
	    	     startAddrFrameKrealloc = (uint32)virtual_address - (pagesNeeded * PAGE_SIZE);
	    	 } else if ((thePrevfreePagesKreallocFunc + theNextfreePagesKreallocFunc) >= pagesNeeded) {
	    	     findIfCanExpand = 1;
	    	     startAddrFrameKrealloc = (uint32)virtual_address - (thePrevfreePagesKreallocFunc * PAGE_SIZE);
	    	 }
			if (findIfCanExpand) {
			    for (int i = 0; i < pagesNeeded; i++) {
					struct FrameInfo *allocatedFrameKreallocFunc;
					if (allocate_frame(&allocatedFrameKreallocFunc) == E_NO_MEM){
						return NULL;
					}
					if (startAddrFrameKrealloc == (uint32) virtual_address) {
						startAddrFrameKrealloc = (uint32) virtual_address+ returnedOldSizeFromGetBlockSize;
					}
					int ret=map_frame(ptr_page_directory, allocatedFrameKreallocFunc,startAddrFrameKrealloc,PERM_WRITEABLE | PERM_PRESENT);
				    if (ret== E_NO_MEM){
						return NULL;
				    }
				    startAddrFrameKrealloc += PAGE_SIZE;
				}
				 if (startAddrFrameKrealloc!= (uint32) virtual_address+ returnedOldSizeFromGetBlockSize) {
					memmove((void*) startAddrFrameKrealloc, virtual_address,returnedOldSizeFromGetBlockSize);
				}
				 if(theNextfreePagesKreallocFunc == pagesNeeded){
				ptrFrameInfoKreallocFunc->size = new_size;
				return (void*) startAddrFrameKrealloc;
			 }else if(thePrevfreePagesKreallocFunc == pagesNeeded){
				 ptrFrameInfoKreallocFunc->size = 0;
				 ptrFrameInfoKreallocFunc = get_frame_info(ptr_page_directory,(uint32)startAddrFrameKrealloc,&ptrtable);
				 ptrFrameInfoKreallocFunc->size = new_size;
				 startAddrFrameKrealloc = (uint32)virtual_address -elfar2BeenEL2Sizes;
				 memcpy((void *)startAddrFrameKrealloc,virtual_address,new_size);
				 return (void *)startAddrFrameKrealloc;
			 }else{
				 ptrFrameInfoKreallocFunc->size = 0;
			     ptrFrameInfoKreallocFunc = get_frame_info(ptr_page_directory,(uint32)startAddrFrameKrealloc,&ptrtable);
				 ptrFrameInfoKreallocFunc->size = new_size;
				 memcpy((void *)startAddrFrameKrealloc,virtual_address,new_size);
				 startAddrFrameKrealloc = (uint32)virtual_address - (thePrevfreePagesKreallocFunc*PAGE_SIZE);
				 return (void *)startAddrFrameKrealloc;
			 }
			}
			else{
			uint32 *newAllocationAddressKreallocFunc = kmalloc(new_size);
			if (newAllocationAddressKreallocFunc != NULL) {
			    memcpy(newAllocationAddressKreallocFunc, virtual_address, returnedOldSizeFromGetBlockSize);
			    kfree(virtual_address);
			}
			return newAllocationAddressKreallocFunc;
		  }
	    }
	}
	if ((uint32) virtual_address < (uint32) kernelHeapStartAddressBoundry || (uint32) virtual_address >= (uint32) kernelHeapEndAddressBoundry) {
			panic("Invalid Address");
	}
	return NULL;
}
