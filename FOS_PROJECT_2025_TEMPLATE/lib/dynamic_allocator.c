/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
#include<lib/concurrency.c>/// needed for bonus ///
//global variables needed//
uint32 min_block_size = 8;
uint32 max_block_size = 2048;
volatile uint32 blockedVarDynallocAllocFunc=0;
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//==================================
//==================================
// [1] GET PAGE VA:
//==================================
__inline__ uint32 to_page_va(struct PageInfoElement *ptrPageInfo) {
	if (ptrPageInfo < &pageBlockInfoArr[0] || ptrPageInfo >= &pageBlockInfoArr[DYN_ALLOC_MAX_SIZE / PAGE_SIZE])
		panic("to_page_va called with invalid pageInfoPtr");
	//Get start VA of the page from the corresponding Page Info pointer
	int idxInPageInfoArr = (ptrPageInfo - pageBlockInfoArr);
	return dynAllocStart + (idxInPageInfoArr << PGSHIFT);
}

//==================================
// [2] GET PAGE INFO OF PAGE VA:
//==================================
__inline__ struct PageInfoElement * to_page_info(uint32 va) {
	int idxInPageInfoArr = (va - dynAllocStart) >> PGSHIFT;
	if (idxInPageInfoArr < 0|| idxInPageInfoArr >= DYN_ALLOC_MAX_SIZE/PAGE_SIZE)
		panic("to_page_info called with invalid pa");
	return &pageBlockInfoArr[idxInPageInfoArr];
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
bool is_initialized = 0;
void initialize_dynamic_allocator(uint32 daStart, uint32 daEnd) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		assert(daEnd <= daStart + DYN_ALLOC_MAX_SIZE);
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================
	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #1 initialize_dynamic_allocator
	//Your code is here
	//Comment the following line
	//	panic("initialize_dynamic_allocator() Not implemented yet");

	////el description bta3 el function////
	//Initialize the dynamic allocator starting from the given “daStart” to “daEnd”
	//The following items should be initialized here:
	//DA Limits
	//Array of Page Info
	//Free Page List
	//Free Block Lists
	dynAllocStart = daStart;
	dynAllocEnd = daEnd;
	uint32 samaNumofpages = (daEnd - daStart) / PAGE_SIZE;
	LIST_INIT(&freePagesList);
	for (uint32 i = 0; i < samaNumofpages; i++) {
		pageBlockInfoArr[i].block_size = 0;
		pageBlockInfoArr[i].num_of_free_blocks = 0;
		LIST_INSERT_TAIL(&freePagesList, &pageBlockInfoArr[i]);
	}
	for (uint32 i = 0; i < 9; i++) {
		LIST_INIT(&freeBlockLists[i]);
	}
	int totalBlockSizesDynallocInit = LOG2_MAX_SIZE - LOG2_MIN_SIZE + 1;
	for (int listIndexDynallocInit = 0; listIndexDynallocInit < totalBlockSizesDynallocInit; listIndexDynallocInit++) {
		LIST_INIT(&freeBlockLists[listIndexDynallocInit]);
	}
}

//===========================
// [2] GET BLOCK SIZE:
//===========================
__inline__ uint32 get_block_size(void *va) {
	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #2 get_block_size
	//Your code is here
	//Comment the following line
	//panic("get_block_size() Not implemented yet");
	uint32 pageidx = ((uint32) va - dynAllocStart) / PAGE_SIZE;
	uint32 blocksize = pageBlockInfoArr[pageidx].block_size;
	cprintf("VA = %x, page index = %d, block size = %d\n", (uint32) va, pageidx, blocksize);
	return blocksize;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================
void *alloc_block(uint32 size) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		assert(size <= DYN_ALLOC_MAX_BLOCK_SIZE);
	}
	//==================================================================================
	//==================================================================================
	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #3 alloc_block
	//Your code is here
	//Comment the following line
	//panic("alloc_block() Not implemented yet");

	////el description bta3 el function////
	//Allocate new block with the given size
	//For the given size, find its nearest pow-of-two
	//Allocate new block according to the previously explained cases->Ma3molah Gwa El code
	//Return the start address of the allocated block OR NULL if the requested size is 0

	if (size > max_block_size) {
		//cprintf("[alloc_block] ERROR: size > max_block_size (%u)\n", max_block_size);
		panic("Block too large to allocate");
	}
	if (size == 0) {
		return NULL;
	}
	//Nearest pow-of-2
	uint32 minForBlockSizeDynalloc = min_block_size;
	int idxForDynallocFunc = 0;
	// 7seb el index fe freeBlockLists elly by match el block size
	// kol list fe freeBlockLists bet7ot blocks elly 2^n size starting men el min_block_size
	while (minForBlockSizeDynalloc < size && minForBlockSizeDynalloc < max_block_size) {
		minForBlockSizeDynalloc *= 2;
		idxForDynallocFunc++;
	}
	uint32 blockSizeDynallocFunc = minForBlockSizeDynalloc;
	//cprintf("[alloc_block] blockSize = %u, index = %d\n", blockSize, idx);

	// CASE 1:if a free block exists Allocate it & update the data structures
	if (!LIST_EMPTY(&freeBlockLists[idxForDynallocFunc])) {
		struct BlockElement *blkEleDynallocFunc = LIST_FIRST(&freeBlockLists[idxForDynallocFunc]);
		//update the data structures
		LIST_REMOVE(&freeBlockLists[idxForDynallocFunc], blkEleDynallocFunc);
		struct PageInfoElement *SamaPage = to_page_info((uint32) blkEleDynallocFunc);
		SamaPage->num_of_free_blocks--;
		//cprintf("[alloc_block] CASE 1: Returned block %p from list[%d]\n", b, idxForDynalloc);
		return (void*) blkEleDynallocFunc;
	}

	//CASE 2:if a free page exists
	//Allocate it from the OS page allocator
	//Split it into blocks of the desired size
	//Add these blocks to the corresponding list
	//Allocate a block & update the data structures
	if (!LIST_EMPTY(&freePagesList)) {
		struct PageInfoElement* newPageDynallocFun = LIST_FIRST(&freePagesList);
		LIST_REMOVE(&freePagesList, newPageDynallocFun);

		uint32 pageVADynallocFunc = to_page_va(newPageDynallocFun);
		//Allocate it from the OS page allocator
		get_page((void*) pageVADynallocFunc);
		int calcNumOfBlockDynallocFun = PAGE_SIZE / blockSizeDynallocFunc;
		newPageDynallocFun->block_size = blockSizeDynallocFunc;
		newPageDynallocFun->num_of_free_blocks = calcNumOfBlockDynallocFun;
		//Split it into blocks of the desired size
		for (int i = 0; i < calcNumOfBlockDynallocFun; i++) {
			struct BlockElement* blkEleDynallocFunc = (struct BlockElement*) (pageVADynallocFunc+ i * blockSizeDynallocFunc);
			//Add these blocks to the corresponding list
			LIST_INSERT_TAIL(&freeBlockLists[idxForDynallocFunc], blkEleDynallocFunc);
		}
		struct BlockElement* blkEleDynallocFunc = LIST_FIRST(&freeBlockLists[idxForDynallocFunc]);
		//update the data structures
		LIST_REMOVE(&freeBlockLists[idxForDynallocFunc], blkEleDynallocFunc);
		struct PageInfoElement *SamaPage = to_page_info((uint32) blkEleDynallocFunc);
		SamaPage->num_of_free_blocks--;
		return (void*) blkEleDynallocFunc;
	}

	//Case 3 allocate block from the next list(s)
	// if there are no free pages or blocks
	if (LIST_EMPTY(&freePagesList) && LIST_EMPTY(&freeBlockLists[idxForDynallocFunc])) {
		for (int i = idxForDynallocFunc; i < 8; i++) {
			if (!LIST_EMPTY(&freeBlockLists[i])) {
				blockSizeDynallocFunc = 1 << (i + 3);
				struct BlockElement *blkEleDynallocFunc = LIST_FIRST(&freeBlockLists[i]);
				LIST_REMOVE(&freeBlockLists[i], blkEleDynallocFunc);
				struct PageInfoElement *SamaPage = to_page_info((uint32) blkEleDynallocFunc);
				SamaPage->num_of_free_blocks--;
				return (void*) blkEleDynallocFunc;
			}
		}
	}
	//Case 4 panic!!!
	//panic("alloc_block: No free blocks available!");
	//return NULL;
	//TODO: [PROJECT'25.BONUS#1] DYNAMIC ALLOCATOR - block if no free block
	blockedVarDynallocAllocFunc = 1;
	uint32 cyclesOnBusyWaitingAllocFunc=100000;
	while (blockedVarDynallocAllocFunc) {
		busy_wait(cyclesOnBusyWaitingAllocFunc);
	}
	return alloc_block(size);
}

//===========================
// [4] FREE BLOCK:
//===========================
void free_block(void *va) {
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		assert((uint32 )va >= dynAllocStart && (uint32 )va < dynAllocEnd);
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'25.GM#1] DYNAMIC ALLOCATOR - #4 free_block
	//Your code is here
	//Comment the following line
	//panic("free_block() Not implemented yet");
	////el description bta3 el function////
	//Free the previously allocated block at the given address “va”
	//Follow the previously explained steps->Ma3molah Gwa El code

	//Find corresponding size from pageBlockkInfoArr
	uint32 pageidxDynallocFreeFunc = ((uint32) va - dynAllocStart) / PAGE_SIZE;
	uint32 blockSizeDynallocFreeFunc = pageBlockInfoArr[pageidxDynallocFreeFunc].block_size;
	assert(blockSizeDynallocFreeFunc != 0);

	// 7seb el index fe freeBlockLists elly by match el block size
	// kol list fe freeBlockLists bet7ot blocks elly 2^n size starting men el min_block_size
	uint32 idxForDynallocFreeFunc = 0;
	uint32 minBlockSizeDynallocFreeFunc = min_block_size;
	while (minBlockSizeDynallocFreeFunc < blockSizeDynallocFreeFunc) {
		minBlockSizeDynallocFreeFunc <<= 1;
		idxForDynallocFreeFunc++;
	}
	struct BlockElement *blkEleDynallocFreeFunIns = (struct BlockElement*) va;
	//Increment #free blocks in pageBlockkInfoArr
	pageBlockInfoArr[pageidxDynallocFreeFunc].num_of_free_blocks++;
	uint32 pageVADynallocFreeFunc = dynAllocStart + pageidxDynallocFreeFunc * PAGE_SIZE;
	uint32 totalBlocksInPage = PAGE_SIZE / blockSizeDynallocFreeFunc;

	//If entire page becomes free
	//Remove all its blocks from corresponding list in freeBlockLists
	//return it to the free frame list
	//add it to the freePagesList
	if (pageBlockInfoArr[pageidxDynallocFreeFunc].num_of_free_blocks >= totalBlocksInPage) {
		struct BlockElement *blkEleDynallocFreeFun = LIST_FIRST(&freeBlockLists[idxForDynallocFreeFunc]);
		while (blkEleDynallocFreeFun != NULL) {
			struct BlockElement *nextBlkDynallocFreeFunc = LIST_NEXT(blkEleDynallocFreeFun);
			if ((uint32) blkEleDynallocFreeFun >= pageVADynallocFreeFunc && (uint32) blkEleDynallocFreeFun < pageVADynallocFreeFunc + PAGE_SIZE) {
				LIST_REMOVE(&freeBlockLists[idxForDynallocFreeFunc], blkEleDynallocFreeFun);
			}
			blkEleDynallocFreeFun = nextBlkDynallocFreeFunc;
		}
		return_page((void*) pageVADynallocFreeFunc);
		//add it to the freePagesList
		LIST_INSERT_HEAD(&freePagesList, &pageBlockInfoArr[pageidxDynallocFreeFunc]);
		pageBlockInfoArr[pageidxDynallocFreeFunc].num_of_free_blocks = 0;
		pageBlockInfoArr[pageidxDynallocFreeFunc].block_size = 0;
	}
	else {
		//Return block to the corresponding list
		LIST_INSERT_TAIL(&freeBlockLists[idxForDynallocFreeFunc], blkEleDynallocFreeFunIns);
		//TODO: [PROJECT'25.BONUS#1] DYNAMIC ALLOCATOR - block if no free block
		//Hena hy3raf en fy block fadiii
		blockedVarDynallocAllocFunc = 0;
	}
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] REALLOCATE BLOCK:
//===========================
void *realloc_block(void* va, uint32 new_size) {
	//TODO: [PROJECT'25.BONUS#2] KERNEL REALLOC - realloc_block
	//Your code is here
	//Comment the following line
	//panic("realloc_block() Not implemented yet");
	if ( va == NULL ) {
		return alloc_block( new_size );
		}
		if ( new_size == 0 ) {
			 free_block(va);
			 return NULL;
		}
		uint32 sizeOfCurrent = get_block_size(va);
		if ( new_size <= sizeOfCurrent ) {
			return va;
		}
		void* latestBlock = NULL;
		while (( latestBlock = alloc_block(new_size)) == NULL ) {
			env_sleep( 1 );
		}
		 memcpy( latestBlock, va, sizeOfCurrent );
		 free_block( va );
		return latestBlock;
}
