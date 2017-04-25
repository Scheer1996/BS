/*
 * FIFO.c
 *
 *  Created on: Apr 16, 2017
 *      Author: Stefan
 */

#include "FIFO.h"

static char puffer[10];
static int pufferPushZeiger;
static int pufferPopZeiger;
static int length;

void FIFO_init(void){
	int i;
	for(i = 0; i < 10; i++){
			puffer[i] = '_';
		}
	pufferPushZeiger = 0;
	pufferPopZeiger = 0;
	length = 0;
}

void FIFO_push(char neuerChar){
	puffer[pufferPushZeiger] = neuerChar;
	pufferPushZeiger++;
	if(pufferPushZeiger == 10){
		pufferPushZeiger = 0;
	}
	length++;
}

char FIFO_pop(void){
	char popChar = puffer[pufferPopZeiger];
	puffer[pufferPopZeiger]='_';
	pufferPopZeiger++;
		if(pufferPopZeiger == 10){
			pufferPopZeiger = 0;
		}
	length--;
	return popChar;
}

int FIFO_getLength(void){
	return length;
}
