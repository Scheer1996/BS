/**
 * @file vmaccess.c
 * @author Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * @date 2010
 * @brief The access functions to virtual memory.
 */

#include "vmem.h"
#include "debug.h"

/*
 * static variables
 */

static struct vmem_struct *vmem = NULL; //!< Reference to virtual memory


/**
 *****************************************************************************************
 *  @brief      This function setup the connection to virtual memory.
 *              The virtual memory has to be created by mmanage.c module.
 *
 *  @return     void
 ****************************************************************************************/
static void vmem_init(void) {
	key_t key;
	int shmid;
	
	key = ftok(SHMKEY , SHMPROCID);
	TEST_AND_EXIT_ERRNO(key == -1,"ftok: ftok failed");
	
	shmid = shmget(key, SHMSIZE, 0666);
	TEST_AND_EXIT_ERRNO(shmid < 0, "shmget: shmget failed");
	
	vmem = (struct vmem_struct*) shmat(shmid, NULL, 0);
	TEST_AND_EXIT_ERRNO(shmid < 0, "shmat: shmat failed");
	
	vmem->adm.req_pageno = VOID_IDX;             //!< number of requested page 
    	vmem->adm.next_alloc_idx = 0;          //!< next frame to allocate by FIFO and CLOCK page replacement algorithm
    	vmem->adm.pf_count = 0;               //!< page fault counter 
    	vmem->adm.g_count = -1;                 //!< global acces counter as quasi-timestamp - will be increment by each memory access
	
	int i;
	for(i = 0; i < VMEM_NPAGES; i++){
		vmem->pt.entries[i].flags = 0;
		vmem->pt.entries[i].frame = VOID_IDX;
		vmem->pt.entries[i].count = 0;
		vmem->pt.entries[i].age = 0;
	}
	for(i = 0; i < VMEM_NFRAMES; i++){
		vmem->pt.framepage[i] = VOID_IDX;
	}
	for(i = 0; i < VMEM_NFRAMES * VMEM_PAGESIZE; i++){
		vmem->data[i] = 0;
	}
}

/**
 *****************************************************************************************
 *  @brief      This function does aging for aging page replacement algorithm.
 *              It will be called periodic based on g_count.
 *              This function must be used only when aging page replacement algorithm is activ.
 *              Otherwise update_age_reset_ref may interfere with other page replacement 
 *              alogrithms that base on PTF_REF bit.
 *
 *  @return     void
 ****************************************************************************************/
static void update_age_reset_ref(void) {
	if((vmem->adm.g_count % UPDATE_AGE_COUNT) == 0){
		int page;
		int i;
		for(i = 0; i < VMEM_NFRAMES; i++){
			page = vmem->pt.framepage[i];
			if(page != VOID_IDX){
				unsigned char referenceBit = vmem->pt.entries[page].flags&PTF_REF;
				vmem->pt.entries[page].flags &= 3;
				vmem->pt.entries[page].age =  (vmem->pt.entries[page].age>>1)|(referenceBit<<5);
			}
		}
	}
}

/**
 *****************************************************************************************
 *  @brief      This function puts a page into memory (if required).
 *              It must be called by vmem_read and vmem_write
 *
 *  @param      address The page that stores the contents of this address will be put in (if required).
 * 
 *  @return     The int value read from virtual memory.
 ****************************************************************************************/
static void vmem_put_page_into_mem(int address) {
	vmem->adm.req_pageno = address / VMEM_PAGESIZE;//Set the page which has to be loaded into memory
	TEST_AND_EXIT_ERRNO(vmem->adm.req_pageno < 0 || vmem->adm.req_pageno >= VMEM_NPAGES,"Die page ist auserhalb der page table 2");
	
	kill(vmem->adm.mmanage_pid,SIGUSR1);//Tell the mmanage process to load the page into memory
	
	sem_wait(sem_open(NAMED_SEM, 0));
}

int vmem_read(int address) {
	
	//Check if shared memory was initialised
	if(vmem == NULL){
		vmem_init();
	}
	
	vmem->adm.g_count++;
	
	int pageNumber = address / VMEM_PAGESIZE;//Get the page number from the address
	TEST_AND_EXIT_ERRNO(pageNumber < 0 || pageNumber >= VMEM_NPAGES,"Die page ist auserhalb der page table r");
	
	int offset = address % VMEM_PAGESIZE;//Get offset inside page of address
	TEST_AND_EXIT_ERRNO(offset < 0 || offset >= VMEM_PAGESIZE,"Der offset ist auserhalb der pagesize r");
	
	
	//If the page isn't in a frame call it into memory
	if((vmem->pt.entries[pageNumber].flags & PTF_PRESENT) == 0){
		vmem_put_page_into_mem(address);//Put it into memory
	}
	
	vmem->pt.entries[pageNumber].flags |= PTF_REF; //Reference the page
	
	if(vmem->adm.page_rep_algo == VMEM_ALGO_AGING){
		update_age_reset_ref();
	}
	
	
	int frame = vmem->pt.entries[pageNumber].frame;//Get the frame where the page is saved
	//Read data from memory
	return vmem->data[frame*VMEM_PAGESIZE + offset];
}

void vmem_write(int address, int data) {
	if(vmem == NULL){
		vmem_init();
	}
	
	vmem->adm.g_count++;
	
	
	int pageNumber = address / VMEM_PAGESIZE;//Get the page number from the address
	TEST_AND_EXIT_ERRNO(pageNumber < 0 || pageNumber >= VMEM_NPAGES,"Die page ist auserhalb der page table w");
	
	int offset = address % VMEM_PAGESIZE;//Get offset inside page of address
	TEST_AND_EXIT_ERRNO(offset < 0 || offset >= VMEM_PAGESIZE,"Der offset ist auserhalb der pagesize w");
	
	
	if((vmem->pt.entries[pageNumber].flags & PTF_PRESENT) == 0){
		vmem_put_page_into_mem(address);
	}
	
	vmem->pt.entries[pageNumber].flags |= PTF_DIRTY; //Set it to dirty so its writen into disk
	vmem->pt.entries[pageNumber].flags |= PTF_REF; //Reference the page
	
	if(vmem->adm.page_rep_algo == VMEM_ALGO_AGING){
		update_age_reset_ref();
	}
	
	int frame = vmem->pt.entries[pageNumber].frame;//Get the frame where the page is saved
	vmem->data[frame*VMEM_PAGESIZE + offset] = data;
}

// EOF
