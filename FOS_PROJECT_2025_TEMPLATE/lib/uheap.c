#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//==============================================
// [1] INITIALIZE USER HEAP:
//==============================================
int __firstTimeFlag = 1;
void uheap_init() {
	if (__firstTimeFlag) {
		initialize_dynamic_allocator(USER_HEAP_START,
		USER_HEAP_START + DYN_ALLOC_MAX_SIZE);
		uheapPlaceStrategy = sys_get_uheap_strategy();
		uheapPageAllocStart = dynAllocEnd + PAGE_SIZE;
		uheapPageAllocBreak = uheapPageAllocStart;
		__firstTimeFlag = 0;
	}
}

//==============================================
// [2] GET A PAGE FROM THE KERNEL FOR DA:
//==============================================
int get_page(void* va) {
	int ret = __sys_allocate_page(ROUNDDOWN(va, PAGE_SIZE),PERM_USER | PERM_WRITEABLE | PERM_UHPAGE);
	if (ret < 0)
		panic("get_page() in user: failed to allocate page from the kernel");
	return 0;
}

//==============================================
// [3] RETURN A PAGE FROM THE DA TO KERNEL:
//==============================================
void return_page(void* va) {
	int ret = __sys_unmap_frame(ROUNDDOWN((uint32 )va, PAGE_SIZE));
	if (ret < 0)
		panic("return_page() in user: failed to return a page to the kernel");
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

struct OURAllocationNode {
	uint32 Bedayet_eladd;
	uint32 size;
	uint32 shareIDSfreeFunc;
	struct OURAllocationNode* next;
};
struct FreeBlockNode {
	uint32 start_va;
	uint32 num_pages;
	struct FreeBlockNode* next;
};

struct OURAllocationNode* alloc_head = NULL;
struct FreeBlockNode* free_head = NULL;
static struct OURAllocationNode* find_allocation(uint32 addd) {
	struct OURAllocationNode* cur = alloc_head;
	while (cur) {
		if (cur->Bedayet_eladd == addd)
			return cur;
		cur = cur->next;
	}

	return NULL;
}

static void allocationGded(uint32 va, uint32 size) {
	struct OURAllocationNode* node = malloc(sizeof(*node));
	node->Bedayet_eladd = va;
	node->size = size;
	node->next = alloc_head;
	alloc_head = node;
}

static void Nemsa7allocation(uint32 va) {
	struct OURAllocationNode *cur = alloc_head, *prev = NULL;
	while (cur) {
		if (cur->Bedayet_eladd == va) {
			if (prev)
				prev->next = cur->next;
			else
				alloc_head = cur->next;

			free(cur);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
}

static void FREEblockk(uint32 start, uint32 num_pages) {
	if (num_pages == 0)
		return;

	struct FreeBlockNode* elly2bly = NULL;
	struct FreeBlockNode* Delwa2ty = free_head;

	while (Delwa2ty) {
		if (Delwa2ty->start_va == start)
			return;
		elly2bly = Delwa2ty;
		Delwa2ty = Delwa2ty->next;
	}

	struct FreeBlockNode* new_node = malloc(sizeof(*new_node));
	if (!new_node)
		panic("malloc failed for free block");
	new_node->start_va = start;
	new_node->num_pages = num_pages;

	elly2bly = NULL;
	Delwa2ty = free_head;
	while (Delwa2ty && Delwa2ty->start_va < start) {
		elly2bly = Delwa2ty;
		Delwa2ty = Delwa2ty->next;
	}
	new_node->next = Delwa2ty;
	if (elly2bly) {
		elly2bly->next = new_node;
	} else {
		free_head = new_node;
	}
}

static uint32 EXACTTT(uint32 page) {
	struct FreeBlockNode* elly2bly = NULL;
	struct FreeBlockNode* Delwa2ty = free_head;

	while (Delwa2ty) {
		if (Delwa2ty->num_pages == page) {
			uint32 addr = ROUNDDOWN(Delwa2ty->start_va, PAGE_SIZE);
			if (elly2bly) {
				elly2bly->next = Delwa2ty->next;
			} else {
				free_head = Delwa2ty->next;
			}
			free(Delwa2ty);
			return addr;
		}
		elly2bly = Delwa2ty;
		Delwa2ty = Delwa2ty->next;
	}
	return 0;
}

static uint32 WORSTTT(uint32 page) {
	struct FreeBlockNode* elly2bly = NULL;
	struct FreeBlockNode* Delwa2ty = free_head;

	struct FreeBlockNode* afdal2bly = NULL;
	struct FreeBlockNode* afdalwa7da = NULL;
	uint32 max_pages = 0;

	while (Delwa2ty) {
		if (Delwa2ty->start_va>= uheapPageAllocStart&& Delwa2ty->start_va < USER_HEAP_MAX) {
			if (Delwa2ty->num_pages >= page) {
				if (Delwa2ty->num_pages > max_pages) {
					max_pages = Delwa2ty->num_pages;
					afdalwa7da = Delwa2ty;
					afdal2bly = elly2bly;
				}
			}
		}
		elly2bly = Delwa2ty;
		Delwa2ty = Delwa2ty->next;
	}
	if (!afdalwa7da)
		return 0;
	uint32 addr = afdalwa7da->start_va;
	uint32 remaining_pages = afdalwa7da->num_pages - page;

	if (remaining_pages > 0) {
		afdalwa7da->start_va += page * PAGE_SIZE;
		afdalwa7da->num_pages = remaining_pages;
	} else {
		if (afdal2bly) {
			afdal2bly->next = afdalwa7da->next;
		} else {
			free_head = afdalwa7da->next;
		}
		free(afdalwa7da);
	}
	return addr;
}
static void MERGEE(void) {
	if (!free_head || !free_head->next)
		return;
	struct FreeBlockNode* Metrateba = NULL;
	struct FreeBlockNode* Delwa2ty = free_head;
	while (Delwa2ty) {
		struct FreeBlockNode* Elgaya = Delwa2ty->next;
		if (!Metrateba || Delwa2ty->start_va < Metrateba->start_va) {
			Delwa2ty->next = Metrateba;
			Metrateba = Delwa2ty;
		} else {
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
		uint32 ENDdelwa2ty = Delwa2ty->start_va+ Delwa2ty->num_pages * PAGE_SIZE;
		if (ENDdelwa2ty == Delwa2ty->next->start_va) {
			struct FreeBlockNode* Haytmes7 = Delwa2ty->next;
			Delwa2ty->num_pages += Haytmes7->num_pages;
			Delwa2ty->next = Haytmes7->next;
			free(Haytmes7);
		} else {
			Delwa2ty = Delwa2ty->next;
		}
	}
}

static void ta7dethEL_breakTofree() {
	uint32 A5erelEND = uheapPageAllocStart;
	struct OURAllocationNode* Elgaya = alloc_head;
	while (Elgaya) {
		uint32 NEWend = Elgaya->Bedayet_eladd + Elgaya->size;
		if (NEWend > A5erelEND) {
			if (NEWend <= USER_HEAP_MAX) {
				A5erelEND = NEWend;
			}
		}
		Elgaya = Elgaya->next;
	}

	if (A5erelEND > USER_HEAP_MAX) {
		A5erelEND = USER_HEAP_MAX;
	}
	if (A5erelEND < uheapPageAllocStart) {
		A5erelEND = uheapPageAllocStart;
	}

	uheapPageAllocBreak = ROUNDUP(A5erelEND, PAGE_SIZE);
}
//=================================
// [1] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	if (size == 0)
		return NULL;
	//==============================================================
	//TODO: [PROJECT'25.IM#2] USER HEAP - #1 malloc
	//Your code is here
	//Comment the following line
	//panic("malloc() is not implemented yet...!!");

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
		return alloc_block(size);
	}
	uint32 pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	if (pages == 0 || pages > (USER_HEAP_MAX - uheapPageAllocStart) / PAGE_SIZE) {
		return NULL;
	}
	uint32 ELsize_need = pages * PAGE_SIZE;
	uint32 ADDRESS = 0;

	ADDRESS = EXACTTT(pages);
	if (ADDRESS == 0)
		ADDRESS = WORSTTT(pages);
	if (ADDRESS == 0) {
		uint32 ELBREAKnow = uheapPageAllocBreak;
		uint32 ELBREAKelgded = ELBREAKnow + ELsize_need;
		if (ELBREAKelgded > USER_HEAP_MAX) {
			return NULL;
		}
		ADDRESS = ELBREAKnow;
		uheapPageAllocBreak = ELBREAKelgded;
	}
	if (ADDRESS < uheapPageAllocStart || ADDRESS >= USER_HEAP_MAX) {
		return NULL;
	}

	sys_allocate_user_mem(ADDRESS, ELsize_need);
	allocationGded(ADDRESS, ELsize_need);

	uint32 x = ADDRESS + ELsize_need;
	if (x > uheapPageAllocBreak) {
		uheapPageAllocBreak = x;
	}
	return (void*) ADDRESS;

}
//=================================
// [2] FREE SPACE IN USER HEAP:
//=================================
void free(void* virtual_address) {
	//TODO: [PROJECT'25.IM#2] USER HEAP - #3 free
	//Your code is here
	//Comment the following line
	//panic("free() is not implemented yet...!!");

	if (!virtual_address)
		return;
	uint32 ADDRESS = (uint32) virtual_address;
	if (ADDRESS == 0) {
		panic("free: NULL pointer");
	}
	uheap_init();
	if (ADDRESS >= USER_HEAP_START) {
		if (ADDRESS < uheapPageAllocStart) {
			free_block(virtual_address);
			return;
		}
	}
	if (ADDRESS >= uheapPageAllocStart) {
		if (ADDRESS < USER_HEAP_MAX) {
			uint32 bdayagdeda = ROUNDDOWN(ADDRESS, PAGE_SIZE);
			struct OURAllocationNode* alloc =(struct OURAllocationNode*) find_allocation(bdayagdeda);
			if (!alloc) {
				panic("free: allocation not found at VA: %08x", bdayagdeda);
			}
			uint32 size = alloc->size;
			uint32 pages = size / PAGE_SIZE;

			Nemsa7allocation(bdayagdeda);
			ta7dethEL_breakTofree();
			sys_free_user_mem(bdayagdeda, size);
			FREEblockk(bdayagdeda, pages);
			MERGEE();
			return;
		}
	}
	panic("free: invalid address %08x", ADDRESS);
}

//=================================
// [3] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	if (size == 0)
		return NULL;
	//==============================================================
	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #2 smalloc
	//Your code is here
	//Comment the following line
	//panic("smalloc() is not implemented yet...!!");

	//applying custom fit
	uint32 numOfPagesNeededSmallocFun = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	uint32 sizeWeNeededSmallocFun = numOfPagesNeededSmallocFun * PAGE_SIZE;
	if (uheapPageAllocBreak + sizeWeNeededSmallocFun > USER_HEAP_MAX) {
		return NULL;
	}
	if ((uheapPageAllocStart & (PAGE_SIZE - 1)) != 0) {
		uheapPageAllocStart = ROUNDUP(uheapPageAllocStart, PAGE_SIZE);
		uheapPageAllocBreak = uheapPageAllocStart;
	}
	uint32 VAOfAllocatedAreaOfSharedObject = 0;
	VAOfAllocatedAreaOfSharedObject = EXACTTT(numOfPagesNeededSmallocFun);
	if (VAOfAllocatedAreaOfSharedObject == 0) {
		VAOfAllocatedAreaOfSharedObject = WORSTTT(numOfPagesNeededSmallocFun);
	}
	if (VAOfAllocatedAreaOfSharedObject == 0) {
		VAOfAllocatedAreaOfSharedObject = uheapPageAllocBreak;
		uheapPageAllocBreak += sizeWeNeededSmallocFun;
		if (uheapPageAllocBreak > USER_HEAP_MAX) {
			uheapPageAllocBreak = VAOfAllocatedAreaOfSharedObject;
			return NULL;
		}
	}
	VAOfAllocatedAreaOfSharedObject = ROUNDUP(VAOfAllocatedAreaOfSharedObject,PAGE_SIZE);

	//invoke the Kernel for allocation of shared variable
	int theCreatedVirtualAddressOfTheSharedArea = sys_create_shared_object(sharedVarName, size, isWritable,(void*) VAOfAllocatedAreaOfSharedObject);
	if (theCreatedVirtualAddressOfTheSharedArea < 0) {
		if (VAOfAllocatedAreaOfSharedObject >= uheapPageAllocStart && VAOfAllocatedAreaOfSharedObject < uheapPageAllocBreak) {
			uint32 thePreviousBreakOfAllocatedArea = uheapPageAllocBreak- sizeWeNeededSmallocFun;
			if (VAOfAllocatedAreaOfSharedObject== thePreviousBreakOfAllocatedArea) {
				uheapPageAllocBreak = thePreviousBreakOfAllocatedArea;
			} else {
				FREEblockk(VAOfAllocatedAreaOfSharedObject,
						numOfPagesNeededSmallocFun);
				MERGEE();
			}
		}
		//If fails, return NULL
		return NULL;
	}
	allocationGded(VAOfAllocatedAreaOfSharedObject, sizeWeNeededSmallocFun);
	//If successful, return its virtual address
	return (void*) VAOfAllocatedAreaOfSharedObject;
}

//========================================
// [4] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName) {
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	uheap_init();
	//==============================================================

	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #4 sget
	//Your code is here
	//Comment the following line
	//panic("sget() is not implemented yet...!!");

	//Get the size of the shared variable
	int gettingSizeOfTheSharedVariable = sys_size_of_shared_object(ownerEnvID,sharedVarName);
	if (gettingSizeOfTheSharedVariable <= 0) {
		//If not exists, return NULL
		return NULL;
	}

	//applying custom fit
	uint32 numOfPagesNeededSgetFun =ROUNDUP(gettingSizeOfTheSharedVariable, PAGE_SIZE) / PAGE_SIZE;
	uint32 sizeWeNeededSgetFun = numOfPagesNeededSgetFun * PAGE_SIZE;
	if (uheapPageAllocBreak + sizeWeNeededSgetFun > USER_HEAP_MAX) {
		return NULL;
	}
	if ((uheapPageAllocStart & (PAGE_SIZE - 1)) != 0) {
		uheapPageAllocStart = ROUNDUP(uheapPageAllocStart, PAGE_SIZE);
		uheapPageAllocBreak = uheapPageAllocStart;
	}
	uint32 VAOfTheSharedObjSgetFun = 0;
	VAOfTheSharedObjSgetFun = EXACTTT(numOfPagesNeededSgetFun);
	if (VAOfTheSharedObjSgetFun == 0) {
		VAOfTheSharedObjSgetFun = WORSTTT(numOfPagesNeededSgetFun);
	}
	if (VAOfTheSharedObjSgetFun == 0) {
		VAOfTheSharedObjSgetFun = uheapPageAllocBreak;
		uheapPageAllocBreak += sizeWeNeededSgetFun;
		if (uheapPageAllocBreak > USER_HEAP_MAX) {
			uheapPageAllocBreak = VAOfTheSharedObjSgetFun;
			return NULL;
		}
	}
	VAOfTheSharedObjSgetFun = ROUNDUP(VAOfTheSharedObjSgetFun, PAGE_SIZE);

	//invoke the Kernel for sharing this variable
	int gettingTheSharedObjectVariable = sys_get_shared_object(ownerEnvID,sharedVarName, (void*) VAOfTheSharedObjSgetFun);
	if (gettingTheSharedObjectVariable < 0) {
		if (VAOfTheSharedObjSgetFun >= uheapPageAllocStart && VAOfTheSharedObjSgetFun < uheapPageAllocBreak) {
			uint32 thePreviousBreakOfSharedObj = uheapPageAllocBreak- sizeWeNeededSgetFun;
			if (VAOfTheSharedObjSgetFun == thePreviousBreakOfSharedObj) {
				uheapPageAllocBreak = thePreviousBreakOfSharedObj;
			} else {
				FREEblockk(VAOfTheSharedObjSgetFun, numOfPagesNeededSgetFun);
				MERGEE();
			}
		}
		//If fails, return NULL
		return NULL;
	}
	allocationGded(VAOfTheSharedObjSgetFun, sizeWeNeededSgetFun);
	//If successful, return its virtual address
	return (void*) VAOfTheSharedObjSgetFun;
}
//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//
//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size) {
//==============================================================
//DON'T CHANGE THIS CODE========================================
	uheap_init();
//==============================================================
	panic("realloc() is not implemented yet...!!");
}

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_delete_shared_object(...); which switches to the kernel mode,
//	calls delete_shared_object(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the delete_shared_object() function is empty, make sure to implement it.
void sfree(void* virtual_address) {
	//TODO: [PROJECT'25.BONUS#5] EXIT #2 - sfree
	//Your code is here
	//Comment the following line
	//panic("sfree() is not implemented yet...!!");

   // 1) you should find the ID of the shared variable at the given address
	uint32 theAddressSfreeFunc = (uint32) virtual_address;
    struct OURAllocationNode *curNodeSfreeFunc = alloc_head, *thePrevPointerSfreeFunc = NULL;
	while (curNodeSfreeFunc) {
	  if (curNodeSfreeFunc->Bedayet_eladd == theAddressSfreeFunc)
		break;
		thePrevPointerSfreeFunc = curNodeSfreeFunc;
		curNodeSfreeFunc = curNodeSfreeFunc->next;
	}
	if (curNodeSfreeFunc == NULL)
		return;
	uint32 shardObjIdSfreeFunc = curNodeSfreeFunc->shareIDSfreeFunc;
	uint32 sizeOfTheCurrentNode = curNodeSfreeFunc->size;
	uint32 numOfPagesSfreeFunc = (sizeOfTheCurrentNode + PAGE_SIZE - 1) / PAGE_SIZE;

	// 2) you need to call sys_freeSharedObject()
	 int returnedDeletedObjSfreeFunc=sys_delete_shared_object(shardObjIdSfreeFunc, virtual_address);
	 if (returnedDeletedObjSfreeFunc < 0){
	       return;
	}
	if (thePrevPointerSfreeFunc)
		thePrevPointerSfreeFunc->next = curNodeSfreeFunc->next;
	else
		alloc_head = curNodeSfreeFunc->next;
	struct FreeBlockNode* freenodeSfreeFunc = malloc(sizeof(struct FreeBlockNode));
	freenodeSfreeFunc->start_va = theAddressSfreeFunc;
	freenodeSfreeFunc->num_pages = numOfPagesSfreeFunc;
	struct FreeBlockNode* elly2bly = NULL;
	struct FreeBlockNode* Delwa2ty = free_head;
	while (Delwa2ty && Delwa2ty->start_va < theAddressSfreeFunc) {
		elly2bly = Delwa2ty;
		Delwa2ty = Delwa2ty->next;
	}
	freenodeSfreeFunc->next = Delwa2ty;
	if (elly2bly) {
		elly2bly->next = freenodeSfreeFunc;
	} else {
		free_head = freenodeSfreeFunc;
	}
	MERGEE();
	free(curNodeSfreeFunc);
}

//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
