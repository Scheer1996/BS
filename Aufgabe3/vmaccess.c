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
	vmem->adm.pf_count++;//Increase the page fail counter
	
	kill(vmem->adm.mmanage_pid,SIGUSR1);//Tell the mmanage process to load the page into memory
	
	sem_wait(sem_open(NAMED_SEM, 0));
}

int vmem_read(int address) {
	
	//Check if it shared memory was initialised
	if(vmem == NULL){
		vmem_init();
	}
	
	
	int pageNumber = address / VMEM_PAGESIZE;//Get the page number from the address
	int offset = address % VMEM_PAGESIZE;//Get offset inside page of address
	int frame = vmem->pt.entries[pageNumber].frame;//Get the frame where the page is saved
	
	//If the page isn't in a frame call it into memory
	if(frame == VOID_IDX){
		vmem_put_page_into_mem(address);//Put it into memory
		frame = vmem->pt.entries[pageNumber].frame;//Get its frame
	}
	
	vmem->pt.entries[pageNumber].flags |= PTF_REF;//Reference the page

	vmem->adm.g_count++;
	if(vmem->adm.page_rep_algo == VMEM_ALGO_AGING){
		update_age_reset_ref();
	}
	
	//Read data from memory
	return vmem->data[frame*VMEM_PAGESIZE + offset];
}

void vmem_write(int address, int data) {
	if(vmem == NULL){
		vmem_init();
	}
	
	int pageNumber = address / VMEM_PAGESIZE;
	int offset = address % VMEM_PAGESIZE;
	int frame = vmem->pt.entries[pageNumber].frame;
	
	if(frame == VOID_IDX){
		vmem_put_page_into_mem(address);
		frame = vmem->pt.entries[pageNumber].frame;
	}
	
	vmem->pt.entries[pageNumber].flags |= PTF_DIRTY; //Set it to dirty so its writen into disk
	vmem->pt.entries[pageNumber].flags |= PTF_REF; //Reference the page

	vmem->adm.g_count++;
	if(vmem->adm.page_rep_algo == VMEM_ALGO_AGING){
		update_age_reset_ref();
	}

	vmem->data[frame*VMEM_PAGESIZE + offset] = data;
}

// EOF
