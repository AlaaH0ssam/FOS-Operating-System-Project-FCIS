#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init() {
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list)
	;
	init_kspinlock(&AllShares.shareslock, "shares lock");
	//init_sleeplock(&AllShares.sharessleeplock, "shares sleep lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//=========================
// [2] Find Share Object:
//=========================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* find_share(int32 ownerID, char* name) {
#if USE_KHEAP
	struct Share * ret = NULL;
	bool wasHeld = holding_kspinlock(&(AllShares.shareslock));
	if (!wasHeld) {
		acquire_kspinlock(&(AllShares.shareslock));
	}
	{
		struct Share * shr;
		LIST_FOREACH(shr, &(AllShares.shares_list))
		{
			//cprintf("shared var name = %s compared with %s\n", name, shr->name);
			if (shr->ownerID == ownerID && strcmp(name, shr->name) == 0) {
				//cprintf("%s found\n", name);
				ret = shr;
				break;
			}
		}
	}
	if (!wasHeld) {
		release_kspinlock(&(AllShares.shareslock));
	}
	return ret;
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [3] Get Size of Share Object:
//==============================
int size_of_shared_object(int32 ownerID, char* shareName) {
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = find_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}
//===========================================================

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=====================================
// [1] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* alloc_share(int32 ownerID, char* shareName, uint32 size,uint8 isWritable) {
	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #1 alloc_share
	//Your code is here
	//Comment the following line
	//panic("alloc_share() is not implemented yet...!!");

	//allocate a new shared object by kmalloc
	uint32 sizeOfStructShare = sizeof(struct Share);
	struct Share *ElShareObjectElgded = kmalloc(sizeOfStructShare);
	if (ElShareObjectElgded == NULL) {
		return NULL;
	}

	//Initialize members
	//references=1
	ElShareObjectElgded->references = 1;

	//VA masking-out its most significant bit
	uint32 ElVAElMasked = (uint32) ElShareObjectElgded;
	uint32 ID = ElVAElMasked & 0x7FFFFFFF;
	ElShareObjectElgded->ID = ID;

	//initialize other members
	ElShareObjectElgded->size = size;
	ElShareObjectElgded->isWritable = isWritable;
	ElShareObjectElgded->ownerID = ownerID;
	strncpy(ElShareObjectElgded->name, shareName, 63);
	ElShareObjectElgded->name[63] = '\0';

	//Create the framesStorage
	int numberOfFrames = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32 creationOfArrayLengthToSavePointers = numberOfFrames* sizeof(struct FrameInfo*);
	ElShareObjectElgded->framesStorage = kmalloc(creationOfArrayLengthToSavePointers);

	//If failed undo allocation & return NULL
	if (ElShareObjectElgded->framesStorage == NULL) {
		kfree(ElShareObjectElgded);
		return NULL;
	}

	//Initialize pointers by ZEROs
	for (int i = 0; i < numberOfFrames; i++) {
		ElShareObjectElgded->framesStorage[i] = NULL;
	}

	//success? pointer to the created object for struct Share
	return ElShareObjectElgded;
}

//=========================
// [4] Create Share Object:
//=========================
int create_shared_object(int32 ownerID, char* shareName, uint32 size,uint8 isWritable, void* virtual_address) {
#if USE_KHEAP
	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #3 create_shared_object
	//Your code is here
	//Comment the following line
	//panic("create_shared_object() is not implemented yet...!!");
	acquire_kspinlock(&(AllShares.shareslock));
	struct Env* myenv = get_cpu_proc(); //The calling environment
	// This function should create the shared object at the given virtual address with the given size
	// and return the ShareObjectID
	// RETURN:
	//	a) ID of the shared object (its VA after masking out its msb) if success
	//	b) E_SHARED_MEM_EXISTS if the shared object already exists
	//	c) E_NO_SHARE if failed to create a shared object

	struct Share* checkingForTheName = NULL;
	checkingForTheName=find_share(ownerID, shareName);
	if (checkingForTheName != NULL) {
		release_kspinlock(&AllShares.shareslock);
		// E_SHARED_MEM_EXISTS if the shared object already exists
		return E_SHARED_MEM_EXISTS;
	}

	//Allocate & Initialize a new share object
	struct Share *newSharedObjectOfCreationFun = alloc_share(ownerID, shareName,size, isWritable);
	if (newSharedObjectOfCreationFun != NULL) {
		LIST_INSERT_HEAD(&AllShares.shares_list, newSharedObjectOfCreationFun);
	} else {
		release_kspinlock(&AllShares.shareslock);
		//E_NO_SHARE if failed to create a shared object
		return E_NO_SHARE;
	}

	//allocate,map,add to frameStorage
	int numOfFramesCreateFun = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	struct FrameInfo *allocatedFrameCreateFun = NULL;
	if ((uint32) virtual_address % PAGE_SIZE != 0) {
		release_kspinlock(&AllShares.shareslock);
		return E_NO_SHARE;
	}
	for (int i = 0; i < numOfFramesCreateFun; i++) {
		int ret = allocate_frame(&allocatedFrameCreateFun);
		if (ret != 0) {
			for (int j = 0; j < i; j++) {
				unmap_frame(myenv->env_page_directory,(uint32) virtual_address + j * PAGE_SIZE);
				free_frame(newSharedObjectOfCreationFun->framesStorage[j]);
			}
			//If failed undo allocation
			LIST_REMOVE(&AllShares.shares_list, newSharedObjectOfCreationFun);
			kfree(newSharedObjectOfCreationFun->framesStorage);
			kfree(newSharedObjectOfCreationFun);
			release_kspinlock(&AllShares.shareslock);
			return E_NO_SHARE;
		}

		newSharedObjectOfCreationFun->framesStorage[i] = allocatedFrameCreateFun;
		uint32 mappedAddress = (uint32) virtual_address + i * PAGE_SIZE;
		int premOfCreatedSharedObjFun = PERM_USER | PERM_WRITEABLE| PERM_PRESENT;
		map_frame(myenv->env_page_directory, allocatedFrameCreateFun, mappedAddress,premOfCreatedSharedObjFun);

	}

	release_kspinlock(&AllShares.shareslock);
	//success? return ID of the shared object (its VA after masking out its msb)
	return newSharedObjectOfCreationFun->ID;
#endif
	//If we change the USE_KHEAP to 0
	return 0;
}


//======================
// [5] Get Share Object:
//======================
int get_shared_object(int32 ownerID, char* shareName, void* virtual_address) {
#if USE_KHEAP
	//TODO: [PROJECT'25.IM#3] SHARED MEMORY - #5 get_shared_object
	//Your code is here
	//Comment the following line
	//panic("get_shared_object() is not implemented yet...!!");

	acquire_kspinlock(&(AllShares.shareslock));
	struct Env* myenv = get_cpu_proc(); //The calling environment

	// 	This function should share the required object in the heap of the current environment
	//	starting from the given virtual_address with the specified permissions of the object: read_only/writable
	// 	and return the ShareObjectID
	// RETURN:
	//	a) ID of the shared object (its VA after masking out its msb) if success
	//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists

	struct Share *savedPointerToTheSharedObj = NULL;
	savedPointerToTheSharedObj = find_share(ownerID, shareName);
	if (savedPointerToTheSharedObj == NULL) {
		release_kspinlock(&AllShares.shareslock);
		//E_SHARED_MEM_NOT_EXISTS if the shared object is NOT exists
		return E_SHARED_MEM_NOT_EXISTS;
	}

	//Get its physical frames from the frames_storage
	int numOfFramesGetShareObject = (savedPointerToTheSharedObj->size+ PAGE_SIZE - 1) / PAGE_SIZE;
	 //Use the flag isWritable
	int isWritableVariable = savedPointerToTheSharedObj->isWritable;
	struct FrameInfo *allocatedFrameGetSharedFun = NULL;
	for (int i = 0; i < numOfFramesGetShareObject; i++) {
		allocatedFrameGetSharedFun = savedPointerToTheSharedObj->framesStorage[i];
		uint32 mappedVASharedObject = (uint32) virtual_address + i * PAGE_SIZE;
		if (isWritableVariable == 0) {
			map_frame(myenv->env_page_directory, allocatedFrameGetSharedFun,mappedVASharedObject, PERM_USER | PERM_PRESENT);
		} else {
			map_frame(myenv->env_page_directory, allocatedFrameGetSharedFun,mappedVASharedObject,PERM_USER | PERM_PRESENT | PERM_WRITEABLE);
		}

	}
	//Update references
	savedPointerToTheSharedObj->references++;
	release_kspinlock(&AllShares.shareslock);
	//success? return ID of the shared object (its VA after masking out its msb)
	return savedPointerToTheSharedObj->ID;
#endif
	//If we change the USE_KHEAP to 0
   return 0;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//
//=========================
// [1] Delete Share Object:
//=========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare) {
#if USE_KHEAP
	//TODO: [PROJECT'25.BONUS#5] EXIT #2 - free_share
	//Your code is here
	//Comment the following line
	//panic("free_share() is not implemented yet...!!");

	//delete the given shared object from the "shares_list"
	if(ptrShare==NULL){
		return;
	}
	 acquire_kspinlock(&(AllShares.shareslock));
	 LIST_REMOVE(&AllShares.shares_list, ptrShare);
	 release_kspinlock(&AllShares.shareslock);

	 //it should free its framesStorage and the share object itself
	 int numberOfFramesFreeShareFun = (ptrShare->size + PAGE_SIZE - 1) / PAGE_SIZE;
	 for (int i = 0; i < numberOfFramesFreeShareFun; i++) {
	         if (ptrShare->framesStorage[i] != NULL) {
	             free_frame(ptrShare->framesStorage[i]);
	             ptrShare->framesStorage[i] = NULL;
	         }
	     }
	 kfree(ptrShare->framesStorage);
	 ptrShare->framesStorage = NULL;
	 kfree(ptrShare);
#endif
}

//=========================
// [2] Free Share Object:
//=========================
int delete_shared_object(int32 sharedObjectID, void *startVA) {
#if USE_KHEAP
	//TODO: [PROJECT'25.BONUS#5] EXIT #2 - delete_shared_object
	//Your code is here
	//Comment the following line
	//panic("delete_shared_object() is not implemented yet...!!");
	acquire_kspinlock(&(AllShares.shareslock));
	struct Env* myenv = get_cpu_proc(); //The calling environment
	// This function should free (delete) the shared object from the User Heapof the current environment
	// If this is the last shared env, then the "frames_store" should be cleared and the shared object should be deleted
	// RETURN:
	//	a) 0 if success
	//	b) E_SHARED_MEM_NOT_EXISTS if the shared object is not exists

	// Steps:
	//	1) Get the shared object from the "shares" array (use get_share_object_ID())
	struct Share *checkOnFoundingID=NULL;
	struct Share * i;
	LIST_FOREACH(i, &(AllShares.shares_list))
	{
		if (i->ID == sharedObjectID) {
			checkOnFoundingID = i;
			break;
		}
	}
	//E_SHARED_MEM_NOT_EXISTS if the shared object is not exists
	if(checkOnFoundingID==NULL){
		release_kspinlock(&AllShares.shareslock);
		return E_SHARED_MEM_NOT_EXISTS;
	}
	//	2) Unmap it from the current environment "myenv"
	int numOfFramesDeleteObjFunc=(checkOnFoundingID->size+PAGE_SIZE-1)/PAGE_SIZE;
	for(int i=0;i<numOfFramesDeleteObjFunc;i++){
		uint32 unmappedFrames=(uint32)startVA + i * PAGE_SIZE;
		unmap_frame(myenv->env_page_directory,unmappedFrames);
	//	3) If one or more table becomes empty, remove it
		uint32 *ptrTable=NULL;
	    get_page_table(myenv->env_page_directory,unmappedFrames,&ptrTable);
	    if(ptrTable!=NULL) {
			int foundingIsEmpty = 1;
			// page table entries per page table
			int numOfTablePageRntries=NPTENTRIES;
			for (int j = 0; j < numOfTablePageRntries; j++) {
				if (ptrTable[j] & PERM_PRESENT) {
					foundingIsEmpty = 0;
					break;
				}
			}
			if (foundingIsEmpty) {
				del_page_table(myenv->env_page_directory, unmappedFrames);
			}
	   }
	}
	//	4) Update references
	  checkOnFoundingID->references--;
	//	5) If this is the last share, delete the share object (use free_share())
	   uint32 DeletedLastShareObj=checkOnFoundingID->references;
	  if(DeletedLastShareObj==0){
		 free_share(checkOnFoundingID);
	  }
	//	6) Flush the cache "tlbflush()"
	 tlbflush();
	 release_kspinlock(&AllShares.shareslock);
	 return 0;
#else
	 //If we change the USE_KHEAP to 0
	 return 0;
#endif
}
